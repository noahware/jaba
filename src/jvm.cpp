#include "jvm.hpp"

#include <print>

std::uint64_t adapter_handler_entry_t::c2i_entry_offset = 0;

std::uint64_t method_counters_t::invocation_counter_offset = 0;

std::uint64_t nmethod_t::marked_for_deoptimization_offset = 0;
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
std::uint64_t method_t::adapter_offset = 0;
std::uint64_t method_t::method_counters_offset = 0;

std::uint64_t java_thread_t::state_offset = 0;
#ifdef _JAVAHOOK_USE_JNI
std::uint64_t java_thread_t::anchor_offset = 0;
std::uint64_t java_thread_t::jni_environment_offset = 0;
#endif

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

void jvm::set_up()
{
	constexpr std::string_view java_thread = "JavaThread";

	java_thread_t::state_offset = get_vm_struct_entry_offset(java_thread, "_thread_state");

#ifdef _JAVAHOOK_USE_JNI
	java_thread_t::anchor_offset = get_vm_struct_entry_offset(java_thread, "_anchor");
	java_thread_t::jni_environment_offset = java_thread_t::anchor_offset + 32;
#endif

	constexpr std::string_view method = "Method";

	method_t::class_size = get_vm_type_entry_size(method);
	method_t::i2i_entry_offset = get_vm_struct_entry_offset(method, "_i2i_entry");
	method_t::from_interpreted_entry_offset = get_vm_struct_entry_offset(method, "_from_interpreted_entry");
	method_t::from_compiled_entry_offset = get_vm_struct_entry_offset(method, "_from_compiled_entry");
	method_t::access_flags_offset = get_vm_struct_entry_offset(method, "_access_flags");
	method_t::intrinsic_id_offset = get_vm_struct_entry_offset(method, "_intrinsic_id");
	method_t::flags_offset = method_t::intrinsic_id_offset + 1;
	method_t::vtable_index_offset = get_vm_struct_entry_offset(method, "_vtable_index");
	method_t::code_offset = get_vm_struct_entry_offset(method, "_code");
	method_t::adapter_offset = get_vm_struct_entry_offset(method, "_adapter");
	method_t::method_counters_offset = get_vm_struct_entry_offset(method, "_method_counters");

	constexpr std::string_view nmethod = "nmethod";

	nmethod_t::marked_for_deoptimization_offset = get_vm_struct_entry_offset(nmethod, "_marked_for_deoptimization");
	nmethod_t::state_offset = get_vm_struct_entry_offset(nmethod, "_state");

	if (nmethod_t::marked_for_deoptimization_offset == 0) // if is later jdk version
	{
		constexpr std::string_view code_blob = "CodeBlob";

		// _mark_for_deoptimization_status is the very first field after the inheritance of CodeBlob
		compiled_method_t::mark_for_deoptimization_status_offset = get_vm_type_entry_size(code_blob);
	}

	constexpr std::string_view method_counters = "MethodCounters";

	method_counters_t::invocation_counter_offset = get_vm_struct_entry_offset(method_counters, "_invocation_counter");

	constexpr std::string_view adapter_handler_entry = "AdapterHandlerEntry";

	adapter_handler_entry_t::c2i_entry_offset = get_vm_struct_entry_offset(adapter_handler_entry, "_c2i_entry");
}
