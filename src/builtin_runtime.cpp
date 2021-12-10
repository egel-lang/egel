#include "runtime.hpp"

#include "utils.hpp"
#include "bytecode.hpp"
#include "serialize.hpp"

#include "builtin_runtime.hpp"

#include <stdlib.h>

/**
 * Egel's runtime querying and modification implementation.
 **/

//## System::dis o - disassemble a combinator object
class Dis: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Dis, "System", "dis");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_bytecode(arg0)) {
            auto s = machine()->get_bytecode(arg0);
            return machine()->create_text(s);
        } else {
            THROW_BADARGS;
        }
    }
};

//## System::asm s - assemble bytecode into a combinator
class Asm: public Unary {
public:
    UNARY_PREAMBLE(VM_SUB_BUILTIN, Asm, "System", "asm");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_bytecode(s);
        } else {
            THROW_BADARGS;
        }
    }
};

//## System::serialize t - serialize a term to a text
class Serialize: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Serialize, "System", "serialize");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        auto s = serialize_to_string(m, arg0);
        return m->create_text(s);
    }
};

//## System::deserialize t - serialize a text to a term
class Deserialize: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Deserialize, "System", "deserialize");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (m->is_text(arg0)) {
            auto s = m->get_text(arg0);
            auto o = deserialize_from_string(m, s);
            return o;
        } else {
            THROW_BADARGS;
        }
    }
};

//## System::modules - list all modules in the runtime
class Modules: public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Modules, "System", "modules");

    VMObjectPtr apply() const override {
        return machine()->query_modules();
    }
};

//## System::is_module m - check we have a module
class IsModule: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsModule, "System", "is_module");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        return machine()->create_bool(machine()->is_module(arg0));
    }
};

//## System::query_module_name m - get the name of the module
class QueryModuleName: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, QueryModuleName, "System", "query_module_name");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_module(arg0)) {
            return machine()->query_module_name(arg0);
        } else {
            THROW_BADARGS;
        }
    }
};

//## System::query_module_path m - get the path of the module
class QueryModulePath: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, QueryModulePath, "System", "query_module_path");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_module(arg0)) {
            return machine()->query_module_path(arg0);
        } else {
            THROW_BADARGS;
        }
    }
};

//## System::query_module_imports m - get the path of the module
class QueryModuleImports: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, QueryModuleImports, "System", "query_module_imports");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_module(arg0)) {
            return machine()->query_module_imports(arg0);
        } else {
            THROW_BADARGS;
        }
    }
};

//## System::query_module_exports m - get the path of the module
class QueryModuleExports: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, QueryModuleExports, "System", "query_module_exports");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_module(arg0)) {
            return machine()->query_module_exports(arg0);
        } else {
            THROW_BADARGS;
        }
    }
};

//## System::query_module_values m - get the path of the module
class QueryModuleValues: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, QueryModuleValues, "System", "query_module_values");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_module(arg0)) {
            return machine()->query_module_values(arg0);
        } else {
            THROW_BADARGS;
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

//## System::get_type s - get the type of symbol s
class GetType: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, GetType, "System", "get_type");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            auto o = machine()->get_data_string(s);

            return VMObjectText::create("stub");
        } else {
            THROW_BADARGS;
        }
    }
};

//## System::set_data s - define symbol s as data
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
            THROW_BADARGS;
        }
    }
};

//## System::set_def s e - define symbol s as expression e
class SetDef: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, SetDef, "System", "set_def");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            auto o = machine()->get_data_string(s);

            return VMObjectText::create("stub");
        } else {
            THROW_BADARGS;
        }
    }
};
*/

std::vector<VMObjectPtr> builtin_runtime(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Dis::create(vm));
    oo.push_back(Asm::create(vm));

    oo.push_back(Serialize::create(vm));
    oo.push_back(Deserialize::create(vm));

    oo.push_back(Modules::create(vm));
    oo.push_back(IsModule::create(vm));
    oo.push_back(QueryModuleName::create(vm));
    oo.push_back(QueryModulePath::create(vm));
    oo.push_back(QueryModuleImports::create(vm));
    oo.push_back(QueryModuleExports::create(vm));
    oo.push_back(QueryModuleValues::create(vm));

    return oo;
}
