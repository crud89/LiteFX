#pragma once

#include <string>
#include <map>
#include <vector>

namespace LiteFX 
{
	typedef std::string String;

	template<class TKey, class TVal>
	class Dictionary : public std::map<TKey, TVal> { };

	template<class TVal>
	class Array : public std::vector<TVal> { };
}