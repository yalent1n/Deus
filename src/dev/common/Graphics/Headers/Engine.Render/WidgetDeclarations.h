#pragma once
#include "Module.h"

#define RW_WIDGETS_MAKE_DELEGATE(w, fn) rw::widgets::EventDelegate<rw::widgets::w>(fn)

namespace rw::widgets
{
template<class TWidget>
using EventDelegate = FastDelegate<void(TWidget *)>;

interface IWidget
{
    virtual int getWidth() const { return 0; }
    virtual void setWidth(int) {}

    virtual int getHeight() const { return 0; }
    virtual void setHeight(int) {}

    virtual const char *getLabel() const { return NIL; }
    virtual void setLabel(const char *) { }

    virtual bool draw() = 0;
};


interface IButton : IWidget
{
    virtual void initWithLabelAndSize(const char *label, int w, int h) = 0;
    
    EventDelegate<IButton> onClick;
};

interface ILabel : IWidget
{
    virtual void initWithLabel(const char *label) = 0;
    virtual void initWithLabelAndURL(const char *label, const char *url) = 0;

    virtual bool isTextWrapped() const = 0;
    virtual void setTextWrapped(bool) = 0;
};

interface ITextBox : IWidget
{
    virtual void initWithLabel(const char *label) = 0;
    virtual void initWithLabelAndHint(const char *label, const char *hint) = 0;

    EventDelegate<ITextBox> onEdit;

    virtual const char *getChars() const = 0;
    virtual void addChars(const char *) { }

    virtual bool isMultiline() const = 0;
    virtual void setMultiline(bool) = 0;
};

interface ICheckBox : IWidget
{

};

interface IComboBox : IWidget
{

};

interface ITreeNode : public IWidget
{
    virtual void initWithLabel(const char *) = 0;

    EventDelegate<ITreeNode> onExpand;
    EventDelegate<ITreeNode> onCollapse;

    virtual int getChildCount() const = 0;
    virtual ITreeNode* getChild(int) = 0;
};

interface ITreeView : public IWidget
{
    virtual void initWithRoot(ITreeNode *) = 0;
};

interface IProperty
{
    virtual const char *getName() const = 0;
    virtual IWidget *getWidget() const = 0;
};

interface IPropertyGroup
{
    virtual const char *getTitle() const = 0;

    virtual const IProperty *getProperty(int) const = 0;
    virtual const IProperty *getProperties() const = 0;
    virtual int getPropertiesCount() const = 0;
};

interface IPropertyGrid : public IWidget
{
    virtual void initWithIDAndGroups(const char *, IPropertyGroup **, int) = 0;
};

interface ILayout : public IWidget
{
    /* not pure virtual because root containers doesn't have an ID */
    virtual void initWithID(const char *, int w = 0, int h = 0) {}

    virtual const char *getID() const = 0;
    virtual void setID(const char*) = 0;

    virtual void addWidget(IWidget *) = 0;
    virtual void removeWidget(IWidget *) = 0;
    virtual IWidget *getWidget(int) = 0; 
};
}