#include "../../src/runtime.hpp"

// XXX: I hard included the entire cpp file to make sure all symbols get resolved
// i don't know of a better way to do this
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

extern "C" std::vector<UnicodeString> egel_imports() {
    return std::vector<UnicodeString>();
}

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Toint(vm).clone());
    oo.push_back(Tofloat(vm).clone());
    oo.push_back(Totext(vm).clone());

    return oo;

}
