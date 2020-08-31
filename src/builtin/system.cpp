#include "system.hpp"

#include "../utils.hpp"
#include "../bytecode.hpp"

#include <stdlib.h>
#include <ostream>
#include <map>

#include <stdlib.h>
#include <math.h>

/**
 * For portable overflow detection use the portable snippets header file.
 * I hope I can once discard this but that depends on C++ compiler implementors.
 *
 * Note: portable snippets generates a warning on Fedora, GCC 7.2.1., so I don't
 * use it for now.
 */
// #include "portable-snippets/safe-math/safe-math.h"

/**
 * Egel's system routines.
 *
 * Basic operators, conversions, and some other.
 **/

class K: public Binary {
public:
    BINARY_PREAMBLE(K, "System", "k");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        return arg0;
    }
};

class Id: public Unary {
public:
    UNARY_PREAMBLE(Id, "System", "id");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        return arg0;
    }
};

class MonMin: public Monadic {
public:
    MONADIC_PREAMBLE(MonMin, "System", "!-");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_INTEGER) {
            auto i = VM_OBJECT_INTEGER_VALUE(arg0);
            vm_int_t res;
            if (__builtin_smull_overflow(-1, i, &res)) {
                OVERFLOW;
            } else {
                return VMObjectInteger(res).clone();
            }
        } else if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return VMObjectFloat(-f).clone();
        } else {
            BADARGS;
        }
    }
};

class Add: public Dyadic {
public:
    DYADIC_PREAMBLE(Add, "System", "+");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_INTEGER) &&
             (arg1->tag() == VM_OBJECT_INTEGER) ) {
            auto i0 = VM_OBJECT_INTEGER_VALUE(arg0);
            auto i1 = VM_OBJECT_INTEGER_VALUE(arg1);
            vm_int_t res;
            if (__builtin_saddl_overflow(i0, i1, &res)) {
                OVERFLOW;
            } else {
                return VMObjectInteger(res).clone();
            }
        } else if ( (arg0->tag() == VM_OBJECT_FLOAT) &&
             (arg1->tag() == VM_OBJECT_FLOAT) ) {
            auto f0 = VM_OBJECT_FLOAT_VALUE(arg0);
            auto f1 = VM_OBJECT_FLOAT_VALUE(arg1);
            return VMObjectFloat(f0+f1).clone();
        } else if ( (arg0->tag() == VM_OBJECT_TEXT) &&
             (arg1->tag() == VM_OBJECT_TEXT) ) {
            auto f0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto f1 = VM_OBJECT_TEXT_VALUE(arg1);
            return VMObjectText(f0+f1).clone();
        } else {
            BADARGS;
        }
    }
};

class Min: public Dyadic {
public:
    DYADIC_PREAMBLE(Min, "System", "-");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_INTEGER) &&
             (arg1->tag() == VM_OBJECT_INTEGER) ) {
            auto i0 = VM_OBJECT_INTEGER_VALUE(arg0);
            auto i1 = VM_OBJECT_INTEGER_VALUE(arg1);
            vm_int_t res;
            if (__builtin_ssubl_overflow(i0, i1, &res)) {
                OVERFLOW;
            } else {
                return VMObjectInteger(res).clone();
            }
        } else if ( (arg0->tag() == VM_OBJECT_FLOAT) &&
             (arg1->tag() == VM_OBJECT_FLOAT) ) {
            auto f0 = VM_OBJECT_FLOAT_VALUE(arg0);
            auto f1 = VM_OBJECT_FLOAT_VALUE(arg1);
            return VMObjectFloat(f0-f1).clone();
        } else {
            BADARGS;
        }
    }
};

class Mul: public Dyadic {
public:
    DYADIC_PREAMBLE(Mul, "System", "*");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_INTEGER) &&
             (arg1->tag() == VM_OBJECT_INTEGER) ) {
            auto i0 = VM_OBJECT_INTEGER_VALUE(arg0);
            auto i1 = VM_OBJECT_INTEGER_VALUE(arg1);
            vm_int_t res;
            if (__builtin_smull_overflow(i0, i1, &res)) {
                OVERFLOW;
            } else {
                return VMObjectInteger(res).clone();
            }
        } else if ( (arg0->tag() == VM_OBJECT_FLOAT) &&
             (arg1->tag() == VM_OBJECT_FLOAT) ) {
            auto f0 = VM_OBJECT_FLOAT_VALUE(arg0);
            auto f1 = VM_OBJECT_FLOAT_VALUE(arg1);
            return VMObjectFloat(f0*f1).clone();
        } else {
            BADARGS;
        }
    }
};

class Div: public Dyadic {
public:
    DYADIC_PREAMBLE(Div, "System", "/");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_INTEGER) &&
             (arg1->tag() == VM_OBJECT_INTEGER) ) {
            auto i0 = VM_OBJECT_INTEGER_VALUE(arg0);
            auto i1 = VM_OBJECT_INTEGER_VALUE(arg1);
            if (i1 == 0) {
                DIVZERO;
            }
            return VMObjectInteger(i0/i1).clone();
        } else if ( (arg0->tag() == VM_OBJECT_FLOAT) &&
             (arg1->tag() == VM_OBJECT_FLOAT) ) {
            auto f0 = VM_OBJECT_FLOAT_VALUE(arg0);
            auto f1 = VM_OBJECT_FLOAT_VALUE(arg1);
            if (f1 == 0.0) {
                DIVZERO;
            }
            return VMObjectFloat(f0/f1).clone();
        } else {
            BADARGS;
        }
    }
};

class Mod: public Dyadic {
public:
    DYADIC_PREAMBLE(Mod, "System", "%");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_INTEGER) &&
             (arg1->tag() == VM_OBJECT_INTEGER) ) {
            auto i0 = VM_OBJECT_INTEGER_VALUE(arg0);
            auto i1 = VM_OBJECT_INTEGER_VALUE(arg1);
            if (i1 == 0) {
                DIVZERO;
            }
            return VMObjectInteger(i0%i1).clone();
        } else {
            BADARGS;
        }
    }
};

class BinAnd: public Dyadic {
public:
    DYADIC_PREAMBLE(BinAnd, "System", "&");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_INTEGER) &&
             (arg1->tag() == VM_OBJECT_INTEGER) ) {
            auto i0 = VM_OBJECT_INTEGER_VALUE(arg0);
            auto i1 = VM_OBJECT_INTEGER_VALUE(arg1);
            return VMObjectInteger(i0&i1).clone();
        } else {
            BADARGS;
        }
    }
};

class BinOr: public Dyadic {
public:
    DYADIC_PREAMBLE(BinOr, "System", "$");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_INTEGER) &&
             (arg1->tag() == VM_OBJECT_INTEGER) ) {
            auto i0 = VM_OBJECT_INTEGER_VALUE(arg0);
            auto i1 = VM_OBJECT_INTEGER_VALUE(arg1);
            return VMObjectInteger(i0|i1).clone();
        } else {
            BADARGS;
        }
    }
};

class BinXOr: public Dyadic {
public:
    DYADIC_PREAMBLE(BinXOr, "System", "^");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_INTEGER) &&
             (arg1->tag() == VM_OBJECT_INTEGER) ) {
            auto i0 = VM_OBJECT_INTEGER_VALUE(arg0);
            auto i1 = VM_OBJECT_INTEGER_VALUE(arg1);
            return VMObjectInteger(i0^i1).clone();
        } else {
            BADARGS;
        }
    }
};

class BinComplement: public Monadic {
public:
    MONADIC_PREAMBLE(BinComplement, "System", "!~");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if ( (arg0->tag() == VM_OBJECT_INTEGER) ) {
            auto i0 = VM_OBJECT_INTEGER_VALUE(arg0);
            return VMObjectInteger(~i0).clone();
        } else {
            BADARGS;
        }
    }
};

class BinLeftShift: public Dyadic {
public:
    DYADIC_PREAMBLE(BinLeftShift, "System", "<<");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_INTEGER) &&
             (arg1->tag() == VM_OBJECT_INTEGER) ) {
            auto i0 = VM_OBJECT_INTEGER_VALUE(arg0);
            auto i1 = VM_OBJECT_INTEGER_VALUE(arg1);
            return VMObjectInteger(i0<<i1).clone();
        } else {
            BADARGS;
        }
    }
};

class BinRightShift: public Dyadic {
public:
    DYADIC_PREAMBLE(BinRightShift, "System", ">>");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_INTEGER) &&
             (arg1->tag() == VM_OBJECT_INTEGER) ) {
            auto i0 = VM_OBJECT_INTEGER_VALUE(arg0);
            auto i1 = VM_OBJECT_INTEGER_VALUE(arg1);
            return VMObjectInteger(i0>>i1).clone();
        } else {
            BADARGS;
        }
    }
};

class Less: public Dyadic {
public:
    DYADIC_PREAMBLE(Less, "System", "<");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        static VMObjectPtr _false = 0;
        static VMObjectPtr _true = 0;
        
        if (_false == 0) _false = machine()->get_data_string("System", "false");
        if (_true == 0)  _true = machine()->get_data_string("System", "true");

        CompareVMObjectPtr compare;
        if (compare(arg0, arg1) < 0) {
            return _true;
        } else {
            return _false;
        }
    }
};

class LessEq: public Dyadic {
public:
    DYADIC_PREAMBLE(LessEq, "System", "<=");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        static VMObjectPtr _false = 0;
        static VMObjectPtr _true = 0;
        
        if (_false == 0) _false = machine()->get_data_string("System", "false");
        if (_true == 0)  _true = machine()->get_data_string("System", "true");

        CompareVMObjectPtr compare;
        if (compare(arg0, arg1) <= 0) {
            return _true;
        } else {
            return _false;
        }
    }
};

class Eq: public Dyadic {
public:
    DYADIC_PREAMBLE(Eq, "System", "==");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        static VMObjectPtr _false = 0;
        static VMObjectPtr _true = 0;
        
        if (_false == 0) _false = machine()->get_data_string("System", "false");
        if (_true == 0)  _true = machine()->get_data_string("System", "true");

        CompareVMObjectPtr compare;
        if (compare(arg0, arg1) == 0) {
            return _true;
        } else {
            return _false;
        }
    }
};

class NegEq: public Dyadic {
public:
    DYADIC_PREAMBLE(NegEq, "System", "/=");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        static VMObjectPtr _false = 0;
        static VMObjectPtr _true = 0;
        
        if (_false == 0) _false = machine()->get_data_string("System", "false");
        if (_true == 0)  _true = machine()->get_data_string("System", "true");

        CompareVMObjectPtr compare;
        if (compare(arg0, arg1) != 0) {
            return _true;
        } else {
            return _false;
        }
    }
};

// System.get F O
// Retrieve an object field
class GetField: public Binary {
public:
    BINARY_PREAMBLE(GetField, "System", "get");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        static symbol_t object = 0;
        if (object == 0) object = machine()->enter_symbol("System", "object");

        if (arg1->tag() == VM_OBJECT_ARRAY) {
            auto ff = VM_OBJECT_ARRAY_VALUE(arg1);
            auto sz = ff.size();
            // check head is an object
            if (sz == 0) INVALID;
            if (ff[0]->symbol() != object) INVALID;
            // search for field
            CompareVMObjectPtr compare;
            unsigned int n;
            for (n = 1; n < sz; n=n+2) {
                if (compare(arg0, ff[n]) == 0) break;
            }
            // return field
            if ( (n+1) < sz ) {
                return ff[n+1];
            } else {
                INVALID;
            }
        } else {
            BADARGS;
        }
    }
};

// System.set F X O
// set an object field
class SetField: public Triadic {
public:
    TRIADIC_PREAMBLE(SetField, "System", "set");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        static symbol_t object = 0;
        if (object == 0) object = machine()->enter_symbol("System", "object");

        if (arg2->tag() == VM_OBJECT_ARRAY) {
            auto ff = VM_OBJECT_ARRAY_VALUE(arg2);
            auto sz = ff.size();
            // check head is an object
            if (sz == 0) INVALID;
            if (ff[0]->symbol() != object) INVALID;
            // search field
            CompareVMObjectPtr compare;
            unsigned int n;
            for (n = 1; n < sz; n=n+2) {
                if (compare(arg0, ff[n]) == 0) break;
            }
            // set field
            if ( (n+1) < sz ) {
                auto arr = VM_OBJECT_ARRAY_CAST(arg2); // XXX: clean up this cast once. need destructive update
                arr->set(n+1, arg1);
                return arg0;
            } else {
                INVALID;
            }
        } else {
            BADARGS;
        }
    }
};

// System.extend O0 O1
// Extend object O0 with every field from O1
class ExtendField: public Dyadic {
public:
    DYADIC_PREAMBLE(ExtendField, "System", "extend");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        static symbol_t object = 0;
        if (object == 0) object = machine()->enter_symbol("System", "object");

        if ( (arg0->tag() == VM_OBJECT_ARRAY) && (arg1->tag() == VM_OBJECT_ARRAY) ) {
            auto ff0 = VM_OBJECT_ARRAY_VALUE(arg0);
            auto sz0 = ff0.size();
            auto ff1 = VM_OBJECT_ARRAY_VALUE(arg1);
            auto sz1 = ff1.size();
            // check head is an object
            if (sz0 == 0) INVALID;
            if (ff0[0]->symbol() != object) INVALID;
            if (sz1 == 0) INVALID;
            if (ff1[0]->symbol() != object) INVALID;
            // create field union
            unsigned int n;
            std::map<VMObjectPtr, VMObjectPtr> fields;
            for (n = 1; n < sz0; n=n+2) {
                if ( (n+1) < sz0)
                fields[ff0[n]] = ff0[n+1];
            }
            for (n = 1; n < sz1; n=n+2) {
                if ( (n+1) < sz1)
                fields[ff1[n]] = ff1[n+1];
            }
            // return object
            VMObjectPtrs oo;
            oo.push_back(machine()->get_data_symbol(object));
            for (const auto& f:fields) {
                oo.push_back(f.first);
                oo.push_back(f.second);
            }

            return VMObjectArray::create(oo);
        } else {
            BADARGS;
        }
    }
};


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
            BADARGS;
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
        } else if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            auto i = convert_to_float(s);
            return create_float(i);
        } else {
            BADARGS;
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
            BADARGS;
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
            if (ff.size() != 2) INVALID;
            if (ff[0]->symbol() != var) INVALID;
            return ff[1];
        } else {
            BADARGS;
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
            if (ff.size() != 2) INVALID;
            if (ff[0]->symbol() != var) INVALID;
            auto arr = VM_OBJECT_ARRAY_CAST(arg0); // XXX: clean up this cast once. need destructive update
            arr->set(1, arg1);
            return arg0;
        } else {
            BADARGS;
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
            BADARGS;
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

        icu::UnicodeString ss;
        auto a = arg0;

        while ( (a->tag() == VM_OBJECT_ARRAY) ) {
            auto aa = VM_OBJECT_ARRAY_VALUE(a);
            if (aa.size() != 3) INVALID;
            if (aa[0]->symbol() != _cons) INVALID;
            if (aa[1]->tag() != VM_OBJECT_CHAR) INVALID;

            auto c = VM_OBJECT_CHAR_VALUE(aa[1]);

            ss += c;

            a = aa[2];
        }

        return VMObjectText(ss).clone();
    }
};

// System.dis o
// Dump the assembly of a combinator object
class Dis: public Monadic {
public:
    MONADIC_PREAMBLE(Dis, "System", "dis");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (auto o = std::dynamic_pointer_cast<VMObjectBytecode>(arg0)) {
            auto s0 = o->text();
            auto s1 = o->disassemble();
            return VMObjectText(s0+ "::" + s1).clone();
        } else {
            BADARGS;
        }
    }
};

// System.asm s0 s1
// Assemble bytecode into a combinator
class Asm: public Unary {
public:
    UNARY_PREAMBLE(Asm, "System", "asm");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            auto l = s.length();
            auto p = s.indexOf("::");
            icu::UnicodeString s0;
            icu::UnicodeString s1;
            s.extract(0, p, s0);
            s.extract(p+2, l-(p+2), s1);
            auto c = VMObjectBytecode(machine(), Code(), s0);
            c.assemble(s1);
            return c.clone();
        } else {
            BADARGS;
        }
    }
};

int     application_argc = 0;
char**  application_argv = nullptr;

// System.arg n
// Return the n-th application argument, otherwise return 'nop'
class Arg: public Monadic {
public:
    MONADIC_PREAMBLE(Arg, "System", "arg");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_INTEGER) {
            auto i = VM_OBJECT_INTEGER_VALUE(arg0);
            if (i < application_argc) {
                return VMObjectText(application_argv[i]).clone();
            } else {
                auto nop = machine()->get_data_string("System", "nop");
                return nop;
            }
        } else {
            BADARGS;
        }
    }
};

// System.getenv s
// Return the value of environment variable s, otherwise return 'nop'
class Getenv: public Monadic {
public:
    MONADIC_PREAMBLE(Getenv, "System", "getenv");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto t = VM_OBJECT_TEXT_VALUE(arg0);
            char* s = unicode_to_char(t);
            char* r = std::getenv(s);
            delete s;
            if (r != nullptr) {
                return VMObjectText(r).clone(); // NOTE: don't call delete on r
            } else {
                auto nop = machine()->get_data_string("System", "nop");
                return nop;
            }
        } else {
            BADARGS;
        }
    }
};

std::vector<VMObjectPtr> builtin_system(VM* vm) {
    std::vector<VMObjectPtr> oo;

    // throw combinator
    oo.push_back(VMThrow(vm).clone());

    // K, Id combinators
    oo.push_back(K(vm).clone());
    oo.push_back(Id(vm).clone());

    // basic constants
    oo.push_back(VMObjectData(vm, "System", "int").clone());
    oo.push_back(VMObjectData(vm, "System", "float").clone());
    oo.push_back(VMObjectData(vm, "System", "char").clone());
    oo.push_back(VMObjectData(vm, "System", "text").clone());
    oo.push_back(VMObjectData(vm, "System", "nil").clone());
    oo.push_back(VMObjectData(vm, "System", "cons").clone());
    oo.push_back(VMObjectData(vm, "System", "nop").clone());
    oo.push_back(VMObjectData(vm, "System", "true").clone());
    oo.push_back(VMObjectData(vm, "System", "false").clone());
    oo.push_back(VMObjectData(vm, "System", "tuple").clone());
    oo.push_back(VMObjectData(vm, "System", "object").clone());

    //oo.push_back(VMObjectData(vm, "System", "divzero").clone());

    // operators
    oo.push_back(MonMin(vm).clone());
    oo.push_back(Add(vm).clone());
    oo.push_back(Min(vm).clone());
    oo.push_back(Mul(vm).clone());
    oo.push_back(Div(vm).clone());
    oo.push_back(Mod(vm).clone());

    oo.push_back(Less(vm).clone());
    oo.push_back(LessEq(vm).clone());
    oo.push_back(Eq(vm).clone());
    oo.push_back(NegEq(vm).clone());

    oo.push_back(BinAnd(vm).clone());
    oo.push_back(BinOr(vm).clone());
    oo.push_back(BinXOr(vm).clone());
    oo.push_back(BinComplement(vm).clone());
    oo.push_back(BinLeftShift(vm).clone());
    oo.push_back(BinRightShift(vm).clone());

    oo.push_back(Toint(vm).clone());
    oo.push_back(Tofloat(vm).clone());
    oo.push_back(Totext(vm).clone());

    oo.push_back(Arg(vm).clone());
    oo.push_back(Getenv(vm).clone());

    // disassemble and reassemble
    oo.push_back(Dis(vm).clone());
    oo.push_back(Asm(vm).clone());

    // move to string?
    oo.push_back(Unpack(vm).clone());
    oo.push_back(Pack(vm).clone());

    // miscellaneous
    oo.push_back(VMObjectData(vm, "System", "v").clone());


    oo.push_back(Get(vm).clone());
    oo.push_back(Set(vm).clone());

    oo.push_back(GetField(vm).clone());
    oo.push_back(SetField(vm).clone());
    oo.push_back(ExtendField(vm).clone());

    return oo;
}
