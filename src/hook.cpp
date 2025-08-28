#include "hook.hpp"

#include <unordered_map>

extern "C" void interpreted_method_hook_handler();
extern "C" void i2i_method_hook_handler();

namespace hook
{
    struct hook_t
    {
        std::uint8_t* original_interpreted_method_handler = nullptr;
        std::uint8_t* original_i2i_method_handler = nullptr;

        handler_t handler = nullptr;
    };

    enum class method_entry_type_t : std::uint8_t
    {
        interpreted = 0,
        i2i = 1
    };

    std::unordered_map<method_t*, hook_t> hooks;
}

void recompile_method_class(JNIEnv* const env, jvmtiEnv* const jvmti_env, const jmethodID method)
{
    jclass declaring_class = nullptr;

    jvmti_env->GetMethodDeclaringClass(method, &declaring_class);
    jvmti_env->RetransformClasses(1, &declaring_class);

    env->DeleteLocalRef(declaring_class);
}

void make_method_interpreted(JNIEnv* const env, jvmtiEnv* const jvmti_env, const jmethodID method_id)
{
    method_t* const method = *reinterpret_cast<method_t**>(method_id);

    method->set_no_inline();
    method->set_access_flags(method->access_flags() | 0xE000000);

    recompile_method_class(env, jvmti_env, method_id);
}

void add_to_hook_list(method_t* const method, hook::handler_t const handler)
{
	hook::hook_t hook;

    hook.original_interpreted_method_handler = method->interpreted_entry();
    hook.original_i2i_method_handler = method->i2i_entry();
    hook.handler = handler;

    hook::hooks[method] = hook;
}

void hook::add_hook(JNIEnv* const env, jvmtiEnv* const jvmti_env, const jmethodID method_id, handler_t const handler)
{
    make_method_interpreted(env, jvmti_env, method_id);

    method_t* const method = *reinterpret_cast<method_t**>(method_id);

    add_to_hook_list(method, handler);

    auto const interpreted_hook_handler = reinterpret_cast<std::uint8_t*>(interpreted_method_hook_handler);
    auto const i2i_hook_handler = reinterpret_cast<std::uint8_t*>(i2i_method_hook_handler);

    method->set_interpreted_entry(interpreted_hook_handler);
    method->set_i2i_entry(i2i_hook_handler);

    method->set_no_inline();
    method->set_access_flags(method->access_flags() | 0xE000000);
}

void hook::remove_hook(const jmethodID method_id)
{
    method_t* const method = *reinterpret_cast<method_t**>(method_id);

    const auto& it = hooks.find(method);

    if (it == hooks.end())
    {
        return;
    }

    const auto& hook = it->second;

    method->set_interpreted_entry(hook.original_interpreted_method_handler);
    method->set_i2i_entry(hook.original_i2i_method_handler);

    hooks.erase(it);
}

extern "C" std::uint64_t java_hook_method_detour(const std::uint64_t starting_rsp, method_t* const method, const hook::method_entry_type_t method_entry_type, std::uint8_t* const should_call_original)
{
    auto const arguments = reinterpret_cast<argument_parser_t*>(starting_rsp + 8);
    auto const original_handler = reinterpret_cast<std::uint8_t**>(starting_rsp - 16);

    const auto& it = hook::hooks.find(method);

    if (it != hook::hooks.end())
    {
        const auto& hook = it->second;

        switch (method_entry_type)
        {
        case hook::method_entry_type_t::interpreted:
        {
            *original_handler = hook.original_interpreted_method_handler;
            break;
        }
        case hook::method_entry_type_t::i2i:
        {
        	*original_handler = hook.original_i2i_method_handler;
        	break;
        }
        }

        return hook.handler(arguments, should_call_original);
    }

    __debugbreak();

    return 0;
}
