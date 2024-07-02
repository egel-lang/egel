#pragma once

#include "runtime.hpp"
#include "ffi.h"
#include <dlfcn.h>

/*

---------------------------------------------------------
| tag          | c language type    | egel representation
---------------------------------------------------------
| c_void       | void               | none
| c_bool       | bool               | true or false
| c_char       | char               | char
| c_wchar      | wchar_t            | text
| c_byte       | char               | int
| c_ubyte      | unsigned char      | int
| c_short      | short              | int
| c_ushort     | unsigned short     | int
| c_int        | int                | int
| c_uint       | unsigned int       | int
| c_long       | long               | int
| c_ulong      | unsigned long      | int
| c_longlong   | long long          | int
| c_ulonglong  | unsigned long long | int
| c_size_t     | size_t             | int
| c_ssize_t    | ssize_t            | int
| c_time_t     | time_t             | int
| c_float      | float              | float
| c_double     | double             | float
| c_longdouble | long double        | float
| c_char_p     | char*              | text or none
| c_wchar_p    | wchar_t*           | text or none
| c_void_p     | void*              | int or none
---------------------------------------------------------

*/

namespace egel {

const icu::UnicodeString FFI = "FFI";
const auto c_void = FFI + "::" + "c_void";
const auto c_bool = FFI + "::" + "c_bool";
const auto c_char = FFI + "::" + "c_char";
const auto c_wchar = FFI + "::" + "c_wchar";
const auto c_byte = FFI + "::" + "c_byte";
const auto c_ubyte = FFI + "::" + "c_ubyte";
const auto c_short = FFI + "::" + "c_short";
const auto c_ushort = FFI + "::" + "c_ushort";
const auto c_int = FFI + "::" + "c_int";
const auto c_uint = FFI + "::" + "c_uint";
const auto c_long = FFI + "::" + "c_long";
const auto c_ulong = FFI + "::" + "c_ulong";
const auto c_longlong = FFI + "::" + "c_longlong";
const auto c_ulonglong = FFI + "::" + "c_ulonglong";
const auto c_size_t = FFI + "::" + "c_size_t";
const auto c_ssize_t = FFI + "::" + "c_ssize_t";
const auto c_time_t = FFI + "::" + "c_time_t";
const auto c_float = FFI + "::" + "c_float";
const auto c_double = FFI + "::" + "c_double";
const auto c_longdouble = FFI + "::" + "c_longdouble";
const auto c_char_p = FFI + "::" + "c_char_p";
const auto c_wchar_p = FFI + "::" + "c_wchar_p";
const auto c_void_p = FFI + "::" + "c_void_p";

// ## FFI::library - opaque library object
class Library : public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_BUILTIN, Library, FFI, "library");

    int compare(const VMObjectPtr &o) {
        return -1; // XXX
    }

    void set_path(const icu::UnicodeString &s) {
        _path = s;
    }

    icu::UnicodeString get_path() {
        return _path;
    }
    void load(const icu::UnicodeString &s) {
        set_path(s);
        auto pth = VM::unicode_to_utf8_chars(get_path());  // XXX: leaks?
        _handle = dlopen(pth, RTLD_LAZY | RTLD_GLOBAL);
        if (!_handle) {
            icu::UnicodeString err = "dynamic load error: ";
            err += dlerror();
            err += " on open(" + get_path() + ")";
            throw ErrorIO(err);
        }

    }

    VMObjectPtr function(const icu::UnicodeString &s) {
        auto sym = VM::unicode_to_utf8_chars(s);  // XXX: leaks?
        auto ptr = dlsym(_handle, sym);
        VMObjectPtrs oo;
        oo.push_back(VMObjectData::create(machine(), c_void_p));
        oo.push_back(machine()->create_integer((long long)ptr));
        return machine()->create_array(oo);
    }

    void unload() {
        dlclose(_handle);
    }

private:
    icu::UnicodeString _path;
    void *_handle;
};


// ## FFI::find_library s - find a library
class FindLibrary : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, FindLibrary, FFI, "find_library");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_none(); // XXX: not implemented yet
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## FFI::load_library s - load a library
class LoadLibrary : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, LoadLibrary, FFI, "load_library");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            auto l = Library::create(machine());
            (std::static_pointer_cast<Library>(l))->load(s);
            return l;
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## FFI::function l s - find a function
class Function : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Function, FFI, "function");

    VMObjectPtr apply(const VMObjectPtr &arg0, const VMObjectPtr &arg1) const override {
        if (machine()->is_type(typeid(Library), arg0) && machine()->is_text(arg1)) {
            auto l = std::static_pointer_cast<Library>(arg0);
            auto s = machine()->get_text(arg1);
            return l->function(s);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## FFI::call f r x - call a function
class Call : public Triadic {
public:
    TRIADIC_PREAMBLE(VM_SUB_BUILTIN, Call, FFI, "call");

    bool is_function_ptr(const VMObjectPtr& o) const {
        return machine()->is_array(o) 
                && (machine()->array_size(o) == 2)
                && machine()->is_data_text(machine()->array_get(o,0), c_void_p)
                && machine()->is_integer(machine()->array_get(o,1));
    }

    bool is_return_type(const VMObjectPtr& o) const {
        return machine()->is_data(o) && (
          machine()->is_data_text(o, c_void)  || 
          machine()->is_data_text(o, c_bool)  || 
          machine()->is_data_text(o, c_char)  || 
          // machine()->is_data_text(o, c_wchar)  || 
          machine()->is_data_text(o, c_byte)  || 
          machine()->is_data_text(o, c_ubyte)  || 
          machine()->is_data_text(o, c_short)  || 
          machine()->is_data_text(o, c_ushort)  || 
          machine()->is_data_text(o, c_int)  || 
          machine()->is_data_text(o, c_uint)  || 
          machine()->is_data_text(o, c_long)  || 
          machine()->is_data_text(o, c_ulong)  || 
          machine()->is_data_text(o, c_longlong)  || 
          machine()->is_data_text(o, c_ulonglong)  || 
          machine()->is_data_text(o, c_size_t)  || 
          machine()->is_data_text(o, c_ssize_t)  || 
          machine()->is_data_text(o, c_time_t)  || 
          machine()->is_data_text(o, c_float)  || 
          machine()->is_data_text(o, c_double)  || 
          machine()->is_data_text(o, c_longdouble)  || 
          machine()->is_data_text(o, c_char_p)  || 
          // machine()->is_data_text(o, c_wchar_p)  || 
          machine()->is_data_text(o, c_void_p)  
        );
    }

    bool is_arg_type(const VMObjectPtr& o) const {
        if (machine()->is_array(o) && (machine()->array_size(o) == 2)) {

            auto o0 = machine()->array_get(o,0);
            auto o1 = machine()->array_get(o,1);

            return (
              (machine()->is_data_text(o0, c_bool) && machine()->is_bool(o1))  || 
              (machine()->is_data_text(o0, c_char) && machine()->is_char(o1))  || 
              // (machine()->is_data_text(o0, c_wchar) && machine()->is_char(o1))  || 
              (machine()->is_data_text(o0, c_byte) && machine()->is_integer(o1))  || 
              (machine()->is_data_text(o0, c_ubyte) && machine()->is_integer(o1))  || 
              (machine()->is_data_text(o0, c_short) && machine()->is_integer(o1)) || 
              (machine()->is_data_text(o0, c_ushort) && machine()->is_integer(o1)) || 
              (machine()->is_data_text(o0, c_int) && machine()->is_integer(o1)) || 
              (machine()->is_data_text(o0, c_uint) && machine()->is_integer(o1)) || 
              (machine()->is_data_text(o0, c_long) && machine()->is_integer(o1)) || 
              (machine()->is_data_text(o0, c_ulong) && machine()->is_integer(o1)) || 
              (machine()->is_data_text(o0, c_longlong) && machine()->is_integer(o1)) || 
              (machine()->is_data_text(o0, c_ulonglong) && machine()->is_integer(o1)) || 
              (machine()->is_data_text(o0, c_size_t) && machine()->is_integer(o1)) || 
              (machine()->is_data_text(o0, c_ssize_t) && machine()->is_integer(o1)) || 
              (machine()->is_data_text(o0, c_time_t) && machine()->is_integer(o1)) || 
              (machine()->is_data_text(o0, c_float) && machine()->is_float(o1)) || 
              (machine()->is_data_text(o0, c_double) && machine()->is_float(o1)) || 
              (machine()->is_data_text(o0, c_longdouble) && machine()->is_float(o1)) || 
              (machine()->is_data_text(o0, c_char_p) && (machine()->is_text(o1) || machine()->is_none(o1)))|| 
              // (machine()->is_data_text(o0, c_wchar_p) && (machine()->is_text(o1) || machine()->is_none(o1))) || 
              (machine()->is_data_text(o0, c_void_p) && (machine()->is_integer(o1) || machine()->is_none(o1))) 
            );
        } else {
            return false;
        }
    }

    ffi_type* to_ffi_type(const VMObjectPtr &o) const {
        if ( machine()->is_data_text(o, c_void) ) {
            return &ffi_type_void;
        } else if ( machine()->is_data_text(o, c_bool) ) {
            return &ffi_type_sint32;
        } else if ( machine()->is_data_text(o, c_char) ) {
            return &ffi_type_uint8;
        /*
        } else if ( machine()->is_data_text(o, c_wchar) ) {
            return &ffi_type_w_char;
        */
        } else if ( machine()->is_data_text(o, c_byte) ) {
            return &ffi_type_sint8;
        } else if ( machine()->is_data_text(o, c_ubyte) ) {
            return &ffi_type_uint8;
        } else if ( machine()->is_data_text(o, c_short) ) {
            return &ffi_type_sint16;
        } else if ( machine()->is_data_text(o, c_ushort) ) {
            return &ffi_type_uint16;
        } else if ( machine()->is_data_text(o, c_int) ) {
            return &ffi_type_sint32;
        } else if ( machine()->is_data_text(o, c_uint) ) {
            return &ffi_type_uint32;
        } else if ( machine()->is_data_text(o, c_long) ) {
            return &ffi_type_sint64;
        } else if ( machine()->is_data_text(o, c_ulong) ) {
            return &ffi_type_uint64;
        } else if ( machine()->is_data_text(o, c_longlong) ) {
            return &ffi_type_sint64;
        } else if ( machine()->is_data_text(o, c_ulonglong) ) {
            return &ffi_type_uint64;
        } else if ( machine()->is_data_text(o, c_size_t) ) {
            return &ffi_type_uint64;
        } else if ( machine()->is_data_text(o, c_ssize_t) ) {
            return &ffi_type_sint64;
        } else if ( machine()->is_data_text(o, c_time_t) ) {
            return &ffi_type_uint64;
        } else if ( machine()->is_data_text(o, c_float) ) {
            return &ffi_type_float;
        } else if ( machine()->is_data_text(o, c_double) ) {
            return &ffi_type_double;
        } else if ( machine()->is_data_text(o, c_longdouble) ) {
            return &ffi_type_longdouble;
        } else if ( machine()->is_data_text(o, c_char_p) ) {
            return &ffi_type_pointer;
        /*
        } else if ( machine()->is_data_text(o, c_wchar_p) ) {
            return &ffi_type_wchar_p;
        */
        } else if ( machine()->is_data_text(o, c_void_p)  ) {
            return &ffi_type_pointer;
        } else {
            PANIC("ffi type expected");
            return nullptr; // keep compiler happy
        }
    }

    void* to_ffi_value(const VMObjectPtr &o) const {
        if (machine()->is_array(o) && (machine()->array_size(o) == 2)) {

            auto o0 = machine()->array_get(o,0);
            auto o1 = machine()->array_get(o,1);
            if ( machine()->is_data_text(o0, c_bool) ) {
                void* p = malloc(sizeof(bool));
                if (machine()->is_false(o1)) {
                    *(static_cast<bool*>(p)) = false;
                } else {
                    *(static_cast<bool*>(p)) = true;
                }
                return p;
            } else if ( machine()->is_data_text(o0, c_char) ) {
                void* p = malloc(sizeof(char));
                *(static_cast<char*>(p)) = static_cast<char>(machine()->get_char(o1));
                return p;
            /*
            } else if ( machine()->is_data_text(o0, c_wchar) ) {
                return (void*) ( (char) machine()->get_char(o1) );
            */
            } else if ( machine()->is_data_text(o0, c_byte) ) {
                void* p = malloc(sizeof(char));
                *(static_cast<char*>(p)) = static_cast<char>(machine()->get_integer(o1));
                return p;
            } else if ( machine()->is_data_text(o0, c_ubyte) ) {
                void* p = malloc(sizeof(char));
                *(static_cast<char*>(p)) = static_cast<char>(machine()->get_integer(o1));
                return p;
            } else if ( machine()->is_data_text(o0, c_short) ) {
                void* p = malloc(sizeof(short));
                *(static_cast<short*>(p)) = static_cast<short>(machine()->get_integer(o1));
                return p;
            } else if ( machine()->is_data_text(o0, c_ushort) ) {
                void* p = malloc(sizeof(unsigned short));
                *(static_cast<unsigned short*>(p)) = static_cast<unsigned short>(machine()->get_integer(o1));
                return p;
            } else if ( machine()->is_data_text(o0, c_int) ) {
                void* p = malloc(sizeof(int));
                *(static_cast<int*>(p)) = static_cast<int>(machine()->get_integer(o1));
                return p;
            } else if ( machine()->is_data_text(o0, c_uint) ) {
                void* p = malloc(sizeof(unsigned int));
                *(static_cast<unsigned int*>(p)) = static_cast<unsigned int>(machine()->get_integer(o1));
                return p;
            } else if ( machine()->is_data_text(o0, c_long) ) {
                void* p = malloc(sizeof(long));
                *(static_cast<long*>(p)) = static_cast<long>(machine()->get_integer(o1));
                return p;
            } else if ( machine()->is_data_text(o0, c_ulong) ) {
                void* p = malloc(sizeof(unsigned long));
                *(static_cast<unsigned long*>(p)) = static_cast<unsigned long>(machine()->get_integer(o1));
                return p;
            } else if ( machine()->is_data_text(o0, c_longlong) ) {
                void* p = malloc(sizeof(long long));
                *(static_cast<long long*>(p)) = static_cast<long long>(machine()->get_integer(o1));
                return p;
            } else if ( machine()->is_data_text(o0, c_ulonglong) ) {
                void* p = malloc(sizeof(unsigned long long));
                *(static_cast<unsigned long long*>(p)) = static_cast<unsigned long long>(machine()->get_integer(o1));
                return p;
            } else if ( machine()->is_data_text(o0, c_size_t) ) {
                void* p = malloc(sizeof(size_t));
                *(static_cast<size_t*>(p)) = static_cast<size_t>(machine()->get_integer(o1));
                return p;
            } else if ( machine()->is_data_text(o0, c_ssize_t) ) {
                void* p = malloc(sizeof(ssize_t));
                *(static_cast<ssize_t*>(p)) = static_cast<ssize_t>(machine()->get_integer(o1));
                return p;
            } else if ( machine()->is_data_text(o0, c_time_t) ) {
                void* p = malloc(sizeof(time_t));
                *(static_cast<time_t*>(p)) = static_cast<time_t>(machine()->get_integer(o1));
                return p;
            } else if ( machine()->is_data_text(o0, c_float) ) {
                void* p = malloc(sizeof(float));
                *(static_cast<float*>(p)) = static_cast<float>(machine()->get_float(o1));
                return p;
            } else if ( machine()->is_data_text(o0, c_double) ) {
                void* p = malloc(sizeof(double));
                *(static_cast<double*>(p)) = static_cast<double>(machine()->get_float(o1));
                return p;
            } else if ( machine()->is_data_text(o0, c_longdouble) ) {
                void* p = malloc(sizeof(long double));
                *(static_cast<long double*>(p)) = static_cast<long double>(machine()->get_float(o1));
                return p;
            } else if ( machine()->is_data_text(o0, c_char_p) ) {
                void* p = malloc(sizeof(void*));
                if (machine()->is_none(o1)) {
                    *(static_cast<char**>(p)) = nullptr;
                    return p;
                } else {
                    auto s = machine()->get_text(o1);
                    auto cc = VM::unicode_to_utf8_chars(s);
                    *(static_cast<char**>(p)) = reinterpret_cast<char*>(cc);
                    return p;
                }
            /*
            } else if ( machine()->is_data_text(o0, c_wchar_p) ) {
                if (machine()->is_none(o1)) {
                    return nullptr;
                } else {
                    auto s = machine()->get_text(o1);
                    return // no wchar
                }
            */
            } else if ( machine()->is_data_text(o0, c_void_p)  ) {
                void* p = malloc(sizeof(void*));
                if (machine()->is_none(o1)) {
                    *(static_cast<void**>(p)) = nullptr;
                    return p;
                } else {
                    *(static_cast<void**>(p)) = reinterpret_cast<void*>(machine()->get_integer(o1));
                    return p;
                }
            } else {
                PANIC("ffi value expected");
                return nullptr; // keep compiler happy
            }
        } else {
            PANIC("ffi value expected");
            return nullptr; // keep compiler happy
        }
    }

    VMObjectPtr from_ffi_value(const VMObjectPtr &t, void* v) const {
        VMObjectPtrs oo;
        oo.push_back(t);

        if ( machine()->is_data_text(t, c_void) ) {
            oo.push_back(machine()->create_none());
        } else if ( machine()->is_data_text(t, c_bool) ) {
            bool b = *(reinterpret_cast<bool*>(v));
            if (b) {
                oo.push_back(machine()->create_true());
            } else {
                oo.push_back(machine()->create_false());
            }
        } else if ( machine()->is_data_text(t, c_char) ) {
            UChar32 c = *(static_cast<UChar32*>(v));
            oo.push_back(machine()->create_char(c));
        //} else if ( machine()->is_data_text(t, c_wchar) ) {
        } else if ( machine()->is_data_text(t, c_byte) ) {
            char b = *(static_cast<char*>(v));
            oo.push_back(machine()->create_integer(static_cast<vm_int_t>(b)));
        } else if ( machine()->is_data_text(t, c_ubyte) ) {
            unsigned char b = *(static_cast<unsigned char*>(v));
            oo.push_back(machine()->create_integer(static_cast<vm_int_t>(b)));
        } else if ( machine()->is_data_text(t, c_short) ) {
            short n = *(static_cast<short*>(v));
            oo.push_back(machine()->create_integer(static_cast<vm_int_t>(n)));
        } else if ( machine()->is_data_text(t, c_ushort) ) {
            unsigned short n = *(static_cast<unsigned short*>(v));
            oo.push_back(machine()->create_integer(static_cast<vm_int_t>(n)));
        } else if ( machine()->is_data_text(t, c_int) ) {
            int n = *(static_cast<int*>(v));
            oo.push_back(machine()->create_integer(static_cast<vm_int_t>(n)));
        } else if ( machine()->is_data_text(t, c_uint) ) {
            unsigned int n = *(static_cast<unsigned int*>(v));
            oo.push_back(machine()->create_integer(static_cast<vm_int_t>(n)));
        } else if ( machine()->is_data_text(t, c_long) ) {
            long n = *(static_cast<long*>(v));
            oo.push_back(machine()->create_integer(static_cast<vm_int_t>(n)));
        } else if ( machine()->is_data_text(t, c_ulong) ) {
            unsigned long n = *(static_cast<unsigned long*>(v));
            oo.push_back(machine()->create_integer(static_cast<vm_int_t>(n)));
        } else if ( machine()->is_data_text(t, c_longlong) ) {
            long long n = *(static_cast<long long*>(v));
            oo.push_back(machine()->create_integer(static_cast<vm_int_t>(n)));
        } else if ( machine()->is_data_text(t, c_ulonglong) ) {
            unsigned long long n = *(static_cast<unsigned long long*>(v));
            oo.push_back(machine()->create_integer(static_cast<vm_int_t>(n)));
        } else if ( machine()->is_data_text(t, c_size_t) ) {
            size_t n = *(static_cast<size_t*>(v));
            oo.push_back(machine()->create_integer(static_cast<vm_int_t>(n)));
        } else if ( machine()->is_data_text(t, c_ssize_t) ) {
            ssize_t n = *(static_cast<ssize_t*>(v));
            oo.push_back(machine()->create_integer(static_cast<vm_int_t>(n)));
        } else if ( machine()->is_data_text(t, c_time_t) ) {
            time_t n = *(static_cast<time_t*>(v));
            oo.push_back(machine()->create_integer(static_cast<vm_int_t>(n)));
        } else if ( machine()->is_data_text(t, c_float) ) {
            float f = *(static_cast<float*>(v));
            oo.push_back(machine()->create_float(static_cast<vm_float_t>(f)));
        } else if ( machine()->is_data_text(t, c_double) ) {
            double f = *(static_cast<double*>(v));
            oo.push_back(machine()->create_float(static_cast<vm_float_t>(f)));
        } else if ( machine()->is_data_text(t, c_longdouble) ) {
            long double f = *(static_cast<long double*>(v));
            oo.push_back(machine()->create_float(static_cast<vm_float_t>(f)));
        } else if ( machine()->is_data_text(t, c_char_p) ) {
            char* cc = *(static_cast<char**>(v));
            if (cc == nullptr) {
                oo.push_back(machine()->create_none());
            } else {
                auto s = VM::unicode_from_utf8_chars(cc);
                oo.push_back(machine()->create_text(s));
            }
        //} else if ( machine()->is_data_text(t, c_wchar_p) ) {
        } else if ( machine()->is_data_text(t, c_void_p)  ) {
            void* p = *(static_cast<void**>(v));
            if (p == nullptr) {
                oo.push_back(machine()->create_none());
            } else {
                oo.push_back(machine()->create_integer(reinterpret_cast<vm_int_t>(p)));
            }
        } else {
            PANIC("ffi type expected");
        }
        return machine()->create_array(oo);
    }
/*
int
     main(int argc, const char **argv)
     {
         ffi_cif cif;
         ffi_type *arg_types[2];
         void *arg_values[2];
         ffi_status status;

         // Because the return value from foo() is smaller than sizeof(long), it
         // must be passed as ffi_arg or ffi_sarg.
         ffi_arg result;

         // Specify the data type of each argument. Available types are defined
         // in <ffi/ffi.h>.
         arg_types[0] = &ffi_type_uint;
         arg_types[1] = &ffi_type_float;

         // Prepare the ffi_cif structure.
         if ((status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI,
             2, &ffi_type_ubyte, arg_types)) != FFI_OK)
         {
             // Handle the ffi_status error.
         }

         // Specify the values of each argument.
         unsigned int arg1 = 42;
         float arg2 = 5.1;

         arg_values[0] = &arg1;
         arg_values[1] = &arg2;

         // Invoke the function.
         ffi_call(&cif, FFI_FN(foo), &result, arg_values);

         // The ffi_arg 'result' now contains the unsigned char returned from foo(),
         // which can be accessed by a typecast.
         printf("result is %hhu", (unsigned char)result);

         return 0;
     }
*/


    VMObjectPtr apply(const VMObjectPtr &arg0, const VMObjectPtr &arg1, const VMObjectPtr &arg2) const override {
        if (is_function_ptr(arg0) && is_return_type(arg1) && machine()->is_list(arg2)) {
            auto oo = machine()->from_list(arg2);
            for (auto &o: oo) {
                if (!is_arg_type(o)) {
                    throw machine()->bad_args(this, arg0, arg1, arg2);
                }
            }

            auto n = oo.size();
            
            ffi_cif cif;
            ffi_type **arg_types = (ffi_type**) malloc(n * sizeof(ffi_type*));
            ffi_type *ret_type = nullptr;
            void **arg_values = (void**) malloc(n * sizeof(void*));
            ffi_status status;
            ffi_arg result;

            ret_type = to_ffi_type(arg1);
            for (size_t i = 0; i < n; i++) {
                auto o = machine()->array_get(oo[i],0);
                arg_types[i] = to_ffi_type(o);
            }

            if ((status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI,
                 n, ret_type, arg_types)) != FFI_OK) {
                 throw machine()->create_text("error in cif");
            }

            for (size_t i = 0; i < n; i++) {
                arg_values[i] = to_ffi_value(oo[i]);
            }

            void** pp = reinterpret_cast<void**>(to_ffi_value(arg0));
            void* p = *pp;

            //std::cerr << "ffi_call " << p << "(n = " << n << ")" << std::endl;
            ffi_call(&cif, FFI_FN(p), &result, arg_values);

            auto r = from_ffi_value(arg1, &result);

            // char* has copying semantics both in and out but prevent double free
            char* cout = nullptr;
            if (machine()->is_data_text(machine()->array_get(r,0), c_char_p)) {
                cout = reinterpret_cast<char*>(result);
            }
            for (size_t i = 0; i < n; i++) {
                if (machine()->is_data_text(machine()->array_get(oo[i],0), c_char_p)) {
                    char *cc = *reinterpret_cast<char**>(arg_values[i]);
                    if ((cc != cout) && (cc != nullptr)) {
                        //std::cerr << "cc " << ((void*) cc) << std::endl;
                        free(cc);
                    }
                }
            }
            if (cout != nullptr) {
                //std::cerr << "cout " << ((void*) cout) << std::endl;
                free(cout);
            }
            
            // free the rest
            for (size_t i = 0; i < n; i++) {
                free(arg_values[i]);
            }
            free(arg_types);
            free(arg_values);
            free(pp);

            return r;
        } else {
            throw machine()->bad_args(this, arg0, arg1, arg2);
        }
    }
};

inline std::vector<VMObjectPtr> builtin_ffi(VM *vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(VMObjectData::create(vm, c_void));
    oo.push_back(VMObjectData::create(vm, c_bool));
    oo.push_back(VMObjectData::create(vm, c_char));
    oo.push_back(VMObjectData::create(vm, c_wchar));
    oo.push_back(VMObjectData::create(vm, c_byte));
    oo.push_back(VMObjectData::create(vm, c_ubyte));
    oo.push_back(VMObjectData::create(vm, c_short));
    oo.push_back(VMObjectData::create(vm, c_ushort));
    oo.push_back(VMObjectData::create(vm, c_int));
    oo.push_back(VMObjectData::create(vm, c_uint));
    oo.push_back(VMObjectData::create(vm, c_long));
    oo.push_back(VMObjectData::create(vm, c_ulong));
    oo.push_back(VMObjectData::create(vm, c_longlong));
    oo.push_back(VMObjectData::create(vm, c_ulonglong));
    oo.push_back(VMObjectData::create(vm, c_size_t));
    oo.push_back(VMObjectData::create(vm, c_ssize_t));
    oo.push_back(VMObjectData::create(vm, c_time_t));
    oo.push_back(VMObjectData::create(vm, c_float));
    oo.push_back(VMObjectData::create(vm, c_double));
    oo.push_back(VMObjectData::create(vm, c_longdouble));
    oo.push_back(VMObjectData::create(vm, c_char_p));
    oo.push_back(VMObjectData::create(vm, c_wchar_p));
    oo.push_back(VMObjectData::create(vm, c_void_p));

    oo.push_back(LoadLibrary::create(vm));
    oo.push_back(Function::create(vm));
    oo.push_back(Call::create(vm));

    return oo;
}

}

