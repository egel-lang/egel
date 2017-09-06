#include "../../src/runtime.hpp"

// XXX: I hard included the entire cpp file to make sure all symbols get resolved
// there is a compiler switch for this but i chose not to use that
#include "utils.cpp"

#include <stdlib.h>
#include <math.h>

/**
 * Egel's utility routines.
 *
 * Conversions and some other.
 **/


// System.toint x
// Try and convert an object to int.
class Toint: public Monadic {
public:
    MONADIC_PREAMBLE(Toint, "System", "toint");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_INTEGER) {
            return arg0;
        } else if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_integer(f);
        } else if (arg0->tag() == VM_OBJECT_CHAR) {
            auto c = VM_OBJECT_CHAR_VALUE(arg0);
            return create_integer(c);
        } else if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            auto i = convert_to_int(s);
            return create_integer(i);
        } else {
            return nullptr;
        }
    }
};

// System.tofloat x
// Try and convert an object to float.
class Tofloat: public Monadic {
public:
    MONADIC_PREAMBLE(Tofloat, "System", "tofloat");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_INTEGER) {
            auto i = VM_OBJECT_INTEGER_VALUE(arg0);
            return create_float(i);
        } else if (arg0->tag() == VM_OBJECT_FLOAT) {
            return arg0;
        } else if (arg0->tag() == VM_OBJECT_CHAR) {
            return nullptr; // couldn't find a rationale for this
        } else if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            auto i = convert_to_float(s);
            return create_float(i);
        } else {
            return nullptr;
        }
    }
};

// System.totext x
// Try and convert an object to text.
class Totext: public Monadic {
public:
    MONADIC_PREAMBLE(Totext, "System", "totext");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_INTEGER) {
            auto i = VM_OBJECT_INTEGER_VALUE(arg0);
            auto s = convert_from_int(i);
            return create_text(s);
        } else if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            auto s = convert_from_float(f);
            return create_text(s);
        } else if (arg0->tag() == VM_OBJECT_CHAR) {
            auto c = VM_OBJECT_CHAR_VALUE(arg0);
            auto s = convert_from_char(c);
            return create_text(s);
        } else if (arg0->tag() == VM_OBJECT_TEXT) {
            return arg0;
        } else {
            return nullptr;
        }
    }
};

// System.getv x
// Retrieve a var field
class Get: public Monadic {
public:
    MONADIC_PREAMBLE(Get, "System", "getv");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        static symbol_t var = 0;
        if (var == 0) var = machine()->enter_symbol("System", "v");

        if (arg0->tag() == VM_OBJECT_ARRAY) {
            auto ff = VM_OBJECT_ARRAY_VALUE(arg0);
            if (ff.size() != 2) return nullptr;
            if (ff[0]->symbol() != var) return nullptr;
            return ff[1];
        } else {
            return nullptr;
        }
    }
};

// System.setv x
// Set a var field
class Set: public Dyadic {
public:
    DYADIC_PREAMBLE(Set, "System", "setv");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        static symbol_t var = 0;
        if (var == 0) var = machine()->enter_symbol("System", "v");

        if (arg0->tag() == VM_OBJECT_ARRAY) {
            auto ff = VM_OBJECT_ARRAY_VALUE(arg0);
            if (ff.size() != 2) return nullptr;
            if (ff[0]->symbol() != var) return nullptr;
            auto arr = VM_OBJECT_ARRAY_CAST(arg0); // XXX: clean up this cast once. need destructive update
            arr->set(1, arg1);
            return arg0;
        } else {
            return nullptr;
        }
    }
};

// System.unpack s
// create a list of UChar32 from a Unicode string
class Unpack: public Monadic {
public:
    MONADIC_PREAMBLE(Unpack, "System", "unpack");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        static VMObjectPtr _nil = nullptr;
        if (_nil == nullptr) _nil = machine()->get_data_string("System", "nil");

        static VMObjectPtr _cons = nullptr;
        if (_cons == nullptr) _cons = machine()->get_data_string("System", "cons");

        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto str = VM_OBJECT_TEXT_VALUE(arg0);

            VMObjectPtr ss = _nil;
            int len = str.length();
            for (int n = len-1; n >= 0; n--) {
                auto c = str.char32At(n);
                auto tt = VMObjectPtrs();
                tt.push_back(_cons);
                tt.push_back(VMObjectChar(c).clone());
                tt.push_back(ss);
                ss = VMObjectArray(tt).clone();
            }
            return ss;
        } else {
            return nullptr;
        }
    }
};


// System.pack s
// create a Unicode string from a list of UChar32
class Pack: public Monadic {
public:
    MONADIC_PREAMBLE(Pack, "System", "pack");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        static symbol_t _nil = 0;
        if (_nil == 0) _nil = machine()->enter_symbol("System", "nil");

        static symbol_t _cons = 0;
        if (_cons == 0) _cons = machine()->enter_symbol("System", "cons");

        UnicodeString ss;
        auto a = arg0;

        while ( (a->tag() == VM_OBJECT_ARRAY) ) {
            auto aa = VM_OBJECT_ARRAY_VALUE(a);
            if (aa.size() != 3) return nullptr;
            if (aa[0]->symbol() != _cons) return nullptr;
            if (aa[1]->tag() != VM_OBJECT_CHAR) return nullptr;

            auto c = VM_OBJECT_CHAR_VALUE(aa[1]);

            ss += c;

            a = aa[2];
        }

        return VMObjectText(ss).clone();
    }
};


extern "C" std::vector<UnicodeString> egel_imports() {
    return std::vector<UnicodeString>();
}

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(VMObjectData(vm, "System", "v").clone());

    oo.push_back(Toint(vm).clone());
    oo.push_back(Tofloat(vm).clone());
    oo.push_back(Totext(vm).clone());

    oo.push_back(Get(vm).clone());
    oo.push_back(Set(vm).clone());

    oo.push_back(Unpack(vm).clone());
    oo.push_back(Pack(vm).clone());

    return oo;

}
