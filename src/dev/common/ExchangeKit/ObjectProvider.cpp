#include <CoreSaturn/ObjectProvider.h>
#include <Engine.Core/diag/ExecutionContext.h>

namespace Deus
{
static const char* _Props_MakePath(IANumber num, StringEditor& vpath)
{
    // XML DataBase
    vpath << "app:/xdb/";

    IANumber::DisplayString recstr;
    IANumber::toDisplayString(num, recstr);
    for(s32 i = 0; i < sizeof(recstr) - 1; i++)
    {
        if(recstr[i] == '-')
            recstr[i] = '/';
    }

    vpath << recstr << ".plist";

    return vpath.c_str();
}

ekvm_Provider_object::ekvm_Provider_object()
{
    StringBuilder<> sb;
    const char* wbdir = _Props_MakePath(thisIAN(), sb);
    const_cast<char*>(wbdir)[rw::lastIndexOf('/', wbdir)] = '\0';
    DEVICE.createDirectory(wbdir);
}

IANumber ekvm_Provider_object::thisIAN()
{
    return { 0, 44, 12, 2 };
}

DbObject ekvm_Provider_object::readOBJECT(IANumber num)
{
    StringBuilder<> sb;
    const char* recpath = _Props_MakePath(num, sb);
    if(!DEVICE.fileExists(recpath))
        return c_NullObject;

    ScopedRef buffer = DEVICE.shareFile(recpath);
    if(buffer.isNull())
        return c_NullObject;

    DbObject plist;
    plist.fromPlistString((const char*) buffer->bytes());
    return plist;
}

IANumber ekvm_Provider_object::applyOBJECT(const DbObject& record)
{
    StringBuilder<> sb;
    const char* recdir = _Props_MakePath(thisIAN(), sb);
    int pos = rw::lastIndexOf('/', recdir);
    const_cast<char*>(recdir)[pos + 1] = '*';
    const_cast<char*>(recdir)[pos + 2] = '.';
    const_cast<char*>(recdir)[pos + 3] = '*';
    const_cast<char*>(recdir)[pos + 4] = '\0';

    FileInfo _Tmp;
    char name[MAX_PATH] {};

    s32 next_object_id = 0;

    FileIterator* it = DEVICE.getFileIterator(recdir);
    while(it->getNextFile(name, sizeof(name), &_Tmp))
        ++next_object_id;

    rw::SkeletonBuildParams params;
    params.pretty = YES;
    params.pretty_text = NO;

    StringBuilder<> plistBuffer;
    record.toPlistString(plistBuffer, params);

    IANumber ia_number = thisIAN();
    ia_number.artifact_id = next_object_id;

    sb.clear();
    new (&sb) StringBuilder<>;
    _Props_MakePath(ia_number, sb);

    ScopedRef buffer = DEVICE.createFile(sb.c_str());
    buffer->write(plistBuffer.c_str(), plistBuffer.size());

    return ia_number;
}
}