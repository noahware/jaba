#include "clazz.hpp"
#include "field.hpp"
#include "env.hpp"

using namespace jaba;
using namespace jaba::jni;

namespace
{
	jclass default_find_class(const zstring_view name)
	{
		return jni::env()->FindClass(name.data());
	}
}

find_class_fn jaba::jni::find_class = default_find_class;

void jaba::jni::set_find_class_fn(const find_class_fn fn)
{
	find_class = fn;
}
