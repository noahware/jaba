#pragma once
#include "field.hpp"
#include "type.hpp"

namespace jaba
{
	class nmethod
	{
		
	};

	class method : jvm_type<"Method">
	{
	public:
		JABA_JVM_FIELD(_code, code, nmethod*)
	};
}
