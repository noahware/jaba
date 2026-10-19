#pragma once
#include <cstddef>
#include <ranges>
#include "object.hpp"
#include "env.hpp"

namespace jaba::jni
{
	template <class T>
	class array : public object
	{
	public:
		using object::object;

		T get(std::size_t i) const noexcept;
		void set(std::size_t i, const T& val) const noexcept;

		// lazy range of the elements, each get() is one jni call. the array must outlive it
		[[nodiscard]] auto view() const;

		[[nodiscard]] std::size_t size() const noexcept
		{
			return jni::env()->GetArrayLength(native());
		}

	protected:
		[[nodiscard]] jarray native() const noexcept
		{
			return reinterpret_cast<jarray>(obj_);
		}
	};
}

#include "array.inl"
