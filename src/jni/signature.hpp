#pragma once
#include <concepts>
#include <format>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <jni/jni.h>
#include "../util/fixed_string.hpp"

namespace jaba
{
	namespace sig
	{
		namespace detail
		{
			// duck typed rather than matching clazz<M, B>, a chain of clazz bases makes deduction ambiguous
			template <class T>
			concept clazz_like = requires
			{
				{ T::native() } -> std::same_as<jclass>;
				{ T::mapping() } -> std::convertible_to<std::string_view>;
			};
		}

		template <class T>
		[[nodiscard]] std::string form()
		{
			if constexpr (detail::clazz_like<T>)
				return std::format("L{};", std::string_view{ T::mapping() });
			else if constexpr (std::is_same_v<T, jboolean>) return "Z";
			else if constexpr (std::is_same_v<T, jbyte>)    return "B";
			else if constexpr (std::is_same_v<T, jchar>)    return "C";
			else if constexpr (std::is_same_v<T, jshort>)   return "S";
			else if constexpr (std::is_same_v<T, jint>)     return "I";
			else if constexpr (std::is_same_v<T, jlong>)    return "J";
			else if constexpr (std::is_same_v<T, jfloat>)   return "F";
			else if constexpr (std::is_same_v<T, jdouble>)  return "D";
			else if constexpr (std::is_void_v<T>)           return "V";
			else static_assert(sizeof(T) == 0, "no jni signature for this type (use a clazz<> type instead of jobject)");

			throw std::runtime_error("unknown type in signature");
		}

		// method descriptor, e.g. <jfloat, jint, entity> -> "(ILa/Entity;)F"
		template <class RetT, class... ArgsT>
		[[nodiscard]] std::string form_method()
		{
			std::string desc = "(";
			((desc += form<ArgsT>()), ...);
			desc += ')';
			desc += form<RetT>();
			return desc;
		}
	}
}
