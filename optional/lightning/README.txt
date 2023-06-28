# Egel ahead-of-time compilation

This was a failed experiment. I chose GNU lightning as a back-end
for ahead-of-time compilation, you can instruct the interpreter
to replace all bytecode combinators with equivalent native code
combinators.

Running on a microbenchmark of summing a list of a million
numbers, performance decreased by a factor 2.

Including the jit in the interpreter gave a 2% boost. 

The numbers:
- baseline, no jit: 3.75s
- including in the runtime: 3.68s
- lazily loading the dynamic library: 8.7s
- eagerly loading the library: 13.6s

That's unfortunately not enough of a performance increase for
me to keep maintaining this library.
