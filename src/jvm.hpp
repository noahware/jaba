#pragma once
#include <string_view>
#include <cstdint>

#ifdef _JAVAHOOK_USE_JNI
#include "../tests/jni/jni.h"
#endif

namespace jvm
{
	void set_up();
}

#define JVM_FIELD_ACCESSORS(field_name, type)\
	[[nodiscard]] type field_name##() const \
	{\
		return *reinterpret_cast<type*>(reinterpret_cast<std::uint64_t>(this) + field_name##_offset);\
	}\
	\
	void set_##field_name##(type __new_##field_name)\
	{\
		*reinterpret_cast<type*>(reinterpret_cast<std::uint8_t*>(this) + field_name##_offset) = __new_##field_name;\
	}\

class method_counters_t
{
public:
	static std::uint64_t invocation_counter_offset;
	static std::uint64_t backedge_counter_offset;

	JVM_FIELD_ACCESSORS(invocation_counter, std::uint32_t)
	JVM_FIELD_ACCESSORS(backedge_counter, std::uint32_t)
};

class method_data_t
{
public:
	static std::uint64_t invocation_counter_offset;
	static std::uint64_t backedge_counter_offset;
	static std::uint64_t arg_stack_offset;
	static std::uint64_t arg_returned_offset;

	JVM_FIELD_ACCESSORS(invocation_counter, std::uint32_t)
	JVM_FIELD_ACCESSORS(backedge_counter, std::uint32_t)
};

// 'marked' deoptimization is moved to CompiledMethod in later jdk versions
class nmethod_t
{
public:
	static std::uint64_t marked_for_deoptimization_offset;
	static std::uint64_t state_offset;

	enum state_t : std::uint8_t
	{
		in_use = 0,
		not_entrant = 1,
		zombie = 2,
		unloaded = 3
	};

	JVM_FIELD_ACCESSORS(marked_for_deoptimization, std::uint8_t)
	JVM_FIELD_ACCESSORS(state, std::uint8_t)
};

class compiled_method_t
{
public:
	static std::uint64_t mark_for_deoptimization_status_offset;

	enum state_t : std::uint8_t
	{
		in_use = 0,
		not_used = 1,
		not_entrant = 2,
		zombie = 3,
		unloaded = 4
	};

	enum deoptimization_status_t : std::int32_t
	{
		not_marked,
		deoptimize,
		deoptimize_noupdate
	};

	JVM_FIELD_ACCESSORS(mark_for_deoptimization_status, deoptimization_status_t)
};

class method_t
{
public:
	static std::uint64_t class_size;

	static std::uint64_t i2i_entry_offset;
	static std::uint64_t from_interpreted_entry_offset;
	static std::uint64_t from_compiled_entry_offset;
	static std::uint64_t access_flags_offset;
	static std::uint64_t intrinsic_id_offset;
	static std::uint64_t flags_offset;
	static std::uint64_t vtable_index_offset;
	static std::uint64_t code_offset;
	static std::uint64_t method_counters_offset;
	static std::uint64_t method_data_offset;

	JVM_FIELD_ACCESSORS(i2i_entry, std::uint8_t*)
	JVM_FIELD_ACCESSORS(from_interpreted_entry, std::uint8_t*)
	JVM_FIELD_ACCESSORS(from_compiled_entry, std::uint8_t*)
	JVM_FIELD_ACCESSORS(access_flags, std::int32_t)
	JVM_FIELD_ACCESSORS(intrinsic_id, std::int8_t)
	JVM_FIELD_ACCESSORS(flags, std::uint16_t)
	JVM_FIELD_ACCESSORS(vtable_index, std::int32_t)
	JVM_FIELD_ACCESSORS(code, void*)
	JVM_FIELD_ACCESSORS(method_counters, method_counters_t*)
	JVM_FIELD_ACCESSORS(method_data, method_data_t*)

	void set_no_inline()
	{
		std::uint16_t method_flags = flags();

		// todo: endianness fix here
		if (nmethod_t::marked_for_deoptimization_offset != 0) // <= jdk8
		{
			struct flags_t
			{
				std::uint8_t jfr_towrite : 1;
				std::uint8_t caller_sensitive : 1;
				std::uint8_t force_inline : 1;
				std::uint8_t hidden : 1;
				std::uint8_t dont_inline : 1;
				std::uint8_t : 3;
			};

			const auto flags_ptr = reinterpret_cast<flags_t*>(&method_flags);

			flags_ptr->dont_inline = 1;
			flags_ptr->force_inline = 0;
		}
		else
		{
			struct flags_t
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

			const auto flags_ptr = reinterpret_cast<flags_t*>(&method_flags);

			flags_ptr->dont_inline = 1;
			flags_ptr->force_inline = 0;
		}

		set_flags(method_flags);
	}

	void decompile()
	{
		const auto nmethod = static_cast<nmethod_t*>(code());

		if (nmethod == nullptr)
		{
			return;
		}

		if (nmethod_t::marked_for_deoptimization_offset != 0) // <= jdk8
		{
			nmethod->set_marked_for_deoptimization(1);
			nmethod->set_state(nmethod_t::state_t::not_entrant);
		}
		else
		{
			const auto compiled_method = reinterpret_cast<compiled_method_t*>(nmethod);

			nmethod->set_state(compiled_method_t::state_t::not_entrant);
			compiled_method->set_mark_for_deoptimization_status(compiled_method_t::deoptimization_status_t::deoptimize);
		}

		reset_counters();
		clear_code();
	}

	void reset_counters() const
	{
		const auto method_counter = method_counters();

		if (method_counter)
		{
			method_counter->set_invocation_counter(0);
			method_counter->set_backedge_counter(0);
		}

		const auto data = method_data();

		if (data)
		{
			data->set_invocation_counter(0);
			data->set_backedge_counter(0);
		}
	}

	void clear_code()
	{
		std::uint8_t* const new_from_interpreted_entry = i2i_entry();

		set_from_interpreted_entry(new_from_interpreted_entry);

		set_code(nullptr);
	}
};

class java_thread_t
{
public:
	enum state_t : std::int32_t
	{
		thread_uninitialized = 0,
		thread_new = 2,
		thread_new_trans = 3,
		thread_in_native = 4,
		thread_in_native_trans = 5,
		thread_in_vm = 6,
		thread_in_vm_trans = 7,
		thread_in_java = 8,
		thread_in_java_trans = 9,
		thread_blocked = 10,
		thread_blocked_trans = 11,
		thread_max_state = 12
	};

	static std::uint64_t state_offset;
#ifdef _JAVAHOOK_USE_JNI
	static std::uint64_t anchor_offset;
	static std::uint64_t jni_environment_offset;

	JNIEnv* jni_environment()
	{
		return reinterpret_cast<JNIEnv*>(reinterpret_cast<std::uint8_t*>(this) + jni_environment_offset);
	}
#endif

	JVM_FIELD_ACCESSORS(state, state_t)
};

class argument_parser_t
{
public:
	template <class t>
	t get_argument(const std::uint64_t i)
	{
		return reinterpret_cast<t>(reinterpret_cast<void**>(this) + i);
	}
};

class vm_struct_entry_t
{
public:
	[[nodiscard]] std::string_view type_name() const
	{
		if (!type_name_)
		{
			return { };
		}

		return type_name_;
	}

	[[nodiscard]] std::string_view field_name() const
	{
		if (!field_name_)
		{
			return { };
		}

		return field_name_;
	}

	[[nodiscard]] std::string_view type_string() const
	{
		if (!type_string_)
		{
			return { };
		}

		return type_string_;
	}

	[[nodiscard]] std::int32_t is_static() const
	{
		return is_static_;
	}

	[[nodiscard]] std::uint64_t offset() const
	{
		return offset_;
	}

	[[nodiscard]] std::uint8_t* address() const
	{
		return address_;
	}

protected:
	const char* type_name_ = nullptr;
	const char* field_name_ = nullptr;
	const char* type_string_ = nullptr;
	std::int32_t is_static_ = 0;
	std::uint64_t offset_ = 0;
	std::uint8_t* address_ = nullptr;
};

class vm_type_entry_t
{
public:
	[[nodiscard]] std::string_view type_name() const
	{
		if (!type_name_)
		{
			return { };
		}

		return type_name_;
	}

	[[nodiscard]] std::string_view super_class_name() const
	{
		if (!super_class_name_)
		{
			return { };
		}

		return super_class_name_;
	}

	[[nodiscard]] std::int32_t is_oop_type() const
	{
		return is_oop_type_;
	}

	[[nodiscard]] std::int32_t is_integer_type() const
	{
		return is_integer_type_;
	}

	[[nodiscard]] std::int32_t is_unsigned() const
	{
		return is_unsigned_;
	}

	[[nodiscard]] std::uint64_t size() const
	{
		return size_;
	}

protected:
	const char* type_name_ = nullptr;
	const char* super_class_name_ = nullptr;
	std::int32_t is_oop_type_ = 0;
	std::int32_t is_integer_type_ = 0;
	std::int32_t is_unsigned_ = 0;
	std::uint64_t size_ = 0;
};
