#pragma once

#include <optional>
#include <stdexcept>
#include <source_location>
#include <stacktrace>
#include <string>
#include <type_traits>
#include <format>

namespace LiteFX {

	/// @brief The base class for exceptions thrown by the SDK.
	class Exception : public std::runtime_error {
	private:
		std::source_location m_location;
		std::stacktrace m_trace;

	protected:
		/// @brief Initializes the exception.
		///
		/// @param message The error message.
		/// @param location The source location of the error.
		/// @param trace The stack trace leading to the error.
		explicit Exception(const std::string& message, const std::source_location& location, std::stacktrace trace) :
			std::runtime_error(message), m_location(location), m_trace(std::move(trace)) { }

	public:
		Exception(const Exception&) = default;
		Exception(Exception&&) noexcept = default;
		~Exception() noexcept override = default;

		Exception& operator=(const Exception&) = default;
		Exception& operator=(Exception&&) noexcept = default;

	public:
		/// @brief Gets the source location that identifies where the exception has been thrown.
		///
		/// @return The source location of the exception.
		const std::source_location& location() const noexcept {
			return m_location;
		}

		/// @brief Gets the stack trace leading to the exception.
		///
		/// @return The stack trace of the exception.
		const std::stacktrace& trace() const noexcept {
			return m_trace;
		}
	};

	/// @brief An exception that is thrown, if a provided argument is not valid.
	class InvalidArgumentException : public Exception {
	private:
		std::string m_argument;

	public:
		/// @brief Initializes a new exception.
		///
		/// @param argument The name of the argument that was invalid.
		explicit InvalidArgumentException(std::string_view argument) :
			Exception(std::format("Invalid argument provided: {}.", argument), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		/// @brief Initializes a new exception.
		///
		/// @param argument The name of the argument that was invalid.
		/// @param message The error message.
		explicit InvalidArgumentException(std::string_view argument, std::string_view message) :
			Exception(std::format("Invalid argument provided: {}. {}", argument, message), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		/// @brief Initializes a new exception.
		///
		/// @param argument The name of the argument that was invalid.
		/// @param format The format string for the error message.
		/// @param args The arguments passed to the error message format string.
		template <typename ...TArgs>
		explicit InvalidArgumentException(std::string_view argument, std::format_string<TArgs...> format, TArgs&&... args) :
			Exception(std::format("Invalid argument provided: {}. {}", argument, std::format(format, std::forward<TArgs>(args)...)), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		InvalidArgumentException(const InvalidArgumentException&) = default;
		InvalidArgumentException(InvalidArgumentException&&) noexcept = default;
		~InvalidArgumentException() noexcept override = default;

		InvalidArgumentException& operator=(const InvalidArgumentException&) = default;
		InvalidArgumentException& operator=(InvalidArgumentException&&) noexcept = default;

	public:
		/// @brief Gets the name of the argument that was invalid.
		///
		/// @return The name of the invalid argument.
		const std::string& argument() const noexcept {
			return m_argument;
		}
	};

	/// @brief An exception that is thrown, if a provided argument is not within the expected range.
	class ArgumentOutOfRangeException : public Exception {
	private:
		std::string m_argument;

	public:
		/// @brief Initializes a new exception.
		///
		/// @param argument The name of the argument that was out of range.
		explicit ArgumentOutOfRangeException(std::string_view argument) :
			Exception(std::format("Argument was out of range: {}.", argument), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		/// @brief Initializes a new exception.
		///
		/// @param argument The name of the argument that was out of range.
		/// @param message The error message.
		explicit ArgumentOutOfRangeException(std::string_view argument, std::string_view message) :
			Exception(std::format("Argument was out of range: {}. {}", argument, message), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		///// @brief Initializes a new exception.
		///// 
		///// @tparam T The type of the argument that was out of range.
		///// @param argument The name of the argument that was out of range.
		///// @param validRange The lower bound and upper bound of the valid range.
		///// @param value The actual value of provided to the argument.
		///// @param message The error message.
		//template <typename T>
		//explicit ArgumentOutOfRangeException(std::string_view argument, std::pair<T, T> validRange, T value, std::string_view message) :
		//	Exception(std::format("Argument was out of range: {} (valid range is [{}, {}] but provided value was {}). {}", argument, validRange.first, validRange.second, value, message), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		/// @brief Initializes a new exception.
		///
		/// @param argument The name of the argument that was out of range.
		/// @param format The format string for the error message.
		/// @param args The arguments passed to the error message format string.
		template <typename ...TArgs>
		explicit ArgumentOutOfRangeException(std::string_view argument, std::format_string<TArgs...> format, TArgs&&... args) :
			Exception(std::format("Argument was out of range: {}. {}", argument, std::format(format, std::forward<TArgs>(args)...)), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		/// @brief Initializes a new exception.
		///
		/// @tparam T The type of the argument that was out of range.
		/// @param argument The name of the argument that was out of range.
		/// @param validRange The lower bound and upper bound of the valid range.
		/// @param value The actual value of provided to the argument.
		/// @param format The format string for the error message.
		/// @param args The arguments passed to the error message format string.
		template <typename T, typename ...TArgs>
		explicit ArgumentOutOfRangeException(std::string_view argument, std::pair<T, T> validRange, T value, std::format_string<TArgs...> format, TArgs&&... args) :
			Exception(std::format("Argument was out of range: {} (valid range is [{}, {}) but actual value was {}). {}", argument, validRange.first, validRange.second, value, std::format(format, std::forward<TArgs>(args)...)), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		ArgumentOutOfRangeException(const ArgumentOutOfRangeException&) = default;
		ArgumentOutOfRangeException(ArgumentOutOfRangeException&&) noexcept = default;
		~ArgumentOutOfRangeException() noexcept override = default;

		ArgumentOutOfRangeException& operator=(const ArgumentOutOfRangeException&) = default;
		ArgumentOutOfRangeException& operator=(ArgumentOutOfRangeException&&) noexcept = default;

	public:
		/// @brief Gets the name of the argument that was out of range.
		///
		/// @return The name of the invalid out of range.
		const std::string& argument() const noexcept {
			return m_argument;
		}
	};

	/// @brief An exception that is thrown, if a provided non-optional argument was not initialized.
	class ArgumentNotInitializedException : public Exception {
	private:
		std::string m_argument;

	public:
		/// @brief Initializes a new exception.
		///
		/// @param argument The name of the argument that was not initialized.
		explicit ArgumentNotInitializedException(std::string_view argument) :
			Exception(std::format("Argument was not initialized: {}.", argument), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		/// @brief Initializes a new exception.
		///
		/// @param argument The name of the argument that was not initialized.
		/// @param message The error message.
		explicit ArgumentNotInitializedException(std::string_view argument, std::string_view message) :
			Exception(std::format("Argument was not initialized: {}. {}", argument, message), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		/// @brief Initializes a new exception.
		///
		/// @param argument The name of the argument that was not initialized.
		/// @param format The format string for the error message.
		/// @param args The arguments passed to the error message format string.
		template <typename ...TArgs>
		explicit ArgumentNotInitializedException(std::string_view argument, std::format_string<TArgs...> format, TArgs&&... args) :
			Exception(std::format("Argument was not initialized: {}. {}", argument, std::format(format, std::forward<TArgs>(args)...)), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		ArgumentNotInitializedException(const ArgumentNotInitializedException&) = default;
		ArgumentNotInitializedException(ArgumentNotInitializedException&&) noexcept = default;
		~ArgumentNotInitializedException() noexcept override = default;

		ArgumentNotInitializedException& operator=(const ArgumentNotInitializedException&) = default;
		ArgumentNotInitializedException& operator=(ArgumentNotInitializedException&&) noexcept = default;

	public:
		/// @brief Gets the name of the argument that was not initialized.
		///
		/// @return The name of the invalid argument.
		const std::string& argument() const noexcept {
			return m_argument;
		}
	};

	/// @brief An exception that is thrown, if a requested operation could not be executed.
	class RuntimeException : public Exception {
	public:
		/// @brief Initializes a new exception.
		explicit RuntimeException() :
			Exception("The operation could not be executed.", std::source_location::current(), std::stacktrace::current()) { }

		/// @brief Initializes a new exception.
		///
		/// @param message The error message.
		explicit RuntimeException(std::string_view message) :
			Exception(std::format("The operation could not be executed: {}", message), std::source_location::current(), std::stacktrace::current()) { }

		/// @brief Initializes a new exception.
		///
		/// @param format The format string for the error message.
		/// @param args The arguments passed to the error message format string.
		template <typename ...TArgs>
		explicit RuntimeException(std::format_string<TArgs...> format, TArgs&&... args) :
			Exception(std::format("The operation could not be executed: {}", std::format(format, std::forward<TArgs>(args)...)), std::source_location::current(), std::stacktrace::current()) { }

		RuntimeException(const RuntimeException&) = default;
		RuntimeException(RuntimeException&&) noexcept = default;
		~RuntimeException() noexcept override = default;

		RuntimeException& operator=(const RuntimeException&) = default;
		RuntimeException& operator=(RuntimeException&&) noexcept = default;
	};
};