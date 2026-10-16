#pragma once
#include <jni/jni.h>
#include "../util/zstring_view.hpp"
#include "wrap.hpp"

namespace jaba
{
	class zstring_view;
	class clazz;
	template <class T>
	class field;
	using find_class_fn = clazz(*)(zstring_view);

	extern find_class_fn find_class;

	void set_find_class_fn(find_class_fn fn);

	class clazz : public jni_wrap<jclass>
	{
	public:
		using jni_wrap::jni_wrap;

		template <class T>
		[[nodiscard]] field<T> find_field(const zstring_view name, const zstring_view sig) const noexcept
		{
			return raw_find_field<T>(name, sig, false);
		}

		template <class T>
		[[nodiscard]] field<T> find_static_field(const zstring_view name, const zstring_view sig) const noexcept
		{
			return raw_find_field<T>(name, sig, true);
		}

	protected:
		template <class T>
		[[nodiscard]] field<T> raw_find_field(zstring_view name, zstring_view sig, bool is_static = false) const noexcept;
	};
}
