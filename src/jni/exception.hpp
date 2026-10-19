#pragma once
#include <stdexcept>
#include <string>
#include <string_view>
#include <jni/jni.h>
#include "object.hpp"

namespace jaba::jni
{
	// A java exception that a call threw. It keeps what was thrown, so it can be told apart:
	//
	//   try { integer::parse_int("x"); }
	//   catch (const jaba::jni::java_exception& e)
	//   {
	//       e.class_name();                       // "java.lang.NumberFormatException"
	//       e.message();                          // "For input string: \"x\""
	//       e.is_a<illegal_argument_exception>()  // true, it's a subclass
	//   }
	//
	// It is a std::runtime_error, so a catch of that gets it too. The throwable is a global
	// ref, so the exception can be copied, kept and thrown again from any thread
	class java_exception : public std::runtime_error
	{
	public:
		// Takes the pending exception off the thread, which has to happen before any other jni
		// call (those are undefined with one pending). context says where it came from
		[[nodiscard]] static java_exception take(JNIEnv* env, std::string_view context);

		// java.lang.IllegalStateException, with dots
		[[nodiscard]] std::string_view class_name() const noexcept
		{
			return class_name_;
		}

		// getMessage(), empty when there is none
		[[nodiscard]] std::string_view message() const noexcept
		{
			return message_;
		}

		[[nodiscard]] bool has_message() const noexcept
		{
			return !message_.empty();
		}

		[[nodiscard]] const object& throwable() const noexcept
		{
			return throwable_;
		}

		// is it an instance of T, or of a class that extends it
		template <class T>
		[[nodiscard]] bool is_a() const
		{
			return throwable_.instance_of<T>();
		}

		// the throwable seen as T, no check (see is_a)
		template <class T>
		[[nodiscard]] T as() const
		{
			return throwable_.as<T>();
		}

	private:
		java_exception(const std::string& what, std::string class_name, std::string message, object throwable)
			:	std::runtime_error(what),
				class_name_(std::move(class_name)),
				message_(std::move(message)),
				throwable_(std::move(throwable)) { }

		std::string class_name_;
		std::string message_;
		object throwable_;
	};
}
