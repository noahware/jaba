#include "field.hpp"
#include "object.hpp"
#include "env.hpp"

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

		JABA_FIELD_ACCESSOR(jboolean, Boolean)
		JABA_FIELD_ACCESSOR(jbyte, Byte)
		JABA_FIELD_ACCESSOR(jchar, Char)
		JABA_FIELD_ACCESSOR(jshort, Short)
		JABA_FIELD_ACCESSOR(jint, Int)
		JABA_FIELD_ACCESSOR(jlong, Long)
		JABA_FIELD_ACCESSOR(jfloat, Float)
		JABA_FIELD_ACCESSOR(jdouble, Double)
		JABA_FIELD_ACCESSOR(jobject, Object)

#undef JABA_FIELD_ACCESSOR
	}

	template <class T>
	T field<T>::get(object obj) const noexcept
	{
		return accessor<T>::get(jni::env(), obj.obj(), obj_, is_static_);
	}

	template <class T>
	void field<T>::set(object obj, T val) noexcept
	{
		accessor<T>::set(jni::env(), obj.obj(), obj_, is_static_, val);
	}

	template class field<jboolean>;
	template class field<jbyte>;
	template class field<jchar>;
	template class field<jshort>;
	template class field<jint>;
	template class field<jlong>;
	template class field<jfloat>;
	template class field<jdouble>;
	template class field<jobject>;
}
