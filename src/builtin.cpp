#include "runtime.hpp"
#include "builtin.hpp"
#include <stdlib.h>
#include <ostream>

// XXX: this needs a better solution but I haven't seen anything around which solves this well

class Monadic: public VMObjectCombinator {
public:
    Monadic(VM* m, const UnicodeString& n0, const UnicodeString& n1): 
         VMObjectCombinator(VM_OBJECT_FLAG_INTERNAL, m, n0, n1) {
    }

    Monadic(VM* m, const symbol_t s): 
         VMObjectCombinator(VM_OBJECT_FLAG_INTERNAL, m, s) {
    }

    virtual VMObjectPtr apply(const VMObjectPtr& arg0) const = 0;
        
    VMObjectPtr reduce(const VMObjectPtr& thunk) const override {
        auto tt  = VM_OBJECT_ARRAY_VALUE(thunk);
        auto rt  = tt[0];
        auto rti = tt[1];
        auto k   = tt[2];

        VMObjectPtr r;
        if (tt.size() > 5) {
            auto arg0 = tt[5];

            r = apply(arg0);
            if (r == nullptr) {
                VMObjectPtrs rr;
                for (uint i = 4; i<tt.size(); i++) {
                    rr.push_back(tt[i]);
                }
                r = VMObjectArray(rr).clone();
            }
        } else {
            VMObjectPtrs rr;
            for (uint i = 4; i<tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray(rr).clone();
        }

        auto index = VM_OBJECT_INTEGER_VALUE(rti);
        auto rta   = VM_OBJECT_ARRAY_CAST(rt);
        rta->set(index, r);

        return k;
    }
};

#define MONADIC_PREAMBLE(c, n0, n1) \
    c(VM* m): Monadic(m, n0, n1) { \
    } \
    c(VM* m, const symbol_t s): Monadic(m, s) { \
    } \
    c(const c& o) : c(o.machine(), o.symbol()) { \
    } \
    VMObjectPtr clone() const { \
        return VMObjectPtr(new c(*this)); \
    }

class Dyadic: public VMObjectCombinator {
public:
    Dyadic(VM* m, const UnicodeString& n0, const UnicodeString& n1): 
         VMObjectCombinator(VM_OBJECT_FLAG_INTERNAL, m, n0, n1) {
    }

    Dyadic(VM* m, const symbol_t s): 
         VMObjectCombinator(VM_OBJECT_FLAG_INTERNAL, m, s) {
    }

    virtual VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const = 0;
        
    VMObjectPtr reduce(const VMObjectPtr& thunk) const override {
        auto tt  = VM_OBJECT_ARRAY_VALUE(thunk);
        auto rt  = tt[0];
        auto rti = tt[1];
        auto k   = tt[2];

        VMObjectPtr r;
        if (tt.size() > 6) {
            auto arg0 = tt[5];
            auto arg1 = tt[6];

            r = apply(arg0, arg1);
            if (r == nullptr) {
                VMObjectPtrs rr;
                for (uint i = 4; i<tt.size(); i++) {
                    rr.push_back(tt[i]);
                }
                r = VMObjectArray(rr).clone();
            }
        } else {
            VMObjectPtrs rr;
            for (uint i = 4; i<tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray(rr).clone();
        }

        auto index = VM_OBJECT_INTEGER_VALUE(rti);
        auto rta   = VM_OBJECT_ARRAY_CAST(rt);
        rta->set(index, r);

        return k;
    }
};

#define DYADIC_PREAMBLE(c, n0, n1) \
    c(VM* m): Dyadic(m, n0, n1) { \
    } \
    c(VM* m, const symbol_t s): Dyadic(m, s) { \
    } \
    c(const c& o) : c(o.machine(), o.symbol()) { \
    } \
    VMObjectPtr clone() const { \
        return VMObjectPtr(new c(*this)); \
    }


// well, this is roughly how it should work
class MonMin: public Monadic {
public:
    MONADIC_PREAMBLE(MonMin, "System", "monmin");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_INTEGER) {
            auto i = VM_OBJECT_INTEGER_VALUE(arg0);
            return VMObjectInteger(-i).clone();
        } else if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return VMObjectFloat(-f).clone();
        } else {
            return nullptr;
        }
    }
};

class Add: public Dyadic {
public:
    DYADIC_PREAMBLE(Add, "System", "add");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_INTEGER) &&
             (arg1->tag() == VM_OBJECT_INTEGER) ) {
            auto i0 = VM_OBJECT_INTEGER_VALUE(arg0);
            auto i1 = VM_OBJECT_INTEGER_VALUE(arg1);
            return VMObjectInteger(i0+i1).clone();
        } else if ( (arg0->tag() == VM_OBJECT_FLOAT) &&
             (arg1->tag() == VM_OBJECT_FLOAT) ) {
            auto f0 = VM_OBJECT_FLOAT_VALUE(arg0);
            auto f1 = VM_OBJECT_FLOAT_VALUE(arg1);
            return VMObjectFloat(f0+f1).clone();
        } else {
            return nullptr;
        }
    }
};

class Min: public Dyadic {
public:
    DYADIC_PREAMBLE(Min, "System", "min");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_INTEGER) &&
             (arg1->tag() == VM_OBJECT_INTEGER) ) {
            auto i0 = VM_OBJECT_INTEGER_VALUE(arg0);
            auto i1 = VM_OBJECT_INTEGER_VALUE(arg1);
            return VMObjectInteger(i0-i1).clone();
        } else if ( (arg0->tag() == VM_OBJECT_FLOAT) &&
             (arg1->tag() == VM_OBJECT_FLOAT) ) {
            auto f0 = VM_OBJECT_FLOAT_VALUE(arg0);
            auto f1 = VM_OBJECT_FLOAT_VALUE(arg1);
            return VMObjectFloat(f0-f1).clone();
        } else {
            return nullptr;
        }
    }
};

class Mul: public Dyadic {
public:
    DYADIC_PREAMBLE(Mul, "System", "mul");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_INTEGER) &&
             (arg1->tag() == VM_OBJECT_INTEGER) ) {
            auto i0 = VM_OBJECT_INTEGER_VALUE(arg0);
            auto i1 = VM_OBJECT_INTEGER_VALUE(arg1);
            return VMObjectInteger(i0*i1).clone();
        } else if ( (arg0->tag() == VM_OBJECT_FLOAT) &&
             (arg1->tag() == VM_OBJECT_FLOAT) ) {
            auto f0 = VM_OBJECT_FLOAT_VALUE(arg0);
            auto f1 = VM_OBJECT_FLOAT_VALUE(arg1);
            return VMObjectFloat(f0*f1).clone();
        } else {
            return nullptr;
        }
    }
};

class Div: public Dyadic {
public:
    DYADIC_PREAMBLE(Div, "System", "div");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_INTEGER) &&
             (arg1->tag() == VM_OBJECT_INTEGER) ) {
            auto i0 = VM_OBJECT_INTEGER_VALUE(arg0);
            auto i1 = VM_OBJECT_INTEGER_VALUE(arg1);
            return VMObjectInteger(i0/i1).clone();
        } else if ( (arg0->tag() == VM_OBJECT_FLOAT) &&
             (arg1->tag() == VM_OBJECT_FLOAT) ) {
            auto f0 = VM_OBJECT_FLOAT_VALUE(arg0);
            auto f1 = VM_OBJECT_FLOAT_VALUE(arg1);
            return VMObjectFloat(f0/f1).clone();
        } else {
            return nullptr;
        }
    }
};

void vm_register(VM* vm) {
    symbol_t symint   = SYMBOL_INT;
    symbol_t symfloat = SYMBOL_FLOAT;

    vm->define_data(VMObjectData(vm, "System", "int").clone());
    vm->define_data(VMObjectData(vm, "System", "float").clone());
    vm->define_data(VMObjectData(vm, "System", "char").clone());
    vm->define_data(VMObjectData(vm, "System", "text").clone());
    vm->define_data(VMObjectData(vm, "System", "nil").clone());
    vm->define_data(VMObjectData(vm, "System", "cons").clone());
    vm->define_data(VMObjectData(vm, "System", "nop").clone());
    vm->define_data(VMObjectData(vm, "System", "true").clone());
    vm->define_data(VMObjectData(vm, "System", "false").clone());
    vm->define_data(VMObjectData(vm, "System", "tuple").clone());

    auto monmin   = MonMin(vm).clone();
    auto opmonmin = VMObjectPrefix(vm, "System", "!-").clone();
    vm->define_data(monmin);
    vm->define_data(opmonmin);
    vm->enter_binding(opmonmin->symbol(), symint, monmin->symbol());
    vm->enter_binding(opmonmin->symbol(), symfloat, monmin->symbol());

    auto add   = Add(vm).clone();
    auto opadd = VMObjectInfix(vm, "System", "+").clone();
    vm->define_data(add);
    vm->define_data(opadd);
    vm->enter_binding(opadd->symbol(), symint, symint, add->symbol());
    vm->enter_binding(opadd->symbol(), symfloat, symfloat, add->symbol());

    auto min    = Min(vm).clone();
    auto opmin  = VMObjectInfix(vm, "System", "-").clone();
    vm->define_data(min);
    vm->define_data(opmin);
    vm->enter_binding(opmin->symbol(), symint, symint, min->symbol());
    vm->enter_binding(opmin->symbol(), symfloat, symfloat, min->symbol());

    auto mul    = Mul(vm).clone();
    auto opmul  = VMObjectInfix(vm, "System", "*").clone();
    vm->define_data(mul);
    vm->define_data(opmul);
    vm->enter_binding(opmul->symbol(), symint, symint, mul->symbol());
    vm->enter_binding(opmul->symbol(), symfloat, symfloat, mul->symbol());

    auto div    = Div(vm).clone();
    auto opdiv  = VMObjectInfix(vm, "System", "/").clone();
    vm->define_data(div);
    vm->define_data(opdiv);
    vm->enter_binding(opdiv->symbol(), symint, symint, div->symbol());
    vm->enter_binding(opdiv->symbol(), symfloat, symfloat, div->symbol());
}

// XXX: this is a bit unfortunate, but I don't want the VM to know about
// the AST, and I don't want the AST to know about the VM.
//
// So, I ended up with this. Double the work.
std::vector<UnicodeString> vm_exports() {
    std::vector<UnicodeString> ss;
    ss.push_back("System.int");
    ss.push_back("System.char");
    ss.push_back("System.float");
    ss.push_back("System.text");
    ss.push_back("System.nil");
    ss.push_back("System.cons");
    ss.push_back("System.nop");
    ss.push_back("System.true");
    ss.push_back("System.false");
    ss.push_back("System.tuple");
    ss.push_back("System.!-");
    ss.push_back("System.+");
    ss.push_back("System.-");
    ss.push_back("System.*");
    ss.push_back("System./");
    return ss;
}
