#pragma once

#include <stdlib.h>

#include "bytecode.hpp"
#include "runtime.hpp"
#include "serialize.hpp"

/**
 * Egel's runtime querying and modification implementation.
 **/

namespace egel {

class Dis : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Dis, "System", "dis");
    DOCSTRING("System::dis o - disassemble a combinator object");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_bytecode(arg0)) {
            auto s = machine()->disassemble(arg0);
            return machine()->create_text(s);
        } else if (machine()->is_data(arg0)) {
            auto s = machine()->disassemble(arg0);
            return machine()->create_text(s);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Asm : public Unary {
public:
    UNARY_PREAMBLE(VM_SUB_BUILTIN, Asm, "System", "asm");
    DOCSTRING("System::asm s - assemble bytecode into a combinator");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->assemble(s);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Serialize : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Serialize, "System", "serialize");
    DOCSTRING("System::serialize t - serialize a term to a text");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        auto m = machine();
        auto s = m->serialize(arg0);
        return m->create_text(s);
    }
};

class Deserialize : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Deserialize, "System", "deserialize");
    DOCSTRING("System::deserialize t - serialize a text to a term");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        auto m = machine();
        if (m->is_text(arg0)) {
            auto s = m->get_text(arg0);
            auto o = m->deserialize(s);
            return o;
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Docstring : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Docstring, "System", "docstring");
    DOCSTRING("System::docstring o - docstring of a module or combinator");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        return machine()->docstring(arg0);
    }
};

class Modules : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Modules, "System", "query_modules");
    DOCSTRING("System::query_modules - list all modules in the runtime");

    VMObjectPtr apply() const override {
        return machine()->query_modules();
    }
};

class IsModule : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsModule, "System", "query_is_module");
    DOCSTRING("System::is_module m - check we have a module");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        return machine()->create_bool(machine()->is_module(arg0));
    }
};

class QueryModuleName : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, QueryModuleName, "System",
                     "query_module_name");
    DOCSTRING("System::query_module_name m - get the name of the module");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_module(arg0)) {
            return machine()->query_module_name(arg0);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class QueryModulePath : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, QueryModulePath, "System",
                     "query_module_path");
    DOCSTRING("System::query_module_path m - get the path of the module");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_module(arg0)) {
            return machine()->query_module_path(arg0);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class QueryModuleImports : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, QueryModuleImports, "System",
                     "query_module_imports");
    DOCSTRING("System::query_module_imports m - get the imports of the module");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_module(arg0)) {
            return machine()->query_module_imports(arg0);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class QueryModuleExports : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, QueryModuleExports, "System",
                     "query_module_exports");
    DOCSTRING("System::query_module_exports m - get the exports of the module");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_module(arg0)) {
            return machine()->query_module_exports(arg0);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class QueryModuleValues : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, QueryModuleValues, "System",
                     "query_module_values");
    DOCSTRING("System::query_module_values m - get the values of the module");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_module(arg0)) {
            return machine()->query_module_values(arg0);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class IsInteger : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsInteger, "System", "is_integer");
    DOCSTRING("System::is_integer o - test for integer");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        return machine()->create_bool(machine()->is_integer(arg0));
    }
};

class IsFloat : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsFloat, "System", "is_float");
    DOCSTRING("System::is_float o - test for float");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        return machine()->create_bool(machine()->is_float(arg0));
    }
};

class IsCharacter : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsCharacter, "System", "is_character");
    DOCSTRING("System::is_character o - test for character");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        return machine()->create_bool(machine()->is_char(arg0));
    }
};

class IsText : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsText, "System", "is_text");
    DOCSTRING("System::is_text o - test for text");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        return machine()->create_bool(machine()->is_text(arg0));
    }
};

class IsCombinator : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsCombinator, "System", "is_combinator");
    DOCSTRING("System::is_combinator o - test for combinator");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        return machine()->create_bool(machine()->is_combinator(arg0));
    }
};

class IsOpaque : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsOpaque, "System", "is_opaque");
    DOCSTRING("System::is_opaque o - test for opaque value");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        return machine()->create_bool(machine()->is_opaque(arg0));
    }
};

class IsArray : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsArray, "System", "is_array");
    DOCSTRING("System::is_array o - test for array value");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        return machine()->create_bool(machine()->is_array(arg0));
    }
};

class IsBytecode : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsBytecode, "System", "is_bytecode");
    DOCSTRING("System::is_bytecode o - test for bytecode value");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        return machine()->create_bool(machine()->is_bytecode(arg0));
    }
};

class GetArray : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, GetArray, "System", "get_array");
    DOCSTRING("System::get_array o - array to list");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_array(arg0)) {
            auto oo = machine()->get_array(arg0);
            return machine()->to_list(oo);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class GetBytecode : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, GetBytecode, "System", "get_bytecode");
    DOCSTRING("System::get_bytecode o - get_bytecode");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_bytecode(arg0)) {
            auto t = machine()->disassemble(arg0);  // XXX XXX: cut this?
            return machine()->create_text(t);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class GetBytedata : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, GetBytedata, "System", "get_bytedata");
    DOCSTRING("System::get_bytedata o - get_bytedata");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_bytecode(arg0)) {
            auto oo = machine()->get_bytedata(arg0);
            return machine()->to_list(oo);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class DebugPtr : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, DebugPtr, "System", "debug_ptr");

    DOCSTRING("System::debug_ptr o - raw pointer to object");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        std::stringstream ss;
        ss << arg0.get();
        icu::UnicodeString u(ss.str().c_str());
        return machine()->create_text(u);
    }
};

class Dependencies : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Dependencies, "System", "dependencies");
    DOCSTRING("System::dependencies o - dependencies of term");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        auto oo = machine()->dependencies(arg0);
        return machine()->to_list(oo);
    }
};

class Tokenize : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Tokenize, "System", "tokenize");
    DOCSTRING("System::tokenize uri s - use builtin tokenize");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if (machine()->is_text(arg0) && machine()->is_text(arg1)) {
            auto s0 = machine()->get_text(arg0);
            auto s1 = machine()->get_text(arg1);
            return machine()->tokenize(s0, s1);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

/*
//## System::symbols - list all symbols in the runtime
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
            aa.push_back(VMObjectText(s));
            aa.push_back(ss);
            aa = VMObjectArray(tt);
       };
       return aa;
    }
};

//## System::set_data s - define text as data
class SetData: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, SetData, "System", "set_data");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            auto c = VMObjectData::create(machine(), s);
            machine()->define_data(c);
            return c;
        } else {
        throw machine()->bad_args(this, arg0);
        }
    }
};

//## System::set_def s e - define text as expression
class SetDef: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, SetDef, "System", "set_def");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const
override { if (machine()->is_text(arg0)) { auto s = machine()->get_text(arg0);
            auto o = machine()->get_data_string(s);

            throw VMObjectText::create("stub");
        } else {
        throw machine()->bad_args(this, arg0, arg1);
        }
    }
};
*/

class RuntimeModule : public CModule {
public:
    virtual ~RuntimeModule() {
    }

    icu::UnicodeString name() const override {
        return "runtime";
    }

    icu::UnicodeString docstring() const override {
        return "The 'runtime' module provides binding to the runtime.";
    }

    std::vector<VMObjectPtr> exports(VM *vm) override {
        std::vector<VMObjectPtr> oo;

        oo.push_back(Dis::create(vm));
        oo.push_back(Asm::create(vm));

        oo.push_back(Serialize::create(vm));
        oo.push_back(Deserialize::create(vm));

        oo.push_back(Tokenize::create(vm));

        oo.push_back(Docstring::create(vm));

        oo.push_back(Modules::create(vm));
        oo.push_back(IsModule::create(vm));
        oo.push_back(QueryModuleName::create(vm));
        oo.push_back(QueryModulePath::create(vm));
        oo.push_back(QueryModuleImports::create(vm));
        oo.push_back(QueryModuleExports::create(vm));
        oo.push_back(QueryModuleValues::create(vm));

        oo.push_back(IsInteger::create(vm));
        oo.push_back(IsFloat::create(vm));
        oo.push_back(IsCharacter::create(vm));
        oo.push_back(IsText::create(vm));
        oo.push_back(IsCombinator::create(vm));
        oo.push_back(IsOpaque::create(vm));
        oo.push_back(IsArray::create(vm));
        oo.push_back(IsBytecode::create(vm));

        oo.push_back(GetArray::create(vm));
        oo.push_back(GetBytecode::create(vm));
        oo.push_back(GetBytedata::create(vm));
        oo.push_back(Dependencies::create(vm));

        oo.push_back(DebugPtr::create(vm));

        return oo;
    }
};

}  // namespace egel
