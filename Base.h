/*
 * syn
 * Copyright (c) 2013-2017, Joshua Scoggins and Contributors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef _SYN_BASE_H
#define _SYN_BASE_H
#define INDIRECTOR(a, ...) PRIMITIVE_INDIRECTOR(a, __VA_ARGS__)
#define PRIMITIVE_INDIRECTOR(a, ...) a ## __VA_ARGS__
#include "BaseTypes.h"
#include "Problem.h"
#include <memory>

namespace syn {

    namespace FieldData {
        constexpr uint8 fields[8] = {
            0,
            8,
            16,
            24,
            32,
            40,
            48,
            56
        };

        template<typename T, int32 index>
        constexpr auto Value = static_cast<T>(0);

        template<typename T, int32 index>
        constexpr auto FieldIndex = static_cast<T>(fields[index]);
    } // end namespace FieldData


#define DefFieldData(type, index, mask) \
    namespace FieldData { \
        template<> constexpr auto Value<type, index>  = static_cast<type>(mask); \
    }

#define DefSignedAndUnsignedFieldData(type, index, mask) \
    DefFieldData(type, index, mask) \
    DefFieldData( u ## type, index, mask)

    DefSignedAndUnsignedFieldData(int16, 0, 0x00FF);
    DefSignedAndUnsignedFieldData(int16, 1, 0xFF00);

	DefSignedAndUnsignedFieldData(int32, 0, 0x000000FF);
	DefSignedAndUnsignedFieldData(int32, 1, 0x0000FF00);
	DefSignedAndUnsignedFieldData(int32, 2, 0x00FF0000);
	DefSignedAndUnsignedFieldData(int32, 3, 0xFF000000);

	DefSignedAndUnsignedFieldData(int64,  0, 0x00000000000000FF);
	DefSignedAndUnsignedFieldData(int64,  1, 0x000000000000FF00);
	DefSignedAndUnsignedFieldData(int64,  2, 0x0000000000FF0000);
	DefSignedAndUnsignedFieldData(int64,  3, 0x00000000FF000000);
	DefSignedAndUnsignedFieldData(int64,  4, 0x000000FF00000000);
	DefSignedAndUnsignedFieldData(int64,  5, 0x0000FF0000000000);
	DefSignedAndUnsignedFieldData(int64,  6, 0x00FF000000000000);
	DefSignedAndUnsignedFieldData(int64,  7, 0xFF00000000000000);

#undef DefSignedAndUnsignedFieldData
#undef DefFieldData

    namespace UpperLowerPair {
        template<typename T>
        struct TypeData {
            TypeData() = delete;
            TypeData(const TypeData&) = delete;
            TypeData(TypeData&&) = delete;
            ~TypeData() = delete;
        };
        template<typename T>
        constexpr auto upperMask = static_cast<T>(0);
        template<typename T>
        constexpr auto lowerMask = static_cast<T>(0);
        template<typename T>
        constexpr auto shiftCount = static_cast<T>(0);
    } // end namespace UpperLowerPair
    #define DefUpperLowerPair(type, halfType, up, low, shift) \
    namespace UpperLowerPair { \
        template<> struct TypeData<type> { \
            TypeData() = delete; \
            TypeData(const TypeData&) = delete; \
            TypeData(TypeData&&) = delete; \
            ~TypeData() = delete; \
            using HalfType = halfType; \
            using DataType = type; \
        }; \
        template<> constexpr auto upperMask<type> = static_cast< type > ( up ); \
        template<> constexpr auto lowerMask<type> = static_cast< type > ( low ) ; \
        template<> constexpr auto shiftCount<type> = static_cast< type > ( shift ); \
    }

    DefUpperLowerPair(uint8, uint8, 0xF0, 0x0F, 4);
    DefUpperLowerPair(int8, int8, 0xF0, 0x0F, 4);
    DefUpperLowerPair(uint16, byte, 0xFF00, 0x00FF, 8);
    DefUpperLowerPair(int16, int8, 0xFF00, 0x00FF, 8);
    DefUpperLowerPair(uint32, uint16, 0xFFFF0000, 0x0000FFFF, 16);
    DefUpperLowerPair(int32, int16, 0xFFFF0000, 0x0000FFFF, 16);
    DefUpperLowerPair(int64, int32, 0xFFFFFFFF00000000, 0x00000000FFFFFFFF, 32);
    DefUpperLowerPair(uint64, uint32, 0xFFFFFFFF00000000, 0x00000000FFFFFFFF, 32);
#undef DefUpperLowerPair


template<typename T>
using HalfType = typename UpperLowerPair::TypeData<T>::HalfType;

template<typename T>
using QuarterType = HalfType<HalfType<T>>;

template<typename T>
using EighthType = HalfType<QuarterType<T>>;

template<typename T>
constexpr T getUpperMask() noexcept {
    return UpperLowerPair::upperMask<T>;
}
template<typename T>
constexpr T getLowerMask() noexcept {
    return UpperLowerPair::lowerMask<T>;
}

template<typename T>
constexpr T getShiftCount() noexcept {
    return UpperLowerPair::shiftCount<T>;
}



template<typename T, T index>
constexpr auto singleBitmaskValue = static_cast<T>(0x1 << index);

template<typename T>
constexpr T computeSingleBitmask(T index) noexcept {
	return static_cast<T>(1 << index);
}

template<typename T, T bitmask>
constexpr T mask(T input) noexcept {
    return input & bitmask;
}

template<> constexpr uint8 mask<uint8, 0xFF>(uint8 value) noexcept { return value; }
template<> constexpr uint8 mask<uint8, 0>(uint8 value) noexcept { return 0; }

template<> constexpr uint16 mask<uint16, 0xFFFF>(uint16 value) noexcept { return value; }
template<> constexpr uint16 mask<uint16, 0>(uint16 value) noexcept { return 0; }

template<> constexpr uint32 mask<uint32, 0xFFFFFFFF>(uint32 value) noexcept { return value; }
template<> constexpr uint32 mask<uint32, 0>(uint32 value) noexcept { return 0; }

template<> constexpr uint64 mask<uint64, 0xFFFFFFFFFFFFFFFF>(uint64 value) noexcept { return value; }
template<> constexpr uint64 mask<uint64, 0>(uint64 value) noexcept { return 0; }

template<typename T, typename F, T bitmask, T shiftcount>
constexpr F decodeBits(T input) noexcept {
    auto result = mask<T, bitmask>(input);
    if (shiftcount != 0) {
        result >>= shiftcount;
    }
    return static_cast<F>(result);
}

template<> constexpr uint8 decodeBits<uint8, uint8, 0xFF, 0>(uint8 input) noexcept { return input; }
template<> constexpr uint8 decodeBits<uint8, uint8, 0, 0>(uint8 input) noexcept { return 0; }
template<> constexpr uint16 decodeBits<uint16, uint16, 0xFFFF, 0>(uint16 input) noexcept { return input; }
template<> constexpr uint16 decodeBits<uint16, uint16, 0, 0>(uint16 input) noexcept { return 0; }
template<> constexpr uint32 decodeBits<uint32, uint32, 0xFFFFFFFF, 0>(uint32 input) noexcept { return input; }
template<> constexpr uint32 decodeBits<uint32, uint32, 0, 0>(uint32 input) noexcept { return 0; }
template<> constexpr uint64 decodeBits<uint64, uint64, 0xFFFFFFFFFFFFFFFF, 0>(uint64 input) noexcept { return input; }
template<> constexpr uint64 decodeBits<uint64, uint64, 0, 0>(uint64 input) noexcept { return 0; }

template<typename T, typename F, int field>
constexpr F decodeField(T input) noexcept {
	return decodeBits<T, F, FieldData::Value<T, field>, FieldData::FieldIndex<T, field>>(input);
}

template<typename T, T mask>
constexpr bool decodeFlag(T input) noexcept {
	return decodeBits<T, bool, mask, static_cast<T>(0)>(input);
}

template<typename T, typename F, T bitmask, T shiftcount>
constexpr T encodeBits(T input, F value) noexcept {
    auto valueToInject = static_cast<T>(value);
    auto maskedValue = input & ~bitmask;
    if (shiftcount != 0) {
        valueToInject <<= shiftcount;
    }
    valueToInject &= bitmask;
    return static_cast<T>(maskedValue | valueToInject);
}
template<> constexpr uint8 encodeBits<uint8, uint8, 0xFF, 0>(uint8 input, uint8 value) noexcept { return value; }
template<> constexpr uint16 encodeBits<uint16, uint16, 0xFFFF, 0>(uint16 input, uint16 value) noexcept { return value; }
template<> constexpr uint32 encodeBits<uint32, uint32, 0xFFFFFFFF, 0>(uint32 input, uint32 value) noexcept { return value; }
template<> constexpr uint64 encodeBits<uint64, uint64, 0xFFFFFFFFFFFFFFFF, 0>(uint64 input, uint64 value) noexcept { return value; }
template<> constexpr uint8 encodeBits<uint8, uint8, 0, 0>(uint8 input, uint8 value) noexcept { return input; }
template<> constexpr uint16 encodeBits<uint16, uint16, 0, 0>(uint16 input, uint16 value) noexcept { return input; }
template<> constexpr uint32 encodeBits<uint32, uint32, 0, 0>(uint32 input, uint32 value) noexcept { return input; }
template<> constexpr uint64 encodeBits<uint64, uint64, 0, 0>(uint64 input, uint64 value) noexcept { return input; }

template<typename T, T mask, T shift>
constexpr T encodeFlag(T input, bool value) noexcept {
	return encodeBits<T, bool, mask, shift>(input, value);
}

template<typename T, typename F, int field>
constexpr T encodeField(T input, F value) noexcept {
	return encodeBits<T, F, FieldData::Value<T, field>, FieldData::FieldIndex<T, field>>(input, value);
}

template<typename T>
constexpr T encodeValueLE(HalfType<T> lower, HalfType<T> upper) noexcept {
    // this will break on int4 and such ;)
    return encodeBits<T, HalfType<T>, getUpperMask<T>(), getShiftCount<T>()>( encodeBits<T, HalfType<T>, getLowerMask<T>(), 0>(0, lower), upper);
}
template<typename T>
constexpr T encodeValueLE(QuarterType<T> lowest, QuarterType<T> upperLower, QuarterType<T> lowerUpper, QuarterType<T> upperMost) noexcept {
    return encodeValueLE<T>(encodeValueLE<HalfType<T>>(lowest, upperLower), encodeValueLE<HalfType<T>>(lowerUpper, upperMost));
}

template<typename T>
constexpr T encodeValueLE(EighthType<T> a, EighthType<T> b, EighthType<T> c, EighthType<T> d, EighthType<T> e, EighthType<T> f, EighthType<T> g, EighthType<T> h) noexcept {
    return encodeValueLE<T>(
            encodeValueLE<QuarterType<T>>(a, b),
            encodeValueLE<QuarterType<T>>(c, d),
            encodeValueLE<QuarterType<T>>(e, f),
            encodeValueLE<QuarterType<T>>(g, h));
}

constexpr uint16 encodeUint16LE(byte a, byte b) noexcept {
    return encodeValueLE<uint16>(a, b);
}
constexpr int16 encodeInt16LE(byte a, byte b) noexcept {
    return encodeValueLE<int16>(a, b);
}
constexpr uint32 encodeUint32LE(byte a, byte b, byte c, byte d)  noexcept {
    return encodeValueLE<uint32>(a, b, c, d);
}
constexpr uint16 encodeUint16LE(byte* buf)  noexcept {
	return encodeUint16LE(buf[0], buf[1]);
}
constexpr uint32 encodeUint32LE(byte* buf)  noexcept {
	return encodeUint32LE(buf[0], buf[1], buf[2], buf[3]);
}
constexpr int32 encodeInt32LE(byte lowest, byte upperLower, byte lowerUpper, byte upperMost)  noexcept {
    return encodeValueLE<int32>(lowest, upperLower, lowerUpper, upperMost);
}

constexpr uint32 encodeUint32LE(uint16 lower, uint16 upper) noexcept {
    return encodeValueLE<uint32>(lower, upper);
}

constexpr int32 encodeInt32LE(int16 lower, int16 upper) noexcept {
    return encodeValueLE<int32>(lower, upper);
}

template<typename T>
inline void decode2byteQuantityLE(T value, byte storage[sizeof(T)], int offset = 0) noexcept {
	static_assert(sizeof(T) >= 2, "Provided type is too small to decode a 2 byte quantity into!");
	using Number = T;
	using Field = byte;
	storage[offset + 0] = decodeField<Number, Field, 0>(value);
	storage[offset + 1] = decodeField<Number, Field, 1>(value);
}

template<typename T>
inline void decode4byteQuantityLE(T value, byte storage[sizeof(T)], int offset = 0) noexcept {
	static_assert(sizeof(T) >= 4, "Provided type is too small to decode a 4 byte quantity into!");
	using Number = T;
	using Field = byte;
	storage[offset + 0] = decodeField<Number, Field, 0>(value);
	storage[offset + 1] = decodeField<Number, Field, 1>(value);
	storage[offset + 2] = decodeField<Number, Field, 2>(value);
	storage[offset + 3] = decodeField<Number, Field, 3>(value);
}

template<typename T>
inline void decode8byteQuantityLE(T value, byte storage[sizeof(T)], int offset = 0) noexcept {
	static_assert(sizeof(T) >= 8, "Provided type is too small to decode an 8 byte quantity into!");
	using Number = T;
	using Field = byte;
	storage[offset+0] = decodeField<Number, Field, 0>(value);
	storage[offset+1] = decodeField<Number, Field, 1>(value);
	storage[offset+2] = decodeField<Number, Field, 2>(value);
	storage[offset+3] = decodeField<Number, Field, 3>(value);
	storage[offset+4] = decodeField<Number, Field, 4>(value);
	storage[offset+5] = decodeField<Number, Field, 5>(value);
	storage[offset+6] = decodeField<Number, Field, 6>(value);
	storage[offset+7] = decodeField<Number, Field, 7>(value);
}
inline void decodeUint32LE(uint32 value, byte storage[sizeof(uint32)], int offset = 0) noexcept {
	decode4byteQuantityLE<uint32>(value, storage, offset);
}

inline void decodeUint16LE(uint16 value, byte storage[sizeof(uint16)], int offset = 0) noexcept {
	decode2byteQuantityLE<uint16>(value, storage, offset);
}
inline void decodeInt32LE(int32 value, byte storage[sizeof(int32)], int offset = 0) noexcept {
	decode4byteQuantityLE<int32>(value, storage, offset);

}

inline void decodeInt16LE(int16 value, byte storage[sizeof(int16)], int offset = 0) noexcept {
	decode2byteQuantityLE<int16>(value, storage, offset);
}
inline void decodeUint64LE(uint64 value, byte storage[sizeof(uint64)], int offset = 0) noexcept {
	decode8byteQuantityLE<uint64>(value, storage, offset);
}

inline void decodeInt64LE(int64 value, byte storage[sizeof(int64)], int offset = 0) noexcept {
	decode8byteQuantityLE<int64>(value, storage, offset);
}


constexpr uint64 encodeUint64LE(uint32 lower, uint32 upper) noexcept {
    return encodeValueLE<uint64>(lower, upper);
}
constexpr uint64 encodeUint64LE(byte a, byte b, byte c, byte d, byte e, byte f, byte g, byte h) noexcept {
    return encodeValueLE<uint64>(a, b, c, d, e, f, g, h);
}

constexpr uint64 encodeUint64LE(uint16 a, uint16 b, uint16 c, uint16 d) noexcept {
    return encodeValueLE<uint64>(a, b, c, d);

}

constexpr uint64 encodeUint64LE(byte* buf) noexcept {
	return encodeUint64LE(buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
}

constexpr int64 encodeInt64LE(uint32 lower, uint32 upper) noexcept {
    return encodeValueLE<int64>(lower, upper);
}
constexpr int64 encodeInt64LE(byte a, byte b, byte c, byte d, byte e, byte f, byte g, byte h) noexcept {
    return encodeValueLE<int64>(a, b, c, d, e, f, g, h);
}

constexpr int64 encodeInt64LE(uint16 a, uint16 b, uint16 c, uint16 d) noexcept {
    return encodeValueLE<int64>(a, b, c, d);
}

constexpr int64 encodeInt64LE(byte* buf) noexcept {
	return encodeInt64LE(buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
}

template<typename T, typename F>
constexpr F decodeBits(T value, T mask, T shiftcount) noexcept {
	return static_cast<F>((value & mask) >> shiftcount);
}

template<typename T, typename F>
constexpr T encodeBits(T input, F value, T bitmask, T shiftcount) noexcept {
	return static_cast<T>((input & ~bitmask) | ((static_cast<T>(value) << shiftcount) & bitmask));
}


template<typename T, T index>
constexpr bool getBit(T value) noexcept {
    return decodeBits<T, bool, singleBitmaskValue<T, index>, index>(value);
}

template<typename T>
constexpr bool getBit(T value, T index) noexcept {
    return decodeBits<T, bool>(value, computeSingleBitmask<T>(index), index);
}

template<> constexpr bool getBit<byte, 0>(byte value) noexcept { return (singleBitmaskValue<byte, 0> & value) != 0; }
template<> constexpr bool getBit<byte, 1>(byte value) noexcept { return (singleBitmaskValue<byte, 1> & value) != 0; }
template<> constexpr bool getBit<byte, 2>(byte value) noexcept { return (singleBitmaskValue<byte, 2> & value) != 0; }
template<> constexpr bool getBit<byte, 3>(byte value) noexcept { return (singleBitmaskValue<byte, 3> & value) != 0; }

constexpr byte expandBit(bool value) noexcept {
    return value ? 0xFF : 0x00;
}

template<typename T, T index>
constexpr T setBit(T value, bool bit) noexcept {
    return syn::encodeBits<T, bool, singleBitmaskValue<T, index>, index>(value, bit);
}

template<typename T>
constexpr T setBit(T value, bool bit, T index) noexcept {
    return encodeBits<T, bool>(value, bit, computeSingleBitmask<T>(index), index);
}

constexpr uint32 expandUInt32LE(bool lowest, bool lowerUpper, bool upperLower, bool upperMost) noexcept {
    return encodeUint32LE(expandBit(lowest), expandBit(lowerUpper), expandBit(upperLower), expandBit(upperMost));
}

constexpr uint16 expandUInt16LE(bool lower, bool upper) noexcept {
    return encodeUint16LE(expandBit(lower), expandBit(upper));
}

constexpr uint64 expandUInt64LE(bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7) noexcept {
    return encodeUint64LE(expandUInt32LE(b0, b1, b2, b3), expandUInt32LE(b4, b5, b6, b7));
}

template<typename T>
constexpr typename UpperLowerPair::TypeData<T>::HalfType getUpperHalf(T value) noexcept {
    using InputType = T;
    using DataPair = UpperLowerPair::TypeData<T>;
    using OutputType = typename DataPair::HalfType;
    return syn::decodeBits<InputType, OutputType, UpperLowerPair::upperMask<T>, UpperLowerPair::shiftCount<T>>(value);
}
template<typename T>
constexpr typename UpperLowerPair::TypeData<T>::HalfType getLowerHalf(T value) noexcept {
    using InputType = T;
    using DataPair = UpperLowerPair::TypeData<T>;
    using OutputType = typename DataPair::HalfType;
    return syn::decodeBits<InputType, OutputType, UpperLowerPair::lowerMask<T>, 0>(value);
}
template<> constexpr byte getLowerHalf<uint16>(uint16 value) noexcept { return static_cast<byte>(value); }
template<> constexpr uint16 getLowerHalf<uint32>(uint32 value) noexcept { return static_cast<uint16>(value); }
template<> constexpr uint32 getLowerHalf<uint64>(uint64 value) noexcept { return static_cast<uint32>(value); }

template<typename T>
constexpr T setLowerHalf(T value, typename UpperLowerPair::TypeData<T>::HalfType lower) noexcept {
    using DataPair = UpperLowerPair::TypeData<T>;
    return syn::encodeBits<T, typename DataPair::HalfType, UpperLowerPair::lowerMask<T>, 0>(value, lower);
}
template<typename T>
constexpr T setUpperHalf(T value, typename UpperLowerPair::TypeData<T>::HalfType upper) noexcept {
    using DataPair = UpperLowerPair::TypeData<T>;
    return syn::encodeBits<T, typename DataPair::HalfType, UpperLowerPair::lowerMask<T>, UpperLowerPair::shiftCount<T>>(value, upper);
}


template<typename T>
inline void swap(T& a, T& b) {
    auto c = b;
    b = a;
    a = c;
}



template<typename T>
union BinaryContainer {
    T value;
    byte bytes[sizeof(T)];
};
using BinaryFloat = BinaryContainer<float>;
using BinaryDouble = BinaryContainer<double>;
using BinaryLongDouble = BinaryContainer<long double>;

template<typename T>
T getc() noexcept {
    byte value = 0;
    std::cin >> std::noskipws >> value;
    return static_cast<T>(value);
}

template<typename T>
void putc(T value) noexcept {
    std::cout << static_cast<char>(value);
}

template<typename T>
constexpr T defaultErrorState = T::Count;

template<typename T>
constexpr bool isErrorState(T op) noexcept {
    return op == defaultErrorState<T>;
}

template<typename T>
constexpr void throwOnErrorState(T value, const std::string& msg) noexcept {
    if (isErrorState<T>(value)) {
        throw syn::Problem(msg);
    }
}

constexpr bool fulfillsCondition(std::true_type) noexcept { return true; }
constexpr bool fulfillsCondition(std::false_type) noexcept { return false; }

template<bool fulfills>
struct ConditionFulfillment : std::integral_constant<bool, fulfills> { };


template<typename T>
constexpr bool fulfillsCondition() noexcept {
	static_assert(!std::is_integral<T>::value, "Provided type must not be an integral!");
	return fulfillsCondition(T());
}

}
#endif // end _SYN_BASE_H
