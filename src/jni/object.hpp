#pragma once
#include <compare>
#include <concepts>
#include <cstdint>
#include <optional>
#include <utility>
#include <jni/jni.h>
#include "env.hpp"
#include "wrap.hpp"

namespace jaba::jni
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

		// whether there is an object at all: if (world), !world
		[[nodiscard]] explicit operator bool() const noexcept
		{
			return obj_ != nullptr;
		}

		[[nodiscard]] bool same_as(const object& other) const
		{
			return jni::env()->IsSameObject(obj_, other.obj_) == JNI_TRUE;
		}

		// == is whether both are the same java object. Not the handles: a copy of a ref has a
		// handle of its own, and without this they would be compared through the conversion
		// to jobject. Classes that have a java equals (the sdk's) compare by that instead
		[[nodiscard]] friend bool operator==(const object& a, const object& b)
		{
			return a.same_as(b);
		}

		// there is no ordering, the handles' would be an arbitrary one that a std::map would
		// quietly use. Classes that are Comparable (the sdk's) have their own
		friend std::strong_ordering operator<=>(const object&, const object&) = delete;

		// T is a clazz<> type
		template <class T>
			requires requires { { T::native() } -> std::same_as<jclass>; }
		[[nodiscard]] bool instance_of() const
		{
			return obj_ && jni::env()->IsInstanceOf(obj_, T::native()) == JNI_TRUE;
		}

		template <class T>
			requires requires { { T::native() } -> std::same_as<jclass>; }
		[[nodiscard]] bool is() const
		{
			return instance_of<T>();
		}

		// same java object seen as another clazz<> type, no check (a wrong guess is a jvm crash
		// the moment you touch a member), the lvalue form takes its own ref, the rvalue form steals
		template <std::derived_from<object> T>
		[[nodiscard]] T as() const &
		{
			return T{ obj_ ? copy_ref() : nullptr, ref_ };
		}

		template <std::derived_from<object> T>
		[[nodiscard]] T as() &&
		{
			return T{ release(), ref_ };
		}

		// the checked cast, nullopt when it isn't an instance of T (or is null)
		template <class T>
			requires requires { { T::native() } -> std::same_as<jclass>; }
		[[nodiscard]] std::optional<T> try_as() const
		{
			if (!instance_of<T>())
				return std::nullopt;

			return as<T>();
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
