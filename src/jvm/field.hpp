#pragma once
#include <cstdint>
#include <stdexcept>
#include <optional>
#include <format>
#include "vm_struct.hpp"

#define JABA_JVM_FIELD(clazz_name, field_name, type) \
    static inline std::optional<std::ptrdiff_t> field_name##_offset = std::nullopt; \
	static void find_##field_name##_off() { \
		if (field_name##_offset)\
			return; \
		const auto vmf = jaba::vm_struct_entry::find(#clazz_name, #field_name); \
        if (!vmf) \
			throw std::runtime_error(std::format("unable to find {}!{}", #clazz_name, #field_name)); \
		field_name##_offset = vmf->offset(); \
	} \
    [[nodiscard]] type field_name() const { \
		find_##field_name##_off(); \
        return *reinterpret_cast<type*>( \
            reinterpret_cast<std::uint8_t*>(const_cast<void*>( \
                static_cast<const void*>(this))) + *field_name##_offset); \
    } \
    void set_##field_name(type v) { \
		find_##field_name##_off(); \
        *reinterpret_cast<type*>( \
            reinterpret_cast<std::uint8_t*>(this) + *field_name##_offset) = v; \
    }
