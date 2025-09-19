#pragma once
#include <string>

namespace signature
{
	std::uint8_t* scan(const std::uint8_t* start, const std::uint8_t* end, std::string_view pattern, std::string_view mask);
	std::uint8_t* scan(const std::uint8_t* start, std::uint64_t scan_size, std::string_view pattern, std::string_view mask);

	std::uint8_t* resolve_rva(const std::uint8_t* address, std::int32_t rva_offset, std::int32_t rip_offset);

	std::uint8_t* rva_scan(const std::uint8_t* start, const std::uint8_t* end, std::string_view pattern, std::string_view mask, std::int32_t rva_offset, std::int32_t rip_offset);
	std::uint8_t* rva_scan(const std::uint8_t* start, std::uint64_t scan_size, std::string_view pattern, std::string_view mask, std::int32_t rva_offset, std::int32_t rip_offset);
}
