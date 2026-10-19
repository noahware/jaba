#pragma once

namespace jaba::jni
{
	using hook_install_fn = void (*)();

	void queue_hook(hook_install_fn install);
	void flush_queued_hooks();

	template <class Method, typename Method::detour_t Detour>
	class jni_hook
	{
	public:
		// queues the hook, runs at static init so it must not touch the JVM
		jni_hook()
		{
			queue_hook(&install);
		}

		static void install();
	};
}

#include "pending.inl"
