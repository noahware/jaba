#pragma once
#include <jni/jni.h>

#include "field.hpp"
#include "type.hpp"

namespace jaba
{
	class nmethod : public jvm_type<"nmethod">
	{
	public:

	};

	class method : public jvm_type<"Method">
	{
	public:
		enum access_flags : std::uint32_t
		{
			af_native = 0x100,
		};

		static method* from(const jmethodID id) noexcept
		{
			return *reinterpret_cast<method**>(id);
		}

		JABA_JVM_FIELD(_code, code, nmethod*)
		JABA_JVM_FIELD(_vtable_index, vtable_idx, std::int32_t)
		JABA_JVM_FIELD(_access_flags, acc_flags, access_flags)
		JABA_JVM_FIELD(_from_interpreted_entry, from_interp_entry, void*)
		JABA_JVM_FIELD(_i2i_entry, i2i_entry, void*)

		void make_native()
		{
			std::uint32_t af = acc_flags();

			af |= af_native;

			set_acc_flags(static_cast<access_flags>(af));
		}
	};
}
