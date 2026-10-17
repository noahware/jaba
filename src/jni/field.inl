#pragma once
#include "field.hpp"
#include "clazz.hpp"
#include "env.hpp"

namespace jaba
{
	template <class ClassT, fixed_string Mapping, class ValT>
	jfieldID field<ClassT, Mapping, ValT>::native()
	{
		static const jfieldID id = [&]
		{
			const jclass cls = ClassT::native();
			//const zstring_view name = mapping();
			//std::string sig(name.begin(), name.end());

			return nullptr;//jni::env()->GetFieldID(cls, name.data(), sig.data());
		}();
		return id;
	}
}
