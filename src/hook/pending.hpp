#pragma once

namespace jaba
{
	class jni_hook
	{
	public:
		virtual void install() = 0;
	};

	void queue_hook(jni_hook* jh);
	void flush_queued_hooks();
}
