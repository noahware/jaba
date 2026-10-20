#include "mapping.hpp"

namespace
{
	jaba::jni::mappings_t mappings;
}

jaba::zstring_view jaba::jni::fetch_mapping(const zstring_view unobf_name)
{
	if (unobf_name == "<init>")
		return unobf_name;

	const auto it = mappings.find(unobf_name);

	if (it != mappings.end())
		return it->second;

	// "qualifier.name" is a name that members share while being obfuscated differently, the
	// qualifier keeps them apart (the sdk generator does this). Without a mapping it is just
	// the name, java names never contain a dot
	if (const std::size_t dot = unobf_name.find('.'); dot != std::string_view::npos)
		return unobf_name.data() + dot + 1;

	// not every name is obfuscated (the jdk's never are), those are used as they are, a wrong
	// one still fails at the jni lookup and that error names it
	return unobf_name;
}

void jaba::jni::load_mappings(mappings_t map)
{
	mappings = std::move(map);
}
