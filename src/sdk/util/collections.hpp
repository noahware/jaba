#pragma once
#include <cstddef>
#include <iterator>
#include <utility>
#include "sdk/lang/core.hpp"

// Java generics are erased, so elements are plain objects. Take one back as what it is with
// as<string>() (unchecked) or try_as<string>() (checked)
namespace jaba::util
{
	JABA_CLASS_BEGIN(iterator, "java/util/Iterator", object)
		JABA_METHOD(jboolean, has_next, "hasNext")
		JABA_METHOD(object, next, "next")
		JABA_METHOD(void, remove, "remove")
	JABA_CLASS_END(iterator)
}

namespace jaba::util::detail
{
	struct sentinel { };

	// a java iterator as a c++ input iterator, what makes a range-for over an iterable work
	class cursor
	{
	public:
		using iterator_category = std::input_iterator_tag;
		using value_type = object;
		using difference_type = std::ptrdiff_t;
		using pointer = const object*;
		using reference = object;

		explicit cursor(iterator it)
			: it_(std::move(it))
		{
			advance();
		}

		[[nodiscard]] object operator*() const
		{
			return current_;
		}

		cursor& operator++()
		{
			advance();
			return *this;
		}

		[[nodiscard]] bool operator==(sentinel) const noexcept
		{
			return !valid_;
		}

	private:
		void advance()
		{
			valid_ = it_.has_next();

			if (valid_)
				current_ = it_.next();
		}

		iterator it_;
		object current_;
		bool valid_ = false;
	};
}

namespace jaba::util
{
	// iter() is java's iterator(), a method can't be named after its own return type
	JABA_CLASS_BEGIN(iterable, "java/lang/Iterable", object)
		JABA_METHOD(iterator, iter, "iterator")

		[[nodiscard]] detail::cursor begin() const
		{
			return detail::cursor{ iter() };
		}

		[[nodiscard]] detail::sentinel end() const noexcept
		{
			return { };
		}
	JABA_CLASS_END(iterable)

	JABA_CLASS_BEGIN(collection, "java/util/Collection", iterable)
		JABA_METHOD(jint, size, "size")
		JABA_METHOD(jboolean, is_empty, "isEmpty")
		JABA_METHOD(jboolean, contains, "contains", object)
		JABA_METHOD(jboolean, add, "add", object)
		JABA_METHOD(jboolean, remove, "remove", object)
		JABA_METHOD(jboolean, add_all, "addAll", collection)
		JABA_METHOD(void, clear, "clear")
	JABA_CLASS_END(collection)

	JABA_CLASS_BEGIN(list, "java/util/List", collection)
		JABA_METHOD(object, get, "get", jint)
		JABA_METHOD(object, set, "set", jint, object)
		JABA_METHOD(void, insert, "add", jint, object)
		JABA_METHOD(object, remove_at, "remove", jint)
		JABA_METHOD(jint, index_of, "indexOf", object)
		JABA_METHOD(list, sub_list, "subList", jint, jint)
	JABA_CLASS_END(list)

	JABA_CLASS_BEGIN(array_list, "java/util/ArrayList", list)
		JABA_CONSTRUCTOR(create)
		JABA_CONSTRUCTOR(create, jint)
		JABA_CONSTRUCTOR(create, collection)
	JABA_CLASS_END(array_list)

	JABA_CLASS_BEGIN(set, "java/util/Set", collection)
	JABA_CLASS_END(set)

	JABA_CLASS_BEGIN(hash_set, "java/util/HashSet", set)
		JABA_CONSTRUCTOR(create)
		JABA_CONSTRUCTOR(create, jint)
		JABA_CONSTRUCTOR(create, collection)
	JABA_CLASS_END(hash_set)
}
