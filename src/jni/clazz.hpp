#pragma once
#include <jni/jni.h>

#include "env.hpp"
#include "../util/zstring_view.hpp"
#include "wrap.hpp"

namespace jaba
{
	class zstring_view;
	using find_class_fn = jclass(*)(zstring_view);

	extern find_class_fn find_class;

	void set_find_class_fn(find_class_fn fn);

	template <fixed_string Mapping>
	class clazz : public mapped<Mapping>
	{
	public:
		static jclass resolve()
		{
			return find_class(mapped<Mapping>::mapping());
		}
	};
}
