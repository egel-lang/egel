#pragma once

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <math.h>
#include <stdlib.h>

#include <iostream>
#include <map>

#include "bytecode.hpp"
#include "runtime.hpp"
#if __has_include(<fmt/args.h>)
#include <fmt/args.h>
#endif

#include "builtin_time.hpp"  // 'unavoidable' abstraction breach due to format

namespace egel {

// globals
extern int application_argc;  // XXX: get rid of this
extern char **application_argv;
extern icu::UnicodeString application_version;

int application_argc = 0;
char **application_argv = nullptr;
icu::UnicodeString application_version = nullptr;

bool add_overflow(long int a, long int b, long int *c) {
    return __builtin_saddl_overflow(a, b, c);
}

bool add_overflow(long long int a, long long int b, long long int *c) {
    return __builtin_saddll_overflow(a, b, c);
}

bool sub_overflow(long int a, long int b, long int *c) {
    return __builtin_ssubl_overflow(a, b, c);
}

bool sub_overflow(long long int a, long long int b, long long int *c) {
    return __builtin_ssubll_overflow(a, b, c);
}

bool mul_overflow(long int a, long int b, long int *c) {
    return __builtin_smull_overflow(a, b, c);
}

bool mul_overflow(long long int a, long long int b, long long int *c) {
    return __builtin_smulll_overflow(a, b, c);
}

/**
 * Egel's system routines.
 *
 * Basic operators, conversions, and some other.
 **/

class K : public Binary {
public:
    BINARY_PREAMBLE(VM_SUB_BUILTIN, K, "System", "k");

    DOCSTRING("System::k x y - k combinator");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        return arg0;
    }
};

class Id : public Unary {
public:
    UNARY_PREAMBLE(VM_SUB_BUILTIN, Id, "System", "id");

    DOCSTRING("System::id x - identity combinator");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        return arg0;
    }
};

class MinInt : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, MinInt, "System", "min_int");

    DOCSTRING("System:min_int - minimum for integers");
    VMObjectPtr apply() const override {
        return machine()->create_integer(std::numeric_limits<vm_int_t>::min());
    }
};

class MaxInt : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, MaxInt, "System", "max_int");

    DOCSTRING("System::max_int - maximum for integers");
    VMObjectPtr apply() const override {
        return machine()->create_integer(std::numeric_limits<vm_int_t>::max());
    }
};

class MonMin : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, MonMin, "System", "!-");

    DOCSTRING("System::!- x - monadic minus");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            vm_int_t res;
            if (mul_overflow((vm_int_t)-1, i, &res)) {
                throw machine()->bad(this, "overflow");
            } else {
                return machine()->create_integer(res);
            }
        } else if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(-f);
        } else if (machine()->is_complex(arg0)) {
            auto f = machine()->get_complex(arg0);
            return machine()->create_complex(-f);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Add : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Add, "System", "+");

    DOCSTRING("System::+ x y - addition");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_integer(arg0)) && (machine()->is_integer(arg1))) {
            auto i0 = machine()->get_integer(arg0);
            auto i1 = machine()->get_integer(arg1);
            vm_int_t res;
            if (add_overflow(i0, i1, &res)) {
                throw machine()->bad(this, "overflow");
            } else {
                return machine()->create_integer(res);
            }
        } else if ((machine()->is_float(arg0)) && (machine()->is_float(arg1))) {
            auto f0 = machine()->get_float(arg0);
            auto f1 = machine()->get_float(arg1);
            return machine()->create_float(f0 + f1);
        } else if ((machine()->is_complex(arg0)) &&
                   (machine()->is_complex(arg1))) {
            auto f0 = machine()->get_complex(arg0);
            auto f1 = machine()->get_complex(arg1);
            return machine()->create_complex(f0 + f1);
        } else if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            auto f0 = machine()->get_text(arg0);
            auto f1 = machine()->get_text(arg1);
            return VMObjectText::create(f0 + f1);
        } else if (machine()->is_opaque(arg0)) {
            auto o = VMObjectOpaque::cast(arg0);
            auto r = o->op_add(arg1);
            if (r != nullptr) {
                return r;
            } else {
                throw machine()->bad_args(this, arg0, arg1);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class Min : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Min, "System", "-");

    DOCSTRING("System::+ x y - substraction");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_integer(arg0)) && (machine()->is_integer(arg1))) {
            auto i0 = machine()->get_integer(arg0);
            auto i1 = machine()->get_integer(arg1);
            vm_int_t res;
            if (sub_overflow(i0, i1, &res)) {
                throw machine()->bad(this, "overflow");
            } else {
                return machine()->create_integer(res);
            }
        } else if ((machine()->is_float(arg0)) && (machine()->is_float(arg1))) {
            auto f0 = machine()->get_float(arg0);
            auto f1 = machine()->get_float(arg1);
            return machine()->create_float(f0 - f1);
        } else if ((machine()->is_complex(arg0)) &&
                   (machine()->is_complex(arg1))) {
            auto f0 = machine()->get_complex(arg0);
            auto f1 = machine()->get_complex(arg1);
            return machine()->create_complex(f0 - f1);
        } else if (machine()->is_opaque(arg0)) {
            auto o = VMObjectOpaque::cast(arg0);
            auto r = o->op_minus(arg1);
            if (r != nullptr) {
                return r;
            } else {
                throw machine()->bad_args(this, arg0, arg1);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class Mul : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Mul, "System", "*");

    DOCSTRING("System::* x y - multiplication");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_integer(arg0)) && (machine()->is_integer(arg1))) {
            auto i0 = machine()->get_integer(arg0);
            auto i1 = machine()->get_integer(arg1);
            vm_int_t res;
            if (mul_overflow(i0, i1, &res)) {
                throw machine()->bad(this, "overflow");
            } else {
                return machine()->create_integer(res);
            }
        } else if ((machine()->is_float(arg0)) && (machine()->is_float(arg1))) {
            auto f0 = machine()->get_float(arg0);
            auto f1 = machine()->get_float(arg1);
            return machine()->create_float(f0 * f1);
        } else if ((machine()->is_complex(arg0)) &&
                   (machine()->is_complex(arg1))) {
            auto f0 = machine()->get_complex(arg0);
            auto f1 = machine()->get_complex(arg1);
            return machine()->create_complex(f0 * f1);
        } else if (machine()->is_opaque(arg0)) {
            auto o = VMObjectOpaque::cast(arg0);
            auto r = o->op_multiply(arg1);
            if (r != nullptr) {
                return r;
            } else {
                throw machine()->bad_args(this, arg0, arg1);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class Div : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Div, "System", "/");

    DOCSTRING("System::/ x y - division");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_integer(arg0)) && (machine()->is_integer(arg1))) {
            auto i0 = machine()->get_integer(arg0);
            auto i1 = machine()->get_integer(arg1);
            if (i1 == 0) {
                throw machine()->bad(this, "divide by zero");
            }
            return machine()->create_integer(i0 / i1);
        } else if ((machine()->is_float(arg0)) && (machine()->is_float(arg1))) {
            auto f0 = machine()->get_float(arg0);
            auto f1 = machine()->get_float(arg1);
            if (f1 == 0.0) {
                throw machine()->bad(this, "divide by zero");
            }
            return machine()->create_float(f0 / f1);
        } else if ((machine()->is_complex(arg0)) &&
                   (machine()->is_complex(arg1))) {
            auto f0 = machine()->get_complex(arg0);
            auto f1 = machine()->get_complex(arg1);
            if (f1.real() == 0.0 && f1.imag() == 0.0) {
                throw machine()->bad(this, "divide by zero");
            }
            return machine()->create_complex(f0 / f1);
        } else if (machine()->is_opaque(arg0)) {
            auto o = VMObjectOpaque::cast(arg0);
            auto r = o->op_divide(arg1);
            if (r != nullptr) {
                return r;
            } else {
                throw machine()->bad_args(this, arg0, arg1);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class Mod : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Mod, "System", "%");

    DOCSTRING("System::% x y - modulo");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_integer(arg0)) && (machine()->is_integer(arg1))) {
            auto i0 = machine()->get_integer(arg0);
            auto i1 = machine()->get_integer(arg1);
            if (i1 == 0) {
                throw machine()->bad(this, "divide by zero");
            }
            return machine()->create_integer(i0 % i1);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class BinAnd : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, BinAnd, "System", "&");

    DOCSTRING("System::& x y - bitwise and");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_integer(arg0)) && (machine()->is_integer(arg1))) {
            auto i0 = machine()->get_integer(arg0);
            auto i1 = machine()->get_integer(arg1);
            return machine()->create_integer(i0 & i1);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class BinOr : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, BinOr, "System", "$");

    DOCSTRING("System::$ x y - bitwise or");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_integer(arg0)) && (machine()->is_integer(arg1))) {
            auto i0 = machine()->get_integer(arg0);
            auto i1 = machine()->get_integer(arg1);
            return machine()->create_integer(i0 | i1);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class BinXOr : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, BinXOr, "System", "^");

    DOCSTRING("System::^ x y - bitwise xor");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_integer(arg0)) && (machine()->is_integer(arg1))) {
            auto i0 = machine()->get_integer(arg0);
            auto i1 = machine()->get_integer(arg1);
            return machine()->create_integer(i0 ^ i1);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class BinComplement : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, BinComplement, "System", "!~");

    DOCSTRING("System::!~ x - bitwise complement");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if ((machine()->is_integer(arg0))) {
            auto i0 = machine()->get_integer(arg0);
            return machine()->create_integer(~i0);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class BinLeftShift : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, BinLeftShift, "System", "<<");

    DOCSTRING("System::<< x y - bitwise left shift");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_integer(arg0)) && (machine()->is_integer(arg1))) {
            auto i0 = machine()->get_integer(arg0);
            auto i1 = machine()->get_integer(arg1);
            return machine()->create_integer(i0 << i1);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class BinRightShift : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, BinRightShift, "System", ">>");

    DOCSTRING("System::>> x y - bitwise right shift");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_integer(arg0)) && (machine()->is_integer(arg1))) {
            auto i0 = machine()->get_integer(arg0);
            auto i1 = machine()->get_integer(arg1);
            return machine()->create_integer(i0 >> i1);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class Less : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Less, "System", "<");

    DOCSTRING("System::< x y - builtin less");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        CompareVMObjectPtr compare;
        if (compare(arg0, arg1) < 0) {
            return machine()->create_true();
        } else {
            return machine()->create_false();
        }
    }
};

class LessEq : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, LessEq, "System", "<=");

    DOCSTRING("System::<= x y - builtin less or equals");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        CompareVMObjectPtr compare;
        if (compare(arg0, arg1) <= 0) {
            return machine()->create_true();
        } else {
            return machine()->create_false();
        }
    }
};

class Greater : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Greater, "System", ">");

    DOCSTRING("System::> x y - builtin greater");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        CompareVMObjectPtr compare;
        if (compare(arg0, arg1) > 0) {
            return machine()->create_true();
        } else {
            return machine()->create_false();
        }
    }
};

class GreaterEq : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, GreaterEq, "System", ">=");

    DOCSTRING("System::>= x y - builtin greater or equals");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        CompareVMObjectPtr compare;
        if (compare(arg0, arg1) >= 0) {
            return machine()->create_true();
        } else {
            return machine()->create_false();
        }
    }
};

class Eq : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Eq, "System", "==");

    DOCSTRING("System::== x y - builtin equality");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        CompareVMObjectPtr compare;
        if (compare(arg0, arg1) == 0) {
            return machine()->create_true();
        } else {
            return machine()->create_false();
        }
    }
};

class NegEq : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, NegEq, "System", "/=");

    DOCSTRING("System::/= x y - builtin inequality");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        CompareVMObjectPtr compare;
        if (compare(arg0, arg1) != 0) {
            return machine()->create_true();
        } else {
            return machine()->create_false();
        }
    }
};

class UseCount : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, UseCount, "System", "use_count");

    DOCSTRING("System::use_count o - the number of references to this term");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        return machine()->create_integer(arg0.use_count());
    }
};

// deprecated! System::get field obj - retrieve an object field
/*
class GetField : public Binary {
public:
    BINARY_PREAMBLE(VM_SUB_BUILTIN, GetField, "System", "get");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        static symbol_t object = 0;
        if (object == 0) object = machine()->enter_symbol("System", "object");

        if (machine()->is_array(arg1)) {
            auto ff = machine()->get_array(arg1);
            auto sz = ff.size();
            // check head is an object
            if (sz == 0) machine()->bad(this, "invalid");
            if (ff[0]->symbol() != object) machine()->bad(this, "invalid");
            // search for field
            CompareVMObjectPtr compare;
            unsigned int n;
            for (n = 1; n < sz; n = n + 2) {
                if (compare(arg0, ff[n]) == 0) break;
            }
            // return field
            if ((n + 1) < sz) {
                return ff[n + 1];
            } else {
                throw machine()->bad(this, "invalid");
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};
*/

// deprecated! System::set field val obj - set an object field
/*
class SetField : public Triadic {
public:
    TRIADIC_PREAMBLE(VM_SUB_BUILTIN, SetField, "System", "set");

    VMObjectPtr apply(const VMObjectPtr &arg0, const VMObjectPtr &arg1,
                      const VMObjectPtr &arg2) const override {
        static symbol_t object = 0;
        if (object == 0) object = machine()->enter_symbol("System", "object");

        if (machine()->is_array(arg2)) {
            auto ff = machine()->get_array(arg2);
            auto sz = ff.size();
            // check head is an object
            if (sz == 0) machine()->bad(this, "invalid");
            if (ff[0]->symbol() != object) machine()->bad(this, "invalid");
            // search field
            CompareVMObjectPtr compare;
            unsigned int n;
            for (n = 1; n < sz; n = n + 2) {
                if (compare(arg0, ff[n]) == 0) break;
            }
            // set field
            if ((n + 1) < sz) {
                // XXX: clean up this cast once. need destructive update
                auto arr = VMObjectArray::cast(arg2);
                arr->set(n + 1, arg1);
                return arg0;
            } else {
                throw machine()->bad(this, "invalid");
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};
*/

// deprecated! System::extend obj0 obj1 - extend object obj0 with every field
// from obj1
/*
class ExtendField : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, ExtendField, "System", "extend");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_array(arg0)) && (machine()->is_array(arg1))) {
            auto ff0 = machine()->get_array(arg0);
            auto sz0 = ff0.size();
            auto ff1 = machine()->get_array(arg1);
            auto sz1 = ff1.size();

            auto object = machine()->get_combinator("System", "object");

            // check head is an object
            if (sz0 == 0) machine()->bad(this, "invalid");
            if (ff0[0] != object) machine()->bad(this, "invalid");
            if (sz1 == 0) machine()->bad(this, "invalid");
            if (ff1[0] != object) machine()->bad(this, "invalid");
            // create field union
            unsigned int n;
            std::map<VMObjectPtr, VMObjectPtr> fields;
            for (n = 1; n < sz0; n = n + 2) {
                if ((n + 1) < sz0) fields[ff0[n]] = ff0[n + 1];
            }
            for (n = 1; n < sz1; n = n + 2) {
                if ((n + 1) < sz1) fields[ff1[n]] = ff1[n + 1];
            }
            // return object
            VMObjectPtrs oo;
            oo.push_back(object);
            for (const auto &f : fields) {
                oo.push_back(f.first);
                oo.push_back(f.second);
            }

            return machine()->create_array(oo);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};
*/

class Toint : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Toint, "System", "to_int");

    DOCSTRING("System::to_int x - Try and convert an object to int");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_integer(arg0)) {
            return arg0;
        } else if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_integer((int)f);
        } else if (machine()->is_char(arg0)) {
            auto c = machine()->get_char(arg0);
            auto i = VM::unicode_to_int(icu::UnicodeString() + c);
            return machine()->create_integer(i);
        } else if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            auto i = VM::unicode_to_int(s);
            return machine()->create_integer(i);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Tofloat : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Tofloat, "System", "to_float");

    DOCSTRING("System::to_float x - try and convert an object to float");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return machine()->create_float(i);
        } else if (machine()->is_float(arg0)) {
            return arg0;
        } else if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            auto f = VM::unicode_to_float(s);
            return machine()->create_float(f);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Tocomplex : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Tocomplex, "System", "to_complex");

    DOCSTRING("System::to_complex x - try and convert an object to complex");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return machine()->create_complex(vm_complex_t(i, 0));
        } else if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_complex(vm_complex_t(f, 0));
        } else if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            auto f = VM::unicode_to_complex(s);
            return machine()->create_complex(f);
        } else if (machine()->is_array(arg0) &&
                   machine()->is_tuple(machine()->array_get(arg0, 0))) {
            auto oo = machine()->from_tuple(arg0);
            if (machine()->is_float(oo[0]) && machine()->is_float(oo[1])) {
                auto f0 = machine()->get_float(oo[0]);
                auto f1 = machine()->get_float(oo[1]);
                return machine()->create_complex(vm_complex_t(f0, f1));
            } else {
                throw machine()->bad_args(this, arg0);
            }
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Fromcomplex : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Fromcomplex, "System", "from_complex");

    DOCSTRING("System::from_complex z - convert complex to tuple");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_complex(arg0)) {
            auto z = machine()->get_complex(arg0);
            VMObjectPtrs oo;
            oo.push_back(machine()->create_float(z.real()));
            oo.push_back(machine()->create_float(z.imag()));
            return machine()->to_tuple(oo);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Totext : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Totext, "System", "to_text");

    DOCSTRING("System::to_text x - try and convert an object to text");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            auto s = VM::unicode_from_int(i);
            return machine()->create_text(s);
        } else if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            auto s = VM::unicode_from_float(f);
            return machine()->create_text(s);
        } else if (machine()->is_complex(arg0)) {
            auto f = machine()->get_complex(arg0);
            auto s = VM::unicode_from_complex(f);
            return machine()->create_text(s);
        } else if (machine()->is_char(arg0)) {
            auto c = machine()->get_char(arg0);
            auto s = VM::unicode_from_char(c);
            return machine()->create_text(s);
        } else if (machine()->is_text(arg0)) {
            return arg0;
        } else {
            return machine()->create_text(arg0->to_text());
            // throw machine()->bad_args(this, arg0);
        }
    }
};

class Reference : public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_BUILTIN, Reference, "System", "reference");

    DOCSTRING("System::reference - an opaque reference object");
    Reference(VM *vm, const VMObjectPtr &r)
        : Opaque(VM_SUB_BUILTIN, vm, "System", "reference") {
        _ref = r;
    }

    Reference(const Reference &ref)
        : Opaque(VM_SUB_BUILTIN, ref.machine(), ref.symbol()) {
        _ref = ref.get_ref();
    }

    static VMObjectPtr create(VM *vm, const VMObjectPtr &ref) {
        return VMObjectPtr(new Reference(vm, ref));
    }

    int compare(const VMObjectPtr &o) override {
        return -1;  // XXX: fix this once
    }

    VMObjectPtr get_ref() const {
        return _ref;
    }

    void set_ref(const VMObjectPtr &r) {
        _ref = r;
    }

protected:
    VMObjectPtr _ref = nullptr;
};

class Ref : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Ref, "System", "ref");

    DOCSTRING("System::ref x - create a reference object");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        auto vm = machine();
        auto r = Reference::create(vm, arg0);
        return r;
    }
};

class Getref : public Unary {
public:
    UNARY_PREAMBLE(VM_SUB_BUILTIN, Getref, "System", "get_ref");

    DOCSTRING("System::get_ref ref - dereference");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (Reference::is_type(arg0)) {
            auto r = Reference::cast(arg0);
            return r->get_ref();
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Setref : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Setref, "System", "set_ref");

    DOCSTRING("System::set_ref ref x - set reference objec");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if (Reference::is_type(arg0)) {
            auto r = Reference::cast(arg0);
            r->set_ref(arg1);
            return arg0;
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class Version : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Version, "System", "version");

    DOCSTRING("System::version - version information of this executable");
    VMObjectPtr apply() const override {
        if (application_version == "") {
            throw machine()->create_text("no version information");
        } else {
            return VMObjectText::create(application_version);
        }
    }
};

class Arg : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Arg, "System", "arg");

    DOCSTRING("System::arg n - the n-th application argument, or none");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (application_argv == nullptr) {
            throw machine()->bad_args(this, arg0);
        }
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            if (i < application_argc) {
                return VMObjectText::create(application_argv[i]);
            } else {
                auto none = machine()->create_none();
                return none;
            }
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Getenv : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Getenv, "System", "get_env");

    DOCSTRING("System::get_env s - the value of environment variable, or none");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto t = machine()->get_text(arg0);
            char *s = VM::unicode_to_utf8_chars(t);
            char *r = std::getenv(s);
            delete s;
            if (r != nullptr) {
                return VMObjectText::create(r);  // NOTE: don't call delete on r
            } else {
                auto none = machine()->create_none();
                return none;
            }
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// deprecated! System::&& - short-circuited and
/*
class LazyAnd : public Binary {
public:
    BINARY_PREAMBLE(VM_SUB_BUILTIN, LazyAnd, "System", "&&");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_combinator(arg0)) &&
            (arg0->symbol() == SYMBOL_FALSE)) {
            return arg0;
        } else if ((machine()->is_combinator(arg0)) &&
                   (arg0->symbol() == SYMBOL_TRUE)) {
            VMObjectPtrs thunk;
            thunk.push_back(arg1);
            thunk.push_back(machine()->create_none());
            return machine()->create_array(thunk);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};
*/

// deprecated! System::|| - short-circuited or
/*
class LazyOr : public Binary {
public:
    BINARY_PREAMBLE(VM_SUB_BUILTIN, LazyOr, "System", "||");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_combinator(arg0)) &&
            (arg0->symbol() == SYMBOL_TRUE)) {
            return arg0;
        } else if ((machine()->is_combinator(arg0)) &&
                   (arg0->symbol() == SYMBOL_FALSE)) {
            VMObjectPtrs thunk;
            thunk.push_back(arg1);
            thunk.push_back(machine()->create_none());
            return machine()->create_array(thunk);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};
*/

class AppToList : public Variadic {
public:
    VARIADIC_PREAMBLE(VM_SUB_BUILTIN, AppToList, "System", "app_to_list");

    DOCSTRING("System::app_to_list o0 .. on - arguments to list");
    VMObjectPtr apply(const VMObjectPtrs &args) const override {
        return machine()->to_list(args);
    }
};

class ListToApp : public Unary {
public:
    UNARY_PREAMBLE(VM_SUB_BUILTIN, ListToApp, "System", "list_to_app");

    DOCSTRING("System::list_to_app {o0 .. on} - list to application");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_list(arg0)) {
            auto oo = machine()->from_list(arg0);
            return machine()->create_array(oo);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class TupleToList : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, TupleToList, "System", "tuple_to_list");

    DOCSTRING("System::tuple_to_list (o0, .., on) - tuple to list");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_array(arg0) &&
            machine()->is_tuple(machine()->array_get(arg0, 0))) {
            auto oo = machine()->from_tuple(arg0);
            return machine()->to_list(oo);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class ListToTuple : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, ListToTuple, "System", "list_to_tuple");

    DOCSTRING("System::list_to_tuple {o0, .., on} - list to tuple");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_list(arg0)) {
            auto oo = machine()->from_list(arg0);
            return machine()->to_tuple(oo);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Print : public Variadic {
public:
    VARIADIC_PREAMBLE(VM_SUB_BUILTIN, Print, "System", "print");

    DOCSTRING("System::print o0 .. on - print terms");
    VMObjectPtr apply(const VMObjectPtrs &args) const override {
        icu::UnicodeString s;
        for (auto &arg : args) {
            if (machine()->is_integer(arg)) {
                s += arg->to_text();
            } else if (machine()->is_float(arg)) {
                s += arg->to_text();
            } else if (machine()->is_complex(arg)) {
                s += arg->to_text();
            } else if (arg->tag() == VM_OBJECT_CHAR) {
                s += machine()->get_char(arg);
            } else if (machine()->is_text(arg)) {
                s += machine()->get_text(arg);
            } else {
                s += arg->to_text();
            }
        }
        std::cout << s;

        return machine()->create_none();
    }
};

class Getline : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Getline, "System", "get_line");

    DOCSTRING("System::get_line - read a line from standard input");
    VMObjectPtr apply() const override {
        std::string line;
        std::getline(std::cin, line);
        icu::UnicodeString str(line.c_str());
        return machine()->create_text(str);
    }
};

class Format : public Variadic {
public:
    VARIADIC_PREAMBLE(VM_SUB_BUILTIN, Format, "System", "format");

    DOCSTRING("System::format fmt x0 ...  - create a formatted strin");
    VMObjectPtr apply(const VMObjectPtrs &args) const override {
        if (args.size() < 1) {
            return nullptr;
        } else {
            auto a0 = args[0];
            if (machine()->is_text(a0)) {
                auto f = machine()->get_text(a0);
                auto fmt = VM::unicode_to_utf8_chars(f);

                fmt::dynamic_format_arg_store<fmt::format_context> store;

                for (int n = 1; n < (int)args.size(); n++) {
                    auto arg = args[n];
                    if (machine()->is_integer(arg)) {
                        auto i = machine()->get_integer(arg);
                        store.push_back(i);
                    } else if (machine()->is_float(arg)) {
                        auto f = machine()->get_float(arg);
                        store.push_back(f);
                    } else if (machine()->is_complex(
                                   arg)) {  // XXX: fmtlib doesn't support
                                            // complex
                        auto s0 = arg->to_text();
                        auto s1 = VM::unicode_to_utf8_chars(s0);
                        store.push_back(s1);
                        delete s1;
                    } else if (machine()->is_char(arg)) {
                        auto c = machine()->get_char(arg);
                        auto s0 = icu::UnicodeString(c);  // XXX: wut?
                        auto s1 = VM::unicode_to_utf8_chars(s0);
                        store.push_back(s1);
                        delete s1;
                    } else if (machine()->is_text(arg)) {
                        auto t = machine()->get_text(arg);
                        auto s0 = VM::unicode_to_utf8_chars(t);
                        store.push_back(s0);
                        delete s0;
                    } else if (TimePoint::is_time_point(arg)) {
                        auto tp = TimePoint::cast(arg);
                        switch (tp->get_clock_type()) {
                            case clock_type::SYSTEM_CLOCK: {
                                auto t0 = tp->time_point_system_clock();
                                store.push_back(t0);
                            } break;
                                /*
                                                        case
                                   clock_type::STEADY_CLOCK: { auto t0 =
                                   tp->time_point_steady_clock();
                                                            store.push_back(t0);
                                                        }
                                                        break;
                                                        case
                                   clock_type::HIGH_RESOLUTION_CLOCK: { auto t0
                                   = tp->time_point_high_resolution_clock();
                                                            store.push_back(t0);
                                                        }
                                                        break;
                                */
                                /*
                                                        case UTC_CLOCK:
                                                            auto t0 =
                                   tp->time_point_utc_clock();
                                                            store.push_back(t0);
                                                        break;
                                                        case TAI_CLOCK:
                                                            auto t0 =
                                   tp->time_point_tai_clock();
                                                            store.push_back(t0);
                                                        break;
                                                        case GPS_CLOCK:
                                                            auto t0 =
                                   tp->time_point_gps_clock();
                                                            store.push_back(t0);
                                                        break;
                                                        case FILE_CLOCK:
                                                            auto t0 =
                                   tp->time_point_file_clock();
                                                            store.push_back(t0);
                                                        break;
                                */
                            default: {
                                PANIC("end of case");
                                return nullptr;
                            }
                        }
                    } else if (Duration::is_duration(arg)) {
                        auto d = Duration::cast(arg);
                        auto d0 = d->duration();
                        store.push_back(d0);
                    } else if (Date::is_date(arg)) {
                        auto d = Date::cast(arg);
                        auto d0 = d->date();
                        store.push_back(d0);
                    } else {
                        auto a = arg->to_text();
                        auto s0 = VM::unicode_to_utf8_chars(a);
                        store.push_back(s0);
                        delete s0;
                    }
                }

                std::string r;
                try {
                    r = fmt::vformat(fmt, store);
                } catch (std::runtime_error &e) {
                    throw machine()->bad(this, "invalid");
                }
                auto u = icu::UnicodeString(r.c_str());
                delete fmt;

                return VMObjectText::create(u);
            } else {
                throw machine()->bad(this, "format");
            }
        }
    }
};

// deprecated! System::fast_foldl f z xx - fast_foldl
/*
class Foldl : public Triadic {
public:
    TRIADIC_PREAMBLE(VM_SUB_BUILTIN, Foldl, "System", "fast_foldl");

    VMObjectPtr apply(const VMObjectPtr &arg0, const VMObjectPtr &arg1, const
VMObjectPtr &arg2) const override { auto vm = machine();

        auto f = arg0;
        auto acc = arg1;
        auto xx = arg2;
        while(!vm->is_nil(xx)) {
            if (vm->is_array(xx) && (vm->array_size(xx) == 3) &&
(vm->is_cons(vm->array_get(xx,0)))) { auto x = vm->array_get(xx,1); xx =
vm->array_get(xx,2);

                auto thunk = vm->create_array(3);
                vm->array_set(thunk, 0, f);
                vm->array_set(thunk, 1, acc);
                vm->array_set(thunk, 2, x);

                auto r = vm->reduce(thunk);
                if (r.exception) {
                    throw r.result;
                } else {
                    acc = r.result;
                }
            } else {
                throw machine()->bad_args(this, arg0, arg1, arg2);
            }
        }

        return acc;
    }
};
*/

class SystemModule : public CModule {
public:
    virtual ~SystemModule() {
    }

    icu::UnicodeString name() const override {
        return "system";
    }

    icu::UnicodeString docstring() const override {
        return "The 'system' module defines primitive combinators.";
    }

    std::vector<VMObjectPtr> exports(VM *vm) override {
        std::vector<VMObjectPtr> oo;

        // throw and stall combinators
        oo.push_back(VMThrow::create(vm));
        oo.push_back(VMHandle::create(vm));
        oo.push_back(VMStall::create(vm));
        oo.push_back(VMNapp::create(vm));

        // K, Id combinators
        oo.push_back(K::create(vm));
        oo.push_back(Id::create(vm));

        // basic constants
        oo.push_back(VMObjectData::create(vm, "System", "int"));
        oo.push_back(VMObjectData::create(vm, "System", "float"));
        oo.push_back(VMObjectData::create(vm, "System", "complex"));
        oo.push_back(VMObjectData::create(vm, "System", "char"));
        oo.push_back(VMObjectData::create(vm, "System", "text"));
        oo.push_back(VMObjectData::create(vm, "System", "nil"));
        oo.push_back(VMObjectData::create(vm, "System", "cons"));
        oo.push_back(VMObjectData::create(vm, "System", "none"));
        oo.push_back(VMObjectData::create(vm, "System", "true"));
        oo.push_back(VMObjectData::create(vm, "System", "false"));
        oo.push_back(VMObjectData::create(vm, "System", "tuple"));
        oo.push_back(VMObjectData::create(vm, "System", "object"));

        // operators

        oo.push_back(MinInt::create(vm));
        oo.push_back(MaxInt::create(vm));
        oo.push_back(Add::create(vm));
        oo.push_back(MonMin::create(vm));  // the dreaded operator
        oo.push_back(Min::create(vm));
        oo.push_back(Mul::create(vm));
        oo.push_back(Div::create(vm));
        oo.push_back(Mod::create(vm));

        oo.push_back(Less::create(vm));
        oo.push_back(LessEq::create(vm));
        oo.push_back(Greater::create(vm));
        oo.push_back(GreaterEq::create(vm));
        oo.push_back(Eq::create(vm));
        oo.push_back(NegEq::create(vm));
        oo.push_back(UseCount::create(vm));

        oo.push_back(BinAnd::create(vm));
        oo.push_back(BinOr::create(vm));
        oo.push_back(BinXOr::create(vm));
        oo.push_back(BinComplement::create(vm));
        oo.push_back(BinLeftShift::create(vm));
        oo.push_back(BinRightShift::create(vm));

        oo.push_back(Toint::create(vm));
        oo.push_back(Tofloat::create(vm));
        oo.push_back(Tocomplex::create(vm));
        oo.push_back(Totext::create(vm));

        oo.push_back(Fromcomplex::create(vm));

        // move to string?
        oo.push_back(ToChars::create(vm));
        oo.push_back(FromChars::create(vm));

        // system info, override if sandboxed
        oo.push_back(Version::create(vm));
        oo.push_back(Arg::create(vm));
        oo.push_back(Getenv::create(vm));

        // munching
        oo.push_back(AppToList::create(vm));
        oo.push_back(ListToApp::create(vm));
        oo.push_back(TupleToList::create(vm));
        oo.push_back(ListToTuple::create(vm));

        // the builtin print & getline, override if sandboxed
        oo.push_back(Print::create(vm));
        oo.push_back(Getline::create(vm));
        oo.push_back(Format::create(vm));

        // references
        oo.push_back(VMObjectStub::create(vm, "System::reference"));  // XXXX
        oo.push_back(Ref::create(vm));
        oo.push_back(Setref::create(vm));
        oo.push_back(Getref::create(vm));

        return oo;
    }
};

}  // namespace egel
