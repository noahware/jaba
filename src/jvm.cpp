#include "jvm.hpp"

#include <print>

#include "signature.hpp"

code_cache_t::make_marked_nmethods_non_entrant_t code_cache_t::make_marked_nmethods_non_entrant = nullptr;
code_cache_t::mark_all_nmethods_for_deoptimization_t code_cache_t::mark_all_nmethods_for_deoptimization = nullptr;
code_cache_t::mark_all_nmethods_for_evol_deoptimization_t code_cache_t::mark_all_nmethods_for_evol_deoptimization = nullptr;

std::uint8_t* abstract_interpreter_t::slow_signature_handler = nullptr;
abstract_interpreter_t::method_entry_table_t abstract_interpreter_t::method_entry_table = nullptr;
std::uint8_t* abstract_interpreter_t::native_entry = nullptr;

std::uint64_t nmethod_t::marked_for_deoptimization_offset = 0;
std::uint64_t nmethod_t::compilation_level_offset = 0;
std::uint64_t nmethod_t::state_offset = 0;

std::uint64_t compiled_method_t::mark_for_deoptimization_status_offset = 0;

std::uint64_t method_t::class_size = 0;
std::uint64_t method_t::i2i_entry_offset = 0;
std::uint64_t method_t::from_interpreted_entry_offset = 0;
std::uint64_t method_t::from_compiled_entry_offset = 0;
std::uint64_t method_t::access_flags_offset = 0;
std::uint64_t method_t::intrinsic_id_offset = 0;
std::uint64_t method_t::flags_offset = 0;
std::uint64_t method_t::vtable_index_offset = 0;
std::uint64_t method_t::code_offset = 0;

extern "C" __declspec(dllimport) vm_struct_entry_t* gHotSpotVMStructs;
extern "C" __declspec(dllimport) vm_type_entry_t* gHotSpotVMTypes;

const vm_type_entry_t* find_vm_type_entry(const std::string_view type_name)
{
	const vm_type_entry_t* current_entry = gHotSpotVMTypes;

	while (!current_entry->type_name().empty())
	{
		if (current_entry->type_name() == type_name)
		{
			return current_entry;
		}

		++current_entry;
	}

	return nullptr;
}

const vm_struct_entry_t* find_vm_struct_entry(const std::string_view type_name, const std::string_view field_name)
{
	const vm_struct_entry_t* current_entry = gHotSpotVMStructs;

	while (!current_entry->type_name().empty())
	{
		if (current_entry->type_name() == type_name && current_entry->field_name() == field_name)
		{
			return current_entry;
		}

		++current_entry;
	}

	return nullptr;
}

std::uint64_t get_vm_struct_entry_offset(const std::string_view type_name, const std::string_view field_name)
{
	const vm_struct_entry_t* const current_entry = find_vm_struct_entry(type_name, field_name);

	if (current_entry == nullptr)
	{
		return 0;
	}

	return current_entry->offset();
}

std::uint64_t get_vm_type_entry_size(const std::string_view type_name)
{
	const vm_type_entry_t* const current_entry = find_vm_type_entry(type_name);

	if (current_entry == nullptr)
	{
		return 0;
	}

	return current_entry->size();
}

std::uint8_t* find_mark_all_nmethods_for_deoptimization(const std::uint8_t* const make_marked_nmethods_non_entrant_xref)
{
	const auto brother_function = reinterpret_cast<std::uint8_t*>(code_cache_t::make_marked_nmethods_non_entrant);
	const auto brother_function_xref = reinterpret_cast<const std::uint8_t*>(make_marked_nmethods_non_entrant_xref);
	const std::uint8_t* current = brother_function_xref;

	for (std::uint64_t i = 0; i < 0x200; i++, current--)
	{
		if (*current == 0xE8) // is a relative call
		{
			std::uint8_t* const call_destination = signature::resolve_rva(current, 1, 5);

			const std::int64_t difference = call_destination - brother_function;

			if (std::abs(difference) < 0x1000)
			{
				return call_destination;
			}
		}
	}

	return nullptr;
}

void set_up_code_cache(std::uint8_t* const jvm_code_base, const std::uint64_t jvm_code_size)
{
	// post jdk 13
	code_cache_t::mark_all_nmethods_for_evol_deoptimization = reinterpret_cast<code_cache_t::mark_all_nmethods_for_evol_deoptimization_t>(
		signature::rva_scan(jvm_code_base, jvm_code_size,
			"\x74\x01\x48\x8D\x0D\x01\x01\x01\x01\xE8\x01\x01\x01\x01\xE8\x01\x01\x01\x01\xEB",
			"x?xxx????x????x????x",
			15,
			19
		)
	);

	if (!code_cache_t::mark_all_nmethods_for_evol_deoptimization) // alternative post jdk 13
	{
		code_cache_t::mark_all_nmethods_for_evol_deoptimization = reinterpret_cast<code_cache_t::mark_all_nmethods_for_evol_deoptimization_t>(
			signature::rva_scan(jvm_code_base, jvm_code_size,
				"\x75\x01\xE8\x01\x01\x01\x01\x48\x8B\x05\x01\x01\x01\x01\x48\x85\xC0",
				"x?x????xx????xx",
				3,
				7
			)
		);

		if (!code_cache_t::mark_all_nmethods_for_evol_deoptimization) // pre jdk 13
		{
			const std::uint8_t* const make_marked_nmethods_non_entrant_xref = signature::scan(jvm_code_base, jvm_code_size,
				"\xC6\x05\x01\x01\x01\x01\x01\xE8\x01\x01\x01\x01\xE8\x01\x01\x01\x01\xC6\x05\x01\x01\x01\x01\x01\xC6\x05",
				"xx?????x????x????xx?????xx"
			);

			code_cache_t::make_marked_nmethods_non_entrant = reinterpret_cast<code_cache_t::make_marked_nmethods_non_entrant_t>(
				signature::resolve_rva(make_marked_nmethods_non_entrant_xref, 13, 17)
			);

			std::uint8_t* const call_destination = find_mark_all_nmethods_for_deoptimization(make_marked_nmethods_non_entrant_xref);

			code_cache_t::mark_all_nmethods_for_deoptimization = reinterpret_cast<code_cache_t::mark_all_nmethods_for_deoptimization_t>(call_destination);
		}
	}
}

void set_up_abstract_interpreter(std::uint8_t* const jvm_code_base, const std::uint64_t jvm_code_size)
{
	abstract_interpreter_t::slow_signature_handler = *reinterpret_cast<std::uint8_t**>(signature::rva_scan(jvm_code_base, jvm_code_size,
			"\x48\x8B\x15\x01\x01\x01\x01\xE9\x01\x01\x01\x01\x48\x8B\x15\x01\x01\x01\x01\xE8",
			"xxx????x????xxx????x",
			3,
			7
		)
	);

	abstract_interpreter_t::method_entry_table = reinterpret_cast<abstract_interpreter_t::method_entry_table_t>(signature::rva_scan(jvm_code_base, jvm_code_size,
			"\x48\x8D\x0D\x01\x01\x01\x01\x48\x89\x14\xC1",
			"xxx????xxxx",
			3,
			7
		)
	);

	abstract_interpreter_t::set_native_entry();
}

void set_up_method()
{
	constexpr std::string_view method = "Method";

	method_t::class_size = get_vm_type_entry_size(method);
	method_t::i2i_entry_offset = get_vm_struct_entry_offset(method, "_i2i_entry");
	method_t::from_interpreted_entry_offset = get_vm_struct_entry_offset(method, "_from_interpreted_entry");
	method_t::from_compiled_entry_offset = get_vm_struct_entry_offset(method, "_from_compiled_entry");
	method_t::access_flags_offset = get_vm_struct_entry_offset(method, "_access_flags");

	method_t::intrinsic_id_offset = get_vm_struct_entry_offset(method, "_intrinsic_id");
	method_t::flags_offset = get_vm_struct_entry_offset(method, "_flags");
	method_t::vtable_index_offset = get_vm_struct_entry_offset(method, "_vtable_index");
	method_t::code_offset = get_vm_struct_entry_offset(method, "_code");

	if (!method_t::flags_offset)
	{
		method_t::flags_offset = method_t::intrinsic_id_offset + 1;
	}
}

void set_up_nmethod()
{
	constexpr std::string_view nmethod = "nmethod";

	nmethod_t::marked_for_deoptimization_offset = get_vm_struct_entry_offset(nmethod, "_marked_for_deoptimization");
	nmethod_t::compilation_level_offset = get_vm_struct_entry_offset(nmethod, "_comp_level");
	nmethod_t::state_offset = get_vm_struct_entry_offset(nmethod, "_state");

	if (!nmethod_t::marked_for_deoptimization_offset) // if is later jdk version
	{
		constexpr std::string_view code_blob = "CodeBlob";

		// _mark_for_deoptimization_status is the very first field after the inheritance of CodeBlob
		compiled_method_t::mark_for_deoptimization_status_offset = get_vm_type_entry_size(code_blob);
	}
}

void jvm::set_up(std::uint8_t* const jvm_code_base, const std::uint64_t jvm_code_size)
{
	set_up_code_cache(jvm_code_base, jvm_code_size);
	set_up_abstract_interpreter(jvm_code_base, jvm_code_size);

	set_up_method();
	set_up_nmethod();
}

void code_cache_t::deoptimize_method(method_t* const method)
{
	if (!method->is_compiled())
	{
		return;
	}

	if (mark_all_nmethods_for_evol_deoptimization)
	{
		mark_all_nmethods_for_evol_deoptimization();
	}
	else
	{
		mark_all_nmethods_for_deoptimization();
		make_marked_nmethods_non_entrant();
	}
}
