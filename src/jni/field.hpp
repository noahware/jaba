#pragma once
#include <jni/jni.h>
#include "wrap.hpp"

namespace jaba
{
	class object;

	template <class T>
	class field : public jni_wrap<jfieldID>
	{
	public:
		field(const jfieldID id, const bool is_static) noexcept
			:	jni_wrap(id), is_static_(is_static) { }

		// for static fields, obj is the declaring class
		[[nodiscard]] T get(object obj) const noexcept;
		void set(object obj, T val) noexcept;

		[[nodiscard]] bool is_static() const noexcept
		{
			return is_static_;
		}

	protected:
		bool is_static_;
	};
}
