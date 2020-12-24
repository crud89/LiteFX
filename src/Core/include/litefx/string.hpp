#pragma once

#include <string>
#include <sstream>
#include <iterator>
#include <vector>
#include <codecvt>

namespace LiteFX {
    using String = std::string;
    using WString = std::wstring;

    // Based on: https://stackoverflow.com/a/5289170/1254352
    template <typename TStrings, typename Value = typename TStrings::value_type>
    String Join(const TStrings& elements, const String& delimiter = "") 
    {
        std::ostringstream stream;

        auto beg = std::begin(elements);
        auto end = std::end(elements);

        if (beg != end) 
        {
            std::copy(beg, std::prev(end), std::ostream_iterator<Value>(stream, delimiter.c_str()));
            beg = std::prev(end);
        }

        if (beg != end)
            stream << *beg;

        return stream.str();
    }

    /// <summary>
    /// Converts an UTF-8 single-byte encoded string into an UTF-16 representation.
    /// </summary>
    /// <param name="utf8"></param>
    /// <returns></returns>
    inline WString Widen(const String& utf8)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<WCHAR>> conv;
        return conv.from_bytes(utf8);
    }

    /// <summary>
    /// Converts an UTF-16 multi-byte encoded string into an UTF-8 representation.
    /// </summary>
    /// <param name="utf16"></param>
    /// <returns></returns>
    inline String Narrow(const WString& utf16)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<WCHAR>> conv;
        return conv.to_bytes(utf16);
    }
}