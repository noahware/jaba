#pragma once
#include <jni/jni.h>
#include "env.hpp"
#include "wrap.hpp"
#include "util/zstring_view.hpp"

namespace jaba
{
	class object;

	template <class_type ClassT, fixed_string Mapping, class ValT>
	class field : public mapped<Mapping>
	{
	public:
		static jfieldID resolve(zstring_view sig, bool is_static);

		// for static fields, obj is the declaring class
		//[[nodiscard]] ValT get(object obj) const noexcept;
		//void set(object obj, ValT val) noexcept;
	};
}

#include "field.inl"
