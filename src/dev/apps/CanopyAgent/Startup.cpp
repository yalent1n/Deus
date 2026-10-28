#include "App.h"
#include <Engine.Base/Log.h>
#include <Engine.Base/String/String.h>
#include <Engine.Base/Buffer/Adapter.h>
#include <Engine.Base/Misc/Time.h>
#include <Engine.Base/Thread/Thread.h>
#include <Engine.Base/Crypto/AES.h>
#include <Engine.Base/Crypto/ECDH.h>
#include <Engine.Base/Crypto/Base64.h>
#include <Engine.Base/Ref.h>
#include <Engine.Base/Misc/Xml.h>
#include <Engine.Base/Misc/Ini.h>
#include <Engine.Base/Database/DbObjectBuilder.h>

#include <Engine.Core/diag/ExecutionContext.h>

#include <Engine.Reflection/Asset.h>
#include <Engine.Reflection/NativeTypeRegistry.h>
#include <Engine.Reflection/Asset.h>
#include <Engine.Reflection/Array.h>
#include <Engine.Reflection/RuntimeDatabasePartition.h>

//#include <Engine.Reflection/ScriptEngine/vm.h>
#include <Engine.Reflection/ScriptEngine/mm.h>
#include <Engine.Reflection/ScriptEngine/image.h>

#include <Engine.Reflection/EnumTypeInfo.h>
#include <Engine.Reflection/GenericDataLayout.h>

#include <Engine.Network/http/server.h>
#include <Engine.Network/Util.h>
#include <Engine.Network/aidl/aidl.h>
#include <Engine.Network/Netcrypt.h>

//#include <Engine.Storage/IoService.h>
#include <Engine.Storage/IoTrustVault.h>
#include <Engine.Storage/IoFriendsManager.h>

#include <Engine.Render/WidgetRenderSurface.h>
#include <Engine.Render/WidgetRenderDevice.h>
#include <Engine.Render/EventQueue.h>

#include <pcap/pcap.h>
#include <bcrypt.h>

#include <CoreSaturn/DirectoryRecord.h>

#include "CommandHandler.h"

namespace rw
{
RuntimeDatabasePartition* createPartition(const SymbolClassInfo* typeInfo, const char* partitionName)
{
    if(!typeInfo->isSubclassOf<Asset>() && typeInfo != Type<Asset>())
    {
        EERRORF("%s is not an asset", typeInfo->getName());
        return nullptr;
    }

    SymbolClassInfo::CreateInfo createInfo;
    createInfo.compartment = Compartment_STATIC;
    createInfo.makeRoomForGuid = true;
    createInfo.makeRoomForPartition = false;
    createInfo.makeUniqueGuid = true;

    TestBedAsset* asset = static_cast<TestBedAsset*>(typeInfo->createInstance(createInfo));
    if(!asset)
        return nullptr;

    asset->setName(partitionName);
    asset->setName2("I dont know what");

    PartitionInitData initData;
    initData.name = partitionName;
    initData.guid = genFastGuid();
    //initData.instances.push_back(asset);

    RuntimeDatabasePartition* partition = new RuntimeDatabasePartition(initData);
    partition->addPartitionInstance(asset);

    ScopedRef builderBuffer = ExecutionContext::getAppLocalFileSystem()->createFile("SP_MainCity.dbx");
    XmlDocumentBuilder builder(builderBuffer.get());
    
    Skeleton skel;
    SkeletonNode* root = skel.appendChild(skel.createElement("partition"));
    
    SkeletonBuildParams params;
    params.pretty = YES;
    params.pretty_text = NO;

    TypeInfo::SkelConversionContext conv;
    conv.skel = &skel;
    conv.root = root;
    SkeletonNode* assetNode = asset->getType()->toSkel(asset, &conv);
    if(builder.buildSkeleton(&skel, &params))
    {
        ELOG("Built asset");
    }

    ELOGF("Partition Type : %s", asset->getType()->getName());
    ELOGF("Partition ID : %s", GuidStr(partition->getPartitionGuid()).str);

    InterpartitionReference ref = InterpartitionReference::make(asset);
    EDEBUGF("Ref: %zu", ref.id);
    Symbol* resolvedSymbol = InterpartitionReference::resolve(ref);
    EDEBUGF("resolvedSymbol=0x%p, asset=0x%p", resolvedSymbol, asset);
    return partition;
}

void BuildTree(const DbObject &tree, widgets::TreeNode *parent)
{
    JsonConversionParams params;
    params.indentRoots = false;
    params.indentChildren = false;

    for(DbObjectElementIterator it(tree); it.more();)
    {
        DbObjectElement e = it.next();
        widgets::TreeNode *child = new widgets::TreeNode(widgets::TreeNodeStyle_PlotLinesFull);
        const char *key = e.name();
        const char *value = nullptr;

        if(e.isDbVariant())
        {
            BuildTree(e.valueObject(), child);
        }
        else
        {
            StringBuilder<> str;
            if(e.isBlob())
            {
                str.appendChars("<data>");
            }
            else
            {
                e.getValueSafe(str);
            }

            value = str.detach();
        }

        const char *label = key;
        if(isnil(label))
        {
            label = (new FormattedStringBuilder<32>("<%s>", tostring(e.type())))->c_str();
        }
        else if(value)
        {
            StringBuilder<> valueLabel;
            valueLabel << tostring(e.type()) << ' ' << key << " = " << value;
            label = valueLabel.detach();
        }

        child->initWithLabel(label);
        parent->addChild(child);
    }
}

void BuildProperties(const Symbol *symbol, const SymbolClassInfo *type, widgets::PropertyGroup *group)
{
    if(type == Type<Asset>())
        return;

    if(type->getBaseType() != type)
    {
        BuildProperties(symbol, (const SymbolClassInfo *)type->getBaseType(), group);
    }

    u32 fcount = type->getFieldCount();
    const FieldInfoData *fieldInfos = type->getFieldInfos();

    TypeInfo::StringConversionParams params;
    params.asMember = true;

    for(u32 i = 0; i < fcount; ++i)
    {
        const FieldInfoData &fid = fieldInfos[i];
        const TypeInfo *fieldType = fid.fieldType;
        void *fieldValue = ((u8*)symbol) + fid.fieldOffset;

        if(fieldType == Type<String>())
        {
            StringBuilder<32> widgetId_;
            widgetId_ << "##" << fid.member.name;
            const char *widgetId = widgetId_.detach();

            auto textBox = new widgets::TextBox();
            textBox->initWithLabel(widgetId);

            const String &str = *(const String*)fieldValue;
            if(str.size())
            {
                textBox->addChars(str.c_str());
            }

            group->addProperty(fid.member.name, textBox);
        }
        else
        {
            StringBuilder<> fieldStr;
            fieldType->toString(fieldValue, &params, fieldStr);

            auto label = new widgets::Label();
            label->initWithLabel(fieldStr.detach());
            group->addProperty(fid.member.name, label);
        }
    }
}

static Aidl ai;
static int _Aidl_System_code = 0;

static eastl::hash_map<u32, widgets::EventDelegate<widgets::IButton>> s_ButtonDelegates;
static eastl::hash_map<u32, widgets::IWidget*> s_WidgetTable;

template<typename T>
static T* UI_Get_Widget(const char* id)
{
    auto i = s_WidgetTable.find(hashQuickLowerCase(id));
    if(i != s_WidgetTable.end())
    {
        return (T*) i->second;
    }

    return nullptr;
}

template<typename T>
static T* UI_Get_Widget(u32 hash)
{
    auto i = s_WidgetTable.find(hash);
    if(i != s_WidgetTable.end())
    {
        return (T*) i->second;
    }

    return nullptr;
}

static void _Aidl_log(const char* fmt, ...)
{
    fprintf(stdout, "[+] aidl_gui: ");

    char* buf = new char[512];

    va_list va;
    va_start(va, fmt);
    vfprintf(stdout, fmt, va);
    vsnprintf(buf, 512, fmt, va);
    va_end(va);
    fputc('\n', stdout);

    UI_Get_Widget<widgets::ListView>("log-console-list")->addWidget(new widgets::Label(buf));
}

void _Handle_data(const void* data, u16 len)
{
    ELOG("<data>");
}

void On_Estabilish(widgets::IButton* btn)
{
    const char* host = UI_Get_Widget<widgets::TextBox>("kit-host")->getChars();
    if(isnil(host))
        return;

    const char* localPort = UI_Get_Widget<widgets::TextBox>("kit-local-port")->getChars();
    u16 nLocalPort;
    if(!tointeger(localPort, nLocalPort)) {
        _Aidl_log("Invalid local port.");
        return;
    }

    _Aidl_System_code = AidlJoin(&ai, host, nLocalPort);
    if(_Aidl_System_code < 0) {
        _Aidl_log("Failed to estaibilish connection with Exchange Kit.");
        return;
    }

    _Aidl_log("Estabilished connection with the Exchange Kit @ %s.", host);
}

widgets::ButtonStyle UI_Parse_ButtonStyle(const char* style)
{
    if(isnil(style))
        return widgets::ButtonStyle_Normal;
        
    if(equalsIgnoreCase("small", style))
        return widgets::ButtonStyle_Small;
    if(equalsIgnoreCase("arrow", style))
        return widgets::ButtonStyle_Arrow;

    return widgets::ButtonStyle_Normal;
}

widgets::ButtonDirection UI_Parse_ButtonDirection(const char* direction)
{
    if(isnil(direction))
        return widgets::ButtonDirection_None;
        
    if(equalsIgnoreCase("left", direction))
        return widgets::ButtonDirection_Left;
    if(equalsIgnoreCase("right", direction))
        return widgets::ButtonDirection_Right;
    if(equalsIgnoreCase("up", direction))
        return widgets::ButtonDirection_Up;
    if(equalsIgnoreCase("down", direction))
        return widgets::ButtonDirection_Down;

    return widgets::ButtonDirection_None;
}

widgets::TextBoxStyle UI_Parse_TextBoxStyle(const char* style)
{
    if(isnil(style))
        return widgets::TextBoxStyle_None;
        
    if(equalsIgnoreCase("hidden", style))
        return widgets::TextBoxStyle_Password;
    if(equalsIgnoreCase("readonly", style))
        return widgets::TextBoxStyle_ReadOnly;

    return widgets::TextBoxStyle_None;
}

widgets::LayoutDirection UI_Parse_LayoutDirection(const char* layout)
{
    if(isnil(layout))
        return widgets::LayoutDirection_Default;
        
    if(equalsIgnoreCase("default", layout))
        return widgets::LayoutDirection_Default;
    if(equalsIgnoreCase("horizontal", layout))
        return widgets::LayoutDirection_Horizontal;
    if(equalsIgnoreCase("vertical", layout))
        return widgets::LayoutDirection_Vertical;

    return widgets::LayoutDirection_Default;
}

widgets::IWidget* UI_Create_Widget(const XmlDocumentNode* node)
{
    widgets::IWidget* w = NULL;

    int Ww = -1, Wh = -1;
    tointeger(node->getAttribute("width"), Ww);
    tointeger(node->getAttribute("height"), Wh);

    const char* Wt = node->getNodeName();
    const char* Wid = node->getAttribute("id");

    if(equalsIgnoreCase("button", Wt)) {
        auto b = new widgets::Button(UI_Parse_ButtonStyle(node->getAttribute("style")), UI_Parse_ButtonDirection(node->getAttribute("direction")));
        if(Ww == -1 || Wh == -1)
            return nullptr;

        if(const char* onclick = node->getAttribute("onclick"); !isnil(onclick))
        {
            b->onClick = s_ButtonDelegates[hashQuickLowerCase(onclick)];
        }

        b->initWithLabelAndSize(node->getText(), Ww, Wh);
        w = b;
    }
    if(equalsIgnoreCase("label", Wt)) {
        auto l = new widgets::Label();
        const char* url = node->getAttribute("ref");

        if(isnil(url))
            l->initWithLabel(node->getText());
        else
            l->initWithLabelAndURL(node->getText(), url);

        w = l;
    }
    if(equalsIgnoreCase("textbox", Wt)) {
        auto tb = new widgets::TextBox(UI_Parse_TextBoxStyle(node->getAttribute("style")));
        tb->initWithLabelAndHint(node->getText(), node->getAttribute("hint"));

        w = tb;
    }
    if(equalsIgnoreCase("list", Wt)) {
        auto lv = new widgets::ListView();
        const char* id = node->getAttribute("id");
        lv->initWithID(id);

        w = lv;
    }
    if(equalsIgnoreCase("panel", Wt)) {
        widgets::UIPanel* panel = new widgets::UIPanel();
        const char* id = node->getAttribute("id");
        const char* layout = node->getAttribute("layout");
        const char* title = node->getAttribute("title");
        panel->initWithLabelAndLayout(title, UI_Parse_LayoutDirection(layout));
        panel->setID(id);

        XmlDocumentNode* child = node->getChild();
        while(child)
        {
            widgets::IWidget* Cw = UI_Create_Widget(child);
            if(Cw)
                panel->addWidget(Cw);

            child = child->getSibling();
        }

        w = panel;
    }

    if(!isnil(Wid))
        s_WidgetTable[hashQuickLowerCase(Wid)] = w;

    if(Ww != -1)
        w->setWidth(Ww);

    if(Wh != -1)
        w->setHeight(Wh);

    return w;
}

int ACanopyAgent::onStart() 
{   
    s_ButtonDelegates[_HASH("On_Estabilish")] = makeDelegate(On_Estabilish);

    new (&ai) Aidl();
    ai.payload_dispatcher = _Handle_data;

    XmlDocument* Xcanopyui = XML_Parse(DEVICE.shareFile("data:/canopyui.xml"));
    XmlDocumentNode* Xui = Xcanopyui->getNode();
    if(!equalsIgnoreCase("ui", Xui->getNodeName()))
    {
        EERROR("invalid root");
        return 1;
    }

    widgets::UIContext ui;

    XmlDocumentNode* node = Xui->getChild();
    while(node)
    {
        if(!equalsIgnoreCase("panel", node->getNodeName())) {
            node = node->getSibling();
            continue;
        }

        widgets::IWidget* panel = UI_Create_Widget(node);
        if(panel)
            ui.addSurface((widgets::UIPanel *)panel);

        node = node->getSibling();
    }

    coreRenderInit(1280, 720, "Canopy");

    while(!coreRenderShouldClose())
    {
        ui.preRenderFrame();
        ui.renderFrame();
        ui.postRenderFrame();
    }

    ELOG("[aidld] deactivating");

    if(AidlClose(&ai) <0)
        return -1;

    coreRenderShutdown();

    return YES;
}
}