#pragma once
#include <string>
#include <string_view>
#include <unordered_map>

namespace jaba
{
	struct string_hash
	{
		using is_transparent = void;

		[[nodiscard]] std::size_t operator()(const std::string_view str) const noexcept
		{
			return std::hash<std::string_view>{ }(str);
		}
	};

	// <unobfuscated, obfuscated>
	using mappings_t = std::unordered_map<std::string, std::string, string_hash, std::equal_to<>>;

	std::string_view fetch_mapping(std::string_view unobf_name);
	void load_mappings(mappings_t map);
}
