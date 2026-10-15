#pragma once
#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <optional>
#include <string_view>
#include <format>
#include "vm_struct.hpp"


#define JABA_JVM_FIELD_IMPL(display_name, type, resolve_offset) \
    static inline std::optional<std::ptrdiff_t> display_name##_offset = std::nullopt; \
	static void find_##display_name##_off() { \
		if (display_name##_offset)\
			return; \
		display_name##_offset = (resolve_offset); \
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

#define JABA_JVM_FIELD(field_name, display_name, type) \
	JABA_JVM_FIELD_IMPL(display_name, type, field_offset(#field_name))

#define JABA_JVM_FIELD_OR(field_name, display_name, type, ...) \
	JABA_JVM_FIELD_IMPL(display_name, type, field_offset_or(#field_name, []{ return static_cast<std::ptrdiff_t>(__VA_ARGS__); }))
