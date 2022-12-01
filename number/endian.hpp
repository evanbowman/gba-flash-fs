////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2022 Evan Bowman
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "numeric.hpp"
#include <bit>
#include <string.h>

namespace flash_filesystem
{

// Because most processors are little endian, I am using little endian byte
// order for binary encoded data.


#ifdef __bswap_constant_16
#undef __bswap_constant_16
#endif
#define __bswap_constant_16(x) ((u16)((((x) >> 8) & 0xff) | (((x)&0xff) << 8)))

#ifdef __bswap_constant_32
#undef __bswap_constant_32
#endif
#define __bswap_constant_32(x)                                                 \
    ((((x)&0xff000000) >> 24) | (((x)&0x00ff0000) >> 8) |                      \
     (((x)&0x0000ff00) << 8) | (((x)&0x000000ff) << 24))

#ifdef __bswap_constant_64
#undef __bswap_constant_64
#endif
#define __bswap_constant_64(x)                                                 \
    ((((x)&0xff00000000000000ull) >> 56) |                                     \
     (((x)&0x00ff000000000000ull) >> 40) |                                     \
     (((x)&0x0000ff0000000000ull) >> 24) |                                     \
     (((x)&0x000000ff00000000ull) >> 8) | (((x)&0x00000000ff000000ull) << 8) | \
     (((x)&0x0000000000ff0000ull) << 24) |                                     \
     (((x)&0x000000000000ff00ull) << 40) |                                     \
     (((x)&0x00000000000000ffull) << 56))


#if defined(__GBA__) or defined(__NDS__)
inline constexpr bool is_little_endian()
{
    return true;
}
#else
inline constexpr bool is_little_endian()
{
    return std::endian::native == std::endian::little;
}
#endif



template <typename T> T to_host_order(T value);


template <> inline u8 to_host_order(u8 val)
{
    return val;
}

template <> inline u16 to_host_order(u16 val)
{
    if (is_little_endian()) {
        return val;
    } else {
        return __bswap_constant_16(val);
    }
}

template <> inline s16 to_host_order(s16 val)
{
    if (is_little_endian()) {
        return val;
    } else {
        return __bswap_constant_16(val);
    }
}

template <> inline u32 to_host_order(u32 val)
{
    if (is_little_endian()) {
        return val;
    } else {
        return __bswap_constant_32(val);
    }
}

template <> inline s32 to_host_order(s32 val)
{
    if (is_little_endian()) {
        return val;
    } else {
        return __bswap_constant_32(val);
    }
}

template <> inline u64 to_host_order(u64 val)
{
    if (is_little_endian()) {
        return val;
    } else {
        return __bswap_constant_64(val);
    }
}

template <> inline s64 to_host_order(s64 val)
{
    if (is_little_endian()) {
        return val;
    } else {
        return __bswap_constant_64(val);
    }
}


template <typename T> class HostInteger
{
public:
    // NOTE: I could overload the cast operator and assignment operators to make
    // this class behave similarly to a plain integer. But then I get
    // class-memaccess errors, which I don't want to disable throughout the
    // project... calling get()/set() isn't so bad.

    HostInteger() = default;

    explicit HostInteger(T value)
    {
        set(value);
    }

    void set(T value)
    {
        value = to_host_order(value);
        memcpy(data_, &value, sizeof(T));
    }

    T get() const
    {
        T value;
        memcpy(&value, data_, sizeof(T));
        return to_host_order(value);
    }

private:
    char data_[sizeof(T)];
};


using host_u16 = HostInteger<u16>;
using host_u32 = HostInteger<u32>;
using host_s16 = HostInteger<s16>;
using host_s32 = HostInteger<s32>;
using host_u64 = HostInteger<u64>;
using host_s64 = HostInteger<s64>;


}
