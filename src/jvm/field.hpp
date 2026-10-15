#pragma once
#include <cstdint>
#include <stdexcept>
#include <optional>
#include <format>
#include "vm_struct.hpp"

#define JABA_JVM_FIELD(field_name, display_name, type) \
    static inline std::optional<std::ptrdiff_t> display_name##_offset = std::nullopt; \
	static void find_##display_name##_off() { \
		if (display_name##_offset)\
			return; \
		const auto vmf = jaba::vm_struct_entry::find(name, #field_name); \
        if (!vmf) \
			throw std::runtime_error(std::format("unable to find {}!{}", name.view(), #field_name)); \
		display_name##_offset = vmf->offset(); \
	} \
    [[nodiscard]] type display_name() const { \
		find_##display_name##_off(); \
        return *reinterpret_cast<type*>( \
            reinterpret_cast<std::uint8_t*>(const_cast<void*>( \
                static_cast<const void*>(this))) + *display_name##_offset); \
    } \
    void set_##display_name(type v) { \
		find_##display_name##_off(); \
        *reinterpret_cast<type*>( \
            reinterpret_cast<std::uint8_t*>(this) + *display_name##_offset) = v; \
    }
