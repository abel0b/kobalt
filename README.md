# Kobalt
![build](https://github.com/abel0b/kobalt/workflows/build/badge.svg)
The versatile language and toolchain for pragmatic programmers.

## Ambitions
#### Versatile
This project aims to create a versatile ecosystem around the Kobalt programming language to make it incrementally-adoptable in an existing codebase. Thanks to a language-agnostic build system and compiler designed with interoperability in mind, Kobalt programs can integrates seamlessly with existing software, especially C/C++ ones.

#### Fast
The kobalt compiler produce fast machine code thanks via a C backend leveraging industry-proven optimising compilers. The compiler is designed for high performance, it uses a finely grained cache system to allow high speed incremental modifications.

#### Safe
Kobalt has a safe and robust type system inherited from the ML family of languages which guarantees the type of each expression at compile time. Execution is predictable and free of undefined behaviors such as NULL dereferencing, so you can be sure that a Kobalt program cannot raise related execution exceptions. By default, the language is designed to allow the programmer to manually manage the memory for producing a solution tailored to its use case. Yet, users are encouraged to use RAII-based safe memory abstractions whenever possible.

#### Pragmatic
The language leaves programmers a free hand to take advantage of a combination of programming paradigms: functional, imperative, or contract-based. Because Kobalt is interoperable, it can leverage and improve upon existing battle-tested libraries from the world's largest programming ecosystems : C/C++, JavaScript and Python.

#### Bloat-free
The toolchain enable users to produce unencumbered software that just work without bloat. The size and performance of binaries may never be burdened by an unused language feature.

***Warning!** The project is in the very early stages of development. Note that most features are either only partially implemented or not implemented at all.* [What works already?](https://github.com/abel0b/kobalt/tree/master/doc/examples)

## Getting started
### Download pre-built binaries
**TLDR.** You can download alpha version pre-built binaries for Windows, Linux and MacOs on the [releases page](https://github.com/abel0b/kobalt/releases).

### Build from sources
#### Requirements
- a C compiler such as clang, msvc or gcc
- [premake5](https://premake.github.io) to generate platform-specific project files. You can download the self-contained binary [here](https://premake.github.io/download.html).

#### Instructions
Premake can generate project files for GNU make, Visual Studio [and more](https://github.com/premake/premake-core/wiki/Using-Premake).

To use Makefile, run the following commands.
```shell
premake5 gmake2
make -j8 config=release
```

If you work on Windows, you may prefer to use Microsoft toolchain.
```shell
premake5 vs2019
msbuild -m -p:Configuration=release kobalt.sln
```

The project has two configurations : debug and release. Release is intended for language users. Debug provide additional logging and debugging features for hacking on Kobalt compiler.

Binaries will be generated in bin directory.

## Resources
#### [Examples](https://github.com/abel0b/kobalt/tree/master/doc/examples)
Showcase programs in Kobalt.

## Legal
This is free software; you can redistribute it and/or modify it under the terms of the MIT license. See LICENSE for details.
