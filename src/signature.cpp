#include "signature.hpp"

std::uint8_t* signature::scan(const std::uint8_t* const start, const std::uint8_t* const end,
	const std::string_view pattern, const std::string_view mask)
{
	const std::uint64_t mask_size = mask.size();

	for (const std::uint8_t* current = start; current < end - mask_size; current++)
	{
		bool pattern_found = true;

		for (std::uint64_t i = 0; i < mask_size; i++)
		{
			const auto current_mask = mask[i];

			if (current_mask == '?')
			{
				continue;
			}

			const auto current_pattern = static_cast<std::uint8_t>(pattern[i]);

			if (current[i] != current_pattern)
			{
				pattern_found = false;
			}
		}

		if (pattern_found)
		{
			return const_cast<std::uint8_t*>(current);
		}
	}

	return nullptr;
}

std::uint8_t* signature::scan(const std::uint8_t* const start, const std::uint64_t scan_size,
	const std::string_view pattern, const std::string_view mask)
{
	return scan(start, start + scan_size, pattern, mask);
}

std::uint8_t* signature::resolve_rva(const std::uint8_t* const address, const std::int32_t rva_offset, const std::int32_t rip_offset)
{
	if (!address)
	{
		return nullptr;
	}

	const std::uint8_t* const rva_location = address + rva_offset;

	const std::uint8_t* const rip = address + rip_offset;
	const std::int32_t rva = *reinterpret_cast<const std::int32_t*>(rva_location);

	return const_cast<std::uint8_t*>(rip + rva);
}

std::uint8_t* signature::rva_scan(const std::uint8_t* const start, const std::uint8_t* const end,
                                  const std::string_view pattern, const std::string_view mask, const std::int32_t rva_offset, const std::int32_t rip_offset)
{
	const std::uint8_t* const scanned_code = scan(start, end, pattern, mask);

	return resolve_rva(scanned_code, rva_offset, rip_offset);
}

std::uint8_t* signature::rva_scan(const std::uint8_t* const start, const std::uint64_t scan_size,
	const std::string_view pattern, const std::string_view mask, const std::int32_t rva_offset, const std::int32_t rip_offset)
{
	return rva_scan(start, start + scan_size, pattern, mask, rva_offset, rip_offset);
}
