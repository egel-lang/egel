# Prebuilt releases

Prebuilt releases from Github are supplied for your convenience.

* Ubuntu releases come with GNU Lightning, ICU, FMT, and FFI.

  Assuming bash as the shell and Egel installed in ~/.egel it is
  recommended to add the following two lines to .bashrc.

```
    alias egel="LD_LIBRARY_PATH=~/.egel ~/.egel/egel" export
    EGEL_PATH=.:$HOME/.egel
```


* MacOS and Windows releases come with the four dependencies as
  dynamic libraries.

* A BSD release is planned for the future.

