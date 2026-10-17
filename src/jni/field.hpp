#pragma once
#include <jni/jni.h>
#include "../util/fixed_string.hpp"
#include "object.hpp"
#include "wrap.hpp"

namespace jaba
{
	class object;

	template <class ClassT, fixed_string Mapping, class ValT, bool Static = false>
	class field : public mapped<Mapping>, public object
	{
	public:
		using mapped<Mapping>::mapping;
		using object::object;

		static jfieldID native();

		[[nodiscard]] ValT get() const;
		void set(ValT val);

		[[nodiscard]] operator ValT() const
		{
			return get();
		}

		field& operator=(ValT val)
		{
			set(val);
			return *this;
		}
	};

	template <class ClassT, fixed_string Mapping, class ValT>
	using static_field = field<ClassT, Mapping, ValT, true>;
}

#include "field.inl"
