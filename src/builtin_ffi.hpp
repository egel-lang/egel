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
| c_struct     | struct             | list of fields
| c_union      | union              | list of fields
| c_array      | array              | list of data
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
const auto c_struct = "c_struct";
const auto c_union = "c_union";
const auto c_array = "c_array";

// ## FFI::library - opaque library object
class Library : public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_BUILTIN, Library, FFI, "library");

    void load(const uci::Unicodestring &s) {
        char *error;

        dlerror();

        //std::cout << "loading: " << get_path() << std::endl; // DEBUG

        auto pth = VM::unicode_to_utf8_chars(get_path());  // XXX: leaks?
        _handle = dlopen(pth, RTLD_LAZY | RTLD_GLOBAL);
        if (!_handle) {
            icu::UnicodeString err = "dynamic load error: ";
            err += dlerror();
            err += " on open(" + get_path() + ")";
            throw ErrorIO(err);
        }

    }

    void unload() override {
        dlclose(_handle);
    }

private:
    void *_handle;
}


// ## FFI::find_library s - try to find a library
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

// ## FFI::load_library s - try to load a library
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
            auto s = machine()->get_text(arg1);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
}

// ## FFI::call f x - call a function

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
    oo.push_back(VMObjectData::create(vm, FFI, c_struct));
    oo.push_back(VMObjectData::create(vm, FFI, c_union));
    oo.push_back(VMObjectData::create(vm, FFI, c_array));
}

}

