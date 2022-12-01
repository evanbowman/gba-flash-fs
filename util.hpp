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

#define COLD [[gnu::cold]]
#define HOT [[gnu::hot]]

#ifdef __GNUC__
#define UNLIKELY(COND) __builtin_expect((COND), false)
#else
#define UNLIKELY(COND) (COND)
#endif

#include <iterator>


#if defined(__GBA__) or defined(__NDS__)
#define READ_ONLY_DATA __attribute__((section(".rodata")))
#else
#define READ_ONLY_DATA
#endif


namespace _detail
{

template <typename T> struct reversion_wrapper
{
    T& iterable;
};


template <typename T> auto begin(reversion_wrapper<T> w)
{
    return std::rbegin(w.iterable);
}


template <typename T> auto end(reversion_wrapper<T> w)
{
    return std::rend(w.iterable);
}


} // namespace _detail


template <typename T> _detail::reversion_wrapper<T> reversed(T&& iterable)
{
    return {iterable};
}
