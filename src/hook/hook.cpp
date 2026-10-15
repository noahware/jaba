#include "hook.hpp"
#include "jvm/compiler.hpp"
#include "jvm/method.hpp"
#include <unordered_map>
#include <functional>
#include <shared_mutex>

using namespace jaba;

extern "C" void jaba_hk_stub();

namespace
{
	using recompile_method_fn = std::function<bool(method*)>;

	struct hook
	{
		method* native_method_ptr; // here for jmethodID to reference it
		jvm_type_copy<method> native_method;
		void* orig_interp_entry;
		void* orig_i2i_entry;
	};

	// key is original method
	std::shared_mutex hooks_mtx;
	std::unordered_map<method*, hook> hooks;

	jvm_type_copy<method> make_native_handler(const method* const m, const method_detour_t detour)
	{
		jvm_type_copy<method> native_method = m->copy<method>();

		native_method->make_native();
		native_method->set_vtable_idx(-2);

		auto& buf = native_method.buf();

		const auto detour_ref = reinterpret_cast<const std::uint8_t*>(&detour);

		buf.insert(buf.end(), detour_ref, detour_ref + sizeof(std::uintptr_t));
		buf.insert(buf.end(), sizeof(std::uintptr_t), 0);

		return native_method;
	}

	std::optional<jmethodID> raw_hook_method(const jmethodID jmethod, const method_detour_t detour,
	                                         const recompile_method_fn& recompile_method)
	{
		method* orig_method = method::from(jmethod);

		{
			std::shared_lock lock(hooks_mtx);

			if (hooks.contains(orig_method))
				return { };
		}

		if (!recompile_method(orig_method))
			return { };

		// the recompile can replace the method object, so the old pointer is stale
		orig_method = method::from(jmethod);

		orig_method->make_no_inline();
		orig_method->make_interpreted();

		std::unique_lock lock(hooks_mtx);

		// under the lock
		if (hooks.contains(orig_method))
			return { };

		jvm_type_copy<method> native_method = make_native_handler(orig_method, detour);

		hook hk = {
			.native_method_ptr = native_method.data(),
			.native_method = std::move(native_method),
			.orig_interp_entry = orig_method->from_interp_entry(),
			.orig_i2i_entry = orig_method->i2i_entry()
		};

		auto& stored = hooks.emplace(orig_method, std::move(hk)).first->second;

		orig_method->set_from_interp_entry(jaba_hk_stub);
		orig_method->set_i2i_entry(jaba_hk_stub);

		return reinterpret_cast<jmethodID>(&stored.native_method_ptr);
	}
}

#ifdef JABA_MHK_USE_JVMTI
// todo: retransform classes will wipe other hooks in same class, fix that
std::optional<jmethodID> jaba::hook_method(const jmethodID jmethod, const method_detour_t detour, jvmtiEnv* const tinv)
{
	return raw_hook_method(jmethod, detour,
		[tinv](method* jvm_method) -> bool
		{
			const auto method = reinterpret_cast<jmethodID>(&jvm_method);

			jclass clazz = nullptr;

			if (tinv->GetMethodDeclaringClass(method, &clazz) != JVMTI_ERROR_NONE)
				return false;
			
			constexpr jint clazz_count = 1;

			if (tinv->RetransformClasses(clazz_count, &clazz) != JVMTI_ERROR_NONE)
				return false;

			return true;
		}
	);
}
#else
std::optional<jmethodID> hook_method(const jmethodID jmethod, const method_detour_t detour)
{
	
}
#endif

bool jaba::unhook_method(const jmethodID jmethod)
{
	method* const orig_method = method::from(jmethod);

	std::unique_lock lock(hooks_mtx);

	const auto it = hooks.find(orig_method);

	if (it == hooks.end())
		return false;

	const auto& hk = it->second;

	orig_method->set_from_interp_entry(hk.orig_interp_entry);
	orig_method->set_i2i_entry(hk.orig_i2i_entry);

	hooks.erase(it);

	return true;
}

extern "C" void jaba_hook_c_handler(method** method)
{
	std::shared_lock lock(hooks_mtx);

	const auto it = hooks.find(*method);

	if (it != hooks.end())
	{
		auto& hk = it->second;

		*method = hk.native_method.data();
	}
}
