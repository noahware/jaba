#pragma once
#include "field.hpp"
#include "clazz.hpp"
#include "env.hpp"
#include "signature.hpp"
#include "types.hpp"

namespace jaba
{
	template <class ValT, class ClassT, fixed_string Mapping, class Loc>
	jfieldID field<ValT, ClassT, Mapping, Loc>::native()
	{
		// a failed lookup throws out of the initializer, so it's retried next call
		static const jfieldID id = []
		{
			const zstring_view name = mapping();
			const std::string desc = sig::form<ValT>();

			JNIEnv* const env = jni::env();

			const jfieldID fid = is_static
				? env->GetStaticFieldID(cls(), name.data(), desc.c_str())
				: env->GetFieldID(cls(), name.data(), desc.c_str());

			if (!fid)
			{
				env->ExceptionClear();
				throw std::runtime_error(std::format("unable to find field '{}' in '{}'", Mapping.view(), std::string_view{ ClassT::mapping() }));
			}

			return fid;
		}();

		return id;
	}

	template <class ValT, class ClassT, fixed_string Mapping, class Loc>
	ValT field<ValT, ClassT, Mapping, Loc>::get() const
	{
		JNIEnv* const env = jni::env();
		const jfieldID id = native();

#define JABA_GET(type, name) \
		if constexpr (is_jni_type<ValT, type>) \
		{ \
			if constexpr (is_static) \
				return detail::wrap<ValT>(env->GetStatic##name##Field(cls(), id)); \
			else \
				return detail::wrap<ValT>(env->Get##name##Field(self(this), id)); \
		}
		JABA_FOREACH_JNI_TYPE_EX(JABA_GET)
#undef JABA_GET
	}

	template <class ValT, class ClassT, fixed_string Mapping, class Loc>
	void field<ValT, ClassT, Mapping, Loc>::set(const ValT& val)
	{
		JNIEnv* const env = jni::env();
		const jfieldID id = native();

#define JABA_SET(type, name) \
		if constexpr (is_jni_type<ValT, type>) \
		{ \
			if constexpr (is_static) \
				env->SetStatic##name##Field(cls(), id, detail::unwrap(val)); \
			else \
				env->Set##name##Field(self(this), id, detail::unwrap(val)); \
		}
		JABA_FOREACH_JNI_TYPE_EX(JABA_SET)
#undef JABA_SET
	}
}
