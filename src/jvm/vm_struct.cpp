#include "vm_struct.hpp"

extern "C" __declspec(dllimport) vm_struct_entry* gHotSpotVMStructs;
extern "C" __declspec(dllimport) vm_type_entry* gHotSpotVMTypes;

vm_struct_entry* vm_struct_entry::find(const std::string_view type_name, const std::string_view field_name) noexcept
{
	vm_struct_entry* current_entry = gHotSpotVMStructs;

	while (!current_entry->type_name().empty())
	{
		if (current_entry->type_name() == type_name && current_entry->field_name() == field_name)
			return current_entry;

		++current_entry;
	}

	return nullptr;
}

vm_type_entry* vm_type_entry::find(const std::string_view type_name) noexcept
{
	vm_type_entry* curr = gHotSpotVMTypes;

	while (!curr->type_name().empty())
	{
		if (curr->type_name() == type_name)
			return curr;

		++curr;
	}

	return nullptr;
}
