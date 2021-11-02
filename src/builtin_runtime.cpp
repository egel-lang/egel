#include "runtime.hpp"

#include "utils.hpp"
#include "bytecode.hpp"

#include "builtin_runtime.hpp"

#include <stdlib.h>

/**
 * Egel's runtime querying and modification implementation.
 **/

//## System:dis o - disassemble a combinator object
class Dis: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Dis, "System", "dis");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
	    auto o = machine()->get_symbol(s);
	    Disassembler d(o);
	    return VMObjectText(d.disassemble()).clone();
        } else {
            THROW_BADARGS;
        }
    }
};

//## System:asm s - assemble bytecode into a combinator
class Asm: public Unary {
public:
    UNARY_PREAMBLE(VM_SUB_BUILTIN, Asm, "System", "asm");

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
            THROW_BADARGS;
        }
    }
};

/*
//## System:symbols - list all symbols in the runtime
class Symbols: public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Symbols, "System", "symbols");

    VMObjectPtr apply() const override {

        auto nil = machine()->create_nil();
        auto cons = machine()->create_cons();

        VMObjectPtr ss = nil;
        int len = (int) machine()->query_symbols_size();
        for (int n = len-1; n >= 0; n--) {
            auto s = machine()->query_symbols_nth(n);
            auto aa = machine()->create_array();
            auto t = machine()->create_text(s);
            machine()->push_array(cons);
            aa.push_back(VMObjectText(s).clone());
            aa.push_back(ss);
            aa = VMObjectArray(tt).clone();
       };
       return aa;
    }
};

//## System:get_type s - get the type of symbol s
class GetType: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, GetType, "System", "get_type");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            auto o = machine()->get_data_string(s);

            return VMObjectText("stub").clone();
        } else {
            THROW_BADARGS;
        }
    }
};

//## System:set_data s - define symbol s as data
class SetData: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, SetData, "System", "set_data");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
	    auto c = VMObjectData(machine(), s).clone();
            machine()->define_data(c);
            return c;
        } else {
            THROW_BADARGS;
        }
    }
};

//## System:set_def s e - define symbol s as expression e
class SetDef: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, SetDef, "System", "set_def");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            auto o = machine()->get_data_string(s);

            return VMObjectText("stub").clone();
        } else {
            THROW_BADARGS;
        }
    }
};
*/

std::vector<VMObjectPtr> builtin_runtime(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Dis(vm).clone());
//    oo.push_back(Asm(vm).clone()); // XXX: not working at the moment
//    oo.push_back(Symbols(vm).clone());
//    oo.push_back(GetType(vm).clone());
//    oo.push_back(SetData(vm).clone());
//    oo.push_back(SetDef(vm).clone());

    return oo;
}
