#pragma once
#include <format>
#include <string>
#include <type_traits>
#include <jni/jni.h>
#include "../util/fixed_string.hpp"

namespace jaba
{
	template <fixed_string Mapping>
	class clazz;

	namespace sig
	{
		namespace detail
		{
			template <fixed_string M>
			void as_clazz(const clazz<M>*);

			template <class T>
			concept clazz_like = requires(T* p) { as_clazz(p); };
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
	}
}
