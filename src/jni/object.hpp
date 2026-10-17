#pragma once
#include <concepts>
#include <utility>
#include <jni/jni.h>
#include "env.hpp"
#include "wrap.hpp"

namespace jaba
{
	// Owns a local ref: the constructor takes the one it is given, a copy makes a new one, and
	// the destructor deletes it. Local refs belong to the thread that made them, so an object
	// must not leave it.
	class object : public jni_wrap<jobject>
	{
	public:
		object() noexcept = default;

		explicit object(const jobject obj) noexcept
			: jni_wrap(obj) { }

		object(const object& other)
			: jni_wrap(other.obj_ ? jni::env()->NewLocalRef(other.obj_) : nullptr) { }

		object(object&& other) noexcept
			: jni_wrap(std::exchange(other.obj_, nullptr)) { }

		// by value so it is both the copy and the move assignment
		object& operator=(object other) noexcept
		{
			std::swap(obj_, other.obj_);
			return *this;
		}

		~object()
		{
			reset();
		}

		void reset() noexcept
		{
			if (!obj_)
				return;

			if (JNIEnv* const env = jni::env())
				env->DeleteLocalRef(obj_);

			obj_ = nullptr;
		}

		// gives the ref up without deleting it, for returning from a native method (the jvm's frame takes over)
		[[nodiscard]] jobject release() noexcept
		{
			return std::exchange(obj_, nullptr);
		}

		[[nodiscard]] bool same_as(const object& other) const
		{
			return jni::env()->IsSameObject(obj_, other.obj_) == JNI_TRUE;
		}

		// T is a clazz<> type
		template <class T>
			requires requires { { T::native() } -> std::same_as<jclass>; }
		[[nodiscard]] bool instance_of() const
		{
			return obj_ && jni::env()->IsInstanceOf(obj_, T::native()) == JNI_TRUE;
		}
	};
}
