# jaba

Modern C++ library which natively hooks java methods. The library requires at least C++ 20. Tested on Windows and JDK 8, 17, 25.

# How does it work

This doesn't rely on JVMTI for deoptimising methods. Instead, it calls internal JVM functions to deoptimise methods.

# Examples

# Building tests

To build the test Windows DLL (targets Minecraft 1.8.9), run the following commands:

```
cmake -B build
cmake --build build --config Release
```
