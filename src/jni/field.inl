#pragma once
#include "field.hpp"
#include "clazz.hpp"
#include "env.hpp"
#include "signature.hpp"
#include "types.hpp"

namespace jaba
{
	namespace detail
	{
		// raw jni value -> ValT (clazz types get wrapped in an object)
		template <class V, class R>
		[[nodiscard]] V wrap(const R raw) noexcept
		{
			if constexpr (std::is_same_v<V, R>)
				return raw;
			else
				return V{ object{ raw } };
		}

		// ValT -> raw jni value
		template <class V>
		[[nodiscard]] auto unwrap(const V& val) noexcept
		{
			if constexpr (std::is_arithmetic_v<V>)
				return val;
			else
				return val.obj().obj();
		}
	}

	template <class ClassT, fixed_string Mapping, class ValT>
	jfieldID field<ClassT, Mapping, ValT>::native()
	{
		static const jfieldID id = [&]
		{
			const jclass cls = ClassT::native();
			const zstring_view name = mapping();
			const std::string desc = sig::form<ValT>();

			return jni::env()->GetFieldID(cls, name.data(), desc.c_str());
		}();
		return id;
	}

	template <class ClassT, fixed_string Mapping, class ValT>
	ValT field<ClassT, Mapping, ValT>::get() const noexcept
	{
		JNIEnv* const env = jni::env();
		const jobject self = obj_.obj();
		const jfieldID id = native();

#define JABA_GET(type, name) \
		if constexpr (is_jni_type<ValT, type>) return detail::wrap<ValT>(env->Get##name##Field(self, id));
		JABA_FOREACH_JNI_TYPE_EX(JABA_GET)
#undef JABA_GET
	}

	template <class ClassT, fixed_string Mapping, class ValT>
	void field<ClassT, Mapping, ValT>::set(ValT val) noexcept
	{
		JNIEnv* const env = jni::env();
		const jobject self = obj_.obj();
		const jfieldID id = native();

#define JABA_SET(type, name) \
		if constexpr (is_jni_type<ValT, type>) env->Set##name##Field(self, id, detail::unwrap(val));
		JABA_FOREACH_JNI_TYPE_EX(JABA_SET)
#undef JABA_SET
	}
}
