#pragma once
#include "pending.hpp"
#include "hook.hpp"
#include <exception>
#include <vector>

namespace jaba::jni
{
	namespace detail
	{
		// function-local so it exists whenever a hook's static constructor runs
		inline std::vector<hook_install_fn>& pending_hks()
		{
			static std::vector<hook_install_fn> hks;
			return hks;
		}
	}

	inline void queue_hook(const hook_install_fn install)
	{
		detail::pending_hks().push_back(install);
	}

	template <class Method, typename Method::detour_t Detour>
	void jni_hook<Method, Detour>::install()
	{
		using hooked = typename Method::template hooked_t<Detour>;

		// hook_method hands back the id that calls the real method
		if (const auto orig = hook_method(Method::native(), reinterpret_cast<method_detour_t>(&hooked::entry)))
			hooked::orig_id = *orig;
	}

	// a hook whose class isn't loaded (native() throws) stays queued for the next flush
	inline void flush_queued_hooks()
	{
		auto& hks = detail::pending_hks();

		std::vector<hook_install_fn> failed;

		for (const hook_install_fn install : hks)
		{
			try
			{
				install();
			}
			catch (const std::exception&)
			{
				failed.push_back(install);
			}
		}

		hks = std::move(failed);
	}
}
