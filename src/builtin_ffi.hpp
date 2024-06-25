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
              (machine()->is_data_text(o0, c_char_p) && machine()->is_int(o1))|| 
              (machine()->is_data_text(o0, c_wchar_p) && machine()->is_int(o1)) || 
              (machine()->is_data_text(o0, c_void_p) && machine()->is_int(o1)) 
            );
        } else {
            return false;
        }
    }

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

