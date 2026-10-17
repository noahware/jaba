#pragma once
#include "field.hpp"
#include "clazz.hpp"

namespace jaba
{
	template <class_type ClassT, fixed_string Mapping, class ValT>
	jfieldID field<ClassT, Mapping, ValT>::resolve()
	{
		static const jfieldID id = [&]
		{
			const jclass cls = ClassT::resolve();
			const auto name = mapped<Mapping>::mapping();
			return jni::env()->GetFieldID(cls, name.data(), sig.data());
		}();
		return id;
	}
}
