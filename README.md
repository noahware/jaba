# jaba

Modern C++ library for interacting with Java apps (including natively hooking Java methods). The library requires at least C++ 20. Tested on x64 Windows and Linux on JDK 8, 17, 25. For the hooks, this rewrites internal JVM method structures to give the user a *native* JNI handler as the detour. It also does **not** use JVMTI for deoptimising methods.

The idea for the hooking method was made in collaboration with [Lefraudeur](https://github.com/Lefraudeur).

# Examples

## Setting up

Before using the library, call the `init` function:

```cpp
if (jaba::init())
    LOG("jaba initialised successfully");
else
    LOG("jaba failed to initialise");
```

You can either use the integrated JNI interface and define hooks from there, or define **standalone** hooks.

## JNI integration
### Defining classes

```cpp
JABA_CLASS_BEGIN(minecraft, "net/minecraft/client/Minecraft")
JABA_CLASS_END(minecraft)
```

Classes can inherit from other defined classes (including multiple), gaining all of their fields and methods:

```cpp
JABA_CLASS_BEGIN(entity_pos, "net/minecraft/entity/Entity")
    JABA_FIELD(jdouble, x, "posX")
    JABA_FIELD(jdouble, y, "posY")
    JABA_FIELD(jdouble, z, "posZ")
JABA_CLASS_END(entity_pos)

JABA_CLASS_BEGIN(living_base, "net/minecraft/entity/EntityLivingBase")
    JABA_FIELD(jint, hurt_time, "hurtTime")
    JABA_FIELD(jfloat, move_forward, "moveForward")
JABA_CLASS_END(living_base)

JABA_CLASS_BEGIN(local_player, "net/minecraft/client/entity/EntityPlayerSP", entity_pos, living_base)
JABA_CLASS_END(local_player)
```

### Fields

```cpp
JABA_CLASS_BEGIN(local_player, "net/minecraft/client/entity/EntityPlayerSP", entity_pos, living_base)
    JABA_FIELD(jboolean, on_ground, "onGround")
JABA_CLASS_END(local_player)

JABA_CLASS_BEGIN(minecraft, "net/minecraft/client/Minecraft")
    JABA_STATIC_FIELD(minecraft, mc, "theMinecraft")
    JABA_FIELD(local_player, player, "thePlayer")
JABA_CLASS_END(minecraft)

const local_player player = minecraft::mc->player;

const double x = player.x;
const int hurt_time = player.hurt_time;
const bool on_ground = player.on_ground;
```

A field can be assigned to, as long as the object is not `const`:

```cpp
local_player player = minecraft::mc->player;

player.on_ground = true;
```

### Methods

```cpp
JABA_CLASS_BEGIN(player_class, "net/minecraft/client/entity/EntityPlayerSP")
    JABA_METHOD(void, set_sprinting, "setSprinting", jboolean)
    JABA_METHOD(void, jump, "jump")
JABA_CLASS_END(player_class)

player.set_sprinting(true);
player.jump();
```

`JABA_STATIC_METHOD` is the same without an instance, it is called on the class:

```cpp
JABA_CLASS_BEGIN(minecraft, "net/minecraft/client/Minecraft")
    JABA_STATIC_METHOD(jint, get_debug_fps, "getDebugFPS")
JABA_CLASS_END(minecraft)

const int fps = minecraft::get_debug_fps();
```

### Integrated hooks

`JABA_HOOK` takes the name of a method defined in the class and a detour. The detour receives the instance followed by the original method's arguments, and the original is available as `<method>_mhk_original`:

```cpp
class entity_living_base;
void on_jump(const entity_living_base& self, void (*original)(const entity_living_base&));

JABA_CLASS_BEGIN(entity_living_base, "net/minecraft/client/entity/EntityPlayerSP", entity_pos, living_base)
    JABA_METHOD(void, jump, "jump")
    JABA_HOOK(jump, on_jump)
JABA_CLASS_END(entity_living_base)

void on_jump(const entity_living_base& self, const entity_living_base::jump_mhk_original original)
{
    LOG("jump called");

    original(self);
}
```

Hooks that could not find their method (e.g. mappings not loaded yet) are queued, and can be placed with `jaba::jni::flush_queued_hooks`:

```cpp
jaba::jni::flush_queued_hooks();
```

You should call `jaba::jni::flush_queued_hooks` once mappings are loaded and Jaba is initialized.

### Mappings

Classes, fields and methods are defined by their unobfuscated names, `jaba::jni::load_mappings` maps them to the obfuscated names. Names that are not in the map are used as they are:

```cpp
jaba::jni::load_mappings({
    { "net/minecraft/client/Minecraft", "ave" },
    { "net/minecraft/client/entity/EntityPlayerSP", "bew" },
    { "net/minecraft/entity/Entity", "pk" },
    { "posX", "s" },
    { "jump", "bF" },
    { "thePlayer", "h" },
});
```

### Class loader

Classes are looked up once (on first use) with `JNIEnv::FindClass` and cached as a global ref.

`jaba::jni::set_find_class_fn` replaces the lookup. It takes a `jclass(*)(zstring_view)` that receives the (mapped) class name and returns the class, or `nullptr` if it could not be found (Jaba then throws `unable to find class '<name>'`).

```cpp
// the default one already does FindClass, so here use a classloader or something
jclass find_class_replacement(const jaba::zstring_view name)
{
    JNIEnv* const env = jaba::jni::env();

    return env->FindClass(name.data());
}

jaba::jni::set_find_class_fn(find_class_replacement);
```

Set it before any class is first used, as the result of the lookup is cached for the lifetime of the program.

### Constructors

`JABA_CONSTRUCTOR` takes the argument types of a constructor. Constructors overload like in Java, so they all share one name:

```cpp
JABA_CLASS_BEGIN(vec3, "net/minecraft/util/Vec3")
    JABA_CONSTRUCTOR(create, jdouble, jdouble, jdouble)
    JABA_CONSTRUCTOR(create, jint)
JABA_CLASS_END(vec3)

const vec3 vec = vec3::create(1.0, 2.0, 3.0);
```

### Arrays

`jaba::jni::array<T>` is a Java array. `T` is a JNI primitive (`jint`, `jdouble`, ...), a class, or another array. It can be a field, an argument or a return type:

```cpp
JABA_CLASS_BEGIN(item_stack, "net/minecraft/item/ItemStack")
JABA_CLASS_END(item_stack)

JABA_CLASS_BEGIN(inventory, "net/minecraft/entity/player/InventoryPlayer")
    JABA_FIELD(jaba::jni::array<item_stack>, main_inventory, "mainInventory")
JABA_CLASS_END(inventory)

const jaba::jni::array<item_stack> items = inv.main_inventory;

const size_t size = items.size();
const item_stack first = items.get(0);

items.set(1, first);
```

`[]` reads and writes an element the same way, and it works on arrays of arrays too:

```cpp
jaba::jni::array<jint> heights = ...;

heights[0] = 64;
const int height = heights[0];

grid[1][2] = 0.5;
```

`view` is a lazy range of the elements, every element is one JNI call:

```cpp
jint total = 0;

for (const jint height : heights.view())
    total += height;
```

An index outside of the array throws a `std::out_of_range`, and a null array is empty. Arrays are created with `create`, the elements start as zero or null (`T` can not be an array here):

```cpp
jaba::jni::array<jint> heights = jaba::jni::array<jint>::create(16);

heights.set(0, 64);
```

### References

A class holds a JNI reference and deletes it when it is destroyed, a copy makes a reference of its own. These are local references, which are only valid on the thread they were made on. To keep an object around (in a global, or to use it on another thread) make it global with `as_global`:

```cpp
const local_player player = minecraft::mc->player;

saved_player = player.as_global();
```

`is_global` and `is_local` tell which one it is. `==` compares the objects and not the references, a copy is equal to the one it was made from.

`release` gives the reference up without deleting it, to return an object from a native method, and `reset` deletes it early. A class made from a raw `jobject` takes ownership of the reference, so give it a copy if the reference is not yours:

```cpp
const local_player player{ env->NewLocalRef(obj) };
```

## Java SDK

`sdk/sdk.hpp` has the Java standard library defined as Jaba classes, so you don't have to define `String` or `ArrayList` yourself. Only what exists on Java 8 and up is in it.

Packages are namespaces without the `java` prefix, and `java.lang` has no namespace. `java.lang.String` is `jaba::string` and `java.util.ArrayList` is `jaba::util::array_list`. The names are snake_case, with an underscore at the end where the name is a C++ keyword (`long_`, `float_`, `double_`, `class_`).

### Strings

A string can be made from a literal or a `std::string`, and `to_std` reads it back:

```cpp
const jaba::string str = "hello";

const int length = str.length();
const std::string upper = str.to_upper_case().to_std();
```

Strings add, compare and print like the C++ ones. `+` is Java's concatenation, anything that is not text goes through `String.valueOf` (and a null is `"null"`). `std::format` prints any SDK object as its `toString`, `to_std` does the same as a `std::string`:

```cpp
jaba::string greeting = str + " world " + 5;

greeting += '!';

if (greeting == "hello world 5!")
    LOG("{}", greeting);
```

### Collections

Constructors are always called `create`. Literals and numbers are boxed when an `object` is asked for, and any iterable works in a range-for:

```cpp
jaba::util::array_list list = jaba::util::array_list::create();

list.add("one");
list.add(2);

for (const jaba::object& obj : list)
    LOG("{}", obj);
```

Generics are erased in Java, so elements are `object`. `as<T>` casts one without checking, `try_as<T>` checks first:

```cpp
const jaba::integer two = list.get(1).as<jaba::integer>();
```

A class converts to the classes it extends, so an `integer` can be passed where a `number` or `object` is asked for. Only exact types are boxed, a `char` becomes an `Integer` and the wider integer types are ambiguous (cast them).

### Comparing

`==` on the SDK classes is Java's `equals` and `<` is `compareTo` (strings and the boxed numbers), `std::hash` is `hashCode`. They work as keys:

```cpp
std::map<jaba::string, int> counts;
std::unordered_set<jaba::integer> seen;
```

A class that is not from the SDK has no `equals`, so `==` on it is whether it is the same object (`same_as`), and it can not be ordered.

### Exceptions

A Java exception thrown by a call is rethrown as a `jaba::jni::java_exception`. It is a `std::runtime_error` that keeps what was thrown, and `is_a` checks the type, subclasses included:

```cpp
try
{
    jaba::integer::parse_int("abc");
}
catch (const jaba::jni::java_exception& e)
{
    LOG("{}: {}", e.class_name(), e.message());

    if (e.is_a<jaba::illegal_argument_exception>())
        LOG("bad argument");
}
```

`as<T>` gives the throwable as a class, and the exception can be copied and kept as it holds a global reference.

### Adding classes

Anything that is not defined can be added like any other class. Extend an SDK class to get its methods, `jaba::object` gives `to_string`, `hash_code`, `equals` and `get_class`:

```cpp
JABA_CLASS_BEGIN(stack, "java/util/Stack", jaba::util::collection)
    JABA_METHOD(jaba::object, push, "push", jaba::object)
JABA_CLASS_END(stack)
```

A class should only extend one chain of classes. Reaching the same base twice (a diamond) is ambiguous.

## Standalone hooks
### Detours

The detours take a JNI env as arg0, the jobject of the instance as arg1, then the hooked function signature is pushed as the remaining arguments.

For example, for setAngles which has the signature `(FF)V` (in C - `void(float yaw, float pitch)`), the detour would look as follows:

```cpp
std::optional<jmethodID> orig_set_angles = nullptr;

void set_angles_detour(JNIEnv* const env, const jobject obj, const jfloat yaw, const jfloat pitch)
{
    env->CallVoidMethod(obj, *orig_set_angles, yaw, pitch);

    LOG("set angles called {} {}", yaw, pitch);
}
```

### Placing

`jaba::hook_method` takes the jmethodID and detour routine. It returns an optional jmethodID (present if hook succeeded) which is the callable original method.

```cpp
const jmethodID set_angles_mid = set_angles_method();

/*std::optional<jmethodID>*/ orig_set_angles = jaba::hook_method(set_angles_mid, set_angles_detour);

if (!orig_set_angles)
    LOG("failed to hook set angles");
```

### Removing

`jaba::unhook_method` takes the the hooked jmethodID and removes it:

```cpp
if (!jaba::unhook_method(set_angles_mid))
    LOG("unable to unhook set angles");
```

# How do hooks work

The internal JVM structure of the method is cloned, and two pointers are inserted at the end of it:

- The detour (native) method handler.
- Nullptr for the signature handler (JVM will populate it).

The copied method is also marked as native.

Instead of using JVMTI, it calls internal JVM functions to deoptimise methods. The method and all potentially inlined call sites are deoptimised through calls to these JVM functions.

The method is then forced to be non-compilable and non-inlineable. The `from_interpreted` and `i2i` routines (stubs the JVM calls to transition into the method) are set to an assembly stub we write. In these transition routines, `rbx` holds the method pointer. The assembly checks if that method is hooked, and if it is then `rbx` is set to the *cloned* copy.

The stub then jumps to the default native handler from the JVM, which processes `rbx` as a native C++ method, which calls the detour (user code) that was inserted to the method copy. The user now is in their detour with hooked function parameters already passed via the native calling convention.

# Tests

## Building
You must copy your JNI files (jni.h, jni.lib, etc) from your JDK directory to `vendor/jni/` or at have it includable under `<jni/jni.h>` and the library linked.

To build the test for your OS (Linux bundles a test java app, Windows targets Minecraft 1.8.9), do the following commands:

```
cmake -B build
cmake --build build --config Release
```

## Running Windows tests

Load the DLL into a javaw.exe instance of vanilla Minecraft 1.8.9 and it will log the yaw and pitch when `setAngles` is called.

# Credits

-  [Lefraudeur](https://github.com/Lefraudeur) for collaborating on the idea for this hooking method with me.

# License

The project uses the Apache-2.0 license.
