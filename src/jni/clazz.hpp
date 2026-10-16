#pragma once
#include <functional>
#include <string_view>
#include <jni/jni.h>
#include "wrap.hpp"

namespace jaba
{
	class zstring_view;
	class clazz;
	using find_class_fn = clazz(*)(zstring_view);

	extern find_class_fn find_class;

	void set_find_class_fn(find_class_fn fn);

	class clazz : public jni_wrap<jclass>
	{
	public:
		using jni_wrap::jni_wrap;
	};
}
