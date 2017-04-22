#include "runtime.hpp"
#include "modules.hpp"
#include "eval.hpp"

class VMObjectMainResult : public VMObjectCombinator {
public:
    VMObjectMainResult(VM* m, const symbol_t s)
        : VMObjectCombinator(VM_OBJECT_FLAG_COMBINATOR, m, s) {
    };

    VMObjectMainResult(const VMObjectMainResult& d)
        : VMObjectMainResult(d.machine(), d.symbol()) {
    }

    VMObjectPtr clone() const {
        return VMObjectPtr(new VMObjectMainResult(*this));
    }

    VMObjectPtr reduce(const VMObjectPtr& thunk) const override {
        auto tt  = VM_OBJECT_ARRAY_VALUE(thunk);
        auto arg0   = tt[5];

        std::cout << arg0 << std::endl;

        return nullptr;
    }
};

class VMObjectMainException : public VMObjectCombinator {
public:
    VMObjectMainException(VM* m, const symbol_t s)
        : VMObjectCombinator(VM_OBJECT_FLAG_COMBINATOR, m, s) {
    };

    VMObjectMainException(const VMObjectMainException& d)
        : VMObjectMainException(d.machine(), d.symbol()) {
    }

    VMObjectPtr clone() const {
        return VMObjectPtr(new VMObjectMainException(*this));
    }

    VMObjectPtr reduce(const VMObjectPtr& thunk) const override {
        auto tt    = VM_OBJECT_ARRAY_VALUE(thunk);
        auto arg0  = tt[5];

        std::cout << "exception(" << arg0 << ")" << std::endl;

        return nullptr;
    }
};

void eval_main(const ModuleManager& mm) {
    auto vm = mm.get_machine();
    auto sr = vm->enter_symbol("Internal", "result");
    auto r = VMObjectMainResult(vm, sr).clone();
    vm->enter_data(r);
    vm->result_handler(r);

    auto se = vm->enter_symbol("Internal", "exception");
    auto e = VMObjectMainException(vm, se).clone();
    vm->enter_data(e);
    vm->exception_handler(e);

    auto c = vm->get_data_string("main");

    if (c->flag() != VM_OBJECT_FLAG_STUB) {
        vm->reduce(c);
    }
}

AstPtr parse_line(ModuleManager& mm, const UnicodeString& line) {
    StringCharReader r = StringCharReader("internal", line);
    TokenReaderPtr tt = tokenize_from_reader(r);
    return ::parse_line(tt);
}

void handle_import(ModuleManager& mm, const AstPtr& a) {
    std::cout << "XXX: not implemented yet." << std::endl;
}

void handle_using(ModuleManager& mm, const AstPtr& a) {
    std::cout << "XXX: not implemented yet." << std::endl;
}

void handle_assignment(ModuleManager& mm, const AstPtr& a) {
    std::cout << "XXX: not implemented yet." << std::endl;
    // ::declare(mm.get_environment(), a);
}

void handle_expression(ModuleManager& mm, const AstPtr& a) {
    auto vm = mm.get_machine();
    auto p = a->position();
    auto d = AstExprCombinator(p, "Dummy").clone();
    auto w = AstDeclDefinition(p, d, a).clone();
        std::cout << w << std::endl;
    try {
        w = ::identify(mm.get_environment(), w);
        w = ::desugar(w);
        w = ::lift(w);
        ::emit_code(vm, w);
        auto c = vm->get_data_string("Dummy");
        if (c->flag() != VM_OBJECT_FLAG_STUB) {
            vm->reduce(c);
        }
    } catch (Error e) {
        std::cerr << e << std::endl;
    }
}

void eval_interactive(ModuleManager& mm) {
    auto vm = mm.get_machine();
    auto sr = vm->enter_symbol("Internal", "result");
    auto r = VMObjectMainResult(vm, sr).clone();
    vm->enter_data(r);
    vm->result_handler(r);

    auto se = vm->enter_symbol("Internal", "exception");
    auto e = VMObjectMainException(vm, se).clone();
    vm->enter_data(e);
    vm->exception_handler(e);

    std::string s;
    std::cout << ">> ";
    while (std::getline(std::cin, s)) {
        auto l = UnicodeString::fromUTF8(StringPiece(s.c_str()));
        AstPtr a;
        try {
            a = parse_line(mm, l);
        } catch (Error e) {
            std::cerr << e << std::endl;
            a = nullptr;
        }

        if (a != nullptr) {
            if (a->tag() == AST_DIRECT_IMPORT) {
                handle_import(mm, a);
            } else if (a->tag() == AST_DIRECT_USING) {
                handle_using(mm, a);
            } else if (a->tag() == AST_DECL_DEFINITION) {
                handle_assignment(mm, a);
            } else {
                handle_expression(mm, a);
            }
        }
        std::cout << ">> ";
    }
    std::cout << std::endl;
}

