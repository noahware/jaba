#pragma once
#include <string_view>

#ifdef _WIN32
#include <pe/image.hpp>

namespace jaba::util
{
	using image = pe::image;
}
#else
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include <link.h>

namespace jaba::util
{
	// loaded elf module, only provides what pe::image does for us (sig_scan)
	class image
	{
	public:
		// first loaded module whose path contains name, e.g. libjvm.so
		[[nodiscard]] static const image* find(const char* const name)
		{
			static image img;

			dl_iterate_phdr(
				[](dl_phdr_info* const info, std::size_t, void* const data) -> int
				{
					if (!std::strstr(info->dlpi_name, static_cast<const char*>(data)))
						return 0;

					for (int i = 0; i < info->dlpi_phnum; ++i)
					{
						const ElfW(Phdr)& ph = info->dlpi_phdr[i];

						if (ph.p_type == PT_LOAD && (ph.p_flags & PF_R))
						{
							const auto begin = reinterpret_cast<const std::uint8_t*>(info->dlpi_addr + ph.p_vaddr);
							img.segments_.emplace_back(begin, begin + ph.p_memsz);
						}
					}

					return 1;
				}, const_cast<char*>(name));

			return img.segments_.empty() ? nullptr : &img;
		}

		// ida style, "48 8B ? ?? 05", the bools are only here for pe::image parity
		[[nodiscard]] const std::uint8_t* sig_scan(const std::string_view sig, bool = true, bool = true) const
		{
			std::vector<std::pair<std::uint8_t, bool>> pat;

			for (std::size_t i = 0; i < sig.size(); ++i)
			{
				if (sig[i] == ' ')
					continue;

				if (sig[i] == '?')
				{
					pat.emplace_back(0, false);

					while (i + 1 < sig.size() && sig[i + 1] == '?')
						++i;
				}
				else
				{
					pat.emplace_back(static_cast<std::uint8_t>(std::strtoul(std::string(sig.substr(i, 2)).c_str(), nullptr, 16)), true);
					++i;
				}
			}

			if (pat.empty())
				return nullptr;

			for (const auto& [begin, end] : segments_)
			{
				const auto it = std::search(begin, end, pat.begin(), pat.end(),
					[](const std::uint8_t b, const std::pair<std::uint8_t, bool>& p) { return !p.second || b == p.first; });

				if (it != end)
					return it;
			}

			return nullptr;
		}

	private:
		std::vector<std::pair<const std::uint8_t*, const std::uint8_t*>> segments_;
	};
}
#endif
