# Egel Remote Procedure Calls

Egel's intended usage is to be able to transparently shoot 
combinators (programs) over the internet. This module builds on top
off grpc (Google's generic rpc implementation).

Dependency packages 'grpc', 'openssl', and 'protobuf-c'.

Follow the usual cmake route to build the module.

Note: on macos

    cmake .. -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/icu4c \
             -DOPENSSL_ROOT_DIR=/opt/homebrew/opt/openssl
