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

#include <CoreSaturn/Management.h>
#include <Engine.Base/Crypto/ECC.h>

#include "DiskManagementService.h"

//#include "Lexer.h"

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

// widgets::UIPanel* sm_ContentBrowserUI;
// widgets::UIPanel* sm_PropertiesUI;
// widgets::UIContext* sm_UIContext;

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

constexpr u16 Port = 443;

bool test_P2P()
{
    Deus::Exchange::Map map {};
    int input_index = 0, output_index = 0;

    input_index = Deus::Exchange::Input(map);
    output_index = Deus::Exchange::Output(map);
    Deus::Exchange::layoutP2PEvents(map, input_index, 100, output_index);
    
    input_index = Deus::Exchange::Input(map);
    output_index = Deus::Exchange::Output(map);
    Deus::Exchange::layoutP2PEvents(map, input_index, 2300, output_index);

    input_index = Deus::Exchange::Input(map);
    output_index = Deus::Exchange::Output(map);
    Deus::Exchange::layoutP2PEvents(map, input_index, 560, output_index);

    input_index = Deus::Exchange::Input(map);
    output_index = Deus::Exchange::Output(map);
    Deus::Exchange::layoutP2PEvents(map, input_index, 40010, output_index);

    ScopedRef _Sb = Deus::Exchange::Build(map);
    Deus::Exchange::Junction* _Junc = (Deus::Exchange::Junction*) _Sb->bytes();
    
    Deus::IANumber ia_number0 = Deus::Management::providerDATA().applyDATA(_Sb);
    Deus::Exchange::Hash jhash = Deus::Exchange::seal(&_Junc->sig_hash, "xchg-link", _Sb->bytes(), _Sb->size64());
    Deus::IANumber ia_number1 = Deus::Management::providerOBJECT().applyOBJECT(DBOBJ(
        "jhash" << jhash
    ));

    Deus::IANumber::DisplayString id_string;
    Deus::IANumber::toDisplayString(ia_number0, id_string);
    ELOGF("P2P Transaction ID : %s , jhash : %s", id_string, Sha256Str(jhash).str);

    return YES;
}

bool test_MLTS()
{
    Deus::Exchange::Map map {};
    int input_index = 0, output_index = 0;

    input_index = Deus::Exchange::Input(map);
    output_index = Deus::Exchange::Output(map);
    Deus::Exchange::Output(map);
    Deus::Exchange::Output(map);
    Deus::Exchange::Output(map);

    Deus::Exchange::layoutMLTSEvents(map, input_index, output_index, {
        100,
        2300,
        560,
        40010
    });

    ScopedRef _Sb = Deus::Exchange::Build(map);
    Deus::Exchange::Junction* _Junc = (Deus::Exchange::Junction*) _Sb->bytes();
    
    Deus::IANumber ia_number0 = Deus::Management::providerDATA().applyDATA(_Sb);
    Deus::Exchange::Hash jhash = Deus::Exchange::seal(&_Junc->sig_hash, "xchg-link", _Sb->bytes(), _Sb->size64());
    Deus::IANumber ia_number1 = Deus::Management::providerOBJECT().applyOBJECT(DBOBJ(
        "jhash" << jhash
    ));

    Deus::IANumber::DisplayString id_string;
    Deus::IANumber::toDisplayString(ia_number0, id_string);
    ELOGF("MLTS Transaction ID : %s , jhash : %s", id_string, Sha256Str(jhash).str);

    return YES;
}

bool test_MLTR()
{
    Deus::Exchange::Map map {};
    int input_index = 0, output_index = 0;

    input_index = Deus::Exchange::Input(map);
    Deus::Exchange::Input(map);
    Deus::Exchange::Input(map);
    Deus::Exchange::Input(map);
    output_index = Deus::Exchange::Output(map);

    Deus::Exchange::layoutMLTREvents(map, output_index, input_index, { 100, 2300, 560, 40010 });
    ScopedRef _Sb = Deus::Exchange::Build(map);
    Deus::Exchange::Junction* _Junc = (Deus::Exchange::Junction*) _Sb->bytes();
    
    Deus::IANumber ia_number0 = Deus::Management::providerDATA().applyDATA(_Sb);
    Deus::Exchange::Hash jhash = Deus::Exchange::seal(&_Junc->sig_hash, "xchg-link", _Sb->bytes(), _Sb->size64());
    Deus::IANumber ia_number1 = Deus::Management::providerOBJECT().applyOBJECT(DBOBJ(
        "jhash" << jhash
    ));

    Deus::IANumber::DisplayString id_string;
    Deus::IANumber::toDisplayString(ia_number0, id_string);
    ELOGF("MLTR Transaction ID : %s , jhash : %s", id_string, Sha256Str(jhash).str);

    return YES;
}

bool test_UNI()
{
    Deus::Exchange::Map map;
    int input_index = 0, output_index = 0;

    input_index = Deus::Exchange::Input(map);
    Deus::Exchange::Input(map);
    Deus::Exchange::Input(map);
    Deus::Exchange::Input(map);
    output_index = Deus::Exchange::Output(map);
    Deus::Exchange::Output(map);
    Deus::Exchange::Output(map);
    Deus::Exchange::Output(map);

    Deus::Exchange::layoutLEDGEREvents(map, input_index, { 100, 2300, 560, 40010 }, output_index, { 1020, 2500, 300, 26000 });
    
    ScopedRef _Sb = Deus::Exchange::Build(map);
    Deus::Exchange::Junction* _Junc = (Deus::Exchange::Junction*) _Sb->bytes();
    
    Deus::IANumber ia_number0 = Deus::Management::providerDATA().applyDATA(_Sb);
    Deus::Exchange::Hash jhash = Deus::Exchange::seal(&_Junc->sig_hash, "xchg-link", _Sb->bytes(), _Sb->size64());
    Deus::IANumber ia_number1 = Deus::Management::providerOBJECT().applyOBJECT(DBOBJ(
        "jhash" << jhash
    ));

    Deus::IANumber::DisplayString id_string;
    Deus::IANumber::toDisplayString(ia_number0, id_string);
    ELOGF("UNI Transaction ID : %s , jhash : %s", id_string, Sha256Str(jhash).str);

    return YES;
}

static void expect(bool ok, const char* name) {
    std::printf("%s: %s\n", name, ok ? "OK" : "FAIL");
}

void _Handle_data(const void* data, u16 len)
{
    ELOG("<data>");
}

int ACanopyServer::onStart() 
{
    Aidl ai {};
    ai.payload_dispatcher = _Handle_data;

    if(AidlOpen(&ai, 1339) <0)
        return -1;

    ELOG("[aidld] active @ 1339");
    for(;;)
    {
        int R = AidlDispatchFrame(&ai, NULL);
        if(R == DL_ERR_WANT_READ)
            continue;

        if(R <= DL_ERR_FAILED)
        {
            EERRORF("[aidld] dispatch fail, Code : %d", R);
            break;
        }

        if(GetAsyncKeyState(VK_END) & 1)
            break;
    }

    ELOG("[aidld] deactivating ");
    AidlClose(&ai);

    // if(!test_P2P())
    // {
    //     EERROR("P2P test failed");
    //     return NO;
    // }
    // if(!test_MLTS())
    // {
    //     EERROR("MLTS test failed");
    //     return NO;
    // }
    // if(!test_MLTR())
    // {
    //     EERROR("MLTR test failed");
    //     return NO;
    // }
    // if(!test_UNI())
    // {
    //     EERROR("UNI test failed");
    //     return NO;
    // }
    
    // DiskManagementService srv(Port, DiskType::Flexible, "localapp:/Hdd0");

    // for(;;)
    // {
    //     // if(GetAsyncKeyState(VK_END) & 1)
    //     // {
    //     //     break;
    //     // }

    //     srv.update();
    // }

    // ELOGF("Shutdown on port %u", Port);
    // srv.shutdown();
    
    // scrDataLayout Layout = ComputeDataLayout((53 MiB));
    // ELOGF("Layout size: %zd", Layout.GetTotalSize());
    return YES;
}
}