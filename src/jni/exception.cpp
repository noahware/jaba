#include "exception.hpp"
#include <exception>
#include <format>
#include "jni.hpp"
#include "utf.hpp"

using namespace jaba::jni;

namespace
{
	// the little of java.lang that describing an exception takes, defined like any other class
	JABA_CLASS_BEGIN(java_string, "java/lang/String")
	JABA_CLASS_END(java_string)

	JABA_CLASS_BEGIN(java_class, "java/lang/Class")
		JABA_METHOD(java_string, get_name, "getName")
	JABA_CLASS_END(java_class)

	JABA_CLASS_BEGIN(java_throwable, "java/lang/Throwable")
		JABA_METHOD(java_string, get_message, "getMessage")
	JABA_CLASS_END(java_throwable)

	// Asking java what was thrown is a call that can throw as well (getMessage can be
	// overridden), and that comes back through here. Those are left bare
	thread_local bool describing = false;

	template <class Call>
	std::string describe(Call&& call)
	{
		try
		{
			const java_string text = call();

			return detail::from_jstring(env(), static_cast<jstring>(text.obj()));
		}
		catch (const std::exception&)
		{
			return { };
		}
	}
}

java_exception java_exception::take(JNIEnv* const jni_env, const std::string_view context)
{
	// the ref is ours to delete. The exception comes off first, jni calls made with one
	// pending are undefined
	const object thrown{ jni_env->ExceptionOccurred() };
	jni_env->ExceptionClear();

	std::string class_name;
	std::string message;

	if (thrown.obj() && !describing)
	{
		describing = true;

		class_name = describe([&] { return java_class{ jni_env->GetObjectClass(thrown.obj()) }.get_name(); });
		message = describe([&] { return thrown.as<java_throwable>().get_message(); });

		describing = false;
	}

	std::string what = class_name.empty() ? "java exception" : class_name;

	if (!message.empty())
		what += ": " + message;

	if (!context.empty())
		what += std::format(" ({})", context);

	return java_exception{ what, std::move(class_name), std::move(message), thrown.as_global_obj() };
}
