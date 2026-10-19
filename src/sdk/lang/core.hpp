#pragma once
#include <cstddef>
#include <string>
#include <string_view>
#include "jni/jni.hpp"
#include "sdk/fwd.hpp"

namespace jaba::inline lang::detail
{
	// jni's own utf8 is the modified one (no 4 byte sequences, nul as 2 bytes), so strings
	// go through utf16, which is what a java string is anyway. Invalid input becomes U+FFFD
	[[nodiscard]] inline std::u16string to_utf16(const std::string_view utf8)
	{
		std::u16string out;
		out.reserve(utf8.size());

		for (std::size_t i = 0; i < utf8.size();)
		{
			const auto lead = static_cast<unsigned char>(utf8[i]);

			char32_t cp = 0;
			std::size_t len = 0;
			char32_t min = 0;

			if (lead < 0x80)
			{
				cp = lead;
				len = 1;
			}
			else if ((lead >> 5) == 0x6)
			{
				cp = lead & 0x1F;
				len = 2;
				min = 0x80;
			}
			else if ((lead >> 4) == 0xE)
			{
				cp = lead & 0x0F;
				len = 3;
				min = 0x800;
			}
			else if ((lead >> 3) == 0x1E)
			{
				cp = lead & 0x07;
				len = 4;
				min = 0x10000;
			}

			bool valid = len != 0 && i + len <= utf8.size();

			for (std::size_t k = 1; valid && k < len; ++k)
			{
				const auto next = static_cast<unsigned char>(utf8[i + k]);

				if ((next & 0xC0) != 0x80)
					valid = false;
				else
					cp = (cp << 6) | (next & 0x3F);
			}

			// overlong, out of range or a surrogate
			if (valid && len > 1 && (cp < min || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)))
				valid = false;

			if (!valid)
			{
				out += static_cast<char16_t>(0xFFFD);
				++i;
				continue;
			}

			i += len;

			if (cp >= 0x10000)
			{
				cp -= 0x10000;
				out += static_cast<char16_t>(0xD800 + (cp >> 10));
				out += static_cast<char16_t>(0xDC00 + (cp & 0x3FF));
			}
			else
				out += static_cast<char16_t>(cp);
		}

		return out;
	}

	[[nodiscard]] inline std::string to_utf8(const char16_t* const utf16, const std::size_t count)
	{
		std::string out;
		out.reserve(count);

		for (std::size_t i = 0; i < count; ++i)
		{
			char32_t cp = utf16[i];

			if (cp >= 0xD800 && cp <= 0xDBFF && i + 1 < count && utf16[i + 1] >= 0xDC00 && utf16[i + 1] <= 0xDFFF)
			{
				cp = 0x10000 + ((cp - 0xD800) << 10) + (utf16[i + 1] - 0xDC00);
				++i;
			}
			else if (cp >= 0xD800 && cp <= 0xDFFF)
				cp = 0xFFFD;

			if (cp < 0x80)
				out += static_cast<char>(cp);
			else if (cp < 0x800)
			{
				out += static_cast<char>(0xC0 | (cp >> 6));
				out += static_cast<char>(0x80 | (cp & 0x3F));
			}
			else if (cp < 0x10000)
			{
				out += static_cast<char>(0xE0 | (cp >> 12));
				out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
				out += static_cast<char>(0x80 | (cp & 0x3F));
			}
			else
			{
				out += static_cast<char>(0xF0 | (cp >> 18));
				out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
				out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
				out += static_cast<char>(0x80 | (cp & 0x3F));
			}
		}

		return out;
	}

	// a new local ref
	[[nodiscard]] inline jstring new_jstring(const std::string_view utf8)
	{
		const std::u16string utf16 = to_utf16(utf8);

		return jni::env()->NewString(reinterpret_cast<const jchar*>(utf16.data()), static_cast<jsize>(utf16.size()));
	}
}

// Every sdk class sits in one single inheritance tree rooted at object, never a diamond: a
// class only mixes in its bases' members, and the same members twice would be ambiguous
namespace jaba::inline lang
{
	JABA_CLASS_BEGIN(object, "java/lang/Object")
		JABA_METHOD(string, to_string, "toString")
		JABA_METHOD(jint, hash_code, "hashCode")
		JABA_METHOD(jboolean, equals, "equals", object)
		JABA_METHOD(class_, get_class, "getClass")

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
			const auto str = static_cast<jstring>(static_cast<const jaba_self&>(*this).obj());

			if (!str)
				return { };

			JNIEnv* const env = jni::env();
			const jsize count = env->GetStringLength(str);

			std::u16string utf16(static_cast<std::size_t>(count), u'\0');
			env->GetStringRegion(str, 0, count, reinterpret_cast<jchar*>(utf16.data()));

			return detail::to_utf8(utf16.data(), utf16.size());
		}

		[[nodiscard]] operator std::string() const
		{
			return to_std();
		}
	JABA_CLASS_END(string)
}
