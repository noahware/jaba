#include "clazz.hpp"
#include "field.hpp"
#include "env.hpp"
#include "types.hpp"

using namespace jaba;

namespace
{
	clazz default_find_class(const zstring_view name)
	{
		return jni::env()->FindClass(name.data());
	}
}

find_class_fn jaba::find_class = default_find_class;

void jaba::set_find_class_fn(const find_class_fn fn)
{
	find_class = fn;
}

template <class T>
field<T> clazz::raw_find_field(const zstring_view name, const zstring_view sig, const bool is_static) const noexcept
{
	JNIEnv* env = jni::env();

	const jfieldID id = is_static
		? env->GetStaticFieldID(obj_, name.data(), sig.data())
		: env->GetFieldID(obj_, name.data(), sig.data());

	return { id, is_static };
}

#define JABA_INSTANTIATE_FIND_FIELD(type, name) \
	template field<type> clazz::raw_find_field<type>(zstring_view, zstring_view, bool) const noexcept;

JABA_FOREACH_JNI_TYPE(JABA_INSTANTIATE_FIND_FIELD)
#undef JABA_INSTANTIATE_FIND_FIELD
