#pragma once
#include <jni/jni.h>
#include "../util/fixed_string.hpp"
#include "object.hpp"
#include "wrap.hpp"

namespace jaba
{
	class object;

	template <class ClassT, fixed_string Mapping, class ValT>
	class field : public mapped<Mapping>, public obj_user
	{
	public:
		using mapped<Mapping>::mapping;
		using obj_user::obj_user;

		static jfieldID native();

		// for static fields, obj is the declaring class
		//[[nodiscard]] ValT get(object obj) const noexcept;
		//void set(object obj, ValT val) noexcept;
	};
}

#include "field.inl"
