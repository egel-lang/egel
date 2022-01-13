export module eval;

import<functional>;

import utils;
import runtime;
import modules;

class EvalResult : public VMObjectCombinator {
public:
    EvalResult(VM *m, const symbol_t s, const callback_t call)
        : VMObjectCombinator(VM_SUB_BUILTIN, m, s), _callback(call){};

    EvalResult(const EvalResult &d)
        : EvalResult(d.machine(), d.symbol(), d.callback()) {
    }

    static VMObjectPtr create(VM *m, const symbol_t s, const callback_t call) {
        return std::make_shared<EvalResult>(m, s, call);
    }

    callback_t callback() const {
        return _callback;
    }

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override {
        auto tt = machine()->get_array(thunk);
        auto arg0 = tt[5];

        (_callback)(machine(), arg0);
        return nullptr;
    }

private:
    callback_t _callback;
};

inline void default_main_callback(VM *vm, const VMObjectPtr &o) {
    if (!(VM_OBJECT_NONE_TEST(o))) {
        std::cout << o << std::endl;
    }
}

inline void default_exception_callback(VM *vm, const VMObjectPtr &e) {
    std::cout << "exception(" << e << ")" << std::endl;
}

// XXX: get rid of this once. simply shouldn't be necessary
class VarCombinator : public VMObjectCombinator {
public:
    VarCombinator(VM *m, const symbol_t s, const VMReduceResult &r)
        : VMObjectCombinator(VM_SUB_BUILTIN, m, s), _result(r) {
    }

    VarCombinator(const VarCombinator &c)
        : VarCombinator(c.machine(), c.symbol(), c.result()) {
    }

    static VMObjectPtr create(VM *m, const symbol_t s,
                              const VMReduceResult &r) {
        return std::make_shared<VarCombinator>(m, s, r);
    }

    static VMObjectPtr create(VM *vm, const VMObjectPtr &o,
                              const VMReduceResult &r) {
        if (o->tag() == VM_OBJECT_COMBINATOR) {
            auto c = VM_OBJECT_COMBINATOR_CAST(o);
            auto v = VarCombinator::create(vm, c->symbol(), r);
            return v;
        } else {
            throw ErrorInternal("failure to create Var");
        }
    }

    VMReduceResult result() const {
        return _result;
    }

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override {
        auto tt = machine()->get_array(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];
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
        return machine()->create_array(rr);
    }

private:
    VMReduceResult _result;
};

class Eval;
using EvalPtr = std::shared_ptr<Eval>;

class Eval {
public:
    Eval() {
    }

    Eval(ModuleManagerPtr mm) : _manager(mm), _usings(AstPtrs()) {
    }

    Eval(const Eval &e) : Eval(e.get_manager()) {
    }

    ~Eval() {
    }

    static EvalPtr create() {
        return std::make_shared<Eval>();
    }

    void init(ModuleManagerPtr mm) {
        _manager = mm;
        _usings = AstPtrs();
    }

    ModuleManagerPtr get_manager() const {
        return _manager;
    }

    VM *machine() {
        return _manager->machine();
    }

    AstPtrs get_usings() {
        return _usings;
    }

    /*
     * Batch evaluation.
     *
     * Batch evalution is easy, it sets up the handlers and reduces the 'main'
     * combinator.
     */
    void eval_main() {
        auto vm = machine();
        auto c = vm->get_combinator("main");

        if (c->subtag() != VM_SUB_STUB) {
            eval_command(icu::UnicodeString("main"));
        }
    }

    void eval_load(const icu::UnicodeString &lib) {
        auto mm = get_manager();
        Position p("", 0, 0);
        mm->load(p, lib);
    }

    // XXX: handle exceptions properly once
    void eval_values() {
        auto vm = machine();
        auto mm = get_manager();
        auto vv = mm->values();
        for (auto &v : vv) {
            // std::cerr << "evaluating: " << v.string() << std::endl;
            auto c = vm->get_combinator(v.string());
            auto sym = c->symbol();
            auto r = vm->reduce(c);
            auto d = VarCombinator::create(vm, sym, r);
            vm->define_data(d);
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
     * + a veriable
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
    void handle_import(const AstPtr &a) {
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

    void handle_using(const AstPtr &a) {
        _usings.push_back(a);
    }

    void handle_definition(const AstPtr &d) {
        auto vm = machine();
        auto mm = get_manager();
        auto p = d->position();

        auto dd = AstPtrs();
        for (auto &u : get_usings()) {
            dd.push_back(u);
        }
        dd.push_back(d);
        auto w = AstWrapper::create(p, dd);

        // bypass standard semantical analysis and declare this def in the
        // context. that manner, definitions may be overridded in interactive
        // mode.
        if (d->tag() ==
            AST_DECL_DEFINITION) {  // start off by (re-)declaring the def
            AST_DECL_DEFINITION_SPLIT(d, p0, c0, e0);
            if (c0->tag() == AST_EXPR_COMBINATOR) {
                AST_EXPR_COMBINATOR_SPLIT(c0, p, nn0, n0);
                auto c1 = AST_EXPR_COMBINATOR_CAST(c0);
                ::declare_implicit(mm->get_environment(), nn0, n0,
                                   c1->to_text());
            }
        }
        if (d->tag() ==
            AST_DECL_OPERATOR) {  // or the operator.. (time to get rid
            // of this alternative?)
            AST_DECL_OPERATOR_SPLIT(d, p0, c0, e0);
            if (c0->tag() == AST_EXPR_COMBINATOR) {
                AST_EXPR_COMBINATOR_SPLIT(c0, p, nn0, n0);
                auto c1 = AST_EXPR_COMBINATOR_CAST(c0);
                ::declare_implicit(mm->get_environment(), nn0, n0,
                                   c1->to_text());
            }
        }
        w = ::identify(mm->get_environment(), w);
        w = ::desugar(w);
        w = ::lift(w);
        ::emit_data(vm, w);
        ::emit_code(vm, w);
    }

    void handle_data(const AstPtr &d) {
        auto vm = machine();
        auto mm = get_manager();
        auto p = d->position();

        auto dd = AstPtrs();
        for (auto &u : get_usings()) {
            dd.push_back(u);
        }
        dd.push_back(d);
        auto w = AstWrapper::create(p, dd);

        // bypass standard semantical analysis and declare the data in the
        // context. that manner, data may be overridded in interactive mode.
        if (d->tag() == AST_DECL_DATA) {  // start off by (re-)declaring the def
            AST_DECL_DATA_SPLIT(d, p0, nn);
            for (auto &e : nn) {
                if (e->tag() == AST_EXPR_COMBINATOR) {
                    AST_EXPR_COMBINATOR_SPLIT(e, p, nn0, n0);
                    auto e1 = AST_EXPR_COMBINATOR_CAST(e);
                    ::declare_implicit(mm->get_environment(), nn0, n0,
                                       e1->to_text());
                }
            }
        }
        w = ::identify(mm->get_environment(), w);
        w = ::desugar(w);
        w = ::lift(w);
        ::emit_data(vm, w);
        ::emit_code(vm, w);
    }

    // XXX XXX XXX: get rid of all of this once. See handle_expression.
    std::mutex _lock;
    uint_t _counter = 0;
    UnicodeString generate_fresh_combinator() {
        int n = _counter;
        _lock.lock();
        _counter++;
        _lock.unlock();
        return UnicodeString("Dummy") + unicode_convert_uint(n);
    }

    /**
     * XXX XXX XXX: Severily hacked since the introduction of the `eval`
     * command. Since `eval` commands can run concurrently but can introduce
     * combinators at the moment I just let it leak. Probably the worst decision
     * I made yet.
     */
    void handle_expression(const AstPtr &a, const VMObjectPtr &r,
                           const VMObjectPtr &exc) {
        auto fv = generate_fresh_combinator();
        auto vm = machine();
        auto p = a->position();
        auto n = AstExprCombinator::create(p, fv);
        auto d = AstDeclDefinition::create(p, n, a);

        // treat it as a definition Dummy
        handle_definition(d);

        // reduce
        auto c = vm->get_combinator(fv);
        if (c->subtag() != VM_SUB_STUB) {
            vm->reduce(c, r, exc);
        }
    }

    // XXX: very much wrong probably. A val declaration should not call the
    // callbacks handlers but either throw a parse error or return none.
    void handle_value(const AstPtr &d) {
        auto vm = machine();
        auto mm = get_manager();
        auto p = d->position();

        if (d->tag() ==
            AST_DECL_VALUE) {  // start off by treating the val as a def
            AST_DECL_VALUE_SPLIT(d, p0, c0, e0);
            handle_definition(AstDeclDefinition::create(p, c0, e0));
            if (c0->tag() == AST_EXPR_COMBINATOR) {
                auto c1 = AST_EXPR_COMBINATOR_CAST(c0);
                auto o = vm->get_combinator(c1->to_text());
                if (o->subtag() != VM_SUB_STUB) {
                    // XXX: handle exceptions properly once
                    auto r = vm->reduce(o);
                    auto v = VarCombinator::create(vm, o, r);
                    vm->define_data(v);
                }
            }
        }
    }

    void return_none(const callback_t &callback) {
        auto vm = machine();
        auto none = vm->create_none();
        (callback)(vm, none);
    }

    /** 'eval_line' is the work horse of the REPL, the IRC bot, and the
     *'System:eval' command.
     *
     * Three things can happen:
     * - the 'in' command reduces to a value, 'main' is called, and 'eval_line'
     *returns.
     * - the 'in' command reduces to an exception, 'exc' is called, and
     *'eval_line' returns.
     * - 'eval_line' throws an Error (due to, for example, improper syntax or
     *I/O failure)
     *
     **/
    void eval_line(const icu::UnicodeString &in, const callback_t &main,
                   const callback_t &exc) {
        auto mm = get_manager();
        auto vm = machine();

        // parse the line
        AstPtr aa;
        StringCharReader r = StringCharReader("internal", in);
        TokenReaderPtr tt = tokenize_from_reader(r);
        aa = ::parse_line(tt);

        // set up the handlers
        auto sr = vm->enter_symbol("Internal", "result");
        auto rr = EvalResult::create(vm, sr, main);
        auto se = vm->enter_symbol("Internal", "exception");
        auto e = EvalResult::create(vm, se, exc);

        // handle the commands
        if (aa->tag() == AST_WRAPPER) {
            AST_WRAPPER_SPLIT(aa, p, aa0);
            for (auto &a : aa0) {
                if (a != nullptr) {
                    if (a->tag() == AST_DIRECT_IMPORT) {
                        handle_import(a);
                        return_none(main);
                    } else if (a->tag() == AST_DIRECT_USING) {
                        handle_using(a);
                        return_none(main);
                    } else if (a->tag() == AST_DECL_DEFINITION) {
                        handle_definition(a);
                        return_none(main);
                    } else if (a->tag() == AST_DECL_OPERATOR) {
                        handle_definition(a);
                        return_none(main);
                    } else if (a->tag() == AST_DECL_DATA) {
                        handle_data(a);
                        return_none(main);
                    } else if (a->tag() == AST_DECL_VALUE) {
                        handle_value(a);
                        return_none(main);
                    } else {
                        handle_expression(a, rr, e);
                    }
                } else {
                    PANIC("no command found");
                }
            }
        } else {
            PANIC("no wrapper found");
        }
    }

    void eval_command(const icu::UnicodeString &in) {
        eval_line(in, &default_main_callback, &default_exception_callback);
    }

    void eval_interactive() {
        auto uu = AstPtrs();

        const char *env_p = std::getenv("EGEL_PS0");
        icu::UnicodeString ps0;
        if (env_p) {
            ps0 = icu::UnicodeString(env_p);
        } else {
            ps0 = ">> ";
        }

        std::string s;
        std::cout << ps0;
        while (std::getline(std::cin, s)) {
            auto in = icu::UnicodeString::fromUTF8(icu::StringPiece(s.c_str()));
            try {
                eval_command(in);
            } catch (Error &e) {
                std::cout << e << std::endl;
            }
            std::cout << ps0;
        }
        std::cout << std::endl;
    }

private:
    ModuleManagerPtr _manager;
    AstPtrs _usings;
};

#endif
