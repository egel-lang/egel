# Egel ahead-of-time compilation

This was a failed experiment. I chose GNU lightning as a back-end
for ahead-of-time compilation, you can instruct the interpreter
to replace all bytecode combinators with equivalent native code
combinators.

Running on a microbenchmark of summing a list of a million
numbers, performance decreased by a factor 2.
