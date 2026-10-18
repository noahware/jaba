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

### Methods

```cpp
JABA_CLASS_BEGIN(player_class, "net/minecraft/client/entity/EntityPlayerSP")
    JABA_METHOD(void, set_sprinting, "setSprinting", jboolean)
    JABA_METHOD(void, jump, "jump")
JABA_CLASS_END(player_class)

player.set_sprinting(true);
player.jump();
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

Hooks that could not find their method (e.g. mappings not loaded yet) are queued, and can be placed with `jaba::flush_queued_hooks`:

```cpp
jaba::flush_queued_hooks();
```

You should call `jaba::flush_queued_hooks` once mappings are loaded and Jaba is initialized.

### Mappings

Classes, fields and methods are defined by their unobfuscated names, `jaba::load_mappings` maps them to the obfuscated names:

```cpp
jaba::load_mappings({
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

`jaba::set_find_class_fn` replaces the lookup. It takes a `jclass(*)(zstring_view)` that receives the (mapped) class name and returns the class, or `nullptr` if it could not be found (Jaba then throws `unable to find class '<name>'`).

```cpp
// the default one already does FindClass, so here use a classloader or something
jclass find_class_replacement(const jaba::zstring_view name)
{
    JNIEnv* const env = jaba::jni::env();

    return env->FindClass(name.data());
}

jaba::set_find_class_fn(find_class_replacement);
```

Set it before any class is first used, as the result of the lookup is cached for the lifetime of the program.

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
