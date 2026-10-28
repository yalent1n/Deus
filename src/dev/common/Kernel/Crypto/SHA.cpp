#include <Engine.Base/Crypto/SHA.h>
#include <Engine.Base/Crypto/SHAFunctions.h>
#include <Engine.Base/String/String.h>
#include <vcruntime_string.h>

namespace rw
{
template<
    typename _TContext, 
    Crypto::CRESULT(*_TContextInitialize)(_TContext&, const void*, size_t),
    Crypto::CRESULT(*_TContextFinalize)(_TContext&, void*)
>
void sha2_calculate(void* _OutBuffer, const void* _Ptr, size_t _Size, const void* _Key = NIL, size_t _KeySize = NIL)
{
    _TContext context;
    _TContextInitialize(context, _Key, _KeySize);
    Crypto::SHA2::SHAUpdate(context, _Ptr, _Size);
    _TContextFinalize(context, _OutBuffer);
}

template<size_t _BitSize>
ShaImpl<_BitSize>::ShaImpl(u8* p)
{
    memcpy(hash, p, _ByteSize);    
}

template<size_t _BitSize>
void ShaImpl<_BitSize>::calculate(const void* _Ptr, u64 _Size)
{
    if constexpr (_BitSize == 160)
        sha2_calculate<Crypto::SHA1Context, Crypto::SHA2::SHA1Initialize, Crypto::SHA2::SHA1Finalize>(hash, _Ptr, _Size);
    else if constexpr (_BitSize == 256)
        sha2_calculate<Crypto::SHA256Context, Crypto::SHA2::SHA256Initialize, Crypto::SHA2::SHA256Finalize>(hash, _Ptr, _Size);
    else if constexpr (_BitSize == 384)
        sha2_calculate<Crypto::SHA384Context, Crypto::SHA2::SHA384Initialize, Crypto::SHA2::SHA384Finalize>(hash, _Ptr, _Size);
    else
        sha2_calculate<Crypto::SHA512Context, Crypto::SHA2::SHA512Initialize, Crypto::SHA2::SHA512Finalize>(hash, _Ptr, _Size);
}

template<size_t _BitSize>
void ShaImpl<_BitSize>::calculateHmac(const void* _Ptr, u64 _Size, const void* _Key, u64 _KeySize)
{
    if constexpr (_BitSize == 160)
        sha2_calculate<Crypto::SHA1Context, Crypto::SHA2::SHA1Initialize, Crypto::SHA2::SHA1Finalize>(hash, _Ptr, _Size, _Key, _KeySize);
    else if constexpr (_BitSize == 256)
        sha2_calculate<Crypto::SHA256Context, Crypto::SHA2::SHA256Initialize, Crypto::SHA2::SHA256Finalize>(hash, _Ptr, _Size, _Key, _KeySize);
    else if constexpr (_BitSize == 384)
        sha2_calculate<Crypto::SHA384Context, Crypto::SHA2::SHA384Initialize, Crypto::SHA2::SHA384Finalize>(hash, _Ptr, _Size, _Key, _KeySize);
    else
        sha2_calculate<Crypto::SHA512Context, Crypto::SHA2::SHA512Initialize, Crypto::SHA2::SHA512Finalize>(hash, _Ptr, _Size, _Key, _KeySize);    
}

template<size_t _BitSize>
bool ShaImpl<_BitSize>::parse(const char* str)
{
    size_t len = strlen(str);
    if(len != (_ByteSize * 2))
        return false;

    fromhexstr(str, len, this, _ByteSize);
    return true;
}

template<size_t _BitSize>
bool ShaImpl<_BitSize>::parse(const char* str, size_t len)
{
    if(isnil(str) || len != (_ByteSize * 2))
        return false;

    fromhexstr(str, len, this, _ByteSize);
    return true;
}

template<size_t _BitSize>
bool ShaImpl<_BitSize>::operator==(const ShaImpl<_BitSize>& other) const
{
    return memcmp(this, &other, _ByteSize) == 0;    
}

template<size_t _BitSize>
bool ShaImpl<_BitSize>::operator!=(const ShaImpl<_BitSize>& other) const
{
    return memcmp(this, &other, _ByteSize) != 0;
}

template<size_t _BitSize>
int ShaImpl<_BitSize>::operator<(const ShaImpl<_BitSize>& other) const
{
    return memcmp(this, &other, _ByteSize);
}

template<class _TImp>
ShaStr<_TImp>::ShaStr(const _TImp& imp)
{
    StringEditor builder(str, BUFFER_LENGTH);
    tohexstr(imp.hash, _TImp::_ByteSize, builder);
    (void) builder.detach();
}

template struct ShaImpl<160>;
template struct ShaImpl<256>;
template struct ShaImpl<384>;
template struct ShaImpl<512>;

template struct ShaStr<SHA1>;
template struct ShaStr<SHA256>;
template struct ShaStr<SHA384>;
template struct ShaStr<SHA512>;
}