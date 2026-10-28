#pragma once
#include "../Bundle.h"
#include "../Misc/EllipticCurve.h"

#define ECC_CURVE_PARAMS(Curve, P_, A_, B_, Gx, Gy) \
struct Curve ## Curve \
{ \
    static inline const BaseEllipticCurveGF P = (P_); \
    static inline const BaseEllipticCurveGF R2 = BaseEllipticCurveGF::ComputeR2(P_); \
    static inline const BaseEllipticCurveGF A = A_; \
    static inline const BaseEllipticCurveGF B = B_; \
    static inline const ECC_point G = { Gx, Gy, false }; \
}; \

typedef rw::GaloisField<256> ECC_field;

inline const ECC_field _ONE = rw::fe_u64(1);
inline const ECC_field _TWO = rw::fe_u64(2);
inline const ECC_field _THREE = rw::fe_u64(3);
inline const ECC_field _FOUR = rw::fe_u64(4);

template<typename _Curve>
struct EllipticCurveGF : ECC_field
{
    EllipticCurveGF() = default;
    EllipticCurveGF(const ECC_field& rhs) : ECC_field(rhs) {}

    inline EllipticCurveGF operator-() const {
        return EllipticCurveGF(ECC_field::neg(_Curve::P));
    }

    inline EllipticCurveGF inv() const {
        return EllipticCurveGF(ECC_field::inv(_Curve::P, _Curve::R2));
    }

    inline EllipticCurveGF operator+(const ECC_field& b) const {
        return EllipticCurveGF(ECC_field::add(b, _Curve::P));
    }

    inline EllipticCurveGF operator-(const ECC_field& b) const {
        return EllipticCurveGF(ECC_field::sub(b, _Curve::P));
    }

    inline EllipticCurveGF operator*(const ECC_field& b) const {
        return EllipticCurveGF(ECC_field::mul(b, _Curve::P, _Curve::R2));
    }

    inline EllipticCurveGF operator/(const ECC_field& b) const {
        return EllipticCurveGF(ECC_field::div(b, _Curve::P, _Curve::R2));
    }

    inline EllipticCurveGF pow(const ECC_field& exp) const {
        return EllipticCurveGF(ECC_field::pow(exp, _Curve::P, _Curve::R2));
    }
};

template<typename _Curve>
struct ECC_curve
{
    EllipticCurveGF<_Curve> x, y;
    bool inf;
};

struct secp256r1Curve
{
    static inline const EllipticCurveGF<secp256r1Curve> P = {{ 
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0xFFFFFFFF 
    }};
    static inline const EllipticCurveGF<secp256r1Curve> R2 = ECC_field::ComputeR2({{ 
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0xFFFFFFFF 
    }});
    static inline const EllipticCurveGF<secp256r1Curve> A = {{ 
        0xFFFFFFFC, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0xFFFFFFFF 
    }};
    static inline const EllipticCurveGF<secp256r1Curve> B = {{
        0x27D2604B, 0x3BCE3C3E, 0xCC53B0F6, 0x651D06B0, 0x769886BC, 0xB3EBBD55, 0xAA3A93E7, 0x5AC635D8
    }};
    static inline const ECC_curve<secp256r1Curve> G = { 
        {{ 0xD898C296, 0xF4A13945, 0x2DEB33A0, 0x77037D81, 0x63A440F2, 0xF8BCE6E5, 0xE12C4247, 0x6B17D1F2 }},
        {{ 0x37BF51F5, 0xCBB64068, 0x6B315ECE, 0x2BCE3357, 0x7C0F9E16, 0x8EE7EB4A, 0xFE1A7F9B, 0x4FE342E2 }},
        false 
    };
};

struct secp256k1Curve
{
    static inline const EllipticCurveGF<secp256r1Curve> P = {{
        0xFFFFFC2F, 0xFFFFFFFE, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
    }};
    static inline const EllipticCurveGF<secp256r1Curve> R2 = ECC_field::ComputeR2(
    {{
        0xFFFFFC2F, 0xFFFFFFFE, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
    }});
    static inline const EllipticCurveGF<secp256r1Curve> A = {{
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
    }};
    static inline const EllipticCurveGF<secp256r1Curve> B = {{
        0x00000007, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
    }};
    static inline const ECC_curve<secp256k1Curve> G = { 
        {{ 0x16F81798, 0x59F2815B, 0x2DCE28D9, 0x029BFCDB, 0xCE870B07, 0x55A06295, 0xF9DCBBAC, 0x79BE667E }},
        {{ 0xFB10D4B8, 0x9C47D08F, 0xA6855419, 0xFD17B448, 0x0E1108A8, 0x5DA4FBFC, 0x26A3C465, 0x483ADA77 }},
        false 
    };
};

struct ECC_public_key {
    ECC_field x, y;
};

struct ECC_key {
    char k[ECC_field::_ByteSize];
};

FB_BASE_API bool ecc_p256_new(ECC_key& o);

// R = 2P
// λ = (3x² + a) / (2y)
// x' = λ² - 2x
// y' = λ(x - x') - y
template<typename _Curve>
ECC_curve<_Curve> secp256_point_double(const ECC_curve<_Curve>& P)
{
    if (P.inf || P.y.is_zero())
        return { {{0}}, {{0}}, true };

    EllipticCurveGF num = ((P.x * P.x) * _THREE) + _Curve::A;
    EllipticCurveGF den = P.y * _TWO;

    EllipticCurveGF lambda = num / den;

    EllipticCurveGF x3 = (lambda * lambda) - P.x - P.x;
    EllipticCurveGF y3 = (lambda * (P.x - x3)) - P.y;

    ECC_curve<_Curve> R {};
    R.x = x3;
    R.y = y3;
    R.inf = false;
    return R;
}

// R = P + Q
template<typename _Curve>
typename ECC_curve<_Curve> secp256_point_add(const ECC_curve<_Curve>& P, const ECC_curve<_Curve>& Q)
{
    if (P.inf) return Q;
    if (Q.inf) return P;

    // P + (-Q) = O
    if ((P.x.eq(Q.x)) && P.y.eq(-Q.y))
        return { {{0}}, {{0}}, true };

    if ((P.x.eq(Q.x)) && P.y.eq(Q.y))
        return secp256_point_double<_Curve>(P);

    // λ = (y2 - y1) / (x2 - x1)
    EllipticCurveGF lambda = (Q.y - P.y) / (Q.x - P.x);
    EllipticCurveGF x3 = lambda * lambda;
    
    x3 = (x3 - P.x) - Q.x;
    EllipticCurveGF y3 = P.x - x3;
    y3 = (lambda * y3) - P.y;

    ECC_curve<_Curve> R;
    R.x = x3;
    R.y = y3;
    R.inf = false;
    return R;
}

template<typename _Curve>
void ecc_p256_mul(ECC_field& x, ECC_field& y, const ECC_key& k, const ECC_curve<_Curve>& G = _Curve::G) {
    ECC_curve<_Curve> Q = { {{0}}, {{0}}, true };

    for (int i = 0; i < 32; ++i) {
        for (int bit = 7; bit >= 0; --bit) {
            Q = secp256_point_double<_Curve>(Q);
            if ((k.k[i] >> bit) & 1)
                Q = secp256_point_add<_Curve>(Q, G);
        }
    }

    x = Q.x;
    y = Q.y;
}

// y^2 == rhs ? 
template<typename _Curve>
bool __ecc_sqrt_test(const EllipticCurveGF<_Curve>& y, const EllipticCurveGF<_Curve>& rhs)
{
    return rhs.eq(y*y);
}

// p ≡ 3 (mod 4):  y = rhs^((p+1)/4)  mod p
template<typename _Curve>
EllipticCurveGF<_Curve> __ecc_sqrt_mod_p3mod4(const EllipticCurveGF<_Curve>& rhs)
{
    EllipticCurveGF<_Curve> e = (_Curve::P + _ONE) / _FOUR;
    return rhs.pow(e);
}

// X + parity → (x,y)
template<typename _Curve>
bool ecc_p256_decode(EllipticCurveGF<_Curve>& y, const EllipticCurveGF<_Curve>& x, bool y_odd) {
    // rhs = x^3 + a*x + b
    EllipticCurveGF x3  = (x * x) * x;
    EllipticCurveGF rhs = x3 + _Curve::A*x + _Curve::B;

    y = __ecc_sqrt_mod_p3mod4<_Curve>(rhs);
    if (!__ecc_sqrt_test<_Curve>(y, rhs))
        return false; // x nincs a görbén

    // y párossága: legalsó bit (limb0 LSB), ha LE limb és „integer” értelemben
    bool odd = (y.d[0] & 1u) != 0;
    if (odd != y_odd)
        y = _Curve::P - y; // -y mod p

    return true;
}