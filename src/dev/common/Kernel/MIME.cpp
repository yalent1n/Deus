#include <Engine.Base/MIME.h>
#include <Engine.Base/String/Util.h>

namespace rw
{
static const char* s_ansiMimeTypes[] =
{
    "*/*", // Any
    "text/plain", // Plain
    "text/html", // Html
    "text/css", // Css
    "text/xml", // Xml (also application/xml)
    "text/javascript", // Javascript
    "application/json", // Json
    "application/fson", // Fson
    "application/zip", // Zip
    "application/octet-stream", // OctetStream & Unknown
    "application/x-www-form-urlencoded", // Url
    "application/flux-cas-patch", // Patch
    "application/flux-cas-encoded" // Compressed
};

static const wchar_t* s_wideMimeTypes[] =
{
    L"*/*", // Any
    L"text/plain", // Plain
    L"text/html", // Html
    L"text/css", // Css
    L"application/xml", // Xml (also text/xml)
    L"application/javascript", // Javascript (also text/javascript & application/ecmascript)
    L"application/json", // Json
    L"application/fson", // Fson
    L"application/zip", // Zip
    L"application/octet-stream", // OctetStream & Unknown
    L"application/x-www-form-urlencoded", // Url
    L"application/x-cas-data-patched", // Patch
    L"application/x-cas-data-encoded" // Compressed
};

const char* CMime::toString(MIME m)
{
    return (m < Mime_enum_Count ? s_ansiMimeTypes[m] : nullptr);
}

const wchar_t* CMime::toWideString(MIME m)
{
    return (m < Mime_enum_Count ? s_wideMimeTypes[m] : nullptr);
}

MIME CMime::fromString(const char* s)
{
    for(size_t i = 0; i < Mime_enum_Count; i++)
    {
        s32 Cmp = compareIgnoreCase(s, s_ansiMimeTypes[i]);
        if(!Cmp)
            return MIME(i);
        else    
        {
            // Handle aliases
            if(i == Mime_Xml)
            {
                Cmp = compareIgnoreCase(s, "text/xml");
            }
            else if(i == Mime_Json)
            {
                Cmp = compareIgnoreCase(s, "text/json");
            }
            else if(i == Mime_Javascript)
            {
                Cmp = compareIgnoreCase(s, "text/javascript");
                if(Cmp)
                    Cmp = compareIgnoreCase(s, "application/ecmascript");
            }

            if(!Cmp)
                return MIME(i);
        }
    }

    return Mime_Any;
}

MIME CMime::fromWideString(const wchar_t* s)
{
    for(size_t i = 0; i < Mime_enum_Count; i++)
    {
        s32 Cmp = compareIgnoreCaseWide(s, s_wideMimeTypes[i]);
        if(!Cmp)
            return MIME(i);
        else
        {
            if(i == Mime_Xml)
            {
                Cmp = compareIgnoreCaseWide(s, L"text/xml");
            }
            else if(i == Mime_Json)
            {
                Cmp = compareIgnoreCaseWide(s, L"text/json");
            }
            else if(i == Mime_Javascript)
            {
                Cmp = compareIgnoreCaseWide(s, L"text/javascript");
                if(Cmp)
                    Cmp = compareIgnoreCaseWide(s, L"application/ecmascript");
            }

            if(!Cmp)
                return MIME(i);
        }
    }

    return Mime_Any;
}
}