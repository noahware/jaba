#pragma once
#include <compare>
#include <concepts>
#include <cstddef>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>
#include "jni/exception.hpp"
#include "jni/jni.hpp"
#include "jni/utf.hpp"
#include "sdk/fwd.hpp"

namespace jaba
{
	// a java exception that a call threw, see jni/exception.hpp
	using java_exception = jni::java_exception;
}

namespace jaba::inline lang
{
	// the sdk's classes, which is everything that extends object (its members carry the marker)
	template <class T>
	concept sdk_object = requires { typename T::jaba_is_object; };
}

namespace jaba::inline lang::detail
{
	using jni::detail::from_jstring;
	using jni::detail::new_jstring;

	// == on the sdk's classes is java's equals, with null equal to null and to nothing else.
	// Whether it's the same object is same_as()
	template <class T>
	[[nodiscard]] bool equal_objects(const T& a, const T& b)
	{
		// the same handle, or both null
		if (a.obj() == b.obj())
			return true;

		if (!a.obj() || !b.obj())
			return false;

		return a.equals(b);
	}

	// <=> on the classes that are Comparable, through compareTo. Null comes first
	template <class T>
	[[nodiscard]] std::strong_ordering compare_objects(const T& a, const T& b)
	{
		const bool a_null = a.obj() == nullptr;
		const bool b_null = b.obj() == nullptr;

		if (a_null || b_null)
			return b_null <=> a_null;

		return a.compare_to(b) <=> 0;
	}

	// what can go on either side of a string's +: text, a number or any other sdk object, the
	// way java concatenates them (String.valueOf). A string is the exact overload
	template <class T>
	concept concat_operand_type =
		!std::is_same_v<std::remove_cvref_t<T>, string> &&
		(std::is_arithmetic_v<std::remove_cvref_t<T>> ||
		 std::is_convertible_v<const T&, std::string_view> ||
		 sdk_object<std::remove_cvref_t<T>>);

	// the same for == against text: text only
	template <class T>
	concept text_type = !sdk_object<std::remove_cvref_t<T>> && std::is_convertible_v<const T&, std::string_view>;

	// defined after string, they return it
	template <class T>
	[[nodiscard]] string concat_operand(const T& value);

	[[nodiscard]] string concat_strings(const string& a, const string& b);

	template <class T>
	[[nodiscard]] std::string object_text(const T& value);
}

// Every sdk class sits in one single inheritance tree rooted at object, never a diamond: a
// class only mixes in its bases' members, and the same members twice would be ambiguous
namespace jaba::inline lang
{
	JABA_CLASS_BEGIN(object, "java/lang/Object")
		// marks the sdk's classes, see sdk_object
		using jaba_is_object = void;

		JABA_METHOD(string, to_string, "toString")
		JABA_METHOD(jint, hash_code, "hashCode")
		JABA_METHOD(jboolean, equals, "equals", object)
		JABA_METHOD(class_, get_class, "getClass")

		// ==, java's equals. Exactly the same class on both sides, anything else is the same
		// object or not (same_as): the handles themselves are never compared, a copy of a ref
		// has a handle of its own
		[[nodiscard]] friend bool operator==(const jaba_self& a, const jaba_self& b)
		{
			return detail::equal_objects(a, b);
		}

		// toString() as utf8, "null" for null. It's also what std::format prints
		[[nodiscard]] std::string to_std() const;

		// A literal or a number where an object is asked for (list.add("a"), list.add(5)),
		// boxed the way java would. Only the exact types below: char converts to int and so
		// would box an Integer, and a wider integer type is ambiguous, cast it. Defined below,
		// and the numbers in boxed.hpp, they return the class itself and that isn't complete yet
		static object jaba_make(std::string_view utf8);
		static object jaba_make(jboolean value);
		static object jaba_make(jchar value);
		static object jaba_make(jint value);
		static object jaba_make(jlong value);
		static object jaba_make(jfloat value);
		static object jaba_make(jdouble value);

		// bool as a template so only a real bool matches: a pointer converts to bool, and that
		// is a standard conversion, which beats the user one to a string_view, so a literal
		// would have been boxed as Boolean.TRUE
		template <std::same_as<bool> B>
		static object jaba_make(B value);
	JABA_CLASS_END(object)

	template <class Self>
	object object_members<Self>::jaba_make(const std::string_view utf8)
	{
		return object{ detail::new_jstring(utf8) };
	}

	template <class Self>
	template <std::same_as<bool> B>
	object object_members<Self>::jaba_make(const B value)
	{
		return jaba_make(static_cast<jboolean>(value));
	}

	JABA_CLASS_BEGIN(class_, "java/lang/Class", object)
		JABA_METHOD(string, get_name, "getName")
		JABA_METHOD(string, get_simple_name, "getSimpleName")
		JABA_METHOD(jboolean, is_instance, "isInstance", object)
		JABA_METHOD(jboolean, is_interface, "isInterface")
		JABA_METHOD(class_, get_superclass, "getSuperclass")
	JABA_CLASS_END(class_)

	JABA_CLASS_BEGIN(char_sequence, "java/lang/CharSequence", object)
		JABA_METHOD(jint, length, "length")
		JABA_METHOD(jchar, char_at, "charAt", jint)
		JABA_METHOD(char_sequence, sub_sequence, "subSequence", jint, jint)

		// a literal where a CharSequence is asked for, a java string is one. Defined below, it
		// returns the class itself and that isn't complete yet
		static char_sequence jaba_make(std::string_view utf8);
	JABA_CLASS_END(char_sequence)

	template <class Self>
	char_sequence char_sequence_members<Self>::jaba_make(const std::string_view utf8)
	{
		return char_sequence{ detail::new_jstring(utf8) };
	}

	JABA_CLASS_BEGIN(string, "java/lang/String", char_sequence)
		JABA_METHOD(jboolean, is_empty, "isEmpty")
		JABA_METHOD(string, substring, "substring", jint)
		JABA_METHOD(string, substring_range, "substring", jint, jint)
		JABA_METHOD(jint, index_of, "indexOf", string)
		JABA_METHOD(jint, last_index_of, "lastIndexOf", string)
		JABA_METHOD(jboolean, contains, "contains", char_sequence)
		JABA_METHOD(jboolean, starts_with, "startsWith", string)
		JABA_METHOD(jboolean, ends_with, "endsWith", string)
		JABA_METHOD(jboolean, equals_ignore_case, "equalsIgnoreCase", string)
		JABA_METHOD(jint, compare_to, "compareTo", string)
		JABA_METHOD(string, to_lower_case, "toLowerCase")
		JABA_METHOD(string, to_upper_case, "toUpperCase")
		JABA_METHOD(string, trim, "trim")
		JABA_METHOD(string, concat, "concat", string)
		JABA_METHOD(string, replace, "replace", char_sequence, char_sequence)
		JABA_STATIC_METHOD(string, value_of, "valueOf", object)

		// string s = "text", also what lets a literal go where a string is asked for
		static jaba_self jaba_make(const std::string_view utf8)
		{
			return jaba_self{ detail::new_jstring(utf8) };
		}

		// utf8, an empty string for null
		[[nodiscard]] std::string to_std() const
		{
			return detail::from_jstring(jni::env(), static_cast<jstring>(static_cast<const jaba_self&>(*this).obj()));
		}

		[[nodiscard]] operator std::string() const
		{
			return to_std();
		}

		// a < b, a == b ... by compareTo
		[[nodiscard]] friend std::strong_ordering operator<=>(const jaba_self& a, const jaba_self& b)
		{
			return detail::compare_objects(a, b);
		}

		// str == "text": the text is the same, null is never equal to any
		template <class T>
			requires detail::text_type<T>
		[[nodiscard]] friend bool operator==(const jaba_self& a, const T& text)
		{
			return a.obj() && a.to_std() == std::string_view{ text };
		}

		// a + b and the rest: java's concatenation. The result is a new java string, and a
		// string that is null counts as "null". Anything that isn't text goes through
		// String.valueOf, a number the way java prints it (1.0 is "1.0", a char is itself)
		[[nodiscard]] friend jaba_self operator+(const jaba_self& a, const jaba_self& b)
		{
			return detail::concat_strings(a, b);
		}

		template <class T>
			requires detail::concat_operand_type<T>
		[[nodiscard]] friend jaba_self operator+(const jaba_self& a, const T& b)
		{
			return detail::concat_strings(a, detail::concat_operand(b));
		}

		template <class T>
			requires detail::concat_operand_type<T>
		[[nodiscard]] friend jaba_self operator+(const T& a, const jaba_self& b)
		{
			return detail::concat_strings(detail::concat_operand(a), b);
		}

		// str += x makes a new string and takes it, it doesn't change the java one
		template <class T>
			requires (detail::concat_operand_type<T> || std::is_same_v<T, jaba_self>)
		jaba_self& operator+=(const T& rhs)
		{
			auto& self = static_cast<jaba_self&>(*this);
			self = self + rhs;
			return self;
		}
	JABA_CLASS_END(string)

	template <class Self>
	std::string object_members<Self>::to_std() const
	{
		return detail::object_text(static_cast<const Self&>(*this));
	}
}

namespace jaba::inline lang::detail
{
	[[nodiscard]] inline string concat_strings(const string& a, const string& b)
	{
		// java concatenation prints a null as "null", and concat() itself throws on one
		const string left = a.obj() ? a : string{ "null" };
		const string right = b.obj() ? b : string{ "null" };

		return left.concat(right);
	}

	template <class T>
	[[nodiscard]] string concat_operand(const T& value)
	{
		using U = std::remove_cvref_t<T>;

		if constexpr (std::is_same_v<U, char>)
			return string::value_of(object{ static_cast<jchar>(static_cast<unsigned char>(value)) });
		else if constexpr (std::is_same_v<U, bool>)
			return string::value_of(object{ value });
		else if constexpr (std::is_floating_point_v<U>)
		{
			if constexpr (std::is_same_v<U, float>)
				return string::value_of(object{ static_cast<jfloat>(value) });
			else
				return string::value_of(object{ static_cast<jdouble>(value) });
		}
		else if constexpr (std::is_arithmetic_v<U>)
		{
			// integers keep their width the way java's would, the exact types box
			if constexpr (sizeof(U) < sizeof(jint) || (sizeof(U) == sizeof(jint) && std::is_signed_v<U>))
				return string::value_of(object{ static_cast<jint>(value) });
			else
				return string::value_of(object{ static_cast<jlong>(value) });
		}
		else if constexpr (std::is_convertible_v<const T&, std::string_view>)
		{
			// a null char* is java's null
			if constexpr (std::is_pointer_v<U>)
			{
				if (!value)
					return string{ "null" };
			}

			return string{ std::string_view{ value } };
		}
		else
			return string::value_of(value);
	}

	template <class T>
	[[nodiscard]] std::string object_text(const T& value)
	{
		if (!value.obj())
			return "null";

		if constexpr (std::is_same_v<T, string>)
			return value.to_std();
		else
			return string::value_of(value).to_std();
	}
}

// the sdk's classes hash by hashCode, which goes with the == above, so they work as the key of
// an unordered_map, and format as their toString
namespace std
{
	template <class T>
		requires jaba::lang::sdk_object<T>
	struct hash<T>
	{
		[[nodiscard]] std::size_t operator()(const T& value) const
		{
			return value.obj() ? static_cast<std::size_t>(static_cast<std::uint32_t>(value.hash_code())) : 0;
		}
	};

	template <class T>
		requires jaba::lang::sdk_object<T>
	struct formatter<T, char> : formatter<std::string, char>
	{
		template <class FormatContext>
		auto format(const T& value, FormatContext& ctx) const
		{
			return formatter<std::string, char>::format(jaba::lang::detail::object_text(value), ctx);
		}
	};
}
