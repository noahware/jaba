#pragma once
#include <jni/jni.h>
#include "wrap.hpp"

namespace jaba
{
	class zstring_view;

	template <class T>
	class field : public jni_wrap<jfieldID>
	{
	public:
		using jni_wrap::jni_wrap;

		[[nodiscard]] T get() const noexcept
		{
			return { };
		}

		void set(const T val) noexcept
		{
			
		}
	};
}
