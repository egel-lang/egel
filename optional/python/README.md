## Python-Egel bridge

The Python bridge is a separate module which allows the Egel 
interpreter to bridge to Python code. At some point, the reverse
will also be added.

This bridge is a separate package you can install independent of the
interpreter. Compiling and installing follows the default cmake
route.

Both Egel and Python need to be installed on your system to compile
the bridge.

```
    user$ mkdir build
    user$ cd build
    user$ cmake ..
    user$ sudo make install
```

This bridge supports minimal functionality. Unfortunately, the 
current Python 3.10 seems buggy and unstable so this development
is halted for a while.

An example can be found in the `example` directory.

