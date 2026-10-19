#pragma once
#include <format>
#include <stdexcept>
#include <type_traits>
#include "array.hpp"
#include "env.hpp"
#include "member.hpp"
#include "types.hpp"

namespace jaba::jni
{
	template <class T>
	array<T> array<T>::create(const std::size_t length)
	{
		static_assert(std::is_arithmetic_v<T> || sig::detail::clazz_like<T>, "an array can be created of a jni primitive or a class");

		JNIEnv* const env = jni::env();
		const auto len = static_cast<jsize>(length);

		jarray created = nullptr;

#define JABA_NEW_ARRE(type, name) \
		if constexpr (is_jni_type<T, type>) \
			created = env->New##name##Array(len);
		JABA_FOREACH_JNI_TYPE(JABA_NEW_ARRE)
#undef JABA_NEW_ARRE

		if constexpr (sig::detail::clazz_like<T>)
			created = env->NewObjectArray(len, T::native(), nullptr);

		if (!created)
		{
			env->ExceptionClear();
			throw std::runtime_error(std::format("unable to create an array of {} elements", length));
		}

		return array{ created };
	}

	template <class T>
	T array<T>::get(const std::size_t i) const
	{
		if (i >= size())
			throw std::out_of_range(std::format("array index {} is out of range for a size of {}", i, size()));

		JNIEnv* const env = jni::env();
		const auto idx = static_cast<jsize>(i);

		// jni has no single-element getter for primitives, so read a region of length 1
#define JABA_GET_ARRE(type, name) \
		if constexpr (is_jni_type<T, type>) \
		{ \
			type val{}; \
			env->Get##name##ArrayRegion(reinterpret_cast<type##Array>(obj_), idx, 1, &val); \
			return val; \
		}
		JABA_FOREACH_JNI_TYPE(JABA_GET_ARRE)
#undef JABA_GET_ARRE

		if constexpr (is_jni_type<T, clazz_any>)
			return detail::wrap<T>(env->GetObjectArrayElement(reinterpret_cast<jobjectArray>(obj_), idx));

		return T{};
	}

	template <class T>
	void array<T>::set(const std::size_t i, const T& val) const
	{
		if (i >= size())
			throw std::out_of_range(std::format("array index {} is out of range for a size of {}", i, size()));

		JNIEnv* const env = jni::env();
		const auto idx = static_cast<jsize>(i);

#define JABA_SET_ARRE(type, name) \
		if constexpr (is_jni_type<T, type>) \
			env->Set##name##ArrayRegion(reinterpret_cast<type##Array>(obj_), idx, 1, &val);
		JABA_FOREACH_JNI_TYPE(JABA_SET_ARRE)
#undef JABA_SET_ARRE

		if constexpr (is_jni_type<T, clazz_any>)
		{
			env->SetObjectArrayElement(reinterpret_cast<jobjectArray>(obj_), idx, detail::unwrap(val));

			// an element of the wrong class for this array
			if (env->ExceptionCheck())
			{
				env->ExceptionClear();
				throw std::runtime_error("unable to store the element, it is not an instance of the array's class");
			}
		}
	}

	template <class T>
	class array<T>::element_ref
	{
	public:
		element_ref(const array& arr, const std::size_t i)
			:	arr_(arr),
				i_(i) { }

		element_ref(const element_ref&) = default;

		[[nodiscard]] T get() const
		{
			return arr_.get(i_);
		}

		[[nodiscard]] operator T() const
		{
			return get();
		}

		element_ref& operator=(const T& val)
		{
			arr_.set(i_, val);
			return *this;
		}

		// a[0] = b[1] copies the element, it doesn't rebind the proxy
		element_ref& operator=(const element_ref& other)
		{
			arr_.set(i_, other.get());
			return *this;
		}

		// arr[i] += 1 and the other compound assignments, for the element types the operator
		// exists for (no %= on a double, nothing on a class). Each one is a read and then a
		// write, two jni calls, so not atomic
#define JABA_ELEMENT_COMPOUND(op) \
		template <class U> \
			requires (std::is_arithmetic_v<T> && requires(T& val, const U& rhs) { val op##= rhs; }) \
		element_ref& operator op##=(const U& rhs) \
		{ \
			T val = get(); \
			val op##= rhs; \
			arr_.set(i_, val); \
			return *this; \
		}
		JABA_ELEMENT_COMPOUND(+)
		JABA_ELEMENT_COMPOUND(-)
		JABA_ELEMENT_COMPOUND(*)
		JABA_ELEMENT_COMPOUND(/)
		JABA_ELEMENT_COMPOUND(%)
		JABA_ELEMENT_COMPOUND(&)
		JABA_ELEMENT_COMPOUND(|)
		JABA_ELEMENT_COMPOUND(^)
		JABA_ELEMENT_COMPOUND(<<)
		JABA_ELEMENT_COMPOUND(>>)
#undef JABA_ELEMENT_COMPOUND

		// ++arr[i] gives the proxy back, arr[i]++ the value it had
		element_ref& operator++()
			requires std::is_arithmetic_v<T>
		{
			T val = get();
			++val;
			arr_.set(i_, val);
			return *this;
		}

		element_ref& operator--()
			requires std::is_arithmetic_v<T>
		{
			T val = get();
			--val;
			arr_.set(i_, val);
			return *this;
		}

		T operator++(int)
			requires std::is_arithmetic_v<T>
		{
			const T old = get();
			T val = old;
			++val;
			arr_.set(i_, val);
			return old;
		}

		T operator--(int)
			requires std::is_arithmetic_v<T>
		{
			const T old = get();
			T val = old;
			--val;
			arr_.set(i_, val);
			return old;
		}

		// arr[i]->method() for an element that is a class, the same as a field has
		struct val_holder
		{
			T val;

			[[nodiscard]] T* operator->() noexcept
			{
				return &val;
			}
		};

		[[nodiscard]] val_holder operator->() const
		{
			return { get() };
		}

		// grid[1][2] for an array of arrays
		[[nodiscard]] auto operator[](const std::size_t j) const
			requires sig::detail::array_like<T>
		{
			return get()[j];
		}

	private:
		array arr_;
		std::size_t i_;
	};

	template <class T>
	typename array<T>::element_ref array<T>::operator[](const std::size_t i) const
	{
		return element_ref{ *this, i };
	}

	template <class T>
	auto array<T>::view() const
	{
		// the lambda keeps its own ref to the array, so the view is fine after the array it came
		// from is gone (names.get().view() in a range-for)
		return std::views::iota(std::size_t{ 0 }, size())
			| std::views::transform([arr = *this](const std::size_t i) { return arr.get(i); });
	}
}
