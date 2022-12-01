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


// NOTE: to compile the unit tests:
// g++ -std=c++20 flash_filesystem.cpp -I ./ -g3 -D__FAKE_VECTOR__ -D__TEST__



#pragma once

#include "function.hpp"
#include "memory/buffer.hpp"
#include "number/endian.hpp"
#include "number/int.hpp"
#include "string.hpp"




// Brief explanation for why this library uses vector for holding file data: In
// Skyland, I didn't use std::vector<T>, but instead, a custom container class
// called Vector<T>. Skyland was unable to allocate large sections of contiguous
// memory due to its unusual custom allocators, so I implemented a segmented
// non-contiguous vector replacement. Vector<T> worked very well for this
// library, but it's highly coupled with other classes in the skyland repo, so
// I've just created an alias to std::vector<T> in this header for people who
// want to use the library.
#ifdef __SKYLAND_SOURCE__
#ifndef __GBA__
#include <fstream>
#include <iostream>
#include <vector>
inline void info(Platform& pfrm, const StringBuffer<200>& msg)
{
    std::cout << msg.c_str() << std::endl;
}
#ifdef __FAKE_VECTOR__
template <typename T> using Vector = std::vector<T>;
#else
#include "containers/vector.hpp"
#endif
#else
#include "containers/vector.hpp"
#endif // __GBA__
#else // not __SKYLAND_SOURCE__
#include <vector>
namespace flash_filesystem
{
template <typename T> using Vector = std::vector<T>;
}
#endif



namespace flash_filesystem
{



class Platform;



#ifndef FS_MAX_PATH
#define FS_MAX_PATH 86
#endif



static constexpr const int max_path = FS_MAX_PATH;



struct Statistics
{
    u16 bytes_used_;
    u16 bytes_available_;
};



Statistics statistics(Platform& pfrm);



enum InitStatus {
    // Newly initialized
    initialized,

    // Previously initialized
    already_initialized,

    failed,
};



InitStatus initialize(Platform& pfrm, u32 offset);



bool store_file_data(Platform&, const char* path, Vector<char>& data);



u32 read_file_data(Platform&, const char* path, Vector<char>& output);



u32 file_size(Platform&, const char* path);



inline u32
read_file_data_text(Platform& pfrm, const char* path, Vector<char>& output)
{
    auto read = read_file_data(pfrm, path, output);
    output.push_back('\0');

    return read;
}



inline bool
store_file_data_text(Platform& pfrm, const char* path, Vector<char>& data)
{
    data.pop_back();
    auto result = store_file_data(pfrm, path, data);
    data.push_back('\0');

    return result;
}



inline u32
read_file_data_binary(Platform& pfrm, const char* path, Vector<char>& output)
{
    return read_file_data(pfrm, path, output);
}



inline bool
store_file_data_binary(Platform& pfrm, const char* path, Vector<char>& data)
{
    return store_file_data(pfrm, path, data);
}



inline bool
store_file_data(Platform& pfrm, const char* path, const char* ptr, u32 length)
{
    Vector<char> buffer;
    for (u32 i = 0; i < length; ++i) {
        buffer.push_back(ptr[i]);
    }
    buffer.push_back('\0');

    return store_file_data_text(pfrm, path, buffer);
}



void walk(Platform& pfrm,
          Function<8 * sizeof(void*), void(const char*)> callback);



template <typename F>
void walk_directory(Platform& pfrm, const char* directory, F callback)
{
    walk(pfrm, [callback, directory](const char* path) {
        auto remainder =
            starts_with(directory, StringBuffer<FS_MAX_PATH>(path));
        if (remainder) {
            callback(remainder);
        }
    });
}



void unlink_file(Platform& pfrm, const char* path);



bool file_exists(Platform& pfrm, const char* path);



void destroy(Platform& pfrm);



void set_log_receiver(Function<8, void(const char*)> callback);



} // namespace flash_filesystem
