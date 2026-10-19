#pragma once
#include <format>
#include <stdexcept>
#include <jni/jni.h>
#include "../util/zstring_view.hpp"
#include "env.hpp"
#include "object.hpp"
#include "wrap.hpp"

// msvc only collapses the first empty base unless told to, which would bloat clazz
#if defined(_MSC_VER)
#define JABA_EMPTY_BASES __declspec(empty_bases)
#else
#define JABA_EMPTY_BASES
#endif

namespace jaba::jni
{
	using find_class_fn = jclass(*)(zstring_view);

	extern find_class_fn find_class;

	void set_find_class_fn(find_class_fn fn);

	// Members are the stateless field holders (see jni.hpp), they hold no instance so any
	// number of them can be inherited, the only instance state is the object base
	template <fixed_string Mapping, class... Members>
	class JABA_EMPTY_BASES clazz : public mapped<Mapping>, public Members..., public object
	{
	public:
		using mapped<Mapping>::mapping;
		using object::object;

		// looked up once and kept as a global ref (a local ref would dangle)
		[[nodiscard]] static jclass native()
		{
			static const jclass cls = []
			{
				const jclass local = find_class(mapping());

				if (!local)
				{
					// FindClass left a NoClassDefFoundError pending, and jni calls made with
					// one pending are undefined (it would also come out of the next native return)
					jni::env()->ExceptionClear();
					throw std::runtime_error(std::format("unable to find class '{}'", Mapping.view()));
				}

				const auto global = static_cast<jclass>(jni::env()->NewGlobalRef(local));
				jni::env()->DeleteLocalRef(local);
				return global;
			}();

			return cls;
		}
	};
}
