#pragma once
#include <cstddef>
#include <string>
#include <string_view>
#include <jni/jni.h>
#include "env.hpp"

namespace jaba::jni::detail
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

	// utf8, an empty string for null. The ref is left alone
	[[nodiscard]] inline std::string from_jstring(JNIEnv* const env, const jstring str)
	{
		if (!str)
			return { };

		const jsize count = env->GetStringLength(str);

		std::u16string utf16(static_cast<std::size_t>(count), u'\0');
		env->GetStringRegion(str, 0, count, reinterpret_cast<jchar*>(utf16.data()));

		return to_utf8(utf16.data(), utf16.size());
	}
}
