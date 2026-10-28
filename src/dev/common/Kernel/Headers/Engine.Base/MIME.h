#pragma once
#include "Bundle.h"

namespace rw
{
enum MIME : u32
{
    Mime_Any,
    Mime_Plain,
    Mime_Html,
    Mime_Css,
    Mime_Xml,
    Mime_Javascript,
    Mime_Json,
    // custom binary json format (DbObject)
    Mime_Fson,
    Mime_Zip,
    Mime_OctetStream,
    Mime_Url,
    Mime_Patch, // for use in StoragePool
    Mime_Compressed, // for use in StoragePool
    Mime_enum_Count,

    // Aliases
    Mime_Unknown = Mime_OctetStream
};

struct CMime
{
FB_BASE_API static const char* toString(MIME);
FB_BASE_API static const wchar_t* toWideString(MIME);

FB_BASE_API static MIME fromString(const char*);
FB_BASE_API static MIME fromWideString(const wchar_t*);
};
}