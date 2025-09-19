#pragma once
#include <string_view>
#include <cstdint>

#include <print>

#ifdef _JAVAHOOK_USE_JNI
#include "../tests/jni/jni.h"
#endif

class method_t;

namespace jvm
{
	void set_up(std::uint8_t* jvm_code_base, std::uint64_t jvm_code_size);
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


class code_cache_t
{
public:
	typedef void(*make_marked_nmethods_non_entrant_t)();
	typedef void(*mark_all_nmethods_for_deoptimization_t)();
	typedef void(*mark_all_nmethods_for_evol_deoptimization_t)();

	static make_marked_nmethods_non_entrant_t make_marked_nmethods_non_entrant;
	static mark_all_nmethods_for_deoptimization_t mark_all_nmethods_for_deoptimization;
	static mark_all_nmethods_for_evol_deoptimization_t mark_all_nmethods_for_evol_deoptimization;

	static void deoptimize_method(method_t* method);
};

class abstract_interpreter_t
{
public:
	static std::uint8_t* slow_signature_handler;

	typedef uint8_t*(*method_entry_table_t)[];

	static method_entry_table_t method_entry_table;
	static std::uint8_t* native_entry;

	static void set_native_entry()
	{
		constexpr std::uint64_t native_method_entry = 2;

		native_entry = (*method_entry_table)[native_method_entry];
	}
};

// 'marked' deoptimization is moved to CompiledMethod in later jdk versions
class nmethod_t
{
public:
	static std::uint64_t marked_for_deoptimization_offset;
	static std::uint64_t compilation_level_offset;
	static std::uint64_t state_offset;

	enum state_t : std::uint8_t
	{
		in_use = 0,
		not_entrant = 1,
		zombie = 2,
		unloaded = 3
	};

	JVM_FIELD_ACCESSORS(marked_for_deoptimization, std::uint8_t)
	JVM_FIELD_ACCESSORS(compilation_level, std::int32_t)
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

	JVM_FIELD_ACCESSORS(i2i_entry, std::uint8_t*)
	JVM_FIELD_ACCESSORS(from_interpreted_entry, std::uint8_t*)
	JVM_FIELD_ACCESSORS(from_compiled_entry, std::uint8_t*)
	JVM_FIELD_ACCESSORS(access_flags, std::int32_t)
	JVM_FIELD_ACCESSORS(intrinsic_id, std::int8_t)
	JVM_FIELD_ACCESSORS(flags, std::uint16_t)
	JVM_FIELD_ACCESSORS(vtable_index, std::int32_t)
	JVM_FIELD_ACCESSORS(code, void*)

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

	bool is_compiled()
	{
		const auto nmethod = static_cast<nmethod_t*>(code());

		if (nmethod == nullptr)
		{
			return false;
		}

		return nmethod->compilation_level() != 0;
	}

	void decompile()
	{
		code_cache_t::deoptimize_method(this);
	}

	void disable_compilation()
	{
		const auto current_access_flags = access_flags();

		set_access_flags(current_access_flags | 0xE000000);
	}

	void make_native()
	{
		const auto current_access_flags = access_flags();

		set_access_flags(current_access_flags | 0x100);
	}
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
