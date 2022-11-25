////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2022  Evan Bowman
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of version 2 of the GNU General Public License as published by the
// Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 51
// Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// GPL2 ONLY. No later versions permitted.
//
////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "int.h"
#include <ciso646> // For MSVC. What an inept excuse for a compiler.


#if defined(__GBA__) or defined(__NDS__)
template <typename T> using Atomic = T;
#else
#include <atomic>
template <typename T> using Atomic = std::atomic<T>;
#endif



inline u8 count_ones(u8 byte)
{
    static const u8 nibble_lut[16] = {
        0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

    return nibble_lut[byte & 0x0F] + nibble_lut[byte >> 4];
}



inline int count_ones(u64 x)
{
    int sum = 0;

    for (int i = 0; i < 8; ++i) {
        sum += count_ones(((u8*)&x)[i]);
    }

    return sum;
}



template <typename T> struct Vec3
{
    T x = 0;
    T y = 0;
    T z = 0;
};



template <typename T> struct Vec2
{
    T x = 0;
    T y = 0;

    template <typename U> Vec2<U> cast() const
    {
        // Note: We could have used a uniform initializer here to
        // prevent narrowing, but there are cases of float->int cast
        // where one might not worry too much about a narrowing
        // conversion.
        Vec2<U> result;
        result.x = x;
        result.y = y;
        return result;
    }
};



template <typename T, typename U = T> struct Rect
{
    T x_off = 0;
    T y_off = 0;
    U w = 0;
    U h = 0;
};


template <typename T> T abs(const T& val)
{
    return (val > 0) ? val : val * -1;
}


// When you don't need an exact value, this works as a fast distance
// approximation.
template <typename T> T manhattan_length(const Vec2<T>& a, const Vec2<T>& b)
{
    return abs(a.x - b.x) + abs(a.y - b.y);
}


using Degree = u16;
using Angle = Degree;



template <typename T> Vec2<T> operator+(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

template <typename T> Vec2<T> operator-(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

template <typename T> Vec2<T> operator*(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    return {lhs.x * rhs.x, lhs.y * rhs.y};
}

template <typename T> Vec2<T> operator/(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    return {lhs.x / rhs.x, lhs.y / rhs.y};
}

template <typename T> Vec2<T> operator+(const Vec2<T>& lhs, const T& rhs)
{
    return {lhs.x + rhs, lhs.y + rhs};
}

template <typename T> Vec2<T> operator-(const Vec2<T>& lhs, const T& rhs)
{
    return {lhs.x - rhs, lhs.y - rhs};
}

template <typename T> Vec2<T> operator*(const Vec2<T>& lhs, const T& rhs)
{
    return {lhs.x * rhs, lhs.y * rhs};
}

template <typename T> Vec2<T> operator*(const T& rhs, const Vec2<T>& lhs)
{
    return {lhs.x * rhs, lhs.y * rhs};
}

template <typename T> Vec2<T> operator/(const Vec2<T>& lhs, const T& rhs)
{
    return {lhs.x / rhs, lhs.y / rhs};
}

template <typename T> Vec2<T> operator/(const T& rhs, const Vec2<T>& lhs)
{
    return {lhs.x / rhs, lhs.y / rhs};
}

template <typename T> bool operator==(const Vec2<T>& rhs, const Vec2<T>& lhs)
{
    return lhs.x == rhs.x and lhs.y == rhs.y;
}

template <typename T> bool operator==(const Vec3<T>& rhs, const Vec3<T>& lhs)
{
    return lhs.x == rhs.x and lhs.y == rhs.y and lhs.z == rhs.z;
}

template <typename T>
bool operator not_eq(const Vec2<T>& rhs, const Vec2<T>& lhs)
{
    return lhs.x not_eq rhs.x or lhs.y not_eq rhs.y;
}

template <typename T> T clamp(T x, T floor, T ceil)
{
    if (x < floor) {
        return floor;
    } else if (x > ceil) {
        return ceil;
    } else {
        return x;
    }
}



inline u8 fast_interpolate(u8 a, u8 b, u8 t)
{
    return b + (u8)(((u16)(a - b) * t) >> 8);
}


using Microseconds = s32; // Therefore, a maximum of ~2147.5 seconds will fit in
                          // this data type.


constexpr Microseconds seconds(u32 count)
{
    return count * 1000000;
}


constexpr Microseconds minutes(u8 count)
{
    return count * seconds(60);
}


constexpr Microseconds milliseconds(u32 count)
{
    return count * 1000;
}



enum class Cardinal : u8 { north, south, west, east };



inline s32 parse_int(const char* str, u32 len)
{
    s32 n = 0;
    for (u32 i = 0; i < len; ++i) {
        n = n * 10 + (str[i] - '0');
    }
    return n;
}
