#pragma once
#include <functional>
#include <string_view>
#include <jni/jni.h>

namespace jaba
{
	class clazz;
	using find_class_fn = clazz(*)(std::string_view);

	extern find_class_fn find_class;

	void set_find_class_fn(find_class_fn fn);

}
