#include "field.hpp"
#include "object.hpp"
#include "env.hpp"
#include "types.hpp"

namespace jaba
{
	namespace
	{
		template <class T>
		struct accessor;

#define JABA_FIELD_ACCESSOR(type, name) \
		template <> \
		struct accessor<type> \
		{ \
			static type get(JNIEnv* env, const jobject obj, const jfieldID id, const bool is_static) noexcept \
			{ \
				return is_static ? env->GetStatic##name##Field(static_cast<jclass>(obj), id) : env->Get##name##Field(obj, id); \
			} \
			static void set(JNIEnv* env, const jobject obj, const jfieldID id, const bool is_static, const type val) noexcept \
			{ \
				if (is_static) \
					env->SetStatic##name##Field(static_cast<jclass>(obj), id, val); \
				else \
					env->Set##name##Field(obj, id, val); \
			} \
		};

		JABA_FOREACH_JNI_TYPE(JABA_FIELD_ACCESSOR)

#undef JABA_FIELD_ACCESSOR
	}

	template <class T>
	T field<T>::get(const object obj) const noexcept
	{
		return accessor<T>::get(jni::env(), obj.obj(), obj_, is_static_);
	}

	template <class T>
	void field<T>::set(const object obj, const T val) noexcept
	{
		accessor<T>::set(jni::env(), obj.obj(), obj_, is_static_, val);
	}

#define JABA_INSTANTIATE_FIELD(type, name) template class field<type>;
	JABA_FOREACH_JNI_TYPE(JABA_INSTANTIATE_FIELD)
#undef JABA_INSTANTIATE_FIELD
}
