# Kobalt
![build](https://github.com/abel0b/kobalt/workflows/build/badge.svg)

Kobalt is a general-purpose programming language in the *very* early stages of development. 

## Build from sources
### Requirements
- a C compiler : clang, msvc, gcc
- [premake5](https://premake.github.io) to generate platform-specific project files. You can download the self-contained binary [here](https://premake.github.io/download.html).

### Instructions
Premake can generate project files for GNU make, Visual Studio [and more](https://github.com/premake/premake-core/wiki/Using-Premake).

To use Makefile with clang, run the following commands.
```shell
premake5 --cc=clang gmake2
make -j8 config=release
```

If you work on Windows, you may prefer to use Microsoft toolchain.
```shell
premake5 --cc=cl vs2019
msbuild -m -p:Configuration=release kobalt.sln
```

You can choose the desired C compiler via --cc option. It must support C99 features.

The project has two configurations : debug and release. Release is intended for language users. Debug provide additional logging and debugging features for hacking on Kobalt compiler.

Binaries will be generated in bin directory.

## Resources
#### [Examples](https://github.com/abel0b/kobalt/tree/master/doc/examples)
Showcase programs in Kobalt.
