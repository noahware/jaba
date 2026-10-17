#pragma once
#include <jni/jni.h>
#include "wrap.hpp"

namespace jaba
{
	class object : public jni_wrap<jobject>
	{
	public:
		using jni_wrap::jni_wrap;
	};
}
