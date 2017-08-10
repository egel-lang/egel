#include "runtime.hpp"
#include "builtin.hpp"
#include <stdlib.h>
#include <ostream>

class MonMin: public Monadic {
public:
    MONADIC_PREAMBLE(MonMin, "System", "!-");

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
    DYADIC_PREAMBLE(Add, "System", "+");

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
        } else if ( (arg0->tag() == VM_OBJECT_TEXT) &&
             (arg1->tag() == VM_OBJECT_TEXT) ) {
            auto f0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto f1 = VM_OBJECT_TEXT_VALUE(arg1);
            return VMObjectText(f0+f1).clone();
        } else {
            return nullptr;
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
    DYADIC_PREAMBLE(Mul, "System", "*");

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
    DYADIC_PREAMBLE(Div, "System", "/");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_INTEGER) &&
             (arg1->tag() == VM_OBJECT_INTEGER) ) {
            auto i0 = VM_OBJECT_INTEGER_VALUE(arg0);
            auto i1 = VM_OBJECT_INTEGER_VALUE(arg1);
            if (i1 == 0) {
                throw machine()->get_data_string("System", "divzero");
            }
            return VMObjectInteger(i0/i1).clone();
        } else if ( (arg0->tag() == VM_OBJECT_FLOAT) &&
             (arg1->tag() == VM_OBJECT_FLOAT) ) {
            auto f0 = VM_OBJECT_FLOAT_VALUE(arg0);
            auto f1 = VM_OBJECT_FLOAT_VALUE(arg1);
            if (f1 == 0.0) {
                throw 
                    machine()->get_data_string("System", "divzero");
            }
            return VMObjectFloat(f0/f1).clone();
        } else {
            return nullptr;
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
                throw machine()->get_data_string("System", "divzero");
            }
            return VMObjectInteger(i0%i1).clone();
        } else {
            return nullptr;
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
    DYADIC_PREAMBLE(NegEq, "System", "!=");

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

// System.get O F
// Retrieve an object field
class GetField: public Binary {
public:
    BINARY_PREAMBLE(GetField, "System", "get_field");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        static symbol_t object = 0;
        if (object == 0) object = machine()->enter_symbol("System", "object");

        if (arg0->tag() == VM_OBJECT_ARRAY) {
            auto ff = VM_OBJECT_ARRAY_VALUE(arg0);
            auto sz = ff.size();
            // check head is an object
            if (sz == 0) return nullptr;
            if (ff[0]->symbol() != object) return nullptr;
            // search for field
            CompareVMObjectPtr compare;
            unsigned int n;
            for (n = 1; n < sz; n=n+2) {
                if (compare(arg1, ff[n]) == 0) break;
            }
            // return field
            if ( (n+1) < sz ) {
                return ff[n+1];
            } else {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }
};

// System.set_field O F X
// set an object field
class SetField: public Triadic {
public:
    TRIADIC_PREAMBLE(SetField, "System", "set_field");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        static symbol_t object = 0;
        if (object == 0) object = machine()->enter_symbol("System", "object");

        if (arg0->tag() == VM_OBJECT_ARRAY) {
            auto ff = VM_OBJECT_ARRAY_VALUE(arg0);
            auto sz = ff.size();
            // check head is an object
            if (sz == 0) return nullptr;
            if (ff[0]->symbol() != object) return nullptr;
            // search field
            CompareVMObjectPtr compare;
            unsigned int n;
            for (n = 1; n < sz; n=n+2) {
                if (compare(arg1, ff[n]) == 0) break;
            }
            // set field
            if ( (n+1) < sz ) {
                auto arr = VM_OBJECT_ARRAY_CAST(arg0); // XXX: clean up this cast once. need destructive update
                arr->set(n+1, arg2);
                return arg0;
            } else {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }
};


std::vector<VMObjectPtr> vm_export(VM* vm) {
    std::vector<VMObjectPtr> oo;
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

    oo.push_back(VMObjectData(vm, "System", "divzero").clone());

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

    oo.push_back(GetField(vm).clone());
    oo.push_back(SetField(vm).clone());

    return oo;
}
