#include "mapping.hpp"

#include <format>
#include <stdexcept>

namespace
{
	jaba::jni::mappings_t mappings;
}

jaba::zstring_view jaba::jni::fetch_mapping(const zstring_view unobf_name)
{
	if (unobf_name == "<init>")
		return unobf_name;

	if (mappings.empty())
		return unobf_name;

	const auto it = mappings.find(unobf_name);

	if (it == mappings.end())
		throw std::runtime_error(std::format("unable to find mapping '{}'", unobf_name));

	return it->second;
}

void jaba::jni::load_mappings(mappings_t map)
{
	mappings = std::move(map);
}
