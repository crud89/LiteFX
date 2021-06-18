#pragma once

#include <stdexcept>
#include <string>
#include <optional>
#include <string>
#include <type_traits>
#include <fmt/format.h>

namespace LiteFX {

	using Exception = std::exception;

	template <typename TBase, typename TException>
	class ExceptionBase : public TBase {
	private:
		std::optional<Exception> m_inner;
	
	public:
		ExceptionBase(const ExceptionBase&) = delete;
		ExceptionBase(ExceptionBase&&) = delete;
		virtual ~ExceptionBase() noexcept = default;

		explicit ExceptionBase() noexcept :
			TBase(fmt::format("{0}", typeid(TException).name())) { }
		explicit ExceptionBase(Exception&& inner) noexcept :
			TBase(fmt::format("{0}\r\n\t{1}", typeid(TException).name(), inner.what())), m_inner(std::move(inner)) { }

		explicit ExceptionBase(char const* const message) noexcept :
			TBase(fmt::format("{0}: {1}", typeid(TException).name(), message)) { }
		explicit ExceptionBase(Exception&& inner, char const* const message) noexcept :
			TBase(fmt::format("{0}: {1}\r\n\t{2}", typeid(TException).name(), message, inner.what())), m_inner(std::move(inner)) { }
		explicit ExceptionBase(const std::string& message) noexcept :
			TBase(fmt::format("{0}: {1}", typeid(TException).name(), message)) { }
		explicit ExceptionBase(Exception&& inner, const std::string& message) noexcept :
			TBase(fmt::format("{0}: {1}\r\n\t{2}", typeid(TException).name(), message, inner.what())), m_inner(std::move(inner)) { }

		template <typename ...TArgs>
		explicit ExceptionBase(const std::string& format, TArgs&&... args) noexcept :
			TBase(fmt::format("{0}: {1}", typeid(TException).name(), fmt::format(format, std::forward<TArgs>(args)...))) { }

		template <typename ...TArgs>
		explicit ExceptionBase(Exception&& inner, const std::string& format, TArgs&&... args) noexcept :
			TBase(fmt::format("{0}: {1}\r\n\t{2}", typeid(TException).name(), fmt::format(format, std::forward<TArgs>(args)...), inner.what())) { }

	public:
		[[nodiscard]]
		virtual const Exception* innerException() const noexcept {
			// NOTE: For some reason, initializing an std::optional by moving the exception into it results in a copy that drops the inherited exception. Be aware, that
			//       you only get an std::exception instance here.
			return m_inner.has_value() ? &m_inner.value() : nullptr; 
		}
	};

#define DEFINE_EXCEPTION(name, base) class name : public ExceptionBase<base, name> { \
	public: \
		using ExceptionBase<base, name>::ExceptionBase; \
	}

	DEFINE_EXCEPTION(InvalidArgumentException, std::invalid_argument);
	DEFINE_EXCEPTION(ArgumentOutOfRangeException, std::out_of_range);
	DEFINE_EXCEPTION(ArgumentNotInitializedException, std::logic_error);
	DEFINE_EXCEPTION(RuntimeException, std::runtime_error);

};