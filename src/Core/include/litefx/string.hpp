#pragma once

#include <string>
#include <sstream>
#include <iterator>
#include <vector>

namespace LiteFX {
    using String = std::string;

    // Based on: https://stackoverflow.com/a/5289170/1254352
    template <typename TStrings, typename Value = typename TStrings::value_type>
    String Join(const TStrings& elements, const String& delimiter) 
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

}