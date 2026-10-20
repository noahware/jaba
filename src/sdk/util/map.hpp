#pragma once
#include "sdk/util/collections.hpp"

namespace jaba::util::raw
{
	// java.util.Map.Entry
	JABA_CLASS_BEGIN(map_entry, "java/util/Map$Entry", object)
		JABA_METHOD(object, get_key, "getKey")
		JABA_METHOD(object, get_value, "getValue")
		JABA_METHOD(object, set_value, "setValue", object)
	JABA_CLASS_END(map_entry)

	JABA_CLASS_BEGIN(map, "java/util/Map", object)
		JABA_METHOD(jint, size, "size")
		JABA_METHOD(jboolean, is_empty, "isEmpty")
		JABA_METHOD(jboolean, contains_key, "containsKey", object)
		JABA_METHOD(jboolean, contains_value, "containsValue", object)
		JABA_METHOD(object, get, "get", object)
		JABA_METHOD(object, put, "put", object, object)
		JABA_METHOD(object, remove, "remove", object)
		JABA_METHOD(void, clear, "clear")
		JABA_METHOD(set, key_set, "keySet")
		JABA_METHOD(collection, values, "values")
		JABA_METHOD(set, entry_set, "entrySet")
	JABA_CLASS_END(map)

	JABA_CLASS_BEGIN(hash_map, "java/util/HashMap", map)
		JABA_CONSTRUCTOR(create)
		JABA_CONSTRUCTOR(create, jint)
	JABA_CLASS_END(hash_map)
}

namespace jaba::util
{
	template <class K, class V>
	class map_entry : public raw::map_entry
	{
	public:
		using raw::map_entry::map_entry;

		map_entry() = default;

		explicit map_entry(const raw::map_entry& raw)
			: raw::map_entry(raw) { }

		[[nodiscard]] K get_key() const
		{
			return detail::retype<K>(raw::map_entry::get_key());
		}

		[[nodiscard]] V get_value() const
		{
			return detail::retype<V>(raw::map_entry::get_value());
		}

		// the value that was there
		V set_value(const V& value) const
		{
			return detail::retype<V>(raw::map_entry::set_value(detail::erase(value)));
		}
	};
}

namespace jaba::util::detail
{
	template <class K, class V, class Raw>
	class map_api : public Raw
	{
	public:
		using Raw::Raw;

		map_api() = default;

		explicit map_api(const Raw& raw)
			: Raw(raw) { }

		[[nodiscard]] jboolean contains_key(const K& key) const
		{
			return Raw::contains_key(erase(key));
		}

		[[nodiscard]] jboolean contains_value(const V& value) const
		{
			return Raw::contains_value(erase(value));
		}

		// null when there is no such key
		[[nodiscard]] V get(const K& key) const
		{
			return retype<V>(Raw::get(erase(key)));
		}

		// the value that was there
		V put(const K& key, const V& value) const
		{
			return retype<V>(Raw::put(erase(key), erase(value)));
		}

		V remove(const K& key) const
		{
			return retype<V>(Raw::remove(erase(key)));
		}

		[[nodiscard]] set<K> key_set() const
		{
			return retype<set<K>>(Raw::key_set());
		}

		[[nodiscard]] collection<V> values() const
		{
			return retype<collection<V>>(Raw::values());
		}

		[[nodiscard]] set<map_entry<K, V>> entry_set() const
		{
			return retype<set<map_entry<K, V>>>(Raw::entry_set());
		}
	};
}

namespace jaba::util
{
	template <class K, class V>
	class hash_map : public detail::map_api<K, V, raw::hash_map>
	{
		using base = detail::map_api<K, V, raw::hash_map>;

	public:
		using base::base;

		hash_map() = default;

		explicit hash_map(const raw::hash_map& raw)
			: base(raw) { }

		[[nodiscard]] static hash_map create()
		{
			return detail::retype<hash_map>(raw::hash_map::create());
		}

		[[nodiscard]] static hash_map create(const jint capacity)
		{
			return detail::retype<hash_map>(raw::hash_map::create(capacity));
		}
	};
}
