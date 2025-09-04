#include "hook.hpp"

#include <memory>
#include <print>
#include <unordered_map>
#include <Windows.h>

#include "trap_frame.hpp"

extern "C" void interpreted_method_hook_handler();
extern "C" void i2i_method_hook_handler();

namespace
{
    struct hook_t
    {
        std::uint8_t* original_interpreted_method_handler = nullptr;
        std::uint8_t* original_i2i_method_handler = nullptr;

        hook::handler_t handler = nullptr;

        std::unique_ptr<method_t> method_copy;
    };

    enum class method_entry_type_t : std::uint8_t
    {
        interpreted = 0,
        i2i = 1
    };

    std::unordered_map<method_t*, hook_t> hook_list;
}

void make_method_interpreted(method_t* const method)
{
    method->set_no_inline();
    method->set_access_flags(method->access_flags() | 0xE000000);

    method->decompile();
}

void add_to_hook_list(method_t* const method, hook::handler_t const handler, std::unique_ptr<method_t> method_copy)
{
	hook_t hook;

    hook.original_interpreted_method_handler = method->interpreted_entry();
    hook.original_i2i_method_handler = method->i2i_entry();
    hook.handler = handler;
    hook.method_copy = std::move(method_copy);

    hook_list[method] = std::move(hook);
}

std::unique_ptr<method_t> make_method_copy(const jmethodID method_id)
{
    method_t* const original_method = *reinterpret_cast<method_t**>(method_id);

    auto new_method = std::make_unique<method_t>(*original_method);

    new_method->set_vtable_index(-2);

    return std::move(new_method);
}

void hook::add_hook(const jmethodID method_id, handler_t const handler)
{
    method_t* const method = *reinterpret_cast<method_t**>(method_id);

    make_method_interpreted(method);

    std::unique_ptr<method_t> method_copy = make_method_copy(method_id);

    add_to_hook_list(method, handler, std::move(method_copy));

    auto const interpreted_hook_handler = reinterpret_cast<std::uint8_t*>(interpreted_method_hook_handler);
    auto const i2i_hook_handler = reinterpret_cast<std::uint8_t*>(i2i_method_hook_handler);

    method->set_interpreted_entry(interpreted_hook_handler);
    method->set_i2i_entry(i2i_hook_handler);
}

void hook::remove_hook(const jmethodID method_id)
{
    method_t* const method = *reinterpret_cast<method_t**>(method_id);

    const auto& it = hook_list.find(method);

    if (it == hook_list.end())
    {
        return;
    }

    const auto& hook = it->second;

    method->set_interpreted_entry(hook.original_interpreted_method_handler);
    method->set_i2i_entry(hook.original_i2i_method_handler);

    hook_list.erase(it);
}

struct java_frame_anchor_t
{
    std::uint8_t* last_stack_pointer;
    std::uint8_t* last_program_counter;
    std::uint8_t* last_frame_pointer;
};

extern "C" void java_hook_method_detour(const std::uint64_t starting_rsp, method_t* const method, const method_entry_type_t method_entry_type, std::uint8_t* const should_call_original)
{
    auto const arguments = reinterpret_cast<argument_parser_t*>(starting_rsp + 8);
    auto const original_handler = reinterpret_cast<std::uint8_t**>(starting_rsp - 16);
    auto const trap_frame = reinterpret_cast<trap_frame_t*>(starting_rsp - 0x88);

    const auto& it = hook_list.find(method);

    if (it != hook_list.end())
    {
        const auto& hook = it->second;

        switch (method_entry_type)
        {
        case method_entry_type_t::interpreted:
        {
            *original_handler = hook.original_interpreted_method_handler;
            break;
        }
        case method_entry_type_t::i2i:
        {
        	*original_handler = hook.original_i2i_method_handler;
        	break;
        }
        }

        const std::uint64_t java_thread = trap_frame->r15;

        constexpr std::uint64_t jni_environment_offset = 504;
        constexpr std::uint64_t thread_state_offset = 624;

        auto const thread_state = reinterpret_cast<std::int32_t*>(java_thread + thread_state_offset);

    	const auto env = reinterpret_cast<JNIEnv*>(java_thread + jni_environment_offset);

        const std::int32_t original_thread_state = *thread_state;

        *thread_state = 4; // native thread state

        method_t* method_copy = hook.method_copy.get();

        const auto method_copy_id = reinterpret_cast<jmethodID>(&method_copy);

		trap_frame->rax = hook.handler(arguments, env, method_copy_id, should_call_original);
    
        *thread_state = original_thread_state;

        return;
    }

    __debugbreak();
}
