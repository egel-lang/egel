#ifndef EVAL_HPP
#define EVAL_HPP

#include <functional>
#include "utils.hpp"
#include "runtime.hpp"
#include "modules.hpp"

typedef std::function<void(VM* vm, const VMObjectPtr&)> callback_t;

class EvalResult : public VMObjectCombinator {
public:
    EvalResult(VM* m, const symbol_t s, const callback_t call)
        : VMObjectCombinator(VM_OBJECT_FLAG_COMBINATOR, m, s), _callback(call) {
    };

    EvalResult(const EvalResult& d)
        : EvalResult(d.machine(), d.symbol(), d.callback()) {
    }

    VMObjectPtr clone() const override {
        return VMObjectPtr(new EvalResult(*this));
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

class VarCombinator: public VMObjectCombinator {
public:
    VarCombinator(VM* m, const symbol_t s, const VMReduceResult& r):
         VMObjectCombinator(VM_OBJECT_FLAG_INTERNAL, m, s), _result(r)  {
    }

    VarCombinator(const VarCombinator& c):
        VarCombinator(c.machine(), c.symbol(), c.result()) {
    }

    VMObjectPtr clone() const override {
        return VMObjectPtr(new VarCombinator(*this));
    }

    VMReduceResult result() const {
        return _result;
    }

    VMObjectPtr reduce(const VMObjectPtr& thunk) const override {
        auto tt  = VM_OBJECT_ARRAY_VALUE(thunk);
        auto rt  = tt[0];
        auto rti = tt[1];
        auto k   = tt[2];
        auto exc = tt[3];
        // auto c   = tt[4];

        // XXX: doesn't handle exceptions yet
        VMObjectPtrs rr;
        rr.push_back(rt);
        rr.push_back(rti);
        rr.push_back(k);
        rr.push_back(exc);
        rr.push_back(_result.result);
        for (uint i = 5; i < tt.size(); i++) {
            rr.push_back(tt[i]);
        }
        return VMObjectArray(rr).clone();
    }
private:
    VMReduceResult _result;
};

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
            eval_command(icu::UnicodeString("main"));
        }
    }

    void eval_load(const icu::UnicodeString& lib) {
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
            } catch (ErrorIO &e) {
                std::cerr << e << std::endl;
            } catch (Error &e) {
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

        // bypass standard semantical analysis and declare this def in the context.
        // that manner, definitions may be overridded in interactive mode.
        if (d->tag() == AST_DECL_DEFINITION) { // start off by (re-)declaring the def
            AST_DECL_DEFINITION_SPLIT(d, p0, c0, e0);
            if (c0->tag() == AST_EXPR_COMBINATOR) {
                AST_EXPR_COMBINATOR_SPLIT(c0, p, nn0, n0);
                auto c1 = AST_EXPR_COMBINATOR_CAST(c0);
                ::declare_implicit(mm->get_environment(), nn0, n0, c1->to_text());
            }
        }
        if (d->tag() == AST_DECL_OPERATOR) { // or the operator.. (time to get rid of this alternative?)
            AST_DECL_OPERATOR_SPLIT(d, p0, c0, e0);
            if (c0->tag() == AST_EXPR_COMBINATOR) {
                AST_EXPR_COMBINATOR_SPLIT(c0, p, nn0, n0);
                auto c1 = AST_EXPR_COMBINATOR_CAST(c0);
                ::declare_implicit(mm->get_environment(), nn0, n0, c1->to_text());
            }
        }
        w = ::identify(mm->get_environment(), w);
        w = ::desugar(w);
        w = ::lift(w);
        ::emit_data(vm, w);
        ::emit_code(vm, w);
    }

    void handle_data(const AstPtr& d) {
        auto vm = get_machine();
        auto mm = get_manager();
        auto p = d->position();

        auto dd = AstPtrs();
        for (auto& u:get_usings()) {
            dd.push_back(u);
        }
        dd.push_back(d);
        auto w = AstWrapper(p, dd).clone();

        // bypass standard semantical analysis and declare the data in the context.
        // that manner, data may be overridded in interactive mode.
        if (d->tag() == AST_DECL_DATA) { // start off by (re-)declaring the def
            AST_DECL_DATA_SPLIT(d, p0, nn);
            for (auto& e:nn) {
                if (e->tag() == AST_EXPR_COMBINATOR) {
                    AST_EXPR_COMBINATOR_SPLIT(e, p, nn0, n0);
                    auto e1 = AST_EXPR_COMBINATOR_CAST(e);
                    ::declare_implicit(mm->get_environment(), nn0, n0, e1->to_text());
                }
            }
        }
        w = ::identify(mm->get_environment(), w);
        w = ::desugar(w);
        w = ::lift(w);
        ::emit_data(vm, w);
        ::emit_code(vm, w);
    }

    void handle_expression(const AstPtr& a, const VMObjectPtr& r, const VMObjectPtr& exc) {
        auto vm = get_machine();
        auto p = a->position();
        auto n = AstExprCombinator(p, "Dummy").clone();
        auto d = AstDeclDefinition(p, n, a).clone();

        // treat it as a definition Dummy
        handle_definition(d);

        // reduce
        auto c = vm->get_data_string("Dummy");
        if (c->flag() != VM_OBJECT_FLAG_STUB) {
            vm->reduce(c, r, exc);
        }
    }

    void handle_var(const AstPtr& d, const VMObjectPtr& r, const VMObjectPtr& exc) {
        auto vm = get_machine();
        auto mm = get_manager();
        auto p = d->position();

        if (d->tag() == AST_VAR) { // start off by treating the var as a def
            AST_VAR_SPLIT(d, p0, c0, e0);
            handle_definition(AstDeclDefinition(p, c0, e0).clone());
            if (c0->tag() == AST_EXPR_COMBINATOR) {
                auto c1 = AST_EXPR_COMBINATOR_CAST(c0);
                auto c   = vm->get_data_string(c1->to_text());
                auto sym = c->symbol();
                if (c->flag() != VM_OBJECT_FLAG_STUB) {
                    auto r = vm->reduce(c);
                    auto v = VarCombinator(vm, sym, r).clone();
                    vm->define_data(v);
                }
            }
        }
    }

    void eval_line(const icu::UnicodeString& in, const callback_t& main, const callback_t& exc) {
        auto mm = get_manager();
        auto vm = get_machine();

        // parse the line
        AstPtr a;
        StringCharReader r = StringCharReader("internal", in);
        TokenReaderPtr tt = tokenize_from_reader(r);
        a = ::parse_line(tt);

        // set up the handlers
        auto sr = vm->enter_symbol("Internal", "result");
        auto rr = EvalResult(vm, sr, main).clone();
        auto se = vm->enter_symbol("Internal", "exception");
        auto e = EvalResult(vm, se, exc).clone();

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
            } else if (a->tag() == AST_DECL_DATA) {
                handle_data(a);
            } else if (a->tag() == AST_VAR) {
                handle_var(a, rr, e);
            } else {
                handle_expression(a, rr, e);
            }
        }
    }

    void eval_command(const icu::UnicodeString& in) {
        eval_line(in, &default_main_callback, &default_exception_callback);
    }

    void eval_interactive() {
        auto uu = AstPtrs();

        std::string s;
        std::cout << ">> ";
        while (std::getline(std::cin, s)) {
            auto in = icu::UnicodeString::fromUTF8(icu::StringPiece(s.c_str()));
            try {
                eval_command(in);
            } catch (Error &e) {
                std::cout << e << std::endl;
            }
            std::cout << ">> ";
        }
        std::cout << std::endl;
    }

private:
    ModuleManagerPtr    _manager;
    AstPtrs             _usings;
};

#endif
