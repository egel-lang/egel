#include "../../../src/runtime.hpp"
#include "ffi.hpp"

int answer() {
    return 42;
}

extern "C" std::vector<UnicodeString> egel_imports() {
    return std::vector<UnicodeString>();
}

int mem = 0;

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(ffi0<vm_int_t>(vm, "Test", "answer", [](){ return (vm_int_t) answer(); } ).clone());
    oo.push_back(ffi0<vm_text_t>(vm, "Test", "hello", [](){ return UnicodeString("hello world!"); } ).clone());
    oo.push_back(ffi1<vm_text_t, vm_int_t>(vm, "Test", "char", [](vm_int_t n){ return UnicodeString((UChar32)  n); } ).clone());

    oo.push_back(ffi0<vm_ptr_t>(vm, "Test", "mem", [](){ return (vm_ptr_t) &mem ; } ).clone());
    oo.push_back(ffi1<vm_int_t, vm_ptr_t>(vm, "Test", "peek", [](vm_ptr_t p){ return (vm_int_t) ( *((int*)p) ) ; } ).clone());
    oo.push_back(ffi2<vm_int_t, vm_ptr_t, vm_int_t>(vm, "Test", "poke", [](vm_ptr_t p, vm_int_t n){ return (vm_int_t) ( *((int*)p) = (int) n ) ; } ).clone());


    return oo;

}

