#include "runtime.hpp"
#include "modules.hpp"
#include "eval.hpp"

// main result and exception handler

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

/*
 * Batch evaluation.
 *
 * Batch evalution is easy, it sets up the handlers and reduces the 'main' combinator.
 */

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

/*
 * Interactive evaluation.
 *
 * Interactive evalutation is somewhat more complex, basically it's 
 * incremental batch evaluation on a line.
 *
 * This is different from most interpreters which handle batch
 * evaluation as a 'piped' form of interactive mode.
 *
 * A line either is:
 * + an import statement
 * + a using statement
 * + a definition
 * + an expression
 *
 * Import statements load modules and therefor modify the state of the
 * module manager.
 *
 * All using statements are stored and prefixed before other 
 * statements. Not the nicest manner but easy and convenient since
 * the module manager only stores the complete environment and 
 * doesn't handle ranges.
 *
 * A definition is parsed and compiled.
 *
 * An expression is put into a definition, compiled, and reduced.
 */
AstPtr parse_line(ModuleManager& mm, const UnicodeString& line) {
    StringCharReader r = StringCharReader("internal", line);
    TokenReaderPtr tt = tokenize_from_reader(r);
    return ::parse_line(tt);
}

void handle_import(ModuleManager& mm, const AstPtr& a) {
    // std::cout << "XXX: not implemented yet." << std::endl;
    if (a->tag() == AST_DIRECT_IMPORT) {
        AST_DIRECT_IMPORT_SPLIT(a, p, s);
        try {
            mm.load(p, unicode_strip_quotes(s)); // XXX: needs an unescape?
        } catch (ErrorIO e) {
            std::cerr << e << std::endl;
        } catch (Error e) {
            std::cerr << e << std::endl;
        }
    }
}

void handle_using(ModuleManager& mm, const AstPtr& a) {
    // nop. maybe push it through semantic analysis later for extra checks.
}

void handle_definition(ModuleManager& mm, const AstPtrs& uu, const AstPtr& d) {
    auto vm = mm.get_machine();
    auto p = d->position();

    auto dd = AstPtrs();
    for (auto& u:uu) {
        dd.push_back(u);
    }
    dd.push_back(d);
    auto w = AstWrapper(p, dd).clone();

    try {
        if (d->tag() == AST_DECL_DEFINITION) { // start off by (re-)declaring the def
            AST_DECL_DEFINITION_SPLIT(d, p0, c0, e0);
            if (c0->tag() == AST_EXPR_COMBINATOR) {
                AST_EXPR_COMBINATOR_SPLIT(c0, p, nn0, n0);
                UnicodeString s;
                for (auto& s0:nn0) {
                    s += s0;
                    s += '.';
                }
                s += n0;
                ::declare_implicit(mm.get_environment(), nn0, n0, s);
            }
        }
        w = ::identify(mm.get_environment(), w);
        w = ::desugar(w);
        w = ::lift(w);
        ::emit_data(vm, w);
        ::emit_code(vm, w);
    } catch (Error e) {
        std::cerr << e << std::endl;
    }
}

void handle_expression(ModuleManager& mm, const AstPtrs& uu, const AstPtr& a) {
    auto vm = mm.get_machine();
    auto p = a->position();
    auto n = AstExprCombinator(p, "Dummy").clone();
    auto d = AstDeclDefinition(p, n, a).clone();

    auto dd = AstPtrs();
    for (auto& u:uu) {
        dd.push_back(u);
    }
    dd.push_back(d);
    auto w = AstWrapper(p, dd).clone();

    try {
        // process
        ::declare_implicit(mm.get_environment(), UnicodeStrings(), "Dummy", "Dummy");
        w = ::identify(mm.get_environment(), w);
        w = ::desugar(w);
        w = ::lift(w);
        ::emit_data(vm, w);
        ::emit_code(vm, w);

        // reduce
        auto c = vm->get_data_string("Dummy");
        if (c->flag() != VM_OBJECT_FLAG_STUB) {
            vm->reduce(c);
        }
    } catch (Error e) {
        std::cerr << e << std::endl;
    }
}

void eval_interactive(ModuleManager& mm) {
    auto uu = AstPtrs();

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
                uu.push_back(a);
                handle_using(mm, a);
            } else if (a->tag() == AST_DECL_DEFINITION) {
                handle_definition(mm, uu, a);
            } else {
                handle_expression(mm, uu, a);
            }
        }
        std::cout << ">> ";
    }
    std::cout << std::endl;
}

