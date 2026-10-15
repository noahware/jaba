#pragma once
#include <jni/jni.h>

#include "field.hpp"
#include "type.hpp"

namespace jaba
{
	class nmethod
	{
		
	};

	class method : public jvm_type<"Method">
	{
	public:
		static method* from(const jmethodID id) noexcept
		{
			return *reinterpret_cast<method**>(id);
		}

		JABA_JVM_FIELD(_code, code, nmethod*)
	};
}
