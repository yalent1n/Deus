#include <Engine.Base/Misc/Xml.h>
#include <Engine.Base/Memory/VirtualAlloc.h>
#include <Engine.Base/String/String.h>

namespace rw
{
XmlDocumentParser::XmlDocumentParser(const void* buf)
    : m_data(static_cast<const char*>(buf))
	, m_len(static_cast<u32>(rw::strlen(m_data)))
{}

XmlDocumentParser::XmlDocumentParser(const void* buf, u32 len)
    : m_data(static_cast<const char*>(buf))
	, m_len(len)
{}

XmlDocumentBuilder::XmlDocumentBuilder(Buffer* buffer, const char* version, const char* encoding)
    : m_buffer(buffer)
    , m_version(version)
    , m_encoding(encoding)
{}

bool XmlDocumentBuilder::buildNode(SkeletonNode* node, SkeletonBuildParams* params, u32 depth)
{
    if(!node)
        return false;

    bool selfclose = isnil(node->m_element.text) && !(node->m_element.child);

    StringBuilder<> beginLine;
    if(params->pretty)
    {
        for(u32 i = 0; i < depth; i++)
            beginLine.appendChar('\t');
    }

    beginLine << '<' << node->getNodeName();

    if(node->m_element.attributes)
    {
        xml_attribute* attr = node->m_element.attributes;
        while(attr)
        {
            beginLine << ' ' << attr->name << "=\"" << attr->value << '\"';
            attr = attr->next;
        }
    }

    if(selfclose)
        beginLine << '/';

    beginLine << '>';
    if(params->pretty && (isnil(node->getText()) && !params->pretty_text))
        beginLine << '\n';

    if(m_buffer->write(c_str(beginLine), beginLine.size()) < E_BIO_SUCCESS)
        return false;

    if(selfclose)
    {
        return true;
    }

    if(!isnil(node->m_element.text))
    {
        if(params->pretty_text)
        {
            for(u32 i = 0; i < depth; i++)
                m_buffer->write('\t');

            m_buffer->write('\t');
        }
            
        m_buffer->write(node->m_element.text, strlen(node->m_element.text));

        if(params->pretty_text)
        {
            m_buffer->write('\n');
        }
    }
    else if(node->m_element.child)
    {
        ++depth;
        xml_element* child = node->m_element.child;
        while(child)
        {
            if(!buildNode((XmlDocumentNode*)child, params, depth))
                return false;

            child = child->sibling;
        }
        --depth;
    }

    FixedStringBuilder<> endLine;
    if(params->pretty && (isnil(node->getText()) && !params->pretty_text))
    {
        for(u32 i = 0; i < depth; i++)
            endLine.appendChar('\t');
    }

    endLine << "</" << node->getNodeName() << '>';

    if(params->pretty)
    {
        endLine << '\n';
    }

    m_buffer->write(c_str(endLine), endLine.size());
    return true;
}

bool XmlDocumentBuilder::buildSkeleton(Skeleton* skel, SkeletonBuildParams* params)
{
    if(!skel)
        return false;

    FormattedStringBuilder<128> xmlHeader("<?xml version=\"%s\" encoding=\"%s\"?>\n", m_version, m_encoding);
    m_buffer->write(xmlHeader.c_str(), strlen(xmlHeader.c_str()));
    return buildNode(skel->getNode(), params);
}

__forceinline const char* skipWhitespace(const char* s)
{
    while(isspace(*s))
        ++s;

    return s;
}

__forceinline const char* skipToWhitespace(const char* s)
{
    while(!isspace(*s))
        ++s;

    return s;
}

const char* xml_read_item_name(const char*& cursor)
{
    const char* name_src = cursor;

    while(isalchar(*cursor) || isalnum(*cursor) || 
    *cursor == '_' || *cursor == '-' || *cursor == '.' ||
    *cursor == '!' || *cursor == '?' || *cursor == ':') // let special elements be handled here
        ++cursor;

    size_t name_sz = size_t(cursor - name_src);
    char* name = (char*) HeapAllocGlobal(name_sz);
    memcpy(name, name_src, name_sz);
    name[name_sz] = 0;
    return name;
}

const char* xml_read_string_escaped(const char*& cursor, char term)
{    
	if(*cursor != term)
        return nullptr;

    ++cursor;

	StringBuilder<> str;
	while(true)
	{
		char c = *cursor;
		if(c == '\\')
		{
			char ctrl = *++cursor;
			switch(ctrl)
			{
			case '\\':
				str.appendChar('\\');
				break;
			case 'b':
				str.appendChar('\b');
				break;
			case 'f':
				str.appendChar('\f');
				break;
			case 'n':
				str.appendChar('\n');
				break;
			case 'r':
				str.appendChar('\r');
				break;
			case 't':
				str.appendChar('\t');
				break;
			case 'u':
				cursor += 4;
				break;
			}

			continue;
		}
		
		if(c == term)
			break;

		str.appendChar(c);
		++cursor;
	}

	++cursor;
	return str.detach();
}

const char* xml_parse_attributes(xml_element* elem, const char* cursor, const char* end)
{
    while(*cursor != '/' && *cursor != '>')
    {
        const char* attr_name = xml_read_item_name(cursor);
        const char* attr_value = nullptr; // attribute can have no value (like !DOCTYPE html)

        // some motherfucker better not be putting random space next to an attribute name
        cursor = skipWhitespace(cursor);
        if(*cursor == '=')
        {
            cursor = skipWhitespace(++cursor);
            attr_value = xml_read_string_escaped(cursor, '\"');
            if(!attr_value)
                return nullptr;
        }
        
        xml_attribute* attr = new xml_attribute();
        attr->name = attr_name;
        attr->value = attr_value;
        
        if(elem->attributes)
        {
            xml_attribute* insert_attr = elem->attributes;
            while(insert_attr->next)
                insert_attr = insert_attr->next;
            insert_attr->next = attr;
        } else
        {
            elem->attributes = attr;
        }

        if(*cursor == '/' || *cursor == '>')
            break;

        if(isspace(*cursor))
        {
            cursor = skipWhitespace(cursor);
        }
        else
        {
            ++cursor;
        }
    }

    if(*cursor == '/')
    {
        elem->text = (const char*) (-1ULL);
        ++cursor;
    }

    cursor = skipWhitespace(++cursor);
    return cursor;
}

size_t xml_adjust_text_end(const char*& text_end, const char* text_start)
{
    if(*text_end == '<')
        return size_t(text_end - text_start);

    while(*text_end != '\r' && *text_end != '\n')
        --text_end;

    if(text_end[-1] == '\r')
        --text_end;

    return size_t(text_end - text_start);
}

const char* xml_element::parse(const char* begin, const char* end)
{
    const char* cursor = begin;
    if(cursor[0] != '<')
        return nullptr;
    ++cursor;

    // skip DTD and XML internal elements and comments
    if(cursor[0] == '!' || cursor[0] == '?')
    {
        while(*cursor != '>')
            ++cursor;

        if(cursor >= end)
        {
            return end;
        }

        cursor = skipWhitespace(++cursor);
        return parse(cursor, end);
    }

    name = xml_read_item_name(cursor);
    //ELOGF("XML ELEMENT %s", name);

    // skip to attributes or self close, or simple end
    cursor = skipWhitespace(cursor);
    
    if(*cursor != '/' && *cursor != '>')
    {
        cursor = xml_parse_attributes(this, cursor, end);
        if(!cursor)
            return nullptr;
    }

    if(text == (const char*)(-1ULL))
    {
        text = nullptr;
        return cursor;
    }
    
    if(*cursor == '>')
        ++cursor;
        
    cursor = skipWhitespace(cursor);
    const char* text_start = cursor;
    while(cursor < end)
    {
        if(*cursor == '\"')
        {
            ++cursor;
            while(cursor < end && *cursor != '\"')
            {
                if(*cursor == '\\')
                {
                    ++cursor;
                    if(*cursor == 'u')
                        cursor += 3;
                }
                ++cursor;
            }

            continue;
        }

        if(*cursor == '/')
        {
            ++cursor;
            if(*cursor == '*') // a comment
            {
                ++cursor;
                while(cursor + 1 < end)
                {
                    if(cursor[0] == '*' && cursor[1] == '/')
                    {
                        cursor += 2;
                        break;
                    }

                    ++cursor;
                }

                if(cursor >= end)
                    return end;
            }
        }

        if(*cursor == '<')
        {
            // 1. Handle closing tag
            if(cursor[1] == '/')
            {
                const char* text_end = cursor;
                size_t text_sz = size_t(text_end - text_start);
                if(text_sz)
                {
                    text_sz = xml_adjust_text_end(text_end, text_start);
                    char* t = (char*) HeapAllocGlobal(text_sz);
                    memcpy(t, text_start, text_sz);
                    t[text_sz] = 0;
                    text = t;
                    ++cursor;
                }

                ++cursor;
                while(*cursor != '>')
                    ++cursor;

                ++cursor;
                if(cursor >= end)
                    return end; 

                while(*cursor == ' ' || *cursor == '\t' || *cursor == '\r' || *cursor == '\n' || *cursor == '\f' || *cursor == '\v')
                    ++cursor;

                return cursor;
            }

            // 2. Handle special elements (comments and XML tags)
            if(cursor[1] == '!' || cursor[1] == '?')
            {
                ++cursor;
                while(*cursor != '>')
                    ++cursor;

                if(cursor >= end)
                {
                    return end;
                }

                cursor = skipWhitespace(++cursor);
            }

            // 3. Handle child element
            if(isalchar(cursor[1]) || isalnum(cursor[1]) || cursor[1] == '_' || cursor[1] == ':')
            {
                bool is_valid_child = cursor[-1] != '\"';
                if(!is_valid_child)
                {
                    ++cursor;
                    continue;
                }

                const char* tmp = cursor + 1;
                while(tmp < end && !isspace(*tmp) && *tmp != '>' && *tmp != '/')
                {
                    bool is_char_allowed = isalchar(*tmp) || isalnum(*tmp) || *tmp == '_' || *tmp == ':';
                    if(!is_char_allowed)
                    {
                        is_valid_child = false;
                        break;
                    }
                    tmp++;
                }

                if(!is_valid_child)
                {
                    ++cursor;
                    continue;
                }

                if(text_start[0] == '/' && text_start[1] == '>')
                {
                    return cursor;
                }

                const char* text_end = cursor;
                size_t text_sz = size_t(text_end - text_start);
                if(text_sz)
                {
                    text_sz = xml_adjust_text_end(text_end, text_start);
                    char* t = (char*) HeapAllocGlobal(text_sz);
                    memcpy(t, text_start, text_sz);
                    t[text_sz] = 0;
                    text = t;
                }
    
                xml_element* new_child = new xml_element();
                cursor = new_child->parse(cursor, end);
                if(!cursor)
                    return nullptr;
        
                if(child)
                {
                    xml_element* insert_sibling = child;
                    while(insert_sibling->sibling)
                    {
                        insert_sibling = insert_sibling->sibling;
                    }

                    insert_sibling->sibling = new_child;
                } else
                {
                    child = new_child;
                }
                
                text_start = cursor;
                continue;
            }
        }
        
        ++cursor;
    }

    return end;
}

xml_element XmlDocumentParser::parse() const
{
    const char* begin = m_data;
	const char* end = m_data + m_len;

	if(begin[0] != '<')
		return xml_element();
	
	xml_element root;
	const char* parseend = root.parse(begin, end);
	if(parseend != end)
	{
        return xml_element();
	}
	
	return root;
}

XmlDocumentNode::XmlDocumentNode(xml_element node)
    : m_element(node)
{

}

const char* XmlDocumentNode::getNodeName() const
{
    return m_element.name;
}

void XmlDocumentNode::setNodeName(const char* name)
{
    m_element.name = name;
}

const char* XmlDocumentNode::getText() const
{
    return m_element.text;
}

void XmlDocumentNode::setText(const char* text)
{
    m_element.text = text;
}

const char* XmlDocumentNode::getAttribute(const char* name) const
{
    xml_attribute* attr = m_element.attributes;
    while(attr)
    {
        if(equalsIgnoreCase(name, attr->name))
            return attr->value;
        attr = attr->next;
    }

    return NIL;
}

void XmlDocumentNode::setAttribute(const char* attrName, const char* attrStr, AddTo addTo)
{
    xml_attribute* attr = new xml_attribute();
    attr->name = attrName;
    attr->value = attrStr;

    if(addTo == AddTo::Back)
    {
        if(m_element.attributes)
        {
            xml_attribute* insert_attr = m_element.attributes;
            while(insert_attr->next)
                insert_attr = insert_attr->next;
            insert_attr->next = attr;
        }
        else
        {
            m_element.attributes = attr;
        }
    }
    else
    {
        attr->next = m_element.attributes;
        m_element.attributes = attr;
    }
}

XmlDocumentNode* XmlDocumentNode::getChild() const
{
    return reinterpret_cast<XmlDocumentNode*>(m_element.child);
}

XmlDocumentNode* XmlDocumentNode::getSibling() const
{
    return reinterpret_cast<XmlDocumentNode*>(m_element.sibling);
}

XmlDocumentNode* XmlDocumentNode::findSibling(const char* name) const
{
    if(equalsIgnoreCase(name, m_element.name))
        return const_cast<XmlDocumentNode*>(this);

    xml_element* sibling = m_element.sibling;
    while(sibling)
    {
        if(equalsIgnoreCase(name, sibling->name))
            return reinterpret_cast<XmlDocumentNode*>(sibling);
        sibling = sibling->sibling;
    }

    return NIL;
}

XmlDocumentNode* XmlDocumentNode::appendSibling(XmlDocumentNode* node, AddTo addTo)
{
    if(addTo == AddTo::Back)
    {
        if(m_element.sibling)
        {
            xml_element* insert_sibling = m_element.sibling;
            while(insert_sibling->sibling)
                insert_sibling = insert_sibling->sibling;
            insert_sibling->sibling = &node->m_element;
        }
        else
        {
            m_element.sibling = &node->m_element;
        }
    }
    else
    {
        node->m_element.sibling = m_element.sibling;
        m_element.sibling = &node->m_element;
    }

    return node;
}

XmlDocumentNode* XmlDocumentNode::appendChild(XmlDocumentNode* node, AddTo addTo)
{
    if(m_element.child)
    {
        ((XmlDocumentNode*)m_element.child)->appendSibling(node, addTo);
    } else 
    {
        m_element.child = &node->m_element;
    }

    return node;
}

XmlDocument::XmlDocument()
    : m_root()
{}

XmlDocumentNode* XmlDocument::getNode() const
{
    return m_root;
}

XmlDocumentNode* XmlDocument::findElement(const char* name) const
{
    if(m_root && m_root->m_element.child)
    {
        return ((XmlDocumentNode*)m_root->m_element.child)->findSibling(name);
    }

    return nullptr;
}

XmlDocumentNode* XmlDocument::createElement(const char* localName, const char* value)
{
    xml_element element;
    element.name = localName;
    element.text = value;
    element.attributes = nullptr;
    element.child = nullptr;
    element.sibling = nullptr;
    return new XmlDocumentNode(element);
}

XmlDocumentNode* XmlDocument::appendChild(XmlDocumentNode* child, AddTo addTo)
{
    if(!m_root)
    {
        m_root = child;
        return m_root;
    }

    return m_root->appendChild(child, addTo);
}
}