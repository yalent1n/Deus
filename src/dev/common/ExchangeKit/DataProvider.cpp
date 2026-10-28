#include <CoreSaturn/DataProvider.h>
#include <Engine.Core/diag/ExecutionContext.h>

namespace Deus
{
static const char* _Props_MakePath(IANumber num, StringEditor& vpath)
{
    // Exchange Binaries
    vpath << "app:/bin/";

    IANumber::DisplayString recstr;
    IANumber::toDisplayString(num, recstr);
    for(s32 i = 0; i < sizeof(recstr) - 1; i++)
    {
        if(recstr[i] == '-')
            recstr[i] = '/';
    }

    vpath << recstr << ".bin";

    return vpath.c_str();
}

ekvm_Provider_data::ekvm_Provider_data()
{
    StringBuilder<> sb;
    const char* wbdir = _Props_MakePath(thisIAN(), sb);
    const_cast<char*>(wbdir)[rw::lastIndexOf('/', wbdir)] = '\0';
    DEVICE.createDirectory(wbdir);
}

IANumber ekvm_Provider_data::thisIAN()
{
    return { 0, 44, 12, 2 };
}

SharedBuffer* ekvm_Provider_data::readDATA(IANumber num)
{
    StringBuilder<> sb;
    const char* recpath = _Props_MakePath(num, sb);
    if(!DEVICE.fileExists(recpath))
        return nullptr;

    ScopedRef buffer = DEVICE.shareFile(recpath);
    if(buffer.isNull())
        return nullptr;

    return buffer.detach();
}

IANumber ekvm_Provider_data::applyDATA(const SharedBuffer* record)
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

    s32 next_item_id = 0;

    FileIterator* it = DEVICE.getFileIterator(recdir);
    while(it->getNextFile(name, sizeof(name), &_Tmp))
        ++next_item_id;

    IANumber ia_number = thisIAN();
    ia_number.artifact_id = next_item_id;

    sb.clear();
    new (&sb) StringBuilder<>;
    _Props_MakePath(ia_number, sb);

    ScopedRef buffer = DEVICE.createFile(sb.c_str());
    buffer->write(record->bytes(), record->size64());

    return ia_number;
}
}