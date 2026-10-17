#pragma once
#include <cstddef>
#include "clazz.hpp"
#include "field.hpp"

namespace jaba
{
	// the classes whose fields a class inherits, none is fine:
	//   JABA_CLASS_BEGIN(entity, "...")
	//   JABA_CLASS_BEGIN(living, "...", entity)
	//   JABA_CLASS_BEGIN(player, "...", living, other)
	template <class... Ts>
	struct bases
	{
		template <class Self>
		struct apply : Ts::template members<Self>... { };
	};
}

#define JABA_CLASS_BEGIN(name, mapping, ...) \
	class name; \
	template <class Self> \
	struct JABA_EMPTY_BASES name##_members : jaba::bases<__VA_ARGS__>::apply<Self> \
	{ \
		using jaba_members = name##_members; \
		using jaba_self = Self; \
		static constexpr jaba::fixed_string jaba_owner{ mapping };

#define JABA_FIELD(type, name, mapping) \
		struct name##_loc \
		{ \
			static const jaba::object& owner(const void* const field) noexcept \
			{ \
				const auto* const members = reinterpret_cast<const jaba_members*>( \
					static_cast<const char*>(field) - offsetof(jaba_members, name)); \
				return static_cast<const jaba_self&>(*members); \
			} \
		}; \
		JABA_NO_UNIQUE_ADDRESS jaba::field<type, jaba_self, mapping, name##_loc> name;

#define JABA_STATIC_FIELD(type, name, mapping) \
		inline static jaba::static_field<type, jaba_self, mapping> name;

#define JABA_CLASS_END(name) \
	}; \
	class name : public jaba::clazz<name##_members<name>::jaba_owner, name##_members<name>> \
	{ \
	public: \
		using clazz::clazz; \
		template <class Self> \
		using members = name##_members<Self>; \
	};
