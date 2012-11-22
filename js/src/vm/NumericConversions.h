/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sw=4 et tw=78:
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NumericConversions_h___
#define NumericConversions_h___

#include "mozilla/FloatingPoint.h"

#include <math.h>

/* A NaN whose bit pattern conforms to JS::Value's bit pattern restrictions. */
extern double js_NaN;

namespace js {

namespace detail {

union DoublePun {
    struct {
#if defined(IS_LITTLE_ENDIAN) && !defined(FPU_IS_ARM_FPA)
        uint32_t lo, hi;
#else
        uint32_t hi, lo;
#endif
    } s;
    uint64_t u64;
    double d;
};

} /* namespace detail */

/* Numeric Conversion base. Round doubles to Ints according to ECMA or WEBIDL standards. */
template<size_t width, typename ResultType>
inline ResultType
ToIntWidth(double d)
{
#if defined(__i386__) || defined(__i386) || defined(__x86_64__) || \
    defined(_M_IX86) || defined(_M_X64)
    detail::DoublePun du, duh, twoWidth;
    uint32_t di_h, u_tmp, expon, shift_amount;
    int32_t mask32;

    /*
     * Algorithm Outline
     *  Step 1. If d is NaN, +/-Inf or |d|>=2^(width + 52) or |d|<1, then return 0
     *          All of this is implemented based on an exponent comparison,
     *          since anything with a higher exponent is either not finite, or
     *          going to round to 0..
     *  Step 2. If |d|<2^(width - 1), then return (int)d
     *          The cast to integer (conversion in RZ mode) returns the correct result.
     *  Step 3. If |d|>=2^width, d:=fmod(d, 2^width) is taken  -- but without a call
     *  Step 4. If |d|>=2^(width - 1), then the fractional bits are cleared before
     *          applying the correction by 2^width:  d - sign(d)*2^width
     *  Step 5. Return (int)d
     */

    du.d = d;
    di_h = du.s.hi;

    u_tmp = (di_h & 0x7ff00000) - 0x3ff00000;
    if (u_tmp >= ((width + 52) << 20)) {
        // d is Nan, +/-Inf or +/-0, or |d|>=2^(width+52) or |d|<1, in which case result=0
        // If we need to shift by more than (width + 52), there are no data bits
        // to preserve, and the mod will turn out 0.
        return 0;
    }

    if (u_tmp < ((width - 1) << 20)) {
        // |d|<2^(width - 1)
        return ResultType(d);
    }

    if (u_tmp > ((width - 1) << 20)) {
        // |d|>=2^width
        // Throw away multiples of 2^width.
        //
        // That is, compute du.d = the value in (-2^width, 2^width)
        // that has the same sign as d and is equal to d modulo 2^width.
        //
        // This can't be done simply by masking away bits of du because
        // the implicit one-bit of the mantissa is one of the ones we want to
        // eliminate. So instead we compute duh.d = the appropriate multiple
        // of 2^width, which *can* be computed by masking, and then we
        // subtract that from du.d.
        expon = u_tmp >> 20;
        shift_amount = expon - (width - 11);
        mask32 = 0x80000000;
        if (shift_amount < 32) {
            // Shift only affects top word.
            mask32 >>= shift_amount;
            duh.s.hi = du.s.hi & mask32;
            duh.s.lo = 0;
        } else {
            // Top word all 1s, shift affects bottom word.
            mask32 >>= (shift_amount-32);
            duh.s.hi = du.s.hi;
            duh.s.lo = du.s.lo & mask32;
        }
        du.d -= duh.d;
    }

    di_h = du.s.hi;

    // Eliminate fractional bits
    u_tmp = (di_h & 0x7ff00000);
    if (u_tmp >= (0x3ff00000 + ((width - 1) << 20))) {
        // |d|>=2^(width - 1)
        expon = u_tmp >> 20;

        // Same idea as before, except save everything non-fractional.
        shift_amount = expon - (0x3ff - 11);
        mask32 = 0x80000000;
        if (shift_amount < 32) {
            // Top word only
            mask32 >>= shift_amount;
            du.s.hi &= mask32;
            du.s.lo = 0;
        } else {
            // Bottom word. Top word all 1s.
            mask32 >>= (shift_amount-32);
            du.s.lo &= mask32;
        }
        // Apply step 4's 2^width correction.
        twoWidth.s.hi = (0x3ff00000 + (width << 20)) ^ (du.s.hi & 0x80000000);
        twoWidth.s.lo = 0;
        du.d -= twoWidth.d;
    }

    return ResultType(du.d);
#else
    double twoWidth, twoWidthMin1;

    if (!MOZ_DOUBLE_IS_FINITE(d))
        return 0;

    /* FIXME: This relies on undefined behavior; see bug 667739. */
    ResultType i = (ResultType) d;
    if ((double) i == d)
        return ResultType(i);

    twoWidth = width == 32 ? 4294967296.0 : 18446744073709551616.0;
    twoWidthMin1 = width == 32 ? 2147483648.0 : 9223372036854775808.0;
    d = fmod(d, twoWidth);
    d = (d >= 0) ? floor(d) : ceil(d) + twoWidth;
    return (ResultType) (d >= twoWidthMin1 ? d - twoWidth : d);
#endif
}




template <class Dest, class Source>
struct BitCastHelper {
  inline static Dest cast(const Source& source) {
    Dest dest;
    memcpy(&dest, &source, sizeof(dest));
    return dest;
  }
};

template <class Dest, class Source>
struct BitCastHelper<Dest, Source*> {
  inline static Dest cast(Source* source) {
    return BitCastHelper<Dest, uintptr_t>::
        cast(reinterpret_cast<uintptr_t>(source));
  }
};

template <class Dest, class Source>
inline Dest BitCast(const Source& source);

template <class Dest, class Source>
inline Dest BitCast(const Source& source) {
  return BitCastHelper<Dest, Source>::cast(source);
}




// This "Do It Yourself Floating Point" class implements a floating-point number
// with a uint64 significand and an int exponent. Normalized DiyFp numbers will
// have the most significant bit of the significand set.
// Multiplication and Subtraction do not normalize their results.
// DiyFp are not designed to contain special doubles (NaN and Infinity).
class DiyFp {
 public:
  static const int kSignificandSize = 64;

  DiyFp() : f_(0), e_(0) {}
  DiyFp(uint64_t f, int e) : f_(f), e_(e) {}

  // this = this - other.
  // The exponents of both numbers must be the same and the significand of this
  // must be bigger than the significand of other.
  // The result will not be normalized.
  void Subtract(const DiyFp& other) {
    f_ -= other.f_;
  }

  // Returns a - b.
  // The exponents of both numbers must be the same and this must be bigger
  // than other. The result will not be normalized.
  static DiyFp Minus(const DiyFp& a, const DiyFp& b) {
    DiyFp result = a;
    result.Subtract(b);
    return result;
  }


  // this = this * other.
  void Multiply(const DiyFp& other);

  // returns a * b;
  static DiyFp Times(const DiyFp& a, const DiyFp& b) {
    DiyFp result = a;
    result.Multiply(b);
    return result;
  }

  void Normalize() {
    uint64_t f = f_;
    int e = e_;

    // This method is mainly called for normalizing boundaries. In general
    // boundaries need to be shifted by 10 bits. We thus optimize for this case.
    const uint64_t k10MSBits = static_cast<uint64_t>(0x3FF) << 54;
    while ((f & k10MSBits) == 0) {
      f <<= 10;
      e -= 10;
    }
    while ((f & kUint64MSB) == 0) {
      f <<= 1;
      e--;
    }
    f_ = f;
    e_ = e;
  }

  static DiyFp Normalize(const DiyFp& a) {
    DiyFp result = a;
    result.Normalize();
    return result;
  }

  uint64_t f() const { return f_; }
  int e() const { return e_; }

  void set_f(uint64_t new_value) { f_ = new_value; }
  void set_e(int new_value) { e_ = new_value; }

 private:
  static const uint64_t kUint64MSB = static_cast<uint64_t>(1) << 63;

  uint64_t f_;
  int e_;
};









// We assume that doubles and uint64_t have the same endianness.
inline uint64_t double_to_uint64(double d) { return BitCast<uint64_t>(d); }
inline double uint64_to_double(uint64_t d64) { return BitCast<double>(d64); }

#define V8_2PART_UINT64_C(a, b) (((static_cast<uint64_t>(a) << 32) + 0x##b##u))

// Helper functions for doubles.
class Double {
 public:
  static const uint64_t kSignMask = V8_2PART_UINT64_C(0x80000000, 00000000);
  static const uint64_t kExponentMask = V8_2PART_UINT64_C(0x7FF00000, 00000000);
  static const uint64_t kSignificandMask =
      V8_2PART_UINT64_C(0x000FFFFF, FFFFFFFF);
  static const uint64_t kHiddenBit = V8_2PART_UINT64_C(0x00100000, 00000000);
  static const int kPhysicalSignificandSize = 52;  // Excludes the hidden bit.
  static const int kSignificandSize = 53;

  Double() : d64_(0) {}
  explicit Double(double d) : d64_(double_to_uint64(d)) {}
  explicit Double(uint64_t d64) : d64_(d64) {}
  explicit Double(DiyFp diy_fp)
    : d64_(DiyFpToUint64(diy_fp)) {}

  // The value encoded by this Double must be greater or equal to +0.0.
  // It must not be special (infinity, or NaN).
  DiyFp AsDiyFp() const {
    return DiyFp(Significand(), Exponent());
  }

  // The value encoded by this Double must be strictly greater than 0.
  DiyFp AsNormalizedDiyFp() const {
    uint64_t f = Significand();
    int e = Exponent();

    // The current double could be a denormal.
    while ((f & kHiddenBit) == 0) {
      f <<= 1;
      e--;
    }
    // Do the final shifts in one go.
    f <<= DiyFp::kSignificandSize - kSignificandSize;
    e -= DiyFp::kSignificandSize - kSignificandSize;
    return DiyFp(f, e);
  }

  // Returns the double's bit as uint64.
  uint64_t AsUint64() const {
    return d64_;
  }

  // Returns the next greater double. Returns +infinity on input +infinity.
  double NextDouble() const {
    if (d64_ == kInfinity) return Double(kInfinity).value();
    if (Sign() < 0 && Significand() == 0) {
      // -0.0
      return 0.0;
    }
    if (Sign() < 0) {
      return Double(d64_ - 1).value();
    } else {
      return Double(d64_ + 1).value();
    }
  }

  int Exponent() const {
    if (IsDenormal()) return kDenormalExponent;

    uint64_t d64 = AsUint64();
    int biased_e =
        static_cast<int>((d64 & kExponentMask) >> kPhysicalSignificandSize);
    return biased_e - kExponentBias;
  }

  uint64_t Significand() const {
    uint64_t d64 = AsUint64();
    uint64_t significand = d64 & kSignificandMask;
    if (!IsDenormal()) {
      return significand + kHiddenBit;
    } else {
      return significand;
    }
  }

  // Returns true if the double is a denormal.
  bool IsDenormal() const {
    uint64_t d64 = AsUint64();
    return (d64 & kExponentMask) == 0;
  }

  // We consider denormals not to be special.
  // Hence only Infinity and NaN are special.
  bool IsSpecial() const {
    uint64_t d64 = AsUint64();
    return (d64 & kExponentMask) == kExponentMask;
  }

  bool IsInfinite() const {
    uint64_t d64 = AsUint64();
    return ((d64 & kExponentMask) == kExponentMask) &&
        ((d64 & kSignificandMask) == 0);
  }

  int Sign() const {
    uint64_t d64 = AsUint64();
    return (d64 & kSignMask) == 0? 1: -1;
  }

  // Precondition: the value encoded by this Double must be greater or equal
  // than +0.0.
  DiyFp UpperBoundary() const {
    return DiyFp(Significand() * 2 + 1, Exponent() - 1);
  }

  // Returns the two boundaries of this.
  // The bigger boundary (m_plus) is normalized. The lower boundary has the same
  // exponent as m_plus.
  // Precondition: the value encoded by this Double must be greater than 0.
  void NormalizedBoundaries(DiyFp* out_m_minus, DiyFp* out_m_plus) const {
    DiyFp v = this->AsDiyFp();
    bool significand_is_zero = (v.f() == kHiddenBit);
    DiyFp m_plus = DiyFp::Normalize(DiyFp((v.f() << 1) + 1, v.e() - 1));
    DiyFp m_minus;
    if (significand_is_zero && v.e() != kDenormalExponent) {
      // The boundary is closer. Think of v = 1000e10 and v- = 9999e9.
      // Then the boundary (== (v - v-)/2) is not just at a distance of 1e9 but
      // at a distance of 1e8.
      // The only exception is for the smallest normal: the largest denormal is
      // at the same distance as its successor.
      // Note: denormals have the same exponent as the smallest normals.
      m_minus = DiyFp((v.f() << 2) - 1, v.e() - 2);
    } else {
      m_minus = DiyFp((v.f() << 1) - 1, v.e() - 1);
    }
    m_minus.set_f(m_minus.f() << (m_minus.e() - m_plus.e()));
    m_minus.set_e(m_plus.e());
    *out_m_plus = m_plus;
    *out_m_minus = m_minus;
  }

  double value() const { return uint64_to_double(d64_); }

  // Returns the significand size for a given order of magnitude.
  // If v = f*2^e with 2^p-1 <= f <= 2^p then p+e is v's order of magnitude.
  // This function returns the number of significant binary digits v will have
  // once its encoded into a double. In almost all cases this is equal to
  // kSignificandSize. The only exception are denormals. They start with leading
  // zeroes and their effective significand-size is hence smaller.
  static int SignificandSizeForOrderOfMagnitude(int order) {
    if (order >= (kDenormalExponent + kSignificandSize)) {
      return kSignificandSize;
    }
    if (order <= kDenormalExponent) return 0;
    return order - kDenormalExponent;
  }

 private:
  static const int kExponentBias = 0x3FF + kPhysicalSignificandSize;
  static const int kDenormalExponent = -kExponentBias + 1;
  static const int kMaxExponent = 0x7FF - kExponentBias;
  static const uint64_t kInfinity = V8_2PART_UINT64_C(0x7FF00000, 00000000);

  const uint64_t d64_;

  static uint64_t DiyFpToUint64(DiyFp diy_fp) {
    uint64_t significand = diy_fp.f();
    int exponent = diy_fp.e();
    while (significand > kHiddenBit + kSignificandMask) {
      significand >>= 1;
      exponent++;
    }
    if (exponent >= kMaxExponent) {
      return kInfinity;
    }
    if (exponent < kDenormalExponent) {
      return 0;
    }
    while (exponent > kDenormalExponent && (significand & kHiddenBit) == 0) {
      significand <<= 1;
      exponent--;
    }
    uint64_t biased_exponent;
    if (exponent == kDenormalExponent && (significand & kHiddenBit) == 0) {
      biased_exponent = 0;
    } else {
      biased_exponent = static_cast<uint64_t>(exponent + kExponentBias);
    }
    return (significand & kSignificandMask) |
        (biased_exponent << kPhysicalSignificandSize);
  }
};


















// The fast double-to-(unsigned-)int conversion routine does not guarantee
// rounding towards zero.
// The result is unspecified if x is infinite or NaN, or if the rounded
// integer value is outside the range of type int.
inline int FastD2I(double x) {
  // The static_cast convertion from double to int used to be slow, but
  // as new benchmarks show, now it is much faster than lrint().
  return static_cast<int>(x);
}

inline double FastI2D(int x) {
  // There is no rounding involved in converting an integer to a
  // double, so this code should compile to a few instructions without
  // any FPU pipeline stalls.
  return static_cast<double>(x);
}

inline int32_t ToInt32(double x) {
  int32_t i = FastD2I(x);
  if (FastI2D(i) == x) return i;
  Double d(x);
  int exponent = d.Exponent();
  if (exponent < 0) {
    if (exponent <= -Double::kSignificandSize) return 0;
    return d.Sign() * static_cast<int32_t>(d.Significand() >> -exponent);
  } else {
    if (exponent > 31) return 0;
    return d.Sign() * static_cast<int32_t>(d.Significand() << exponent);
  }
}

/* ES5 9.6 (specialized for doubles). */
inline uint32_t
ToUint32(double d)
{
    return uint32_t(ToInt32(d));
}

/* WEBIDL 4.2.10 */
inline int64_t
ToInt64(double d)
{
    return ToIntWidth<64, int64_t>(d);
}

/* WEBIDL 4.2.11 */
inline uint64_t
ToUint64(double d)
{
    return uint64_t(ToInt64(d));
}

/* ES5 9.4 ToInteger (specialized for doubles). */
inline double
ToInteger(double d)
{
    if (d == 0)
        return d;

    if (!MOZ_DOUBLE_IS_FINITE(d)) {
        if (MOZ_DOUBLE_IS_NaN(d))
            return 0;
        return d;
    }

    bool neg = (d < 0);
    d = floor(neg ? -d : d);
    return neg ? -d : d;
}

} /* namespace js */

#endif /* NumericConversions_h__ */
