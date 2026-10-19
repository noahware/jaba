#pragma once
#include "sdk/util/collections.hpp"

namespace jaba::util
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
