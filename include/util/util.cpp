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
            return VMObjectInteger(f).clone();
        } else if (arg0->tag() == VM_OBJECT_CHAR) {
            auto c = VM_OBJECT_CHAR_VALUE(arg0);
            return VMObjectInteger(c).clone();
        } else if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            auto i = convert_to_int(s);
            return VMObjectInteger(i).clone();
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
            return VMObjectFloat(i).clone();
        } else if (arg0->tag() == VM_OBJECT_FLOAT) {
            return arg0;
        } else if (arg0->tag() == VM_OBJECT_CHAR) {
            return nullptr; // couldn't find a rationale for this
        } else if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            auto i = convert_to_float(s);
            return VMObjectFloat(i).clone();
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
            return VMObjectText(s).clone();
        } else if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            auto s = convert_from_float(f);
            return VMObjectText(s).clone();
        } else if (arg0->tag() == VM_OBJECT_CHAR) {
            auto c = VM_OBJECT_CHAR_VALUE(arg0);
            auto s = convert_from_char(c);
            return VMObjectText(s).clone();
        } else if (arg0->tag() == VM_OBJECT_TEXT) {
            return arg0;
        } else {
            return nullptr;
        }
    }
};

// System.get x
// Retrieve a var field
class Get: public Monadic {
public:
    MONADIC_PREAMBLE(Get, "System", "get");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        static symbol_t var = 0;
        if (var == 0) var = machine()->enter_symbol("System", "var");

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

// System.set x
// Set a var field
class Set: public Dyadic {
public:
    DYADIC_PREAMBLE(Set, "System", "set");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        static symbol_t var = 0;
        if (var == 0) var = machine()->enter_symbol("System", "var");

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

extern "C" std::vector<UnicodeString> egel_imports() {
    return std::vector<UnicodeString>();
}

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(VMObjectData(vm, "System", "var").clone());

    oo.push_back(Toint(vm).clone());
    oo.push_back(Tofloat(vm).clone());
    oo.push_back(Totext(vm).clone());

    oo.push_back(Get(vm).clone());
    oo.push_back(Set(vm).clone());

    return oo;

}
