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

  MacOS runs with some elaborate security mechanism for trusted
  apps these days and the application and dynamic libraries are
  _not_ signed with a developer certificate.

  I was able to run the interpreter on MacOS by signing the
  the binaries and removing the quarantine attribute.

  For signing, make a local certificate and use codesign

```
    codesign --force --deep --sign "Local Development" <library.dylib>

```

  To remove the quarantine attribute use xattr.

```
    xattr -d com.apple.quarantine egel lib*
```

* A BSD release is planned for the future.

