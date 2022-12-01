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


#include "memory/buffer.hpp"
#include "number/numeric.hpp"


namespace flash_filesystem
{



inline u32 str_len(const char* str)
{
    const char* s;

    for (s = str; *s; ++s)
        ;
    return (s - str);
}


inline void str_reverse(char str[], int length)
{
    int start = 0;
    int end = length - 1;

    while (start < end) {
        std::swap(*(str + start), *(str + end));
        start++;
        end--;
    }
}


inline bool str_eq(const char* p1, const char* p2)
{
    while (true) {
        if (*p1 not_eq *p2) {
            return false;
        }
        if (*p1 == '\0' or *p2 == '\0') {
            return true;
        }
        ++p1;
        ++p2;
    }
}


inline int str_cmp(const char* p1, const char* p2)
{
    const unsigned char* s1 = (const unsigned char*)p1;
    const unsigned char* s2 = (const unsigned char*)p2;

    unsigned char c1, c2;

    do {
        c1 = (unsigned char)*s1++;
        c2 = (unsigned char)*s2++;

        if (c1 == '\0') {
            return c1 - c2;
        }

    } while (c1 == c2);

    return c1 - c2;
}


// NOTE: Capacity is a holdover from before StringAdapter was a template,
// originally, the class was backed by a Buffer<>. Eventually maybe I'll
// refactor out the parameter.
//
// NOTE: Capacity ideally wouldn't be a template parameter, but I haven't
// removed it yet, as we do want to be able to copy a string with a smaller
// capacity into a larger one, without compiler errors due to mismatched
// types. Of course, there are solutions, but I have to make some changes.
template <u32 Capacity, typename Memory> class StringAdapter
{
public:
    using Buffer = Memory;


    StringAdapter(const char* init)
    {
        while (*init not_eq '\0') {
            mem_.push_back(*init);
            ++init;
        }

        if (mem_.full()) {
            mem_.pop_back();
        }

        mem_.push_back('\0');
    }

    StringAdapter(char c, int fill_count)
    {
        for (int i = 0; i < std::min((int)Capacity - 1, fill_count); ++i) {
            mem_.push_back(c);
        }
        mem_.push_back('\0');
    }

    StringAdapter()
    {
        mem_.push_back('\0');
    }

    StringAdapter(const StringAdapter& other)
    {
        clear();

        for (auto it = other.begin(); it not_eq other.end(); ++it) {
            push_back(*it);
        }
    }

    const StringAdapter& operator=(const StringAdapter& other)
    {
        clear();

        for (auto it = other.begin(); it not_eq other.end(); ++it) {
            push_back(*it);
        }
        return *this;
    }


    // const StringAdapter& operator=(StringAdapter&& other)
    // {
    //     clear();

    //     for (auto it = other.begin(); it not_eq other.end(); ++it) {
    //         push_back(*it);
    //     }
    //     return *this;
    // }


    template <u32 OtherCapacity, typename OtherMem>
    StringAdapter(const StringAdapter<OtherCapacity, OtherMem>& other)
    {
        static_assert(OtherCapacity <= Capacity);

        clear();

        for (auto it = other.begin(); it not_eq other.end(); ++it) {
            push_back(*it);
        }
    }

    template <u32 OtherCapacity, typename OtherMem>
    const StringAdapter&
    operator=(const StringAdapter<OtherCapacity, OtherMem>& other)
    {
        clear();

        for (auto it = other.begin(); it not_eq other.end(); ++it) {
            push_back(*it);
        }
        return *this;
    }

    char& operator[](int pos)
    {
        return mem_[pos];
    }

    const char& operator[](int pos) const
    {
        return mem_[pos];
    }

    void push_back(char c)
    {
        if (not mem_.full()) {
            mem_[mem_.size() - 1] = c;
            mem_.push_back('\0');
        }
    }

    void pop_back()
    {
        mem_.pop_back();
        mem_.pop_back();
        mem_.push_back('\0');
    }

    typename Buffer::Iterator begin() const
    {
        return mem_.begin();
    }

    typename Buffer::Iterator end() const
    {
        return mem_.end() - 1;
    }

    typename Buffer::Iterator insert(typename Buffer::Iterator pos, char val)
    {
        return mem_.insert(pos, val);
    }

    typename Buffer::Iterator erase(typename Buffer::Iterator slot)
    {
        return mem_.erase(slot);
    }

    StringAdapter& operator+=(const char* str)
    {
        while (*str not_eq '\0') {
            push_back(*(str++));
        }
        return *this;
    }

    template <u32 OtherCapacity, typename OtherMem>
    StringAdapter&
    operator+=(const StringAdapter<OtherCapacity, OtherMem>& other)
    {
        (*this) += other.c_str();
        return *this;
    }

    StringAdapter& operator=(const char* str)
    {
        this->clear();

        while (*str not_eq '\0') {
            push_back(*str);
            ++str;
        }

        return *this;
    }

    bool operator==(const char* str)
    {
        return str_eq(str, this->c_str());
    }

    bool full() const
    {
        return mem_.full();
    }

    u32 length() const
    {
        return mem_.size() - 1;
    }

    bool empty() const
    {
        return mem_.size() == 1;
    }

    void clear()
    {
        mem_.clear();
        mem_.push_back('\0');
    }

    const char* c_str() const
    {
        return mem_.data();
    }

    u32 remaining() const
    {
        return (mem_.capacity() - 1) - mem_.size();
    }

private:
    Buffer mem_;
};



template <u32 Capacity>
using StringBuffer = StringAdapter<Capacity, Buffer<char, Capacity + 1>>;



//
// Returns a pointer to the trailing text, if finds the match string. Otherwise,
// returns nullptr;
//
template <u32 Capacity>
const char* starts_with(const char* match, const StringBuffer<Capacity>& buffer)
{
    auto it = buffer.begin();
    while (*match not_eq '\0' and it not_eq buffer.end()) {
        if (*match not_eq *it) {
            return nullptr;
        }

        ++match;
        ++it;
    }

    if (*match not_eq '\0') {
        return nullptr;
    }

    if (it == buffer.end()) {
        return nullptr;
    }

    return it;
}



template <u32 Capacity, u32 MatchCapacity>
bool ends_with(const StringBuffer<MatchCapacity>& match,
               const StringBuffer<Capacity>& buffer)
{
    if (buffer.empty()) {
        return false;
    }

    if (buffer.length() <= match.length()) {
        return false;
    }

    auto it1 = buffer.end() - 1;
    auto it2 = match.end() - 1;
    while (it2 not_eq match.begin()) {
        if (*it1 not_eq *it2) {
            return false;
        }

        --it1;
        --it2;
    }

    return true;
}



template <u32 Capacity, typename Mem>
bool is_numeric(const StringAdapter<Capacity, Mem>& buf)
{
    for (auto c : buf) {
        if (c < '0' or c > '9') {
            return false;
        }
    }

    return true;
}


template <u32 Capacity, typename Mem>
bool operator==(StringAdapter<Capacity, Mem> buf, const char* str)
{
    return str_cmp(str, buf.c_str()) == 0;
}



StringBuffer<12> stringify(s32 num);

// passthrough stringify just defined for convenience in function templates.
inline const char* stringify(const char* arg)
{
    return arg;
}



template <u32 size, typename Arg, typename... Args>
void make_format(StringBuffer<size>& output,
                 const char* fmt_str,
                 Arg&& arg,
                 Args&&... args)
{
    while (*fmt_str not_eq '\0') {
        if (*fmt_str == '%') {
            output += stringify(arg);
            if constexpr (sizeof...(args)) {
                make_format(output, ++fmt_str, std::forward<Args>(args)...);
                break;
            }
        } else {
            output.push_back(*fmt_str);
        }
        ++fmt_str;
    }
}



template <u32 size = 128, typename... Args>
StringBuffer<size> format(const char* fmt_str, Args&&... args)
{
    StringBuffer<size> result;
    make_format(result, fmt_str, std::forward<Args>(args)...);

    return result;
}




}
