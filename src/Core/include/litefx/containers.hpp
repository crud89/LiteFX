#pragma once

// Disable warning C4251: STL class needs to have dll-interface to be used by clients.
// See: https://stackoverflow.com/a/22054743/1254352
#pragma warning(disable: 4251)

#include <string>
#include <map>
#include <vector>

namespace LiteFX 
{
	using String = std::string;

	template<class TKey, class TVal>
	using Dictionary = std::map<TKey, TVal>;

	template<class TVal>
	using Array = std::vector<TVal>;
}