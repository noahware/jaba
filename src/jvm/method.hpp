#pragma once
#include "field.hpp"
#include "type.hpp"
#include "version.hpp"
#include <jni/jni.h>

namespace jaba
{
	class nmethod : public jvm_type<"nmethod">
	{
	public:

	};

	class method : public jvm_type<"Method">
	{
	public:
		struct pre_jdk9_flags
		{
			std::uint8_t jfr_towrite : 1;
			std::uint8_t caller_sensitive : 1;
			std::uint8_t force_inline : 1;
			std::uint8_t hidden : 1;
			std::uint8_t dont_inline : 1;
			std::uint8_t : 3;
		};

		struct post_jdk9_flags
		{
			std::uint8_t caller_sensitive : 1;
			std::uint8_t force_inline : 1;
			std::uint8_t dont_inline : 1;
			std::uint8_t hidden : 1;
			std::uint8_t has_injected_profile : 1;
			std::uint8_t intrinsic_candidate : 1;
			std::uint8_t reserved_stack_access : 1;
			std::uint8_t scoped : 1;
		};

		enum access_flags : std::uint32_t
		{
			af_native = 0x100,
			af_interpreted = 0xE000000,
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
		JABA_JVM_FIELD_OR(_flags, flags, std::uint8_t, field_offset("_intrinsic_id") + 1)

		void make_native()
		{
			std::uint32_t af = acc_flags();

			af |= af_native;

			set_acc_flags(static_cast<access_flags>(af));
		}

		void make_interpreted()
		{
			std::uint32_t af = acc_flags();

			af |= af_interpreted;

			set_acc_flags(static_cast<access_flags>(af));
		}

		void make_no_inline()
		{
			std::uint8_t raw_flags = flags();

			if (pre_jdk9())
			{
				const auto fl = reinterpret_cast<pre_jdk9_flags*>(&raw_flags);

				fl->dont_inline = true;
				fl->force_inline = false;
			}
			else
			{
				const auto fl = reinterpret_cast<post_jdk9_flags*>(&raw_flags);

				fl->dont_inline = true;
				fl->force_inline = false;
			}

			set_flags(raw_flags);
		}
	};
}
