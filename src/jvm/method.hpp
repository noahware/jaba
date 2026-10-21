#pragma once
#include "field.hpp"
#include "type.hpp"
#include "version.hpp"
#include <jni/jni.h>
#include <algorithm>

namespace jaba::jvm
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
		// _flags is not exported. from jdk 20 on it is a 4 byte MethodFlags right before _intrinsic_id, with
		// _access_flags and _vtable_index (in either order) in front of it. before that it is a byte that
		// follows _intrinsic_id
		JABA_JVM_FIELD_OR(_flags, flags, std::uint8_t,
			wide_flags()
				? field_offset("_intrinsic_id") - sizeof(std::uint32_t)
				: field_offset("_intrinsic_id") + 1)

		// MethodFlags bits, the compile and inline flags moved in there from AccessFlags
		enum wide_flag_bits : std::uint32_t
		{
			wf_not_c2_compilable = 1u << 8,
			wf_not_c1_compilable = 1u << 9,
			wf_not_c2_osr_compilable = 1u << 10,
			wf_force_inline = 1u << 11,
			wf_dont_inline = 1u << 12,
		};

		[[nodiscard]] static bool wide_flags()
		{
			static const bool wide = field_offset("_intrinsic_id") -
				(std::max(field_offset("_vtable_index"), field_offset("_access_flags")) + sizeof(std::int32_t)) >= sizeof(std::uint32_t);

			return wide;
		}

		// all of the flags, flags() is only the first byte of them
		[[nodiscard]] std::uint32_t flag_bits() const
		{
			if (!wide_flags())
				return flags();

			flags();

			return *reinterpret_cast<const std::uint32_t*>(reinterpret_cast<const std::uint8_t*>(this) + *flags_offset);
		}

		void set_flag_bits(const std::uint32_t bits)
		{
			if (!wide_flags())
			{
				set_flags(static_cast<std::uint8_t>(bits));

				return;
			}

			flags();

			*reinterpret_cast<std::uint32_t*>(reinterpret_cast<std::uint8_t*>(this) + *flags_offset) = bits;
		}

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

			if (wide_flags())
				set_flag_bits(flag_bits() | wf_not_c2_compilable | wf_not_c1_compilable | wf_not_c2_osr_compilable);
		}

		void make_no_inline()
		{
			if (wide_flags())
			{
				set_flag_bits((flag_bits() | wf_dont_inline) & ~wf_force_inline);

				return;
			}

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
