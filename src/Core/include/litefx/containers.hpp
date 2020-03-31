#pragma once

// Disable warning C4251: STL class needs to have dll-interface to be used by clients.
// See: https://stackoverflow.com/a/22054743/1254352
#pragma warning(disable: 4251)

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <stdexcept>

namespace LiteFX 
{
	using String = std::string;
	using Exception = std::exception;

	template<class TKey, class TVal>
	using Dictionary = std::map<TKey, TVal>;

	template<class T>
	using Array = std::vector<T>;

	template<class T, class TDeleter = std::default_delete<T>>
	using UniquePtr = std::unique_ptr<T, TDeleter>;

	template <class T>
	using SharedPtr = std::shared_ptr<T>;

	template <class T>
	UniquePtr<T> makeUnique()
	{
		return std::make_unique<T>();
	}

	template <class T, class... TArgs>
	UniquePtr<T> makeUnique(TArgs&&... _args)
	{
		return std::make_unique<T>(std::forward<TArgs>(_args)...);
	}

	template <class T>
	SharedPtr<T> makeShared()
	{
		return std::make_shared<T>()
	}

	template <class T, class... TArgs>
	SharedPtr<T> makeShared(TArgs&&... _args)
	{
		return std::make_shared<T>(std::forward<TArgs>(_args)...);
	}

	template <class T>
	SharedPtr<T> makeShared(UniquePtr<T>& ptr)
	{
		return std::make_shared<T>(ptr.release());
	}
}