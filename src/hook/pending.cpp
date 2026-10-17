#include "pending.hpp" 
#include <vector>

namespace
{
	std::vector<jaba::jni_hook*> pending_hks;
}

void jaba::queue_hook(jni_hook* const jh)
{
	pending_hks.push_back(jh);
}

void jaba::flush_queued_hooks()
{
	for (jni_hook* const hk : pending_hks)
		hk->install();

	pending_hks.clear();
}
