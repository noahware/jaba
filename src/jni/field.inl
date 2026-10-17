#pragma once
#include "field.hpp"
#include "clazz.hpp"
#include "env.hpp"

namespace jaba
{
	template <class_type ClassT, fixed_string Mapping, class ValT>
	jfieldID field<ClassT, Mapping, ValT>::native()
	{
		static const jfieldID id = [&]
		{
			const jclass cls = ClassT::resolve();
			const auto name = mapping();
			return jni::env()->GetFieldID(cls, name.data(), nullptr);
		}();
		return id;
	}
}
