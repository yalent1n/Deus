#pragma once
#include "../Bundle.h"
#include "../String/String.h"
#include <cstring>

#define FE_SIZE_DEFAULT 256

namespace rw
{
template<u32 S = FE_SIZE_DEFAULT>
struct GaloisField
{
    CompileTimeAssertion((S % (sizeof(u32)*8)) == 0);

    static inline constexpr size_t _BitSize = S;
    static inline constexpr size_t _NarrowSize = S / (sizeof(u32)*8);
    static inline constexpr size_t _WideSize = _NarrowSize * 2;
    static inline constexpr size_t _ByteSize = S / (sizeof(u8)*8);
    u32 d[_NarrowSize] {};

    // pszeudo: R2 = 2^(2*32*S) mod p
    static inline GaloisField ComputeR2(const GaloisField& p) {
        GaloisField x = {0};
        x.d[0] = 1;
        for (int i = 0; i < 2 * 32 * _NarrowSize; ++i)
            x = x.add(x, p);

        return x;
    }

    inline bool is_zero() const {
        u32 t = 0;
        for(int i = 0; i < _NarrowSize; ++i) t |= d[i];
        return !t;
    }

    inline bool eq(const GaloisField<S>& b) const {
        return memcmp(d, b.d, _ByteSize) == 0;
    }

    inline int cmp(const GaloisField<S>& b) const {
        for(int i = _NarrowSize - 1; i >= 0; --i) {
            if(d[i] < b.d[i]) return -1;
            if(d[i] > b.d[i]) return 1;
        }
        return 0;
    }

    inline GaloisField radd(const GaloisField& b) const {
        GaloisField<S> r {};

        u64 c = 0;
        for(int i = 0; i < _NarrowSize; ++i) {
            c += (u64)(d[i] + b.d[i]);
            r.d[i] = (u32)c;
            c >>= 32;
        }

        (void) c;
        return r;
    }

    inline GaloisField<S> add(const GaloisField& b, const GaloisField& p) const {
        GaloisField<S> r {};

        u64 c = 0;
        for(int i = 0; i < _NarrowSize; ++i) {
            c += (u64)d[i] + b.d[i];
            r.d[i] = (u32)c;
            c >>= 32;
        }

        if(c || (r.cmp(p) >= 0)) {
            u64 br = 0;
            for(int i = 0; i < _NarrowSize; ++i) {
                u64 t = (u64)r.d[i] - p.d[i] - br;
                r.d[i] = (u32)t;
                br = (t >> 63) & 1;
            }
        }

        return r;
    }

    inline GaloisField sub(const GaloisField& b, const GaloisField& p) const {
        GaloisField<S> r {};

        s64 br = 0;
        for(int i = 0; i < _NarrowSize; ++i) {
            s64 t = (s64)d[i] - b.d[i] - br;
            if(t < 0) { t += (1LL << 32); br = 1; }
            else br = 0;
            r.d[i] = (u32)t;
        }

        if(br) {
            u64 c = 0;
            for(int i = 0; i < _NarrowSize; ++i) {
                c += (u64)r.d[i] + p.d[i];
                r.d[i] = (u32)c;
                c >>= 32;
            }
        }

        return r;
    }

    inline GaloisField neg(const GaloisField& p) const {
        if(is_zero()) 
            return *this;

        return p.sub(*this, p);
    }

    /* p0_inv: -inv(p[0]) mod 2^32 */
    u32 inv_0() const
    {
        /* Newton: 32-bit odd inverse */
        u32 x = 1;
        for (int i = 0; i < 5; ++i) /* 2^5 > 32 */
            x *= 2 - d[0] * x;
        return (u32)(0u - x); /* -p0^{-1} */
    }

    inline void rmul(u32 o[_WideSize], const GaloisField& b) const {
        for (int i = 0; i < _WideSize; ++i) o[i] = 0;

        for (int i = 0; i < _NarrowSize; ++i) {
            u64 c = 0;
            for (int j = 0; j < _NarrowSize; ++j) {
                c += (u64)o[i+j] + (u64)d[i] * b.d[j];
                o[i+j] = (u32)c;
                c >>= 32;
            }
            o[i+8] += (u32)c;
        }
    }

    inline void o_tomontgomery(GaloisField& out, const GaloisField& r2, const GaloisField& p) const {
        ommul(out, r2, p);
    }

    inline GaloisField to_montgomery(const GaloisField& r2, const GaloisField& p) const {
        GaloisField<S> r {};
        o_tomontgomery(r, r2, p);
        return r;
    }

    inline void o_frommontgomery(GaloisField& out, const GaloisField& p) const {
        GaloisField<S> one {};
        one.d[0] = 1;
        ommul(out, one, p);
    }

    inline GaloisField from_montgomery(const GaloisField& p) const {
        GaloisField<S> r {};
        o_frommontgomery(r, p);
        return r;
    }

    inline void ommul(GaloisField& out, const GaloisField& b, const GaloisField& p) const {
        u32 t[_NarrowSize + 2] = {0}; // Extra biztonsági puffer a c-nek

        for (int i = 0; i < _NarrowSize; ++i) {
            // 1. t += d[i] * b
            u64 c = 0;
            for (int j = 0; j < _NarrowSize; ++j) {
                c += (u64)t[j] + (u64)d[i] * b.d[j];
                t[j] = (u32)c;
                c >>= 32;
            }
            c += (u64)t[_NarrowSize];
            t[_NarrowSize] = (u32)c;
            t[_NarrowSize + 1] = (u32)(c >> 32);

            // 2. Montgomery reduction: m = t[0] * p0_inv mod 2^32
            u32 m = (u32)((u64)t[0] * p.inv_0());

            // 3. t += m * p és eltolás (shift right 1 limb)
            c = (u64)t[0] + (u64)m * p.d[0];
            c >>= 32; // t[0] most 0, elhagyjuk

            for (int j = 1; j < _NarrowSize; ++j) {
                c += (u64)t[j] + (u64)m * p.d[j];
                t[j - 1] = (u32)c;
                c >>= 32;
            }
            c += (u64)t[_NarrowSize];
            t[_NarrowSize - 1] = (u32)c;
            
            c = (c >> 32) + (u64)t[_NarrowSize + 1];
            t[_NarrowSize] = (u32)c;
            t[_NarrowSize + 1] = 0;
        }

        // Végső redukció: r = (t >= p) ? (t - p) : t
        s64 borrow = 0;
        u32 sub_res[_NarrowSize];
        for (int i = 0; i < _NarrowSize; ++i) {
            s64 diff = (s64)t[i] - p.d[i] - borrow;
            if (diff < 0) {
                diff += (1LL << 32);
                borrow = 1;
            } else {
                borrow = 0;
            }
            sub_res[i] = (u32)diff;
        }

        // Ha maradt legfelső carry (t[_NarrowSize] > 0) vagy a kivonás nem ment negatívba:
        if (t[_NarrowSize] > 0 || borrow == 0) {
            for (int i = 0; i < _NarrowSize; ++i) out.d[i] = sub_res[i];
        } else {
            for (int i = 0; i < _NarrowSize; ++i) out.d[i] = t[i];
        }
    }

    inline GaloisField mmul(const GaloisField& b, const GaloisField& p) const {
        GaloisField<S> r {};
        ommul(r, b, p);
        return r;
    }

    inline GaloisField mul(const GaloisField& b, const GaloisField& p, const GaloisField& r2) const {
        GaloisField<S> am {}, bm {}, cm {}, r {};
        o_tomontgomery(am, r2, p);
        b.o_tomontgomery(bm, r2, p);
        am.ommul(cm, bm, p);
        cm.o_frommontgomery(r, p);
        return r;
    }

    static inline void mone(GaloisField& r, const GaloisField& r2, const GaloisField& p) {
        GaloisField<S> one {0};
        one.d[0] = 1;
        one.ommul(r, r2, p);
    }

    inline void ompow(GaloisField& out, const GaloisField& exp, const GaloisField& p, const GaloisField& r2) const {
        GaloisField<S> acc {}; // Montgomery '1', azaz (1 * R) mod
        mone(acc, r2, p);

        // Bitről bitre haladunk a legfelső bittől a legalsóig
        for (int i = 255; i >= 0; --i) {
            acc = acc.mmul(acc, p);

            // Ha az exp i-edik bitje 1
            uint32_t limb = exp.d[i / 32];
            if ((limb >> (i % 32)) & 1) {
                acc = acc.mmul(*this, p);
            }
        }
        out = acc;
    }

    inline GaloisField mpow(const GaloisField& exp, const GaloisField& p, const GaloisField& r2) const {
        GaloisField<S> r {};
        ompow(r, exp, p, r2);
        return r;
    }

    inline GaloisField pow(const GaloisField& exp, const GaloisField& p, const GaloisField& r2) const {
        GaloisField<S> bm {}, rm {}, r {};
        o_tomontgomery(bm, r2, p);
        bm.ompow(rm, exp, p, r2);
        rm.o_frommontgomery(r, p);
        return r;
    }

    // inv: a^(p-2) mod p   (p prím, a ≠ 0)
    inline void ominv(GaloisField& out, const GaloisField& p, const GaloisField& r2) const {
        GaloisField<S> pm2 = p;
        pm2.d[0] -= 2;
        ompow(out, pm2, p, r2);
    }

    inline GaloisField minv(const GaloisField& p, const GaloisField& r2) const {
        GaloisField<S> r {};
        ominv(r, p, r2);
        return r;
    }

    inline GaloisField inv(const GaloisField& p, const GaloisField& r2) const {
        GaloisField<S> am {}, rm {}, r {};
        o_tomontgomery(am, r2, p);
        am.ominv(rm, p, r2);
        rm.o_frommontgomery(r, p);
        return r;
    }
    
    inline void omdiv(GaloisField& out, const GaloisField& b, const GaloisField& p, const GaloisField& r2) const {
        GaloisField<S> bminv {};
        b.ominv(bminv, p, r2);
        ommul(out, bminv, p);
    }

    inline GaloisField mdiv(const GaloisField& b, const GaloisField& p, const GaloisField& r2) const {
        GaloisField<S> r {};
        omdiv(r, b, p, r2);
        return r;
    }

    inline GaloisField div(const GaloisField& b, const GaloisField& p, const GaloisField& r2) const {
        GaloisField am, bm, rm {};
        o_tomontgomery(am, r2, p);
        b.o_tomontgomery(bm, r2, p);
        am.omdiv(rm, bm, p, r2);
        return rm.from_montgomery(p);
    }

    inline GaloisField shr(int shift) {
        if (shift <= 0) 
            return {};

        constexpr int limb_bits = _ByteSize; // pl. 32 vagy 64 bit limb-enként

        GaloisField r = *this;

        // Ha a toltás nagyobb vagy egyenlő egy limb méretével, kezelhetnénk szavanként is,
        // de a (p+1)/4 miatt most elegendő a kis bit-eltolás (pl. shift = 2).
        for (int i = 0; i < S - 1; ++i) {
            r.d[i] = (r.d[i] >> shift) | (r.d[i + 1] << (limb_bits - shift));
        }
        r.d[S - 1] >>= shift;

        return r;
    }
};

template<u32 S = FE_SIZE_DEFAULT>
inline constexpr GaloisField<S> fe_zero() {
    return {};
}

template<u32 S = FE_SIZE_DEFAULT>
inline GaloisField<S> fe_u64(u64 v) {
    GaloisField<S> fe;
    fe.d[0] = (u32) v;
    fe.d[1] = (u32) (v >> 32);
    return fe;
}

template<u32 S = FE_SIZE_DEFAULT>
bool fe_hex(GaloisField<S>& out, const char* hex)
{
    out = fe_zero();
    // skip 0x
    if (hex[0]=='0' && (hex[1]|32)=='x') hex += 2;
    size_t n = strlen(hex);
    if (n > 64) return false;
    // pad left with zeros → 64 nibbles, fill BE into limbs
    char pad[65];
    memset(pad, '0', 64);
    memcpy(pad + (64 - n), hex, n);
    pad[64] = 0;
    for (int i = 0; i < 8; ++i) {
        u32 v = 0;
        for (int j = 0; j < 8; ++j) {
            char c = pad[i*8 + j];
            int nibble = (c >= '0' && c <= '9') ? c - '0' :
                         (c >= 'a' && c <= 'f') ? c - 'a' + 10 :
                         (c >= 'A' && c <= 'F') ? c - 'A' + 10 : -1;
            if (nibble < 0) return false;
            v = (v << 4) | (u32)nibble;
        }
        out.d[7 - i] = v; // first hex chunk = MSB limb
    }
    return true;
}

template<u32 S>
inline const char* fe_to_hex(const GaloisField<S>& a, StringEditor& se, bool leading_zeros = true)
{
    static const char* H = "0123456789abcdef";

    bool started = leading_zeros;
    for (int i = 7; i >= 0; --i) {          // MSB limb először
        for (int n = 7; n >= 0; --n) {      // 8 nibble limb-enként
            uint8_t nib = (a.d[i] >> (n * 4)) & 0xF;
            if (!started) {
                if (nib == 0) continue;
                started = true;
            }
            se << H[nib];
        }
    }
    if (se.isEmpty()) se << '0';
    return se.c_str();
}

inline void cswap_limbs(u32* a, u32* b_, size_t numLimbs, uint32_t bit) {
    // mask = 0xFFFFFFFF if bit==1, 0x00000000 if bit==0 -- pure arithmetic,
    // no comparison/branch involved in producing it.
    uint32_t mask = static_cast<uint32_t>(-static_cast<int32_t>(bit & 1));
    for (size_t i = 0; i < numLimbs; ++i) {
        uint32_t t = mask & (a[i] ^ b_[i]);
        a[i]  ^= t;
        b_[i] ^= t;
    }
}

template<u32 S>
inline void cswap_fe(GaloisField<S>& x, GaloisField<S>& y, u32 bit) {
    cswap_limbs(x.d, y.d, GaloisField<S>::_NarrowSize, bit);
}

template<u32 S>
inline void cswap_pt(GaloisField<S>::Point& p0, GaloisField<S>::Point& p1, u32 bit) {
    cswap_fe(p0.x, p1.x, bit);
    cswap_fe(p0.y, p1.y, bit);
    // cswap_fe256(p0.Z, p1.Z, bit);   // <- uncomment if/when you add Z (Jacobian)
}
}