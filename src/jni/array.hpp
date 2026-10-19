#pragma once
#include <cstddef>
#include <ranges>
#include "object.hpp"
#include "env.hpp"

namespace jaba::jni
{
	// A java array. T is a jni primitive (jint, jdouble, ...), a class, or another array:
	//   JABA_FIELD(jaba::jni::array<jint>, heights, "heights")
	template <class T>
	class array : public object
	{
	public:
		using object::object;

		// what sig::form reads the descriptor from
		using jaba_array_element = T;

		// a new array, every element zero or null. T can't be an array itself here
		[[nodiscard]] static array create(std::size_t length);

		// both throw std::out_of_range for an index outside the array, a null array is empty
		T get(std::size_t i) const;
		void set(std::size_t i, const T& val) const;

		// arr[i] = value is set(i, value), and reading it (int x = arr[i]) is get(i). It has
		// to be a proxy: c++ can't tell the two apart. The proxy holds a ref to the array of
		// its own, so it is also fine to take from a temporary
		class element_ref;

		[[nodiscard]] element_ref operator[](std::size_t i) const;

		// lazy range of the elements, each get() is one jni call
		[[nodiscard]] auto view() const;

		[[nodiscard]] std::size_t size() const noexcept
		{
			return obj_ ? jni::env()->GetArrayLength(native()) : 0;
		}

	protected:
		[[nodiscard]] jarray native() const noexcept
		{
			return reinterpret_cast<jarray>(obj_);
		}
	};
}

#include "array.inl"
