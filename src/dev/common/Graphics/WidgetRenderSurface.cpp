#include <Engine.Render/WidgetRenderSurface.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace rw::widgets
{
UILayout::UILayout(LayoutDirection direction)
    : m_ID(NIL)
    , m_Direction(direction)
{}

const char *UILayout::getID() const
{
    return m_ID;
}

void UILayout::setID(const char* id)
{
    m_ID = id;
}

int UILayout::getWidth() const
{
    return m_Width;
}

int UILayout::getHeight() const
{
    return m_Height;
}

void UILayout::setWidth(int w)
{
    m_Width = w;
}

void UILayout::setHeight(int h)
{
    m_Height = h;
}

void UILayout::addWidget(IWidget *w)
{
    EASSERT(w, "Widgets: attempted to add a NULL widget");
    m_Collection.push_back(w);
}

struct widget_compare
{
    inline bool operator()(IWidget* a, IWidget* b) const
    {
        return a == b;
    }
};

void UILayout::removeWidget(IWidget *w)
{
    if(!w)
        return;

    WidgetCollection::iterator i = m_Collection.find(w, widget_compare());
    if(i != m_Collection.end())
    {
        m_Collection.erase(i);
    }
}

IWidget *UILayout::getWidget(int id)
{
    if(m_Collection.size() <= id)
        return nullptr;

    return m_Collection[id];
}

LayoutDirection UILayout::getDirection() const
{
    return m_Direction;
}

void UILayout::setDirection(LayoutDirection direction)
{
    m_Direction = direction;
}

const char *UIPanel::getLabel() const
{
    return m_Title;
}

void UIPanel::setLabel(const char* label)
{
    m_Title = label;
}

void UIPanel::initWithLabelAndLayout(const char *label, LayoutDirection direction)
{
    m_Title = label;
    setID(label);

    m_Direction = direction;
}

void UIPanel::getFrameSize(int *x, int *y) const
{
    coreRenderGetFramebufferSize(x, y);
}

void UIPanel::drawFrame()
{
    (void) draw();
}

bool UIPanel::draw() 
{
    ImGui::Begin(m_Title);

    for(IWidget* w : m_Collection)
    {
        (void) w->draw();
        if(m_Direction == LayoutDirection_Horizontal)
            ImGui::SameLine(); 
    }

    ImGui::End();
    return YES;
}

/* ****************** */
/*       Button       */
/* ****************** */

Button::Button(ButtonStyle style, ButtonDirection direction)
    : m_Style(style)
    , m_Direction(direction)
{}

void Button::initWithLabelAndSize(const char *label, int w, int h)
{
    m_Label = label;
    m_Width = w;
    m_Height = h;
}

int Button::getWidth() const
{
    return m_Width;
}

int Button::getHeight() const
{
    return m_Height;
}

void Button::setWidth(int w)
{
    m_Width = w;
}

void Button::setHeight(int h)
{
    m_Height = h;
}

bool Button::draw()
{
    bool clicked = false;
    switch(m_Style)
    {
    default:
    case ButtonStyle_Normal:
        clicked = ImGui::Button(m_Label, ImVec2((float)m_Width, (float)m_Height));
        break;
    case ButtonStyle_Small: 
        clicked = ImGui::SmallButton(m_Label);
        break;
    case ButtonStyle_Arrow: 
        clicked = ImGui::ArrowButton(m_Label, (ImGuiDir)m_Direction);
        break;
    }

    if(clicked && !onClick.empty())
    {
        onClick(this);
    }

    return clicked;
}

const char *Button::getLabel() const
{
    return m_Label;
}

void Button::setLabel(const char *label)
{
    m_Label = label;
}

/* ****************** */
/*        Label       */
/* ****************** */

Label::Label()
    : m_Label("")
    , m_LabelURL("")
    , m_Wrapped(false)
{}

Label::Label(const char *label)
    : m_Label(label)
    , m_LabelURL("")
    , m_Wrapped(false)
{}

void Label::initWithLabel(const char *label)
{
    m_Label = label;
}

void Label::initWithLabelAndURL(const char *label, const char *url)
{
    m_Label = label;
    m_LabelURL = url;
}

bool Label::draw()
{
    ImGuiContext& g = *GImGui;
    const bool needsBackup = (g.CurrentWindow->DC.TextWrapPos < 0.0f);  // Keep existing wrap position if one is already set

    if (needsBackup && m_Wrapped)
        ImGui::PushTextWrapPos(0.0f);

    ImGui::Text("%s", m_Label);

    if (needsBackup && m_Wrapped)
        ImGui::PopTextWrapPos();

    return YES;
}

bool Label::isTextWrapped() const
{
    return m_Wrapped;
}

void Label::setTextWrapped(bool wrapped)
{
    m_Wrapped = wrapped;
}

const char *Label::getLabel() const
{
    return m_Label;
}

void Label::setLabel(const char *label)
{
    m_Label = label;
}

/* ****************** */
/*      TextBox       */
/* ****************** */

TextBox::TextBox(TextBoxStyle style)
    : m_Label(NIL)
    , m_LabelHint(NIL)
    , m_String(true)
    , m_Style(style)
    , m_Width(0)
    , m_Height(0)
    , m_Multiline(false)
{}

void TextBox::initWithLabel(const char *label)
{
    m_Label = label;
}

void TextBox::initWithLabelAndHint(const char *label, const char *hint)
{
    m_Label = label;
    m_LabelHint = hint;
}

int TextBox::getWidth() const
{
    return m_Width;
}

int TextBox::getHeight() const
{
    return m_Height;
}

void TextBox::setWidth(int w)
{
    m_Width = w;
}

void TextBox::setHeight(int h)
{
    m_Height = h;
}

static int _UIInputTextCallback(ImGuiInputTextCallbackData* data)
{
    TextBoxString &s = *(TextBoxString*)data->UserData;
    s.reserve(data->BufTextLen + 1);
    s.begin()[data->BufTextLen] = 0;
    return YES;
}

bool TextBox::draw()
{
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackResize;
    if(m_Style & TextBoxStyle_Password)
        flags |= ImGuiInputTextFlags_Password;
    if(m_Style & TextBoxStyle_ReadOnly)
        flags |= ImGuiInputTextFlags_ReadOnly;

    bool changed = ImGui::InputTextEx(m_Label, m_LabelHint, m_String.begin(), (int)m_String.capacity(), ImVec2((float)m_Width, (float)m_Height), flags, _UIInputTextCallback, &m_String);
    if(changed && !onEdit.empty())
    {
        onEdit(this);
    }

    return changed;
}

const char *TextBox::getChars() const
{
    return safedup(m_String.begin());
}

void TextBox::addChars(const char *chars)
{
    if(chars)
        m_String.insert_range(m_String.end(), chars, chars + strlen(chars));

    m_String.push_back('\0');
}

bool TextBox::isMultiline() const
{
    return m_Multiline;
}

void TextBox::setMultiline(bool multiline)
{
    m_Multiline = multiline;
}

const char *TextBox::getLabel() const
{
    return m_Label;
}

void TextBox::setLabel(const char *label)
{
    m_Label = label;
}

/* ****************** */
/*      ListView      */
/* ****************** */

ListView::ListView()
    : UILayout()
    , m_SelectedIndex(-1)
{}

void ListView::initWithID(const char * id, int w, int h)
{
    setID(id);
    setWidth(w);
    setHeight(h);
}

int ListView::getIndex() const
{
    return m_SelectedIndex;
}

bool ListView::draw()
{
    ImVec2 size;
    size.x = (float)getWidth();
    size.y = (float)getHeight();

    /* no auto-height */
    if(!size.y)
    {
        size.y = ImGui::GetTextLineHeightWithSpacing() * m_Collection.size();
    }

    ImGui::BeginChild(getID(), size);

    bool selected = false;
    for(WidgetCollection::iterator i = m_Collection.begin(), e = m_Collection.end(); i < e; ++i)
    {
        IWidget *w = *i;

        if(ImGui::Selectable(w->getLabel(), &selected, 0, ImVec2((float)w->getWidth(), (float)w->getHeight())))
        {
            m_SelectedIndex = (int)(i - m_Collection.begin());
        }
    }

    if(selected && !onPick.empty())
    {
        onPick(this);
    }

    ImGui::EndChild();
    return YES;
}

/* ****************** */
/*      TreeNode      */
/* ****************** */

TreeNode::TreeNode(TreeNodeStyle style)
    : m_Label("")
    , m_Collection()
    , m_Style(style)
    , m_Open(false)
{}

void TreeNode::initWithLabel(const char *label)
{
    m_Label = label;
}

const char *TreeNode::getLabel() const
{
    return m_Label;
}

void TreeNode::setLabel(const char *label)
{
    m_Label = label;
}

int TreeNode::getChildCount() const
{
    return m_Collection.size();
}

TreeNode* TreeNode::getChild(int index)
{
    if(index >= m_Collection.size())
        return nullptr;

    return m_Collection[index];
}

bool TreeNode::draw()
{
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_None;
    if(m_Style & TreeNodeStyle_Bullet)
        nodeFlags |= ImGuiTreeNodeFlags_Bullet;

    if(m_Style & TreeNodeStyle_PlotLinesFull)
        nodeFlags |= ImGuiTreeNodeFlags_DrawLinesFull;

    if(m_Style & TreeNodeStyle_PlotLinesNodes)
        nodeFlags |= ImGuiTreeNodeFlags_DrawLinesToNodes;

    if(m_Style & TreeNodeStyle_DefaultOpen)
        nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;

    if(m_Style & TreeNodeStyle_Span)
        nodeFlags |= ImGuiTreeNodeFlags_SpanAvailWidth;

    if(m_Collection.empty())
        nodeFlags |= ImGuiTreeNodeFlags_Leaf;

    bool open = ImGui::TreeNodeEx((void*)this, nodeFlags, "%s", m_Label);
    if(open != m_Open)
    {
        m_Open = open;
        if(m_Open && onExpand)
            onExpand(this);
        else if(!m_Open && onCollapse)
            onCollapse(this);
    }

    if(open)
    {
        for(TreeNode* node : m_Collection)
        {
            node->draw();
        }

        ImGui::TreePop();
    }

    return open;
}

struct TreeNodeComparer
{
    inline bool operator()(const TreeNode* a, const TreeNode* b) const
    {
        return a == b;
    }
};

bool TreeNode::isExpanded() const
{
    return m_Open;
}

int TreeNode::addChild(TreeNode* node)
{
    int index = m_Collection.size();
    m_Collection.push_back(node);
    return index;
}

void TreeNode::removeChild(int index)
{
    m_Collection.erase(m_Collection.begin() + index);
}

/* ****************** */
/*      TreeView      */
/* ****************** */

TreeView::TreeView()
    : m_Node(nullptr)
{}

void TreeView::initWithRoot(ITreeNode *node)
{
    m_Node = (TreeNode *)node;
}

bool TreeView::draw()
{
    if(m_Node)
        return m_Node->draw();
}

TreeNode* TreeView::getNode() const
{
    return m_Node;
}

/* ****************** */
/*    PropertyGrid    */
/* ****************** */

PropertyHolder::PropertyHolder(const char *name, IWidget *widget)
    : m_Name(name)
    , m_Widget(widget)
{}

const char *PropertyHolder::getName() const
{
    return m_Name;
}

IWidget *PropertyHolder::getWidget() const
{
    return m_Widget;
}

void PropertyHolder::setName(const char *name)
{
    m_Name = name;
}

void PropertyHolder::setWidget(IWidget *widget)
{
    m_Widget = widget;
}

PropertyGroup::PropertyGroup(const char *title)
    : m_Title(title)
{}

const char *PropertyGroup::getTitle() const
{
    return m_Title;
}

const IProperty *PropertyGroup::getProperty(int index) const
{
    __builtin_assert(index < m_Collection.size());
    return m_Collection.begin() + index;
}

const IProperty *PropertyGroup::getProperties() const
{
    return m_Collection.begin();
}

int PropertyGroup::getPropertiesCount() const
{
    return m_Collection.size();
}

void PropertyGroup::setTitle(const char *title)
{
    m_Title = title;
}

int PropertyGroup::addProperty(const char *name, IWidget *widget)
{
    int index = m_Collection.size();
    m_Collection.push_back({name, widget});
    return index;
}

PropertyGrid::PropertyGrid()
{}

void PropertyGrid::initWithIDAndGroups(const char *id, IPropertyGroup **groups, int groupCount)
{
    EASSERT(!isnil(id), "WG: failed to init property grid (null id)");
    EASSERT(groups, "WG: failed to init property grid (null groups)");
    EASSERT(groupCount, "WG: failed to init property grid (null count)");

    m_ID = id;
    m_Groups = groups;
    m_GroupCount = groupCount;
}

bool PropertyGrid::draw()
{
    for(int i = 0; i < m_GroupCount; ++i)
    {
        IPropertyGroup *group = m_Groups[i];
        
        bool open = ImGui::TreeNodeEx(group->getTitle(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth);
        if(open)
        {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 1);

            ImGui::Unindent(ImGui::GetStyle().IndentSpacing);

            if(ImGui::BeginTable(m_ID, 2, ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable))
            {
                for(int j = 0, e = group->getPropertiesCount(); j < e; j++)
                {
                    const IProperty *prop = group->getProperty(j);
                    ImGui::TableNextRow();
                    
                    /* draw name in row */
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0x2D, 0x2F, 0x34, 255));
                    ImGui::Text(prop->getName());
                    
                    /* draw value in row */
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0x20, 0x22, 0x26, 255));
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    (void) prop->getWidget()->draw();
                }
                ImGui::EndTable();
            }

            ImGui::Indent(ImGui::GetStyle().IndentSpacing);

            ImGui::TreePop();
        }
    }
    return YES;
}

const char *PropertyGrid::getID()
{
    return m_ID;
}

void PropertyGrid::setID(const char *id)
{
    m_ID = id;
}
}