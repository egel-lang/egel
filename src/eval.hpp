#ifndef EVAL_HPP
#define EVAL_HPP

#include "utils.hpp"
#include "runtime.hpp"
#include "modules.hpp"

typedef void (*callback_t)(VM* vm, const VMObjectPtr&);

class VMObjectResult : public VMObjectCombinator {
public:
    VMObjectResult(VM* m, const symbol_t s, const callback_t call)
        : VMObjectCombinator(VM_OBJECT_FLAG_COMBINATOR, m, s), _callback(call) {
    };

    VMObjectResult(const VMObjectResult& d)
        : VMObjectResult(d.machine(), d.symbol(), d.callback()) {
    }

    VMObjectPtr clone() const {
        return VMObjectPtr(new VMObjectResult(*this));
    }

    callback_t callback() const {
        return _callback;
    }

    VMObjectPtr reduce(const VMObjectPtr& thunk) const override {
        auto tt  = VM_OBJECT_ARRAY_VALUE(thunk);
        auto arg0   = tt[5];

        (_callback)(machine(), arg0);
        return nullptr;
    }

private:
    callback_t  _callback;
};

inline void default_main_callback(VM* vm, const VMObjectPtr& o) {
        symbol_t nop = vm->enter_symbol("System", "nop");
        if (o->symbol() != nop) {
            std::cout << o << std::endl;
        }
}


inline void default_exception_callback(VM* vm, const VMObjectPtr& e) {
        std::cout << "exception(" << e << ")" << std::endl;
}

class Eval {
public:

    Eval(ModuleManagerPtr mm): _manager(mm), _usings(AstPtrs()) {
    }

    ModuleManagerPtr get_manager() {
        return _manager;
    }

    VM* get_machine() {
        return _manager->get_machine();
    }

    AstPtrs get_usings() {
        return _usings;
    }

    /*
     * Batch evaluation.
     *
     * Batch evalution is easy, it sets up the handlers and reduces the 'main' combinator.
     */
    void eval_main() {
        auto vm = get_machine();
        auto c = vm->get_data_string("main");

        if (c->flag() != VM_OBJECT_FLAG_STUB) {
            eval_command(UnicodeString("main"));
        }
    }

    void eval_load(const UnicodeString& lib) {
        auto mm = get_manager();
        Position p("", 0, 0);
        mm->load(p, lib);
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
    void handle_import(const AstPtr& a) {
        auto mm = get_manager();
        if (a->tag() == AST_DIRECT_IMPORT) {
            AST_DIRECT_IMPORT_SPLIT(a, p, s);
            try {
                mm->load(p, unicode_strip_quotes(s));
            } catch (ErrorIO e) {
                std::cerr << e << std::endl;
            } catch (Error e) {
                std::cerr << e << std::endl;
            }
        }
    }

    void handle_using(const AstPtr& a) {
        _usings.push_back(a);
    }

    void handle_definition(const AstPtr& d) {
        auto vm = get_machine();
        auto mm = get_manager();
        auto p = d->position();

        auto dd = AstPtrs();
        for (auto& u:get_usings()) {
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
                    ::declare_implicit(mm->get_environment(), nn0, n0, s);
                }
            }
            if (d->tag() == AST_DECL_OPERATOR) { // or the operator.. (time to get rid of this alternative?)
                AST_DECL_OPERATOR_SPLIT(d, p0, c0, e0);
                if (c0->tag() == AST_EXPR_COMBINATOR) {
                    AST_EXPR_COMBINATOR_SPLIT(c0, p, nn0, n0);
                    UnicodeString s;
                    for (auto& s0:nn0) {
                        s += s0;
                        s += '.';
                    }
                    s += n0;
                    ::declare_implicit(mm->get_environment(), nn0, n0, s);
                }
            }
            w = ::identify(mm->get_environment(), w);
            w = ::desugar(w);
            w = ::lift(w);
            ::emit_data(vm, w);
            ::emit_code(vm, w);
        } catch (Error e) {
            std::cerr << e << std::endl;
        }
    }

    void handle_expression(const AstPtr& a) {
        auto vm = get_machine();
        auto mm = get_manager();
        auto p = a->position();
        auto n = AstExprCombinator(p, "Dummy").clone();
        auto d = AstDeclDefinition(p, n, a).clone();

        auto dd = AstPtrs();
        for (auto& u:get_usings()) {
            dd.push_back(u);
        }
        dd.push_back(d);
        auto w = AstWrapper(p, dd).clone();

        try {
            // process
            ::declare_implicit(mm->get_environment(), UnicodeStrings(), "Dummy", "Dummy");
            w = ::identify(mm->get_environment(), w);
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

    void eval_line(const UnicodeString& in, const callback_t& main, const callback_t& exc) {
        auto mm = get_manager();
        auto vm = get_machine();

        // parse the line
        AstPtr a;
        try {
            StringCharReader r = StringCharReader("internal", in);
            TokenReaderPtr tt = tokenize_from_reader(r);
            a = ::parse_line(tt);
        } catch (Error e) {
            std::cerr << e << std::endl;
            a = nullptr;
        }

        // set up the handlers
        auto sr = vm->enter_symbol("Internal", "result");
        auto r = VMObjectResult(vm, sr, main).clone();
        vm->enter_data(r);
        vm->result_handler(r);

        auto se = vm->enter_symbol("Internal", "exception");
        auto e = VMObjectResult(vm, se, exc).clone();
        vm->enter_data(e);
        vm->exception_handler(e);

        // handle the command
        if (a != nullptr) {
            if (a->tag() == AST_DIRECT_IMPORT) {
                handle_import(a);
            } else if (a->tag() == AST_DIRECT_USING) {
                handle_using(a);
            } else if (a->tag() == AST_DECL_DEFINITION) {
                handle_definition(a);
            } else if (a->tag() == AST_DECL_OPERATOR) {
                handle_definition(a);
            } else {
                handle_expression(a);
            }
        }
    }

    void eval_command(const UnicodeString& in) {
        eval_line(in, &default_main_callback, &default_exception_callback);
    }

    void eval_interactive() {
        auto uu = AstPtrs();

        std::string s;
        std::cout << ">> ";
        while (std::getline(std::cin, s)) {
            auto in = UnicodeString::fromUTF8(StringPiece(s.c_str()));
            eval_command(in);
            std::cout << ">> ";
        }
        std::cout << std::endl;
    }

private:
    ModuleManagerPtr    _manager;
    AstPtrs             _usings;
};

#endif
