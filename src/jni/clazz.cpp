#include "clazz.hpp"
#include "env.hpp"
#include "../util/zstring_view.hpp"

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
