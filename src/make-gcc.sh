CXX=/usr/local/gcc-12.x.0/bin/g++-12.x

$CXX -c -std=c++20 -fmodules-ts -x c++ utils.ixx
$CXX -c -std=c++20 -fmodules-ts -x c++ constants.ixx
$CXX -c -std=c++20 -fmodules-ts -x c++ position.ixx
$CXX -c -std=c++20 -fmodules-ts -x c++ reader.ixx
$CXX -c -std=c++20 -fmodules-ts -x c++ error.ixx
$CXX -c -std=c++20 -fmodules-ts -x c++ lexical.ixx
