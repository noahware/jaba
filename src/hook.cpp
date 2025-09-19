#include "hook.hpp"

#include <memory>
#include <print>
#include <unordered_map>

#include "trap_frame.hpp"

extern "C" void generic_method_handler();
extern "C" std::uint8_t* native_method_handler = nullptr;

namespace
{
    struct hook_t
    {
        std::uint8_t* original_interpreted_method_handler = nullptr;
        std::uint8_t* original_i2i_method_handler = nullptr;

        void* handler = nullptr;

        std::unique_ptr<std::vector<std::uint8_t>> method_copy;
    };

    std::unordered_map<method_t*, hook_t> hook_list;
}

void hook::set_up(std::uint8_t* const jvm_code_base, const std::uint64_t jvm_code_size)
{
    jvm::set_up(jvm_code_base, jvm_code_size);
}

void make_method_interpreted(method_t* const method)
{
    method->set_no_inline();

    method->disable_compilation();
    method->decompile();
}

void add_to_hook_list(method_t* const method, void* const handler, std::unique_ptr<std::vector<std::uint8_t>> method_copy)
{
    hook_t hook;

    hook.original_interpreted_method_handler = method->from_interpreted_entry();
    hook.original_i2i_method_handler = method->i2i_entry();
    hook.handler = handler;
    hook.method_copy = std::move(method_copy);

    hook_list[method] = std::move(hook);
}

std::unique_ptr<std::vector<std::uint8_t>> make_method_copy(method_t* const original_method)
{
    const auto buffer_start = reinterpret_cast<std::uint8_t*>(original_method);

    auto new_method_buffer = std::make_unique<std::vector<std::uint8_t>>(buffer_start, buffer_start + method_t::class_size);

    const auto new_method = reinterpret_cast<method_t*>(new_method_buffer->data());

    new_method->set_vtable_index(-2);
    new_method->make_native();

    return std::move(new_method_buffer);
}

void hook::add_hook(method_t* const method, void* const handler)
{
    make_method_interpreted(method);

    std::unique_ptr<std::vector<std::uint8_t>> method_copy = make_method_copy(method);

    std::uint8_t* const slow_signature_handler = abstract_interpreter_t::slow_signature_handler;

    method_copy->insert(method_copy->end(), reinterpret_cast<const std::uint8_t*>(&handler), reinterpret_cast<const std::uint8_t*>(&handler) + sizeof(handler));
    method_copy->insert(method_copy->end(), reinterpret_cast<const std::uint8_t*>(&slow_signature_handler), reinterpret_cast<const std::uint8_t*>(&slow_signature_handler) + sizeof(slow_signature_handler));

    add_to_hook_list(method, handler, std::move(method_copy));

    const auto method_handler = reinterpret_cast<std::uint8_t*>(generic_method_handler);

    method->set_from_interpreted_entry(method_handler);
    method->set_i2i_entry(method_handler);
}

void hook::remove_hook(method_t* const method)
{
    const auto& it = hook_list.find(method);

    if (it == hook_list.end())
    {
        return;
    }

    const auto& hook = it->second;

    method->set_from_interpreted_entry(hook.original_interpreted_method_handler);
    method->set_i2i_entry(hook.original_i2i_method_handler);

    hook_list.erase(it);
}

method_t* hook::get_callable_method(method_t* const original_method)
{
    const auto& it = hook_list.find(original_method);

    if (it == hook_list.end())
    {
        return nullptr;
    }

    const auto& hook = it->second;

    return reinterpret_cast<method_t*>(hook.method_copy->data());
}

#ifdef _JAVAHOOK_USE_JNI
void hook::jni::add_hook(const jmethodID method_id, void* const handler)
{
    method_t* const method = *reinterpret_cast<method_t**>(method_id);

    hook::add_hook(method, handler);
}

void hook::jni::remove_hook(const jmethodID method_id)
{
    method_t* const method = *reinterpret_cast<method_t**>(method_id);

    hook::remove_hook(method);
}

method_t* hook::jni::get_callable_method(const jmethodID original_method_id)
{
    if (!original_method_id)
    {
        return nullptr;
    }

    method_t* const original_method = *reinterpret_cast<method_t**>(original_method_id);

	return hook::get_callable_method(original_method);
}
#endif

extern "C" void java_hook_method_detour(trap_frame_t* const trap_frame)
{
	const auto method = reinterpret_cast<method_t*>(trap_frame->rbx);

    const auto& it = hook_list.find(method);

    if (it != hook_list.end())
    {
        const auto& hook = it->second;

        const auto method_copy = hook.method_copy->data();

        trap_frame->rbx = reinterpret_cast<std::uint64_t>(method_copy);

        return;
    }

    __debugbreak();
}
