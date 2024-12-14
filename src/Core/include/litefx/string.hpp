#pragma once

#include <string>
#include <string_view>
#include <iterator>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <ranges>

#if defined _WIN32 || defined WINCE
#define LITEFX_CODECVT_USE_WIN32
#include <Windows.h>
#else
// Continue using std::codecvt for now, since other platforms than Win32 are currently unsupported anyway.
#include <codecvt>
#endif

namespace LiteFX {
    
    using namespace std::string_literals;
    using namespace std::string_view_literals;

    using String = std::string;
    using WString = std::wstring;
    using StringView = std::string_view;
    using WStringView = std::wstring_view;

    constexpr auto Join(std::ranges::input_range auto&& elements, StringView delimiter = ""sv) requires
        std::convertible_to<std::ranges::range_value_t<decltype(elements)>, String>
    {
        return std::ranges::fold_left(elements | std::views::join_with(delimiter), String{}, std::plus<>{});
    }

    constexpr auto WJoin(std::ranges::input_range auto&& elements, WStringView delimiter = L""sv) requires
        std::convertible_to<std::ranges::range_value_t<decltype(elements)>, String>
    {
        return std::ranges::fold_left(elements | std::views::join_with(delimiter), WString{}, std::plus<>{});
    }

    /// <summary>
    /// Computes the FNVa hash for <paramref name="string" />.
    /// </summary>
    /// <param name="string">The string to hash.</param>
    /// <returns>The FNVa hash for <paramref name="string" />.</returns>
    constexpr static std::uint64_t hash(StringView string) noexcept 
    {
        const std::uint64_t prime = 0x00000100000001b3;
        std::uint64_t seed = 0xcbf29ce484222325; // NOLINT
        
        for (auto ptr = string.begin(); ptr != string.end(); ptr++)
            seed = (seed ^ *ptr) * prime;

        return seed;
    }

    /// <summary>
    /// Computes the FNVa hash for <paramref name="string" />.
    /// </summary>
    /// <param name="string">The string to hash.</param>
    /// <returns>The FNVa hash for <paramref name="string" />.</returns>
    constexpr static std::uint64_t hash(WStringView string) noexcept 
    {
        const std::uint64_t prime = 0x00000100000001b3;
        std::uint64_t seed  = 0xcbf29ce484222325; // NOLINT

        for (auto ptr = string.begin(); ptr != string.end(); ptr++)
            seed = (seed ^ *ptr) * prime;

        return seed;
    }

    /// <summary>
    /// Computes the FNVa hash for <paramref name="string" />.
    /// </summary>
    /// <param name="string">The string to hash.</param>
    /// <param name="chars">The number of characters in the string.</param>
    /// <returns>The FNVa hash for <paramref name="string" />.</returns>
    consteval std::uint64_t operator"" _hash(const char* string, size_t chars) noexcept 
    {
        return hash(StringView(string, chars));
    }

    /// <summary>
    /// Computes the FNVa hash for <paramref name="string" />.
    /// </summary>
    /// <param name="string">The string to hash.</param>
    /// <param name="chars">The number of characters in the string.</param>
    /// <returns>The FNVa hash for <paramref name="string" />.</returns>
    consteval std::uint64_t operator"" _hash(const wchar_t* string, size_t chars) noexcept 
    {
        return hash(WStringView(string, chars));
    }

    /// <summary>
    /// Converts an UTF-8 single-byte encoded string into an UTF-16 representation.
    /// </summary>
    /// <param name="utf8"></param>
    /// <returns></returns>
    inline WString Widen(StringView utf8)
    {
#if defined LITEFX_CODECVT_USE_WIN32
        if (utf8.empty())
            return L"";

        const auto size = ::MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);

        if (size <= 0)
            throw std::runtime_error("Unable to convert string to UTF-16: " + std::to_string(size));

        WString result(size, 0);
        ::MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), result.data(), size);

        return result;
#else
        std::wstring_convert<std::codecvt_utf8_utf16<WCHAR>> conv;
        return conv.from_bytes(utf8);
#endif
    }

    /// <summary>
    /// Converts an UTF-16 multi-byte encoded string into an UTF-8 representation.
    /// </summary>
    /// <param name="utf16"></param>
    /// <returns></returns>
    inline String Narrow(WStringView utf16)
    {
#if defined LITEFX_CODECVT_USE_WIN32
        if (utf16.empty())
            return "";

        const auto size = ::WideCharToMultiByte(CP_UTF8, 0, utf16.data(), static_cast<int>(utf16.size()), nullptr, 0, nullptr, nullptr);

        if (size <= 0)
            throw std::runtime_error("Unable to convert string to UTF-8: " + std::to_string(size));

        String result(size, 0);
        ::WideCharToMultiByte(CP_UTF8, 0, utf16.data(), static_cast<int>(utf16.size()), result.data(), size, nullptr, nullptr);

        return result;
#else
        std::wstring_convert<std::codecvt_utf8_utf16<WCHAR>> conv;
        return conv.to_bytes(utf16);
#endif
    }
}

#undef LITEFX_CODECVT_USE_WIN32