#pragma once
#include <concepts>
#include <utility>
#include <jni/jni.h>
#include "env.hpp"
#include "wrap.hpp"

namespace jaba
{
	class object : public jni_wrap<jobject>
	{
	public:
		enum class ref_t : std::uint8_t
		{
			local,
			global
		};

		object() noexcept = default;

		explicit object(const jobject obj, const ref_t ref = ref_t::local) noexcept
			:	jni_wrap(obj),
				ref_(ref) { }

		object(const object& other)
			:	jni_wrap(other.obj_ ? other.copy_ref() : nullptr),
				ref_(other.ref_) { }

		object(object&& other) noexcept
			:	jni_wrap(std::exchange(other.obj_, nullptr)),
				ref_(other.ref_) { }

		// by value so it is both the copy and the move assignment
		object& operator=(object other) noexcept
		{
			std::swap(obj_, other.obj_);
			std::swap(ref_, other.ref_);

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
			{
				if (ref_ == ref_t::local)
					env->DeleteLocalRef(obj_);
				else
					env->DeleteGlobalRef(obj_);
			}

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

		[[nodiscard]] bool is_local() const noexcept
		{
			return ref_ == ref_t::local;
		}

		[[nodiscard]] bool is_global() const noexcept
		{
			return ref_ == ref_t::global;
		}

		[[nodiscard]] ref_t ref() const noexcept
		{
			return ref_;
		}

		[[nodiscard]] object as_global_obj() const noexcept
		{
			return object{ jni::env()->NewGlobalRef(obj_), ref_t::global };
		}

	protected:
		[[nodiscard]] jobject copy_ref() const noexcept
		{
			const auto env = jni::env();

			return ref_ == ref_t::local ?
				env->NewLocalRef(obj_) :
				env->NewGlobalRef(obj_);
		}

		ref_t ref_ = ref_t::local;
	};
}
