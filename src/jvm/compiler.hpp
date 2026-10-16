#pragma once
#include <cstdint>
#include <jni/jni.h>

#include "method.hpp"

namespace jaba
{
	class abstract_interpreter
	{
	public:
		static bool init(JNIEnv* const env)
		{
			const jclass object = env->FindClass("java/lang/Object");
			const jmethodID hash_code = object ? env->GetMethodID(object, "hashCode", "()I") : nullptr;

			if (!hash_code)
				return false;

			native_entry = static_cast<std::uint8_t*>(method::from(hash_code)->i2i_entry());

			return native_entry;
		}

		// hidden so the linux stub can jmp through it pc relative from inside a shared object
#ifndef _WIN32
		__attribute__((visibility("hidden")))
#endif
		static std::uint8_t* native_entry;
	};
}
