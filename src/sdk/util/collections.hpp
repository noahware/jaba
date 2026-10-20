#pragma once
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>
#include "sdk/lang/core.hpp"

// The classes in jaba::util::raw are java's, their elements are plain objects (generics are
// erased, and so are the jni signatures). The ones in jaba::util wrap them with the element
// type, list<string> is a java.util.List whose get() gives a string. The type is only the c++
// side's view: a wrong one isn't checked, it is the same unchecked cast as as<T>()
//
// The arguments default to object, which is a java raw type: list<> is a List of anything
namespace jaba::util::raw
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

	// a raw or generic class seen as the generic class T, no check
	template <class T, class R>
	[[nodiscard]] T retype(R&& raw)
	{
		if constexpr (std::is_same_v<T, std::remove_cvref_t<R>>)
			return std::forward<R>(raw);
		else
			return std::move(raw).template as<T>();
	}

	// an element where the raw class asks for an object
	template <class T>
	[[nodiscard]] object erase(const T& value)
	{
		if constexpr (std::is_same_v<T, object>)
			return value;
		else
			return value.template as<object>();
	}

	// a java iterator as a c++ input iterator, what makes a range-for over an iterable work
	template <class T>
	class cursor
	{
	public:
		using iterator_category = std::input_iterator_tag;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = const T*;
		using reference = T;

		explicit cursor(raw::iterator it)
			: it_(std::move(it))
		{
			advance();
		}

		[[nodiscard]] T operator*() const
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
				current_ = retype<T>(it_.next());
		}

		raw::iterator it_;
		T current_;
		bool valid_ = false;
	};
}

namespace jaba::util::raw
{
	// iter() is java's iterator(), a method can't be named after its own return type
	JABA_CLASS_BEGIN(iterable, "java/lang/Iterable", object)
		JABA_METHOD(iterator, iter, "iterator")

		[[nodiscard]] detail::cursor<object> begin() const
		{
			return detail::cursor<object>{ iter() };
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

namespace jaba::util
{
	template <class T>
	class iterator : public raw::iterator
	{
	public:
		using raw::iterator::iterator;

		iterator() = default;

		explicit iterator(const raw::iterator& raw)
			: raw::iterator(raw) { }

		[[nodiscard]] T next() const
		{
			return detail::retype<T>(raw::iterator::next());
		}
	};
}

// Each level hides the methods of the one under it that take or give an element, and is made
// of the raw class it wraps: a list<T> is a collection_api<T, raw::list> inside a
// list_api<T, raw::list>, an array_list<T> the same on raw::array_list. That keeps the class's
// own java name for the jni signatures. The inherited constructors don't include the one from
// Raw (it's the base's own), so each level has it.
//
// A generic class converts to the ones it extends the way every sdk class does (see
// JABA_CLASS_END): array_list<string> goes where a list<string> or a collection<> is asked
// for. The element type isn't checked there either
namespace jaba::util::detail
{
	template <class T, class Raw>
	class iterable_api : public Raw
	{
	public:
		using Raw::Raw;

		iterable_api() = default;

		explicit iterable_api(const Raw& raw)
			: Raw(raw) { }

		[[nodiscard]] iterator<T> iter() const
		{
			return retype<iterator<T>>(Raw::iter());
		}

		[[nodiscard]] cursor<T> begin() const
		{
			return cursor<T>{ Raw::iter() };
		}

		[[nodiscard]] sentinel end() const noexcept
		{
			return { };
		}
	};

	template <class T, class Raw>
	class collection_api : public iterable_api<T, Raw>
	{
		using base = iterable_api<T, Raw>;

	public:
		using base::base;

		collection_api() = default;

		explicit collection_api(const Raw& raw)
			: base(raw) { }

		[[nodiscard]] jboolean contains(const T& value) const
		{
			return Raw::contains(erase(value));
		}

		jboolean add(const T& value) const
		{
			return Raw::add(erase(value));
		}

		jboolean remove(const T& value) const
		{
			return Raw::remove(erase(value));
		}

		jboolean add_all(const collection<T>& other) const
		{
			return Raw::add_all(other);
		}
	};

	template <class T, class Raw>
	class list_api : public collection_api<T, Raw>
	{
		using base = collection_api<T, Raw>;

	public:
		using base::base;

		list_api() = default;

		explicit list_api(const Raw& raw)
			: base(raw) { }

		[[nodiscard]] T get(const jint index) const
		{
			return retype<T>(Raw::get(index));
		}

		// the element that was there
		T set(const jint index, const T& value) const
		{
			return retype<T>(Raw::set(index, erase(value)));
		}

		void insert(const jint index, const T& value) const
		{
			Raw::insert(index, erase(value));
		}

		T remove_at(const jint index) const
		{
			return retype<T>(Raw::remove_at(index));
		}

		[[nodiscard]] jint index_of(const T& value) const
		{
			return Raw::index_of(erase(value));
		}

		[[nodiscard]] list<T> sub_list(const jint from, const jint to) const
		{
			return retype<list<T>>(Raw::sub_list(from, to));
		}
	};
}

namespace jaba::util
{
	template <class T>
	class array_list : public detail::list_api<T, raw::array_list>
	{
		using base = detail::list_api<T, raw::array_list>;

	public:
		using base::base;

		array_list() = default;

		explicit array_list(const raw::array_list& raw)
			: base(raw) { }

		[[nodiscard]] static array_list create()
		{
			return detail::retype<array_list>(raw::array_list::create());
		}

		[[nodiscard]] static array_list create(const jint capacity)
		{
			return detail::retype<array_list>(raw::array_list::create(capacity));
		}

		[[nodiscard]] static array_list create(const collection<T>& other)
		{
			return detail::retype<array_list>(raw::array_list::create(other));
		}
	};

	template <class T>
	class hash_set : public detail::collection_api<T, raw::hash_set>
	{
		using base = detail::collection_api<T, raw::hash_set>;

	public:
		using base::base;

		hash_set() = default;

		explicit hash_set(const raw::hash_set& raw)
			: base(raw) { }

		[[nodiscard]] static hash_set create()
		{
			return detail::retype<hash_set>(raw::hash_set::create());
		}

		[[nodiscard]] static hash_set create(const jint capacity)
		{
			return detail::retype<hash_set>(raw::hash_set::create(capacity));
		}

		[[nodiscard]] static hash_set create(const collection<T>& other)
		{
			return detail::retype<hash_set>(raw::hash_set::create(other));
		}
	};
}
