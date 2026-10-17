#pragma once
#include <jni/jni.h>
#include "../util/zstring_view.hpp"
#include "field.hpp"
#include "object.hpp"
#include "wrap.hpp"

namespace jaba
{
	class zstring_view;
	using find_class_fn = jclass(*)(zstring_view);

	extern find_class_fn find_class;

	void set_find_class_fn(find_class_fn fn);

	template <fixed_string Mapping>
	class clazz : public mapped<Mapping>, public object
	{
	public:
		using mapped<Mapping>::mapping;
		using object::object;

		static jclass native()
		{
			static const jclass cls = []
				{
					const jclass local = find_class(mapping());
					if (!local)
						throw std::runtime_error(std::format("unable to find class '{}'", Mapping.view()));

					const auto global = static_cast<jclass>(jni::env()->NewGlobalRef(local));
					jni::env()->DeleteLocalRef(local);
					return global;
				}();

			return cls;
		}
	};
}
