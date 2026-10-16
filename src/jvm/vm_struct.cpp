#include "vm_struct.hpp"

#ifdef _WIN32
extern "C" __declspec(dllimport) jaba::vm_struct_entry* gHotSpotVMStructs;
extern "C" __declspec(dllimport) jaba::vm_type_entry* gHotSpotVMTypes;
#else
#include <dlfcn.h>

// libjvm.so exports these, looked up at runtime so nothing has to link against it
#define gHotSpotVMStructs (*static_cast<jaba::vm_struct_entry**>(dlsym(RTLD_DEFAULT, "gHotSpotVMStructs")))
#define gHotSpotVMTypes (*static_cast<jaba::vm_type_entry**>(dlsym(RTLD_DEFAULT, "gHotSpotVMTypes")))
#endif

namespace jaba
{
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
}
