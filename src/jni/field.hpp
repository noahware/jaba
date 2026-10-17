#pragma once
#include <jni/jni.h>
#include "../util/fixed_string.hpp"
#include "object.hpp"
#include "wrap.hpp"

namespace jaba
{
	class object;

	template <class ClassT, fixed_string Mapping, class ValT, bool Static = false>
	class field : public mapped<Mapping>, public obj_user
	{
	public:
		using mapped<Mapping>::mapping;
		using obj_user::obj_user;

		static jfieldID native();

		[[nodiscard]] ValT get() const noexcept;
		void set(ValT val) noexcept;

		[[nodiscard]] operator ValT() const noexcept
		{
			return get();
		}

		field& operator=(ValT val) noexcept
		{
			set(val);
			return *this;
		}
	};

	template <class ClassT, fixed_string Mapping, class ValT>
	using static_field = field<ClassT, Mapping, ValT, true>;
}

#include "field.inl"
