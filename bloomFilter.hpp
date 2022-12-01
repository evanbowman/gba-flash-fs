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

#include "bitvector.hpp"
#include "fnv.hpp"

namespace flash_filesystem
{

template <u32 bits> class BloomFilter
{
public:
    void insert(const char* data, u32 data_length)
    {
        const u32 fnv = fnv32(data, data_length);
        const u32 murmur = murmurhash(data, data_length, 0);

        static_assert(bits % 2 == 0,
                      "By instantiating the bloom filter with a non-"
                      "power-of-two bitsize, the compiler may use an "
                      "inefficient division. Remove this assertion if you "
                      "don't care.");

        bitset_.set(fnv % bits, true);
        bitset_.set(murmur % bits, true);
    }


    bool exists(const char* data, u32 data_length) const
    {
        const u32 fnv = fnv32(data, data_length);
        const u32 murmur = murmurhash(data, data_length, 0);

        return bitset_[fnv % bits] and bitset_[murmur % bits];
    }


    void clear()
    {
        bitset_.clear();
    }


private:
    u32 murmurhash(const char* key, u32 len, u32 seed) const
    {
        // The MIT License (MIT)

        // Copyright (c) 2014 Joseph Werle

        // Permission is hereby granted, free of charge, to any person obtaining
        // a copy of this software and associated documentation files (the
        // "Software"), to deal in the Software without restriction, including
        // without limitation the rights to use, copy, modify, merge, publish,
        // distribute, sublicense, and/or sell copies of the Software, and to
        // permit persons to whom the Software is furnished to do so, subject to
        // the following conditions:

        // The above copyright notice and this permission notice shall be
        // included in all copies or substantial portions of the Software.

        // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
        // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
        // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
        // NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
        // BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
        // ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
        // CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
        // SOFTWARE.

        u32 c1 = 0xcc9e2d51;
        u32 c2 = 0x1b873593;
        u32 r1 = 15;
        u32 r2 = 13;
        u32 m = 5;
        u32 n = 0xe6546b64;
        u32 h = 0;
        u32 k = 0;
        u8* d = (u8*)key;
        const u32* chunks = NULL;
        const u8* tail = NULL;
        int i = 0;
        int l = len / 4;

        h = seed;

        chunks = (const u32*)(d + l * 4);
        tail = (const u8*)(d + l * 4);

        for (i = -l; i != 0; ++i) {
            k = chunks[i];

            k *= c1;
            k = (k << r1) | (k >> (32 - r1));
            k *= c2;

            h ^= k;
            h = (h << r2) | (h >> (32 - r2));
            h = h * m + n;
        }

        k = 0;

        switch (len & 3) {
        case 3:
            k ^= (tail[2] << 16);
        case 2:
            k ^= (tail[1] << 8);

        case 1:
            k ^= tail[0];
            k *= c1;
            k = (k << r1) | (k >> (32 - r1));
            k *= c2;
            h ^= k;
        }

        h ^= len;

        h ^= (h >> 16);
        h *= 0x85ebca6b;
        h ^= (h >> 13);
        h *= 0xc2b2ae35;
        h ^= (h >> 16);

        return h;
    }


    Bitvector<bits> bitset_;
};
}
