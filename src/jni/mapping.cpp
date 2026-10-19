#include "mapping.hpp"

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

	// not every name is obfuscated (the jdk's never are), those are used as they are, a wrong
	// one still fails at the jni lookup and that error names it
	if (it == mappings.end())
		return unobf_name;

	return it->second;
}

void jaba::jni::load_mappings(mappings_t map)
{
	mappings = std::move(map);
}
