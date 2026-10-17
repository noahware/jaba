#pragma once
#include "field.hpp"
#include "clazz.hpp"
#include "env.hpp"
#include "signature.hpp"

namespace jaba
{
	template <class ClassT, fixed_string Mapping, class ValT>
	jfieldID field<ClassT, Mapping, ValT>::native()
	{
		static const jfieldID id = [&]
		{
			const jclass cls = ClassT::native();
			const zstring_view name = mapping();
			const std::string desc = sig::form<ValT>();

			return jni::env()->GetFieldID(cls, name.data(), desc.c_str());
		}();
		return id;
	}
}
