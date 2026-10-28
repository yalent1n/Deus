#pragma once
#include "../Bundle.h"
#include "../Buffer/Buffer.h"
#include "../Buffer/SharedBuffer.h"

namespace rw
{
struct xml_attribute
{
    const char* name;
    const char* value;
    xml_attribute* next;
};

struct xml_element
{
    const char* name {NO};
    const char* text {NO};
    xml_attribute* attributes {NO};
    xml_element* child {NO};
    xml_element* sibling {NO};

    FB_BASE_API const char* parse(const char* begin, const char* end);
};

class XmlDocumentParser
{
public:
    FB_BASE_API XmlDocumentParser(const void* buf);
    FB_BASE_API XmlDocumentParser(const void* buf, u32 len);
    
    FB_BASE_API xml_element parse() const;

private:
    const char* m_data;
    u32 m_len;
};

class XmlDocument;
typedef XmlDocument Skeleton;

class XmlDocumentNode;
typedef XmlDocumentNode SkeletonNode;

struct SkeletonBuildParams
{
    bool pretty = true; // elements are indented if true
    bool pretty_text = false; // indent node text  if true
};

class XmlDocumentBuilder
{
public:
    FB_BASE_API XmlDocumentBuilder(Buffer* buffer, const char* version = "1.0", const char* encoding = "UTF-8");
    
    FB_BASE_API bool buildNode(SkeletonNode* node, SkeletonBuildParams* params, u32 depth = 0);
    FB_BASE_API bool buildSkeleton(Skeleton* skel, SkeletonBuildParams* params);

private:
    Buffer* m_buffer;
    const char* m_version;
    const char* m_encoding;
};

enum class AddTo
{
    Back,
    Front
};

class XmlDocumentNode
{
    friend class XmlDocumentBuilder;
    friend class XmlDocument;

public:
    FB_BASE_API XmlDocumentNode(xml_element node);

    FB_BASE_API const char* getNodeName() const;
    FB_BASE_API void setNodeName(const char* name);
    
    FB_BASE_API const char* getText() const;
    FB_BASE_API void setText(const char* text);

    FB_BASE_API const char* getAttribute(const char* name) const;
    FB_BASE_API void setAttribute(const char* attrName, const char* attrStr, AddTo addTo = AddTo::Back);

    FB_BASE_API XmlDocumentNode* getChild() const;
    
    FB_BASE_API XmlDocumentNode* getSibling() const;
    FB_BASE_API XmlDocumentNode* findSibling(const char* name) const;
    FB_BASE_API XmlDocumentNode* appendSibling(XmlDocumentNode* node, AddTo addTo = AddTo::Back);
    FB_BASE_API XmlDocumentNode* appendChild(XmlDocumentNode* node, AddTo addTo = AddTo::Back);

    inline operator bool() const
    {
        return !!(m_element.name);
    }
private:
    xml_element m_element;
};

class XmlDocument
{
public:
    FB_BASE_API XmlDocument();

    FB_BASE_API XmlDocumentNode* getNode() const;

    FB_BASE_API XmlDocumentNode* findElement(const char* name) const;
    FB_BASE_API XmlDocumentNode* createElement(const char* name, const char* value = NO);
    FB_BASE_API XmlDocumentNode* appendChild(XmlDocumentNode* child, AddTo addTo = AddTo::Back);

    inline XmlDocumentNode* operator[](const char* name) const
    {
        return findElement(name);
    }

    inline operator bool() const
    {
        return !!(m_root);
    }
private:
    XmlDocumentNode* m_root;
};

inline XmlDocument* XML_Parse(const SharedBuffer* buffer)
{
    XmlDocument* xml = new XmlDocument();
    XmlDocumentNode* xmlRoot = new XmlDocumentNode((XmlDocumentParser(buffer->bytes(), buffer->size())).parse());
    xml->appendChild(xmlRoot);
    return xml;
}
}