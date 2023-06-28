# Egel ahead-of-time compilation

This was a failed experiment. I chose GNU lightning as a back-end
for ahead-of-time compilation, you can instruct the interpreter
to replace all bytecode combinators with equivalent native code
combinators.

Running on a microbenchmark of summing a list of a million
numbers, performance decreased by a factor 2. This turned
out to be due to lazily loading the dynamic library that
causes an extra indirection.

Including the jit in the interpreter gave a 2% boost. Not
lazily loading the dynamic library a 1% boost.

That's unfortunately not enough of a performance increase for
me to keep maintaining this library.
