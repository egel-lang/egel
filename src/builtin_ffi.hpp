/*

---------------------------------------------------------
| tag          | c language type    | egel representation
---------------------------------------------------------
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

const auto FFI = "Foreign";
const auto c_bool = "c_bool";
const auto c_char = "c_char";
const auto c_wchar = "c_wchar";
const auto c_byte = "c_byte";
const auto c_ubyte = "c_ubyte";
const auto c_short = "c_short";
const auto c_ushort = "c_ushort";
const auto c_int = "c_int";
const auto c_uint = "c_uint";
const auto c_long = "c_long";
const auto c_ulong = "c_ulong";
const auto c_longlong = "c_longlong";
const auto c_ulonglong = "c_ulonglong";
const auto c_size_t = "c_size_t";
const auto c_ssize_t = "c_ssize_t";
const auto c_time_t = "c_time_t";
const auto c_float = "c_float";
const auto c_double = "c_double";
const auto c_longdouble = "c_longdouble";
const auto c_char_p = "c_char_p";
const auto c_wchar_p = "c_wchar_p";
const auto c_void_p = "c_void_p";

// ## FFI::library - opaque library object
class Library : public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_BUILTIN, Library, FFI, "library");

    void load(const uci::Unicodestring &s) {
        char *error;

        dlerror();

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
        auto sym = VM::unicode_to_utf8_chars(get_path());  // XXX: leaks?
        auto ptr = dlsym(_handle, sym);
        VMObjectPtrs oo;
        oo.push_back(VMObjectData::create(vm, FFI, c_void_p));
        oo.push_back(machine()->create_int((int)ptr));
        return machine()->create_array(oo);
    }

    void unload() override {
        dlclose(_handle);
    }

private:
    void *_handle;
}


// ## FFI::find_library s - find a library
class FindLibrary : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, FindLibrary, FFI, "find_library");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
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
            auto l = FFI::Library::create();
            FFI::Library::cast(l)->load(s);
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
            auto l = Library;;cast(arg0);
            auto s = machine()->get_text(arg1);
            return l->function(s);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
}

// ## FFI::call f r x - call a function
class Call : public Triadic {
public:
    TRIADIC_PREAMBLE(VM_SUB_BUILTIN, Call, FFI, "call");

    bool is_data_text(const VMObjectPtr& o, const icu::UnicodeString &s) {
        return machine()->is_data(o) && (machine()->get_text(o) == s);
    }

    bool is_function_ptr(const VMObjectPtr& o) {
        return machine()->is_array(o) 
                && (machine()->array_size(o) == 2)
                && machine()->is_data_text(machine()->array_get(o,0), c_void_p)
                && machine()->is_int(machine()->array_get(o,1));
    }

    bool is_return_type(const VMObjectPtr& o) {
        return machine->is_data(o) && (
          machine()->is_data_text(o, c_bool)  || 
          machine()->is_data_text(o, c_char)  || 
          machine()->is_data_text(o, c_wchar)  || 
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
          machine()->is_data_text(o, c_wchar_p)  || 
          machine()->is_data_text(o, c_void_p)  
        )
    }

    bool is_arg_type(const VMObjectPtr& o) {
        if (machine()->is_array(o) && (machine()->array_size(o) == 2)) {

            auto o0 = machine()->array_get(o,0);
            auto o1 = machine()->array_get(o,1);

            return (
              (machine()->is_data_text(o0, c_bool) && machine()->is_bool(o1))  || 
              (machine()->is_data_text(o0, c_char) && machine()->is_char(o1))  || 
              (machine()->is_data_text(o0, c_wchar) && machine()->is_char(o1))  || 
              (machine()->is_data_text(o0, c_byte) && machine()->is_int(o1))  || 
              (machine()->is_data_text(o0, c_ubyte) && machine()->is_int(o1))  || 
              (machine()->is_data_text(o0, c_short) && machine()->is_int(o1)) || 
              (machine()->is_data_text(o0, c_ushort) && machine()->is_int(o1)) || 
              (machine()->is_data_text(o0, c_int) && machine()->is_int(o1)) || 
              (machine()->is_data_text(o0, c_uint) && machine()->is_int(o1)) || 
              (machine()->is_data_text(o0, c_long) && machine()->is_int(o1)) || 
              (machine()->is_data_text(o0, c_ulong) && machine()->is_int(o1)) || 
              (machine()->is_data_text(o0, c_longlong) && machine()->is_int(o1)) || 
              (machine()->is_data_text(o0, c_ulonglong) && machine()->is_int(o1)) || 
              (machine()->is_data_text(o0, c_size_t) && machine()->is_int(o1)) || 
              (machine()->is_data_text(o0, c_ssize_t) && machine()->is_int(o1)) || 
              (machine()->is_data_text(o0, c_time_t) && machine()->is_int(o1)) || 
              (machine()->is_data_text(o0, c_float) && machine()->is_float(o1)) || 
              (machine()->is_data_text(o0, c_double) && machine()->is_float(o1)) || 
              (machine()->is_data_text(o0, c_longdouble) && machine()->is_float(o1)) || 
              (machine()->is_data_text(o0, c_char_p) && (machine()->is_text(o1) || machine()->is_none(o1)))|| 
              (machine()->is_data_text(o0, c_wchar_p) && (machine()->is_text(o1) || machine()->is_none(o1))) || 
              (machine()->is_data_text(o0, c_void_p) && (machine()->is_int(o1) || machine()->is_none(o1))) 
            );
        } else {
            return false;
        }
    }

    ffi_type* to_ffi_type(const VMObjectPtr &o) {
        if ( machine()->is_data_text(o, c_bool) ) {
            return &ffi_type_bool;
        } else if ( machine()->is_data_text(o, c_char) ) {
            return &ffi_type_char;
        } else if ( machine()->is_data_text(o, c_wchar) ) {
            return &ffi_type_w_char;
        } else if ( machine()->is_data_text(o, c_byte) ) {
            return &ffi_type_byte;
        } else if ( machine()->is_data_text(o, c_ubyte) ) {
            return &ffi_type_ubyte;
        } else if ( machine()->is_data_text(o, c_short) ) {
            return &ffi_type_short;
        } else if ( machine()->is_data_text(o, c_ushort) ) {
            return &ffi_type_ushort;
        } else if ( machine()->is_data_text(o, c_int) ) {
            return &ffi_type_int;
        } else if ( machine()->is_data_text(o, c_uint) ) {
            return &ffi_type_uint;
        } else if ( machine()->is_data_text(o, c_long) ) {
            return &ffi_type_long;
        } else if ( machine()->is_data_text(o, c_ulong) ) {
            return &ffi_type_ulong;
        } else if ( machine()->is_data_text(o, c_longlong) ) {
            return &ffi_type_longlong;
        } else if ( machine()->is_data_text(o, c_ulonglong) ) {
            return &ffi_type_ulonglong;
        } else if ( machine()->is_data_text(o, c_size_t) ) {
            return &ffi_type_size_t;
        } else if ( machine()->is_data_text(o, c_ssize_t) ) {
            return &ffi_type_ssize_t;
        } else if ( machine()->is_data_text(o, c_time_t) ) {
            return &ffi_type_time_t;
        } else if ( machine()->is_data_text(o, c_float) ) {
            return &ffi_type_float;
        } else if ( machine()->is_data_text(o, c_double) ) {
            return &ffi_type_double;
        } else if ( machine()->is_data_text(o, c_longdouble) ) {
            return &ffi_type_longdouble;
        } else if ( machine()->is_data_text(o, c_char_p) ) {
            return &ffi_type_char_p;
        } else if ( machine()->is_data_text(o, c_wchar_p) ) {
            return &ffi_type_wchar_p;
        } else if ( machine()->is_data_text(o, c_void_p)  ) {
            return &ffi_type_void_p;
        } else {
            PANIC("ffi type expected");
        }
    }

    void* to_ffi_value(const VMObjectPtr &o) {
        if (machine()->is_array(o) && (machine()->array_size(o) == 2)) {

            auto o0 = machine()->array_get(o,0);
            auto o1 = machine()->array_get(o,1);
            if ( machine()->is_data_text(o0, c_bool) ) {
                if (machine()->is_false(o1)) {
                    return (void*) false;
                } else {
                    return (void*) true;
                }
            } else if ( machine()->is_data_text(o0, c_char) ) {
                return (void*) ( (char) machine()->get_char(o1) );
            } else if ( machine()->is_data_text(o0, c_wchar) ) {
                return (void*) ( (char) machine()->get_char(o1) );
            } else if ( machine()->is_data_text(o0, c_byte) ) {
                return (void*) ( (char) machine()->get_int(o1) );
            } else if ( machine()->is_data_text(o0, c_ubyte) ) {
                return (void*) ( (char) machine()->get_int(o1) );
            } else if ( machine()->is_data_text(o0, c_short) ) {
                return (void*) ( (short) machine()->get_int(o1) );
            } else if ( machine()->is_data_text(o0, c_ushort) ) {
                return (void*) ( (unsigned short) machine()->get_int(o1) );
            } else if ( machine()->is_data_text(o0, c_int) ) {
                return (void*) ( (int) machine()->get_int(o1) );
            } else if ( machine()->is_data_text(o0, c_uint) ) {
                return (void*) ( (unsigned int) machine()->get_int(o1) );
            } else if ( machine()->is_data_text(o0, c_long) ) {
                return (void*) ( (long) machine()->get_int(o1) );
            } else if ( machine()->is_data_text(o0, c_ulong) ) {
                return (void*) ( (unsigned long) machine()->get_int(o1) );
            } else if ( machine()->is_data_text(o0, c_longlong) ) {
                return (void*) ( (long long) machine()->get_int(o1) );
            } else if ( machine()->is_data_text(o0, c_ulonglong) ) {
                return (void*) ( (unsigned long long) machine()->get_int(o1) );
            } else if ( machine()->is_data_text(o0, c_size_t) ) {
                return (void*) ( (size_t) machine()->get_int(o1) );
            } else if ( machine()->is_data_text(o0, c_ssize_t) ) {
                return (void*) ( (ssize_t) machine()->get_int(o1) );
            } else if ( machine()->is_data_text(o0, c_time_t) ) {
                return (void*) ( (time_t) machine()->get_int(o1) );
            } else if ( machine()->is_data_text(o0, c_float) ) {
                return (void*) ( (float) machine()->get_float(o1) );
            } else if ( machine()->is_data_text(o0, c_double) ) {
                return (void*) ( (double) machine()->get_float(o1) );
            } else if ( machine()->is_data_text(o0, c_longdouble) ) {
                return (void*) ( (long double) machine()->get_float(o1) );
            } else if ( machine()->is_data_text(o0, c_char_p) ) {
                if (machine()->is_none(o1)) {
                    return nullptr;
                } else {
                }
            } else if ( machine()->is_data_text(o0, c_wchar_p) ) {
                if (machine()->is_none(o1)) {
                    return nullptr;
                } else {
                }
            } else if ( machine()->is_data_text(o0, c_void_p)  ) {
                if (machine()->is_none(o1)) {
                    return nullptr;
                } else {
                }
            } else {
                PANIC("ffi value expected");
            }
        } else {
            PANIC("ffi value expected");
        }
    }

    VMObjectPtr from_ffi_value(const ffi_type* t, const void* v) {
        if ( &ffi_type_bool == t ) {
            if ( ((bool) v) == false ) {
                return machine()->create_false();
            } else {
                return machine()->create_true();
            }
        } else if ( &ffi_type_char == t ) {
            return machine()->create_char((UChar32) v);
        } else if ( &ffi_type_w_char == t ) {
            return machine()->create_char((UChar32) v);
        } else if ( &ffi_type_byte == t ) {
            return machine()->create_int((int) ((char) v));
        } else if ( &ffi_type_ubyte == t ) {
            return machine()->create_int((int) ((unsigned char) v));
        } else if ( &ffi_type_short == t ) {
            return machine()->create_int((int) ((short) v));
        } else if ( &ffi_type_ushort == t ) {
            return machine()->create_int((int) ((unsigned short) v));
        } else if ( &ffi_type_int == t ) {
            return machine()->create_int((int) ((int) v));
        } else if ( &ffi_type_uint == t ) {
            return machine()->create_int((int) ((unsigned int) v));
        } else if ( &ffi_type_long == t ) {
            return machine()->create_int((int) ((long) v));
        } else if ( &ffi_type_ulong == t ) {
            return machine()->create_int((int) ((unsigned long) v));
        } else if ( &ffi_type_longlong == t ) {
            return machine()->create_int((int) ((long) v));
        } else if ( &ffi_type_ulonglong == t ) {
            return machine()->create_int((int) ((unsigned long long) v));
        } else if ( &ffi_type_size_t == t ) {
            return machine()->create_int((int) ((size_t) v));
        } else if ( &ffi_type_ssize_t == t ) {
            return machine()->create_int((int) ((ssize_t) v));
        } else if ( &ffi_type_time_t == t ) {
            return machine()->create_int((int) ((time_t) v));
        } else if ( &ffi_type_float == t ) {
            return machine()->create_float((long double) ((float) v));
        } else if ( &ffi_type_double == t ) {
            return machine()->create_float((long double) ((double) v));
        } else if ( &ffi_type_longdouble == t ) {
            return machine()->create_float((long double) ((long double) v));
        } else if ( &ffi_type_char_p == t ) {
            if ( v == nullptr ) {
                return machine()->create_none();
            } else {
                return machine()->create_int((int) v);
            }
        } else if ( &ffi_type_wchar_p == t ) {
            if ( v == nullptr ) {
                return machine()->create_none();
            } else {
                return machine()->create_int((int) v);
            }
        } else if ( &ffi_type_void_p == t ) {
            if ( v == nullptr ) {
                return machine()->create_none();
            } else {
                return machine()->create_int((int) v);
            }
        } else {
            PANIC("ffi type expected");
        }
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
             2, &ffi_type_uint8, arg_types)) != FFI_OK)
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
        if (machine()->is_type(typeid(Library), arg0) && machine()->is_text(arg1)) {
            auto l = Library;;cast(arg0);
            auto s = machine()->get_text(arg1);
            return l->function(s);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
}

inline std::vector<VMObjectPtr> builtin_system(VM *vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(VMObjectData::create(vm, FFI, c_bool));
    oo.push_back(VMObjectData::create(vm, FFI, c_char));
    oo.push_back(VMObjectData::create(vm, FFI, c_wchar));
    oo.push_back(VMObjectData::create(vm, FFI, c_byte));
    oo.push_back(VMObjectData::create(vm, FFI, c_ubyte));
    oo.push_back(VMObjectData::create(vm, FFI, c_short));
    oo.push_back(VMObjectData::create(vm, FFI, c_ushort));
    oo.push_back(VMObjectData::create(vm, FFI, c_int));
    oo.push_back(VMObjectData::create(vm, FFI, c_uint));
    oo.push_back(VMObjectData::create(vm, FFI, c_long));
    oo.push_back(VMObjectData::create(vm, FFI, c_ulong));
    oo.push_back(VMObjectData::create(vm, FFI, c_longlong));
    oo.push_back(VMObjectData::create(vm, FFI, c_ulonglong));
    oo.push_back(VMObjectData::create(vm, FFI, c_size_t));
    oo.push_back(VMObjectData::create(vm, FFI, c_ssize_t));
    oo.push_back(VMObjectData::create(vm, FFI, c_time_t));
    oo.push_back(VMObjectData::create(vm, FFI, c_float));
    oo.push_back(VMObjectData::create(vm, FFI, c_double));
    oo.push_back(VMObjectData::create(vm, FFI, c_longdouble));
    oo.push_back(VMObjectData::create(vm, FFI, c_char_p));
    oo.push_back(VMObjectData::create(vm, FFI, c_wchar_p));
    oo.push_back(VMObjectData::create(vm, FFI, c_void_p));
}

}

