# Kobalt
![build](https://github.com/abel0b/kobalt/workflows/build/badge.svg)

The versatile language and toolchain for pragmatic programmers.

***Warning!** The project is in the very early stages of development. Note that most features are either only partially implemented or not implemented at all.* [What works already?](https://github.com/abel0b/kobalt/tree/master/doc/examples)

## Getting started
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

#### [Releases](https://github.com/abel0b/kobalt/releases)
Last kobalt releases and changelog.

## Legal
This is free software; you can redistribute it and/or modify it under the terms of the MIT license. See LICENSE for details.
