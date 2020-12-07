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
