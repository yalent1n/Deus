#pragma once
#include "IRenderSurface.h"
#include "EventQueue.h"

namespace rw::widgets
{
using WidgetCollection = eastl::vector<IWidget*>;

type_event(UI)
{
    IWidget *src;
};

enum LayoutDirection : s8
{
    LayoutDirection_Horizontal = 0,
    LayoutDirection_Vertical = 1,
    LayoutDirection_Default = LayoutDirection_Vertical
};

class UILayout : public ILayout
{
public:
    FB_RENDER_API UILayout(LayoutDirection = LayoutDirection_Default);

    FB_RENDER_API const char *getID() const override;
    FB_RENDER_API void setID(const char*) override;

    FB_RENDER_API int getWidth() const override;
    FB_RENDER_API int getHeight() const override;
    
    FB_RENDER_API void setWidth(int) override;
    FB_RENDER_API void setHeight(int) override;

    FB_RENDER_API void addWidget(IWidget *) override;
    FB_RENDER_API void removeWidget(IWidget *) override;
    FB_RENDER_API IWidget *getWidget(int id) override;

    FB_RENDER_API LayoutDirection getDirection() const;
    FB_RENDER_API void setDirection(LayoutDirection);

protected:
    const char *m_ID;
    int m_Width;
    int m_Height;
    WidgetCollection m_Collection;
    LayoutDirection m_Direction;
};

/* in UI context a surface is a root container */
class UIPanel : public IRenderSurface, public UILayout
{
public:
    FB_RENDER_API void initWithLabelAndLayout(const char *label, LayoutDirection layout = LayoutDirection_Default);

    FB_RENDER_API const char *getLabel() const override;
    FB_RENDER_API void setLabel(const char*) override;

    FB_RENDER_API void getFrameSize(int *x, int *y) const override;
    FB_RENDER_API void drawFrame() override;
    FB_RENDER_API bool draw() override;

private:
    const char *m_Title;
};

enum ButtonStyle : s8
{
    ButtonStyle_Normal,
    ButtonStyle_Small,
    ButtonStyle_Arrow
};

/* only for arrow-style buttons */
enum ButtonDirection : s8
{
    ButtonDirection_None    = -1,
    ButtonDirection_Left    = 0,
    ButtonDirection_Right   = 1,
    ButtonDirection_Up      = 2,
    ButtonDirection_Down    = 3
};

type_event(OnClick) : UIEvent {};

class Button : public IButton
{
public:
    FB_RENDER_API Button(ButtonStyle style = ButtonStyle_Normal, ButtonDirection direction = ButtonDirection_None);
    FB_RENDER_API void initWithLabelAndSize(const char *label, int w, int h) override;
    
    FB_RENDER_API int getWidth() const override;
    FB_RENDER_API int getHeight() const override;
    
    FB_RENDER_API void setWidth(int) override;
    FB_RENDER_API void setHeight(int) override;

    FB_RENDER_API bool draw() override;
    FB_RENDER_API const char *getLabel() const override;
    FB_RENDER_API void setLabel(const char *) override;

private:
    const char* m_Label;
    int m_Width;
    int m_Height;
    ButtonStyle m_Style;
    ButtonDirection m_Direction;
};

class Label : public ILabel
{
public:
    FB_RENDER_API Label();
    FB_RENDER_API explicit Label(const char *);
    FB_RENDER_API void initWithLabel(const char *label) override;
    FB_RENDER_API void initWithLabelAndURL(const char *label, const char *url) override;
    
    FB_RENDER_API bool draw() override;
    FB_RENDER_API bool isTextWrapped() const override;
    FB_RENDER_API void setTextWrapped(bool = true) override;

    FB_RENDER_API const char *getLabel() const override;
    FB_RENDER_API void setLabel(const char *) override;
private:
    const char* m_Label;
    const char* m_LabelURL;
    bool m_Wrapped;
};

enum TextBoxStyle : s8
{
    TextBoxStyle_None       = 0,
    TextBoxStyle_ReadOnly   = _BIT(1),
    TextBoxStyle_Password   = _BIT(2),
};

using TextBoxString = eastl::vector<char>;

type_event(OnTextChanged) : UIEvent
{
    const char *chars;
    TextBoxString::size_type charsLength;
};

class TextBox : public ITextBox
{
public:
    FB_RENDER_API TextBox(TextBoxStyle = TextBoxStyle_None);
    FB_RENDER_API void initWithLabel(const char *label) override;
    FB_RENDER_API void initWithLabelAndHint(const char *label, const char* hint) override;

    FB_RENDER_API int getWidth() const override;
    FB_RENDER_API int getHeight() const override;

    FB_RENDER_API void setWidth(int) override;
    FB_RENDER_API void setHeight(int) override;

    FB_RENDER_API bool draw() override;

    /* be aware! getChars returns the duplicated chars. */
    FB_RENDER_API const char *getChars() const override;
    FB_RENDER_API void addChars(const char *chars) override;

    FB_RENDER_API bool isMultiline() const override;
    FB_RENDER_API void setMultiline(bool = true) override;

    FB_RENDER_API const char *getLabel() const override;
    FB_RENDER_API void setLabel(const char *) override;

private:
    const char* m_Label;
    const char* m_LabelHint;
    TextBoxString m_String;
    TextBoxStyle m_Style;
    int m_Width;
    int m_Height;
    bool m_Multiline;
};

type_event(OnPick) : UIEvent
{
    int index;
};

class ListView : public UILayout
{
public:
    EventDelegate<ListView> onPick;

public:
    FB_RENDER_API ListView();
    FB_RENDER_API void initWithID(const char *, int w = 0, int h = 0) override;

    FB_RENDER_API int getIndex() const;

    FB_RENDER_API bool draw() override;

private:
    int m_SelectedIndex;
};

enum TreeNodeStyle : s8
{
    TreeNodeStyle_None = 0,
    TreeNodeStyle_Bullet = _BIT(1),
    TreeNodeStyle_PlotLinesFull = _BIT(2),
    TreeNodeStyle_PlotLinesNodes = _BIT(3),
    TreeNodeStyle_DefaultOpen = _BIT(4),
    TreeNodeStyle_Span = _BIT(5)
};

class TreeNode;
using TreeNodeCollection = eastl::vector<TreeNode*>;

type_event(OnExpand) : UIEvent {};
type_event(OnCollapse) : UIEvent {};

class TreeNode : public ITreeNode
{
public:
    FB_RENDER_API TreeNode(TreeNodeStyle = TreeNodeStyle_None);
    FB_RENDER_API void initWithLabel(const char *) override;

    FB_RENDER_API const char *getLabel() const override;
    FB_RENDER_API void setLabel(const char *) override;

    FB_RENDER_API int getChildCount() const override;
    FB_RENDER_API TreeNode* getChild(int) override;

    FB_RENDER_API bool draw() override;

    FB_RENDER_API bool isExpanded() const;

    FB_RENDER_API int addChild(TreeNode*);
    FB_RENDER_API void removeChild(int);

private:
    const char* m_Label;
    TreeNodeCollection m_Collection;
    TreeNodeStyle m_Style;
    bool m_Open;
};

class TreeView : public ITreeView
{
public:
    FB_RENDER_API TreeView();
    FB_RENDER_API void initWithRoot(ITreeNode *node) override;

    FB_RENDER_API bool draw() override;

    FB_RENDER_API TreeNode* getNode() const;
private:
    TreeNode* m_Node;
};

class PropertyHolder : public IProperty
{
public:
    FB_RENDER_API PropertyHolder(const char *, IWidget *);
    
    FB_RENDER_API const char *getName() const override;
    FB_RENDER_API IWidget *getWidget() const override;

    FB_RENDER_API void setName(const char *);
    FB_RENDER_API void setWidget(IWidget *);
private:
    const char *m_Name;
    IWidget* m_Widget;
    // void* m_This;
    // const FieldInfoData* m_FieldInfo;
};

using PropertyCollection = eastl::vector<PropertyHolder>;

class PropertyGroup : public IPropertyGroup
{
public:
    FB_RENDER_API PropertyGroup(const char *);

    FB_RENDER_API const char *getTitle() const override;
    FB_RENDER_API const IProperty *getProperty(int) const override;
    FB_RENDER_API const IProperty *getProperties() const override;
    FB_RENDER_API int getPropertiesCount() const override;

    FB_RENDER_API void setTitle(const char *);
    FB_RENDER_API int addProperty(const char *, IWidget *);

private:
    const char* m_Title;
    PropertyCollection m_Collection;
};

class PropertyGrid : public IPropertyGrid
{
public:
    FB_RENDER_API PropertyGrid();
    FB_RENDER_API void initWithIDAndGroups(const char *id, IPropertyGroup **groups, int groupCount) override;

    FB_RENDER_API bool draw() override;

    FB_RENDER_API const char *getID();
    FB_RENDER_API void setID(const char *id);

private:
    const char *m_ID;
    IPropertyGroup **m_Groups;
    int m_GroupCount;
};
}