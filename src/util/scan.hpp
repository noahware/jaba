#pragma once
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <pe/image.hpp>

namespace jaba::util
{
	template <class T = std::uint8_t*>
	[[nodiscard]] T resolve_rel(const std::uint8_t* const base, const std::uint32_t rva_off, const std::uint32_t rip_off)
	{
		const auto rva = *reinterpret_cast<const std::int32_t*>(base + rva_off);
		const auto rip = reinterpret_cast<std::uintptr_t>(base + rip_off);

		const auto addr = rip + rva;

		if constexpr (std::is_pointer_v<T>)
			return reinterpret_cast<T>(addr);
		else
			return static_cast<T>(addr);
	}

	template <class T = std::uint8_t*>
	[[nodiscard]] T scan_rel(const pe::image* const img, const std::string_view sig, const std::uint32_t rva_off, const std::uint32_t rip_off)
	{
		const auto base = img->sig_scan(sig);

		if (!base)
			return T{ };

		return resolve_rel<T>(base, rva_off, rip_off);
	}
}
