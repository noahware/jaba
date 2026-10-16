#include "mapping.hpp"

#include <format>
#include <stdexcept>

namespace
{
	jaba::mappings_t mappings;
}

std::string_view jaba::fetch_mapping(const std::string_view unobf_name)
{
	if (mappings.empty())
		return unobf_name;

	const auto it = mappings.find(unobf_name);

	if (it == mappings.end())
		throw std::runtime_error(std::format("unable to find mapping '{}'", unobf_name));

	return it->second;
}

void jaba::load_mappings(mappings_t map)
{
	mappings = std::move(map);
}
