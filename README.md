# jaba

Modern C++ library which natively hooks java methods. The library requires at least C++ 20. Tested on x64 Windows and Linux on JDK 8, 17, 25. This basically rewrites internal JVM method structures to give the user a *native* JNI handler as the detour. It also does **not** use JVMTI for deoptimising methods.

The idea for the hooking method was made in collaboration with [Lefraudeur](https://github.com/Lefraudeur).

# Examples

## Setting up

Before setting up any hooks, call the `init` function:

```cpp
if (jaba::init())
    LOG("jaba initialised successfully");
else
    LOG("jaba failed to initialise");
```

## Hook detours

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

## Placing hooks

`jaba::hook_method` takes the jmethodID and detour routine. It returns an optional jmethodID (present if hook succeeded) which is the callable original method.

```cpp
const jmethodID set_angles_mid = set_angles_method();

/*std::optional<jmethodID>*/ orig_set_angles = jaba::hook_method(set_angles_mid, set_angles_detour);

if (!orig_set_angles)
    LOG("failed to hook set angles");
```

## Removing hooks

`jaba::unhook_method` takes the the hooked jmethodID and removes it:

```cpp
if (!jaba::unhook_method(set_angles_mid))
    LOG("unable to unhook set angles");
```

# How does it work

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
