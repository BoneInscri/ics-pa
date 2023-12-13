1. **-flto**

This option runs the standard link-time optimizer.

When invoked with source code, it generates GIMPLE (one of GCC’s internal representations) and writes it to special ELF sections in the object file. 

**When the object files are linked together, all the function bodies are read from these ELF sections and instantiated as if they had been part of the same translation unit.**

It is recommended that you compile all the files participating in the same link with the same options and also specify those options **at link time.**



2. **-Og**

Like ‘-O0’, ‘-Og’ completely **disables a number of optimizatio**n passes so that individual options controlling them have no effect.

debug 模式可以开启-Og进行调试，去除代码优化！





3. **-fsanitize=address**

Enable **AddressSanitizer**, a fast memory error detector. Memory access instructions are instrumented to detect out-of-bounds and use-after-free bugs.

检测代码越界和释放后使用的bug！



4. **-ggdb**

Produce debugging information for use by GDB. This means to use the most expressive format available (DWARF, stabs, or the native format if neither of those are supported), including GDB extensions if at all possible.



5. **-I**

Include path specified with **-I** and -L ‘devices.csv’ will be searched for in each of the directories specified by include paths and linker library search paths.





6. **-fstack-protector**

Emit extra code to check for buffer overflows, such as stack smashing attacks. This is done by adding a guard variable to functions with vulnerable objects. This includes functions that call alloca, and functions with buffers larger than or equal to 8 bytes. The guards are initialized when a function is entered and then checked when the function exits. **If a guard check fails, an error message is printed and the program exits.** Only variables that are actually allocated on the stack are considered, optimized away variables or variables allocated in registers don’t count.





7. **-ffreestanding**

Assert that compilation targets a freestanding environment

A freestanding environment is one in which **the standard library may not exist**, and program startup may not necessarily be at main. **The most obvious example is an OS kernel.**



8. **-fno-rtti**

Disable generation of information about every class with virtual functions for use by the C++ run-time type identification features (dynamic_cast and typeid). **If you don’t use those parts of the language, you can save some space by using this flag.** Note that exception handling uses the same information, but G++ generates it as needed. The dynamic_cast operator can still be used for casts that do not require run-time type information, i.e. casts to void * or to unambiguous base classes.



9. **-fdata-sections**

Place each function or data item into its own section in the output file if the target supports arbitrary sections. The name of the function or the name of the data item determines the section’s name in the output file.

