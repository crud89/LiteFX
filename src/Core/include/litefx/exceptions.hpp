#pragma once

#include <optional>
#include <stdexcept>
#include <source_location>
#include <stacktrace>
#include <string>
#include <type_traits>
#include <format>

namespace LiteFX {

	/// <summary>
	/// The base class for exceptions thrown by the SDK.
	/// </summary>
	class Exception : public std::runtime_error {
	private:
		std::source_location m_location;
		std::stacktrace m_trace;

	protected:
		/// <summary>
		/// Initializes the exception.
		/// </summary>
		/// <param name="message">The error message.</param>
		/// <param name="location">The source location of the error.</param>
		/// <param name="trace">The stack trace leading to the error.</param>
		explicit Exception(std::string message, const std::source_location& location, std::stacktrace trace) noexcept :
			std::runtime_error(message.c_str()), m_location(location), m_trace(trace) { }

	public:
		Exception(const Exception&) = default;
		Exception(Exception&&) = default;
		~Exception() noexcept override = default;

		Exception& operator=(const Exception&) = default;
		Exception& operator=(Exception&&) = default;

	public:
		/// <summary>
		/// Gets the source location that identifies where the exception has been thrown.
		/// </summary>
		/// <returns>The source location of the exception.</returns>
		const std::source_location& location() const noexcept {
			return m_location;
		}

		/// <summary>
		/// Gets the stack trace leading to the exception.
		/// </summary>
		/// <returns>The stack trace of the exception.</returns>
		const std::stacktrace& trace() const noexcept {
			return m_trace;
		}
	};

	/// <summary>
	/// An exception that is thrown, if a provided argument is not valid.
	/// </summary>
	class InvalidArgumentException : public Exception {
	private:
		std::string m_argument;

	public:
		/// <summary>
		/// Initializes a new exception.
		/// </summary>
		/// <param name="argument">The name of the argument that was invalid.</param>
		explicit InvalidArgumentException(std::string_view argument) noexcept :
			Exception(std::format("Invalid argument provided: {}.", argument), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		/// <summary>
		/// Initializes a new exception.
		/// </summary>
		/// <param name="argument">The name of the argument that was invalid.</param>
		/// <param name="message">The error message.</param>
		explicit InvalidArgumentException(std::string_view argument, std::string_view message) noexcept :
			Exception(std::format("Invalid argument provided: {}. {}", argument, message), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		/// <summary>
		/// Initializes a new exception.
		/// </summary>
		/// <param name="argument">The name of the argument that was invalid.</param>
		/// <param name="format">The format string for the error message.</param>
		/// <param name="args">The arguments passed to the error message format string.</param>
		template <typename ...TArgs>
		explicit InvalidArgumentException(std::string_view argument, std::string_view format, TArgs&&... args) noexcept :
			Exception(std::format("Invalid argument provided: {}. {}", argument, std::vformat(format, std::make_format_args(std::forward<TArgs>(args)...))), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		InvalidArgumentException(const InvalidArgumentException&) = default;
		InvalidArgumentException(InvalidArgumentException&&) = default;
		~InvalidArgumentException() noexcept override = default;

		InvalidArgumentException& operator=(const InvalidArgumentException&) = default;
		InvalidArgumentException& operator=(InvalidArgumentException&&) = default;

	public:
		/// <summary>
		/// Gets the name of the argument that was invalid.
		/// </summary>
		/// <returns>The name of the invalid argument.</returns>
		const std::string& argument() const noexcept {
			return m_argument;
		}
	};

	/// <summary>
	/// An exception that is thrown, if a provided argument is not within the expected range.
	/// </summary>
	class ArgumentOutOfRangeException : public Exception {
	private:
		std::string m_argument;

	public:
		/// <summary>
		/// Initializes a new exception.
		/// </summary>
		/// <param name="argument">The name of the argument that was out of range.</param>
		explicit ArgumentOutOfRangeException(std::string_view argument) noexcept :
			Exception(std::format("Argument was out of range: {}.", argument), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		/// <summary>
		/// Initializes a new exception.
		/// </summary>
		/// <param name="argument">The name of the argument that was out of range.</param>
		/// <param name="message">The error message.</param>
		explicit ArgumentOutOfRangeException(std::string_view argument, std::string_view message) noexcept :
			Exception(std::format("Argument was out of range: {}. {}", argument, message), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		/// <summary>
		/// Initializes a new exception.
		/// </summary>
		/// <typeparam name="T">The type of the argument that was out of range.</typeparam>
		/// <param name="argument">The name of the argument that was out of range.</param>
		/// <param name="validRange">The lower bound and upper bound of the valid range.</param>
		/// <param name="value">The actual value of provided to the argument.</param>
		/// <param name="message">The error message.</param>
		template <typename T>
		explicit ArgumentOutOfRangeException(std::string_view argument, std::pair<T, T> validRange, T value, std::string_view message) noexcept :
			Exception(std::format("Argument was out of range: {} (valid range is [{}, {}) but actual value was {}). {}", argument, validRange.first, validRange.second, value, message), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		/// <summary>
		/// Initializes a new exception.
		/// </summary>
		/// <param name="argument">The name of the argument that was out of range.</param>
		/// <param name="format">The format string for the error message.</param>
		/// <param name="args">The arguments passed to the error message format string.</param>
		template <typename ...TArgs>
		explicit ArgumentOutOfRangeException(std::string_view argument, std::string_view format, TArgs&&... args) noexcept :
			Exception(std::format("Argument was out of range: {}. {}", argument, std::vformat(format, std::make_format_args(std::forward<TArgs>(args)...))), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		/// <summary>
		/// Initializes a new exception.
		/// </summary>
		/// <typeparam name="T">The type of the argument that was out of range.</typeparam>
		/// <param name="argument">The name of the argument that was out of range.</param>
		/// <param name="validRange">The lower bound and upper bound of the valid range.</param>
		/// <param name="value">The actual value of provided to the argument.</param>
		/// <param name="format">The format string for the error message.</param>
		/// <param name="args">The arguments passed to the error message format string.</param>
		template <typename TRange, typename TValue, typename ...TArgs>
		explicit ArgumentOutOfRangeException(std::string_view argument, std::pair<TRange, TRange> validRange, TValue value, std::string_view format, TArgs&&... args) noexcept :
			Exception(std::format("Argument was out of range: {} (valid range is [{}, {}) but actual value was {}). {}", argument, validRange.first, validRange.second, value, std::vformat(format, std::make_format_args(std::forward<TArgs>(args)...))), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		ArgumentOutOfRangeException(const ArgumentOutOfRangeException&) = default;
		ArgumentOutOfRangeException(ArgumentOutOfRangeException&&) = default;
		~ArgumentOutOfRangeException() noexcept override = default;

		ArgumentOutOfRangeException& operator=(const ArgumentOutOfRangeException&) = default;
		ArgumentOutOfRangeException& operator=(ArgumentOutOfRangeException&&) = default;

	public:
		/// <summary>
		/// Gets the name of the argument that was out of range.
		/// </summary>
		/// <returns>The name of the invalid out of range.</returns>
		const std::string& argument() const noexcept {
			return m_argument;
		}
	};

	/// <summary>
	/// An exception that is thrown, if a provided non-optional argument was not initialized.
	/// </summary>
	class ArgumentNotInitializedException : public Exception {
	private:
		std::string m_argument;

	public:
		/// <summary>
		/// Initializes a new exception.
		/// </summary>
		/// <param name="argument">The name of the argument that was not initialized.</param>
		explicit ArgumentNotInitializedException(std::string_view argument) noexcept :
			Exception(std::format("Argument was not initialized: {}.", argument), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		/// <summary>
		/// Initializes a new exception.
		/// </summary>
		/// <param name="argument">The name of the argument that was not initialized.</param>
		/// <param name="message">The error message.</param>
		explicit ArgumentNotInitializedException(std::string_view argument, std::string_view message) noexcept :
			Exception(std::format("Argument was not initialized: {}. {}", argument, message), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		/// <summary>
		/// Initializes a new exception.
		/// </summary>
		/// <param name="argument">The name of the argument that was not initialized.</param>
		/// <param name="format">The format string for the error message.</param>
		/// <param name="args">The arguments passed to the error message format string.</param>
		template <typename ...TArgs>
		explicit ArgumentNotInitializedException(std::string_view argument, std::string_view format, TArgs&&... args) noexcept :
			Exception(std::format("Argument was not initialized: {}. {}", argument, std::vformat(format, std::make_format_args(std::forward<TArgs>(args)...))), std::source_location::current(), std::stacktrace::current()), m_argument(argument) { }

		ArgumentNotInitializedException(const ArgumentNotInitializedException&) = default;
		ArgumentNotInitializedException(ArgumentNotInitializedException&&) = default;
		~ArgumentNotInitializedException() noexcept override = default;

		ArgumentNotInitializedException& operator=(const ArgumentNotInitializedException&) = default;
		ArgumentNotInitializedException& operator=(ArgumentNotInitializedException&&) = default;

	public:
		/// <summary>
		/// Gets the name of the argument that was not initialized.
		/// </summary>
		/// <returns>The name of the invalid argument.</returns>
		const std::string& argument() const noexcept {
			return m_argument;
		}
	};

	/// <summary>
	/// An exception that is thrown, if a requested operation could not be executed.
	/// </summary>
	class RuntimeException : public Exception {
	public:
		/// <summary>
		/// Initializes a new exception.
		/// </summary>
		explicit RuntimeException() noexcept :
			Exception("The operation could not be executed.", std::source_location::current(), std::stacktrace::current()) { }

		/// <summary>
		/// Initializes a new exception.
		/// </summary>
		/// <param name="message">The error message.</param>
		explicit RuntimeException(std::string_view message) noexcept :
			Exception(std::format("The operation could not be executed: {}", message), std::source_location::current(), std::stacktrace::current()) { }

		/// <summary>
		/// Initializes a new exception.
		/// </summary>
		/// <param name="format">The format string for the error message.</param>
		/// <param name="args">The arguments passed to the error message format string.</param>
		template <typename ...TArgs>
		explicit RuntimeException(std::string_view format, TArgs&&... args) noexcept :
			Exception(std::format("The operation could not be executed: {}", std::vformat(format, std::make_format_args(std::forward<TArgs>(args)...))), std::source_location::current(), std::stacktrace::current()) { }

		RuntimeException(const RuntimeException&) = default;
		RuntimeException(RuntimeException&&) = default;
		~RuntimeException() noexcept override = default;

		RuntimeException& operator=(const RuntimeException&) = default;
		RuntimeException& operator=(RuntimeException&&) = default;
	};
};