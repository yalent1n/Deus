#include <Engine.Base/Crypto/Base64.h>
#include <Engine.Base/String/String.h>
#include <Engine.Base/Memory/VirtualAlloc.h>

namespace rw
{
static inline constexpr char s_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz" "0123456789+/";

size_t base64_encodelength(size_t srcLen)
{
    if(srcLen == 0)
        return 0;

    return 4 * ((srcLen + 2) / 3);
}

size_t base64_decodelength(size_t srcLen)
{
    if(srcLen == 0)
        return 0;

    // Base64 length must be dividable by 4.
    if(srcLen % 4)
        return 0;
    
    return (srcLen / 4) * 3; 
}

size_t base64_encode(const void* src, size_t srcLen, void* dst, size_t dstLen)
{
    if(src == nullptr || dst == nullptr)
        return 0;

    if(srcLen == 0 || dstLen == 0)
        return 0;

    size_t outputLen = base64_encodelength(srcLen);
    if(dstLen < outputLen)
        return 0;
    
    const u8* str = static_cast<const u8*>(src);
    u8* buf = static_cast<u8*>(dst);

    size_t i = 0, j = 0;
    
    for(; i < srcLen;)
    {
        u32 octet0 = i < srcLen ? str[i++] : 0;
        u32 octet1 = i < srcLen ? str[i++] : 0;
        u32 octet2 = i < srcLen ? str[i++] : 0;
        u32 triplet = (octet0 << 16) + (octet1 << 8) + octet2;
        buf[j++] = s_chars[(triplet >> 18) & 0x3F];
        buf[j++] = s_chars[(triplet >> 12) & 0x3F];
        buf[j++] = s_chars[(triplet >> 6) & 0x3F];
        buf[j++] = s_chars[(triplet >> 0) & 0x3F];
    }

    for(i = 0; i < (3 - (srcLen % 3)) % 3; i++)
        buf[outputLen - 1 - i] = '=';

    return outputLen;
}

size_t base64_encode(const void* src, size_t srcLen, StringEditor& dst)
{
    if(src == nullptr || srcLen == 0)
        return 0;

    size_t outputLen = base64_encodelength(srcLen);
    
    if(dst.capacity() < outputLen && !dst.isDynamic())
        return 0;
    
    const u8* str = static_cast<const u8*>(src);

    size_t i = 0;
    
    for(; i < srcLen;)
    {
        u32 octet0 = i < srcLen ? str[i++] : 0;
        u32 octet1 = i < srcLen ? str[i++] : 0;
        u32 octet2 = i < srcLen ? str[i++] : 0;
        u32 triplet = octet0 << 16 | octet1 << 8 | octet2;
        dst << s_chars[(triplet >> 18) & 0x3F];
        dst << s_chars[(triplet >> 12) & 0x3F];
        dst << s_chars[(triplet >> 6) & 0x3F];
        dst << s_chars[(triplet >> 0) & 0x3F];
    }

    u8* padPtr = (u8*)dst.data() + (dst.size() - outputLen);
    for(i = 0; i < (3 - (srcLen % 3)) % 3; i++)
    {
        padPtr[outputLen - 1 - i] = '=';
    }

    return outputLen;
}

u32 b64_ch2i(const u8 c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';           // 0-25
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;      // 26-51
    if (c >= '0' && c <= '9') return c - '0' + 52;      // 52-61
    if (c == '+') return 62;                            // 62
    if (c == '/') return 63;                            // 63
    return u32(-1); // Invalid character
}

size_t base64_decode(const void* src, size_t srcLen, void* dst, size_t dstLen)
{
    if(src == nullptr || dst == nullptr)
        return 0;

    size_t outputLen = base64_decodelength(srcLen);
    if(outputLen == 0)
        return 0;

    const u8* str = static_cast<const u8*>(src);
    u8* buf = static_cast<u8*>(dst);
    
    size_t padding = 0;
    if(*(str + srcLen - 1) == '=') padding++;
    if(*(str + srcLen - 2) == '=') padding++;
    outputLen -= padding;

    if(dstLen < outputLen)
        return 0;

    size_t i = 0, j = 0;

    for(;i < srcLen;)
    {
        u32 a = b64_ch2i(str[i++]);
        u32 b = b64_ch2i(str[i++]);
        u32 c = b64_ch2i(str[i++]);
        u32 d = b64_ch2i(str[i++]);

        buf[j++] = (u8) ((a << 2) | ((b&0x30) >> 4));
        buf[j++] = (u8) (((b&0xf) << 4) | ((c&0x3c) >> 2));
        buf[j++] = (u8) (((c&0x3) << 6) | d);
    }

    return outputLen;
}

size_t base64_decode(const void* src, size_t srcLen, StringEditor& dst)
{
    if(src == nullptr)
        return 0;

    size_t outputLen = base64_decodelength(srcLen);
    if(outputLen == 0)
        return 0;

    const u8* str = static_cast<const u8*>(src);

    size_t padding = 0;
    if(str[srcLen - 1] == '=') padding++;
    if(str[srcLen - 2] == '=') padding++;
    outputLen -= padding;

    if(dst.capacity() < outputLen && !dst.isDynamic())
        return 0;

    size_t i = 0;

    // NOTE:
    // If 'c' or 'd' is a '=' sign, ch2i with result in -1, which means
    // that the iteration has to stop, so convert them to 0. ('c' is zero because
    // the bitop would fuck up the actual 2nd char 'octet' translation and
    // an additonal 0xFF char would be appended).
    for(;i < srcLen;)
    {
        u32 a = b64_ch2i(str[i++]);
        u32 b = b64_ch2i(str[i++]);
        u32 c = b64_ch2i(str[i++]);
        u32 d = b64_ch2i(str[i++]);

        // who the actual fuck would append enormous bytes of '=' to a base64... fuck this shit.
        if(a == u32(-1) || b == u32(-1))
            break;

        if(c==u32(-1))c=0;
        if(d==u32(-1))d=0;
        
        dst << (char) ((a << 2) | ((b&0x30) >> 4));
        dst << (char) (((b&0xf) << 4) | ((c&0x3c) >> 2));
        if(d==0)
            break;
        dst << (char) (((c&0x3) << 6) | d);
    }

    return outputLen;
}

size_t base64_urlencode(const void* src, size_t srcLen, void* dst, size_t dstLen)
{
    if(src == nullptr)
        return 0;
    
    size_t rawLen = base64_encodelength(srcLen);
    if(rawLen == 0)
        return 0;

    if(rawLen > dstLen)
        return 0;

    // for simplicity, we allocate on heap for raw base 64 and later free it.
    char* raw = static_cast<char*>(HeapAllocGlobal(rawLen + 1));
    raw[rawLen] = '\0';
    base64_encode(src, srcLen, raw, rawLen);

    char* csr = (char*) dst;

    size_t i;
    for(i = 0; i < rawLen; i++)
    {
        switch(raw[i])
        {
        case '=':
            HeapFreeGlobal(raw);
            return i;
        case '+':
            *csr++ = '-';
            break;
        case '/':
            *csr++ = '_';
            break;
        default:
            *csr++ = raw[i];
            break;
        }
    }

    HeapFreeGlobal(raw);
    return i;
}

size_t base64_urlencode(const void* src, size_t srcLen, StringEditor& dst)
{
    if(src == nullptr)
        return 0;
    
    size_t rawLen = base64_encodelength(srcLen);
    if(rawLen == 0)
        return 0;

    if(rawLen > dst.capacity() && !dst.isDynamic())
        return 0;

    // for simplicity, we allocate on heap for raw base 64 and later free it.
    char* raw = static_cast<char*>(HeapAllocGlobal(rawLen + 1));
    raw[rawLen] = '\0';
    
    base64_encode(src, srcLen, raw, rawLen);

    size_t i;
    for(i = 0; i < rawLen; i++)
    {
        switch(raw[i])
        {
        case '=':
            HeapFreeGlobal(raw);
            return i;
        case '+':
            dst << '-';
            break;
        case '/':
            dst << '_';
            break;
        default:
            dst << raw[i];
            break;
        }
    }

    HeapFreeGlobal(raw);
    return i;
}

size_t base64_urldecode_internal(StringEditor& src)
{
    const u32 size = src.size();
    
    for(u32 i = 0; i < size; i++)
    {
        const char c = src.c_str()[i];
        if(c == '-')
            src.data()[i] = '+';
        if(c == '_')
            src.data()[i] = '/';
    }

    while(src.size() % 4 != 0)
        src << '=';

    return src.size();
}

size_t base64_urldecode(const void* src, size_t srcLen, void* dst, size_t dstLen)
{
    if(src == nullptr || srcLen == 0)
        return 0;

    StringBuilder<> encoded;
    encoded.appendChars(static_cast<const char*>(src), static_cast<u32>(srcLen));
    base64_urldecode_internal(encoded);
    
    return base64_decode(encoded.c_str(), encoded.size(), dst, dstLen);
}

size_t base64_urldecode(const void* src, size_t srcLen, StringEditor& dst)
{
    if(src == nullptr || srcLen == 0)
        return 0;

    StringBuilder<> encoded;
    encoded.appendChars(static_cast<const char*>(src), static_cast<u32>(srcLen));
    base64_urldecode_internal(encoded);
    
    return base64_decode(encoded.c_str(), encoded.size(), dst);
}
}