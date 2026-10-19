#pragma once
#include "array.hpp"
#include "env.hpp"
#include "member.hpp"
#include "types.hpp"

namespace jaba::jni
{
	template <class T>
	T array<T>::get(const std::size_t i) const noexcept
	{
		JNIEnv* const env = jni::env();
		const auto idx = static_cast<jsize>(i);

		// jni has no single-element getter for primitives, so read a region of length 1
#define JABA_GET_ARRE(type, name) \
		if constexpr (is_jni_type<T, type>) \
		{ \
			type val{}; \
			env->Get##name##ArrayRegion(reinterpret_cast<type##Array>(obj_), idx, 1, &val); \
			return val; \
		}
		JABA_FOREACH_JNI_TYPE(JABA_GET_ARRE)
#undef JABA_GET_ARRE

		if constexpr (is_jni_type<T, clazz_any>)
			return detail::wrap<T>(env->GetObjectArrayElement(reinterpret_cast<jobjectArray>(obj_), idx));

		return T{};
	}

	template <class T>
	void array<T>::set(const std::size_t i, const T& val) const noexcept
	{
		JNIEnv* const env = jni::env();
		const auto idx = static_cast<jsize>(i);

#define JABA_SET_ARRE(type, name) \
		if constexpr (is_jni_type<T, type>) \
			env->Set##name##ArrayRegion(reinterpret_cast<type##Array>(obj_), idx, 1, &val);
		JABA_FOREACH_JNI_TYPE(JABA_SET_ARRE)
#undef JABA_SET_ARRE

		if constexpr (is_jni_type<T, clazz_any>)
			env->SetObjectArrayElement(reinterpret_cast<jobjectArray>(obj_), idx, detail::unwrap(val));
	}

	template <class T>
	auto array<T>::view() const
	{
		return std::views::iota(std::size_t{ 0 }, size())
			| std::views::transform([this](const std::size_t i) { return get(i); });
	}
}
