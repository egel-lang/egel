#include "builtin_system.hpp"

#include "utils.hpp"
#include "bytecode.hpp"

#include <stdlib.h>
#include <ostream>
#include <map>
#include <stdlib.h>
#include <math.h>
#include <fmt/core.h> // std::format

// globals
int    application_argc = 0;
char** application_argv = nullptr;

/**
 * Egel's system routines.
 *
 * Basic operators, conversions, and some other.
 **/

//## System:k x y - k combinator
class K: public Binary {
public:
    BINARY_PREAMBLE(K, "System", "k");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        return arg0;
    }
};

//## System:id x - identity combinator
class Id: public Unary {
public:
    UNARY_PREAMBLE(Id, "System", "id");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        return arg0;
    }
};

//## System:!- x - monadic minus
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

//## System:+ x y - addition
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

//## System:+ x y - substraction
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

//## System:* x y - multiplication
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

//## System:/ x y - division
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

//## System:% x y - modulo
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

//## System:& x y - bitwise and
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

//## System:$ x y - bitwise or
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

//## System:^ x y - bitwise xor
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

//## System:!~ x - bitwise complement
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

//## System:<< x y - bitwise left shift
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

//## System:>> x y - bitwise right shift
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

//## System:< x y - builtin less
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

//## System:<= x y - builtin less or equals
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

//## System:== x y - builtin equality
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

//## System:/= x y - builtin inequality
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

//## System:get field obj - retrieve an object field
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

//## System:set field val obj - set an object field
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

//## System:extend obj0 obj1 - extend object obj0 with every field from obj1
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


//## System:toint x - Try and convert an object to int
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

//## System:tofloat x - try and convert an object to float
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

//## System:totext x - try and convert an object to text
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

//## System:reference - an opaque reference object
class Reference : public Opaque {
public:
    OPAQUE_PREAMBLE(Reference, "System", "reference");

    Reference(VM* vm, const VMObjectPtr& r)
        : Opaque(vm, "System", "reference") {
        _ref = r;
    }

    Reference(const Reference& ref): Opaque(ref.machine(), ref.symbol()) {
        _ref = ref.getref();
    }

    VMObjectPtr clone() const override {
        return VMObjectPtr(new Reference(*this));
    }

    int compare(const VMObjectPtr& o) override {
        return -1; // XXX: fix this once
    }

    VMObjectPtr getref() const {
        return _ref;
    }

    void setref(const VMObjectPtr& r) {
        _ref = r;
    }

protected:
    VMObjectPtr _ref = nullptr;
};

//## System:ref x - create a reference object from x
class Ref: public Monadic {
public:
    MONADIC_PREAMBLE(Ref, "System", "ref");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto vm = machine();
        auto r = Reference(vm, arg0).clone();
        return r;
    }
};

//## System:getref ref - get the stored value from ref
class Getref: public Unary {
public:
    UNARY_PREAMBLE(Getref, "System", "getref");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        symbol_t sym = machine()->enter_symbol("System", "reference");

        if ((arg0->tag() == VM_OBJECT_OPAQUE) && (arg0->symbol() == sym)) {
            auto r = std::static_pointer_cast<Reference>(arg0);
            return r->getref();
        } else {
            BADARGS;
        }
    }
};

//## System:setref ref x - set reference object ref to x
class Setref: public Dyadic {
public:
    DYADIC_PREAMBLE(Setref, "System", "setref");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        symbol_t sym = machine()->enter_symbol("System", "reference");

        if ((arg0->tag() == VM_OBJECT_OPAQUE) && (arg0->symbol() == sym)) {
            auto r = std::static_pointer_cast<Reference>(arg0);
            r->setref(arg1);
            return arg0;
        } else {
            BADARGS;
        }
    }
};

//## System:unpack s - create a list of chars from a string
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

//## System:pack s - create a string from a list of code points
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

//## System:dis o - disassemble a combinator object
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

//## System:asm s0 s1 - assemble bytecode into a combinator
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

//## System:arg n - return the n-th application argument, otherwise return 'nop'
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

//## System:getenv s - return the value of environment variable s, otherwise return 'nop'
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

//## System:&& - short-circuited and
class LazyAnd: public Binary {
public:
    BINARY_PREAMBLE(LazyAnd, "System", "&&");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_COMBINATOR) &&
             (arg0->symbol() == SYMBOL_FALSE) ) {
            return arg0;
        } else if ( (arg0->tag() == VM_OBJECT_COMBINATOR) &&
             (arg0->symbol() == SYMBOL_TRUE) ) {
            VMObjectPtrs thunk;
            thunk.push_back(arg1);
            thunk.push_back(machine()->get_data_string("System", "nop"));
            return VMObjectArray(thunk).clone();
        } else {
            BADARGS;
        }
    }
};

//## System:|| - short-circuited or
class LazyOr: public Binary {
public:
    BINARY_PREAMBLE(LazyOr, "System", "||");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_COMBINATOR) &&
             (arg0->symbol() == SYMBOL_TRUE) ) {
            return arg0;
        } else if ( (arg0->tag() == VM_OBJECT_COMBINATOR) &&
             (arg0->symbol() == SYMBOL_FALSE) ) {
            VMObjectPtrs thunk;
            thunk.push_back(arg1);
            thunk.push_back(machine()->get_data_string("System", "nop"));
            return VMObjectArray(thunk).clone();
        } else {
            BADARGS;
        }
    }
};

//## System:print o0 .. on - print terms, don't escape characters or texts 
class Print: public Variadic {
public:
    VARIADIC_PREAMBLE(Print, "System", "print");

    VMObjectPtr apply(const VMObjectPtrs& args) const override {

        icu::UnicodeString s;
        for (auto& arg:args) {
            if (arg->tag() == VM_OBJECT_INTEGER) {
                s += arg->to_text();
            } else if (arg->tag() == VM_OBJECT_FLOAT) {
                s += arg->to_text();
            } else if (arg->tag() == VM_OBJECT_CHAR) {
                s += VM_OBJECT_CHAR_VALUE(arg);
            } else if (arg->tag() == VM_OBJECT_TEXT) {
                s += VM_OBJECT_TEXT_VALUE(arg);
            } else {
                s += arg->to_text();
            }
        }
        std::cout << s;

        return create_nop();
    }
};

//## System:format fmt x ...  - create a string from formatted string fmt and objects x,..
class Format: public Variadic {
public:
    VARIADIC_PREAMBLE(Format, "System", "format");

    VMObjectPtr apply(const VMObjectPtrs& args) const override {

        if (args.size() < 1) {
            return nullptr;
        } else {
            auto a0 = args[0];
            if (a0->tag() == VM_OBJECT_TEXT) {
                auto f = VM_OBJECT_TEXT_VALUE(a0);
                auto fmt = unicode_to_char(f);

                fmt::dynamic_format_arg_store<fmt::format_context> store;
                
                for (int n = 1; n < (int) args.size(); n++) {
                    auto arg = args[n];
                    if (arg->tag() == VM_OBJECT_INTEGER) {
                        auto i = VM_OBJECT_INTEGER_VALUE(arg);
                        store.push_back(i);
                    } else if (arg->tag() == VM_OBJECT_FLOAT) {
                        auto f = VM_OBJECT_FLOAT_VALUE(arg);
                        store.push_back(f);
                    } else if (arg->tag() == VM_OBJECT_CHAR) {
                        auto c = VM_OBJECT_CHAR_VALUE(arg);
                        auto s0 = icu::UnicodeString(c);
                        auto s1 = unicode_to_char(s0);
                        store.push_back(s1);
                        delete s1;
                    } else if (arg->tag() == VM_OBJECT_TEXT) {
                        auto t = VM_OBJECT_TEXT_VALUE(arg);
                        auto s0 = unicode_to_char(t);
                        store.push_back(s0);
                        delete s0;
                    } else {
                        auto a = arg->to_text();
                        auto s0 = unicode_to_char(a);
                        store.push_back(s0);
                        delete s0;
                    }
                }

                std::string r;
                try {
                    r = fmt::vformat(fmt, store);
                } catch (std::runtime_error& e) {
                    INVALID;
                }
                auto u = icu::UnicodeString(r.c_str());
                delete fmt;

                return VMObjectText(u).clone();
            } else {
                INVALID;
            }
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

    // disassemble and reassemble
    oo.push_back(Dis(vm).clone());
    oo.push_back(Asm(vm).clone());

    // move to string?
    oo.push_back(Unpack(vm).clone());
    oo.push_back(Pack(vm).clone());

    // lazy operators
    oo.push_back(LazyAnd(vm).clone());
    oo.push_back(LazyOr(vm).clone());

    // system info, override if sandboxed
    oo.push_back(Arg(vm).clone());
    oo.push_back(Getenv(vm).clone());

    // the builtin print, override if sandboxed
    oo.push_back(Print(vm).clone());
    oo.push_back(Format(vm).clone());

    // references
    oo.push_back(Reference(vm).clone());
    oo.push_back(Ref(vm).clone());
    oo.push_back(Setref(vm).clone());
    oo.push_back(Getref(vm).clone());

    // OO fields
    oo.push_back(GetField(vm).clone());
    oo.push_back(SetField(vm).clone());
    oo.push_back(ExtendField(vm).clone());

    return oo;
}
