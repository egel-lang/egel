#pragma once

#include <memory>
#include <vector>

#include "ast.hpp"
#include "bytecode.hpp"
#include "environment.hpp"
#include "transform.hpp"

namespace egel {

inline std::vector<VMObjectPtr> emit_data(VM *vm, const ptr<Ast> &a);
inline std::vector<VMObjectPtr> emit_code(VM *vm, const ptr<Ast> &a);

using RegisterMap = std::map<icu::UnicodeString, reg_t>;
using CoderPtr = std::unique_ptr<Coder>;

class EmitData : public Visit {
public:
    std::vector<VMObjectPtr> emit(VM *m, const ptr<Ast> &a) {
        _machine = m;
        visit(a);
        return _out;
    }

    void visit_directive_import(const Position &p,
                                const icu::UnicodeString &i) override {
    }

    void visit_expr_combinator(const Position &p, const UnicodeStrings &nn,
                               const icu::UnicodeString &n) override {
        auto c = VMObjectData::create(_machine, nn, n);
        _machine->define_data(c);
        _out.push_back(c);
    }

    void visit_decl_data(const Position &p, const ptrs<Ast> &nn) override {
        visits(nn);
    }

    // cut
    void visit_decl_definition(const Position &p, const ptr<Ast> &n,
                               const ptr<Ast> &e) override {
    }

    // cut
    void visit_decl_value(const Position &p, const ptr<Ast> &n,
                          const ptr<Ast> &e) override {
    }

    // cut
    void visit_decl_operator(const Position &p, const ptr<Ast> &c,
                             const ptr<Ast> &e) override {
    }

private:
    VM *_machine;
    std::vector<VMObjectPtr>    _out;
};

std::vector<VMObjectPtr> emit_data(VM *m, const ptr<Ast> &a) {
    EmitData emit;
    return emit.emit(m, a);
}

enum emit_state_t {
    EMIT_PATTERN,
    EMIT_EXPR,
    EMIT_EXPR_ROOT,
    //    EMIT_EXPR_CONSTANT, // XXX constant optimization not implemented yet
};

class EmitCode : public Visit {
public:
    std::vector<VMObjectPtr> emit(VM *vm, const ptr<Ast> &a) {
        _machine = vm;
        _coder = std::unique_ptr<Coder>(new Coder(vm));
        visit(a);
        return _out;
    }

    void set_state(const emit_state_t s) {
        _state = s;
    }

    emit_state_t get_state() const {
        return _state;
    }

    VM *machine() const {
        return _machine;
    }

    Coder *get_coder() {
        return _coder.get();
    }

    void set_register_frame(reg_t r) {
        _register_frame = r;
    }

    reg_t get_register_frame() const {
        return _register_frame;
    }

    void set_register_rt(reg_t r) {
        _register_rt = r;
    }

    reg_t get_register_rt() const {
        return _register_rt;
    }

    void set_register_rti(reg_t r) {
        _register_rti = r;
    }

    reg_t get_register_rti() const {
        return _register_rti;
    }

    void set_register_k(reg_t r) {
        _register_k = r;
    }

    reg_t get_register_k() const {
        return _register_k;
    }

    void set_register_exc(reg_t r) {
        _register_exc = r;
    }

    reg_t get_register_exc() const {
        return _register_exc;
    }

    void set_current_register(reg_t r) {
        _current_reg = r;
    }

    reg_t get_current_register() const {
        return _current_reg;
    }

    void set_fail_label(label_t l) {
        _fail = l;
    }

    label_t get_fail_label() const {
        return _fail;
    }

    void set_arity(int a) {
        _arity = a;
    }

    int get_arity() {
        return _arity;
    }

    void add_variable_binding(const icu::UnicodeString &v, const reg_t t) {
        _variables[v] = t;
    }

    reg_t get_variable_binding(const icu::UnicodeString &v) {
        return _variables[v];
    }

    void visit_constant(const VMObjectPtr &o) {
        switch (get_state()) {
            case EMIT_PATTERN: {
                auto r = get_current_register();
                auto l = get_fail_label();
                auto ri = get_coder()->generate_register();

                auto d = get_coder()->emit_data(o);
                get_coder()->emit_op_data(ri, d);
                get_coder()->emit_op_test(r, ri);
                get_coder()->emit_op_fail(l);
            } break;
            case EMIT_EXPR_ROOT: {
                auto rt = get_coder()->generate_register();
                auto rti = get_coder()->generate_register();
                auto k = get_coder()->generate_register();
                auto exc = get_coder()->generate_register();
                auto c = get_coder()->generate_register();

                auto t = get_coder()->generate_register();

                get_coder()->emit_op_mov(rt, get_register_rt());
                get_coder()->emit_op_mov(rti, get_register_rti());
                get_coder()->emit_op_mov(k, get_register_k());
                get_coder()->emit_op_mov(exc, get_register_exc());

                auto d = get_coder()->emit_data(o);
                get_coder()->emit_op_data(c, d);

                get_coder()->emit_op_array(t, rt, c);

                if (get_state() == EMIT_EXPR_ROOT) {
                    set_state(EMIT_EXPR);
                    auto x = get_coder()->generate_register();
                    auto f = get_register_frame();

                    get_coder()->emit_op_concatx(x, t, f, 5 + get_arity());
                    set_register_k(x);
                } else {
                    set_register_k(t);
                }
            } break;
            case EMIT_EXPR: {
                auto c = get_coder()->generate_register();
                auto d = get_coder()->emit_data(o);
                get_coder()->emit_op_data(c, d);

                auto rt = get_register_rt();
                auto rti = get_register_rti();
                get_coder()->emit_op_set(rt, rti, c);
            } break;
        }
    }

    void visit_combinator(const VMObjectPtr &o) {
        switch (get_state()) {
            case EMIT_PATTERN: {
                auto r = get_current_register();
                auto l = get_fail_label();
                auto ri = get_coder()->generate_register();

                auto d = get_coder()->emit_data(o);
                get_coder()->emit_op_data(ri, d);
                get_coder()->emit_op_test(r, ri);
                get_coder()->emit_op_fail(l);
            } break;
            case EMIT_EXPR:
            case EMIT_EXPR_ROOT: {
                auto rt = get_coder()->generate_register();
                auto rti = get_coder()->generate_register();
                auto k = get_coder()->generate_register();
                auto exc = get_coder()->generate_register();
                auto c = get_coder()->generate_register();

                auto t = get_coder()->generate_register();

                get_coder()->emit_op_mov(rt, get_register_rt());
                get_coder()->emit_op_mov(rti, get_register_rti());
                get_coder()->emit_op_mov(k, get_register_k());
                get_coder()->emit_op_mov(exc, get_register_exc());

                auto d = get_coder()->emit_data(o);
                get_coder()->emit_op_data(c, d);

                get_coder()->emit_op_array(t, rt, c);

                if (get_state() == EMIT_EXPR_ROOT) {
                    set_state(EMIT_EXPR);
                    auto x = get_coder()->generate_register();
                    auto f = get_register_frame();

                    get_coder()->emit_op_concatx(x, t, f, 5 + get_arity());
                    set_register_k(x);
                } else {
                    set_register_k(t);
                }
            } break;
        }
    }

    void visit_expr_integer(const Position &p,
                            const icu::UnicodeString &v) override {
        if (v.startsWith("0x")) {
            auto i = VM::unicode_to_hexint(v);
            auto o = machine()->create_integer(i);
            visit_constant(o);
        } else {
            auto i = VM::unicode_to_int(v);
            auto o = machine()->create_integer(i);
            visit_constant(o);
        }
    }

    void visit_expr_float(const Position &p,
                          const icu::UnicodeString &v) override {
        auto f = VM::unicode_to_float(v);
        auto o = machine()->create_float(f);
        visit_constant(o);
    }

    void visit_expr_character(const Position &p,
                              const icu::UnicodeString &v) override {
        auto c = VM::unicode_to_char(v);
        auto o = machine()->create_char(c);
        visit_constant(o);
    }

    void visit_expr_text(const Position &p,
                         const icu::UnicodeString &v) override {
        auto t = VM::unicode_to_text(v);
        auto o = machine()->create_text(t);
        visit_constant(o);
    }

    void visit_expr_combinator(const Position &p, const UnicodeStrings &nn,
                               const icu::UnicodeString &n) override {
        auto c = machine()->get_combinator(nn, n);
        visit_combinator(c);
    }

    void visit_expr_operator(const Position &p, const UnicodeStrings &nn,
                             const icu::UnicodeString &n) override {
        visit_expr_combinator(p, nn, n);
    }

    void visit_expr_variable(const Position &p,
                             const icu::UnicodeString &n) override {
        switch (get_state()) {
            case EMIT_PATTERN: {
                auto r = get_current_register();
                add_variable_binding(n, r);
            } break;
            case EMIT_EXPR_ROOT: {
                set_state(EMIT_EXPR);
                auto r = get_variable_binding(n);

                auto rt = get_coder()->generate_register();
                auto rti = get_coder()->generate_register();
                auto k = get_coder()->generate_register();
                auto exc = get_coder()->generate_register();
                auto c = get_coder()->generate_register();

                auto t = get_coder()->generate_register();

                get_coder()->emit_op_mov(rt, get_register_rt());
                get_coder()->emit_op_mov(rti, get_register_rti());
                get_coder()->emit_op_mov(k, get_register_k());
                get_coder()->emit_op_mov(exc, get_register_exc());
                get_coder()->emit_op_mov(c, r);
                get_coder()->emit_op_array(t, rt, c);

                auto x = get_coder()->generate_register();
                auto f = get_register_frame();

                get_coder()->emit_op_concatx(x, t, f, 5 + get_arity());
                set_register_k(x);
            } break;
            case EMIT_EXPR: {
                auto r = get_variable_binding(n);

                auto rt = get_register_rt();
                auto rti = get_register_rti();
                get_coder()->emit_op_set(rt, rti, r);
            } break;
        }
    }

    void visit_expr_application(const Position &p,
                                const ptrs<Ast> &aa) override {
        switch (get_state()) {
            case EMIT_PATTERN: {
                auto r = get_current_register();
                auto l = get_fail_label();

                reg_t x = 0, y = 0;
                for (size_t n = 0; n < aa.size(); n++) {
                    y = get_coder()->generate_register();
                    if (n == 0) x = y;
                }

                get_coder()->emit_op_split(x, y, r);
                get_coder()->emit_op_fail(l);

                reg_t n = x;
                for (auto &a : aa) {
                    set_current_register(n);
                    visit(a);
                    n++;
                }
            } break;

            case EMIT_EXPR_ROOT:
            case EMIT_EXPR: {  // XXX

                // generate labels rt, rti, k, exc, c, x .. y
                auto rt = get_coder()->generate_register();
                auto rti = get_coder()->generate_register();
                auto k = get_coder()->generate_register();
                auto exc = get_coder()->generate_register();
                auto c = get_coder()->generate_register();

                reg_t x = 0, y = 0;
                int sz = aa.size();
                for (int n = 1; n < sz; n++) {
                    y = get_coder()->generate_register();
                    if (n == 1) x = y;
                }

                // generate thunk label
                auto t = get_coder()->generate_register();

                // fill rt, rti, k, exc, c, x .. y
                get_coder()->emit_op_mov(rt, get_register_rt());
                get_coder()->emit_op_mov(rti, get_register_rti());
                get_coder()->emit_op_mov(k, get_register_k());
                get_coder()->emit_op_mov(exc, get_register_exc());

                auto a = aa[0];
                bool head_flag;  // generate more efficient code for vars and
                                 // combinators
                if (a->tag() == AST_EXPR_VARIABLE) {
                    auto [p, n] = AstExprVariable::split(a);
                    auto r = get_variable_binding(n);
                    get_coder()->emit_op_mov(c, r);
                    head_flag = true;
                } else if (a->tag() == AST_EXPR_COMBINATOR) {
                    auto [p, nn, n] = AstExprCombinator::split(a);
                    auto v = machine()->get_combinator(nn, n);
                    auto d = get_coder()->emit_data(v);
                    get_coder()->emit_op_data(c, d);
                    head_flag = true;
                } else {
                    get_coder()->emit_op_nil(c);
                    head_flag = false;
                }

                reg_t z = x;
                for (int n = 1; n < sz; n++) {
                    get_coder()->emit_op_nil(z);
                    z++;
                }
                get_coder()->emit_op_array(t, rt, y);

                // adjust for root
                auto root = get_coder()->generate_register();
                auto state = get_state();
                if (state == EMIT_EXPR_ROOT) {
                    set_state(EMIT_EXPR);
                    auto f = get_register_frame();

                    get_coder()->emit_op_concatx(root, t, f, 5 + get_arity());
                } else {
                    root = t;  // XXX: no mov?
                }
                k = root;
                set_register_k(k);
                rt = root;
                set_register_rt(rt);

                // generate thunks for nil fields
                if (!head_flag) {
                    auto i = machine()->create_integer(4);
                    auto d = get_coder()->emit_data(i);
                    get_coder()->emit_op_data(rti, d);

                    set_register_rt(rt);
                    set_register_rti(rti);

                    visit(aa[0]);
                }

                for (int n = 1; n < sz; n++) {
                    auto i = machine()->create_integer(n + 4);
                    auto d = get_coder()->emit_data(i);
                    reg_t q = get_coder()->generate_register();
                    get_coder()->emit_op_data(q, d);

                    set_register_rt(rt);
                    set_register_rti(q);

                    visit(aa[n]);
                }

                break;
            }
        }
    }

    void visit_expr_tag(const Position &p, const ptr<Ast> &v,
                        const ptr<Ast> &t) override {
        switch (get_state()) {
            case EMIT_PATTERN: {
                auto r = get_current_register();
                auto l = get_fail_label();

                if (v->tag() == AST_EXPR_VARIABLE) {
                    visit(v);  // register the binding
                } else {
                    PANIC("variable expected");  // XXX: turn into asserts
                }

                if (t->tag() == AST_EXPR_COMBINATOR) {
                    auto [p, nn, n] = AstExprCombinator::split(t);
                    auto o = machine()->get_combinator(nn, n);
                    auto d = get_coder()->emit_data(o);

                    auto rt = get_coder()->generate_register();

                    get_coder()->emit_op_data(rt, d);
                    get_coder()->emit_op_tag(r, rt);
                    get_coder()->emit_op_fail(l);
                } else {
                    PANIC("combinator expected");
                }
            } break;
            case EMIT_EXPR_ROOT:
            case EMIT_EXPR: {
                PANIC("tag in expression");
            } break;
        }
    }

    void visit_expr_match(const Position &p, const ptrs<Ast> &mm,
                          const ptr<Ast> &g, const ptr<Ast> &e) override {
        // we have memberberries
        auto member = get_coder()->peek_register();
        auto r = get_register_frame();

        auto l = get_coder()->generate_label();
        set_fail_label(l);

        int arity = mm.size();
        set_arity(arity);
        reg_t x = 0, y = 0;
        for (int n = 0; n < arity; n++) {
            y = get_coder()->generate_register();
            if (n == 0) x = y;
        }

        if (arity > 0) {
            get_coder()->emit_op_takex(x, y, r, 5);
            get_coder()->emit_op_fail(l);
        }

        set_state(EMIT_PATTERN);
        reg_t n = x;
        for (auto &m : mm) {
            set_current_register(n);
            n++;
            visit(m);
        }

        set_state(EMIT_EXPR_ROOT);
        visit(e);

        // all matches end with a return
        auto k = get_register_k();
        get_coder()->emit_op_return(k);

        // generate a label at the end of the match
        get_coder()->emit_label(l);
        get_coder()->restore_register(member);
    }

    void visit_expr_block(const Position &p, const ptrs<Ast> &alts) override {
        // keep link registers invariant
        auto rt = get_register_rt();
        auto rti = get_register_rti();
        auto k = get_register_k();
        auto exc = get_register_exc();

        for (auto &a : alts) {
            set_register_rt(rt);
            set_register_rti(rti);
            set_register_k(k);
            set_register_exc(exc);
            visit(a);
        }
    }

    void visit_expr_try(const Position &p, const ptr<Ast> &t,
                        const ptr<Ast> &c) override {
        auto rt = get_register_rt();
        auto rti = get_register_rti();
        auto k = get_register_k();
        auto exc = get_register_exc();

        // set up the exception thunk
        auto e_rt = get_coder()->generate_register();
        auto e_rti = get_coder()->generate_register();
        auto e_k = get_coder()->generate_register();
        auto e_exc = get_coder()->generate_register();
        auto e_arg0 = get_coder()->generate_register();
        auto e_arg1 = get_coder()->generate_register();

        get_coder()->emit_op_mov(e_rt, rt);
        get_coder()->emit_op_mov(e_rti, rti);
        get_coder()->emit_op_mov(e_k, k);
        get_coder()->emit_op_mov(e_exc, exc);
        get_coder()->emit_op_nil(e_arg0);
        get_coder()->emit_op_nil(e_arg1);

        auto new_exc = get_coder()->generate_register();
        get_coder()->emit_op_array(new_exc, e_rt, e_arg1);

        // the try thunk evaluates with this exception thunk as its handler
        set_register_exc(new_exc);
        visit(t);

        // the catch thunk evaluates with the old exception and places its
        // result in the handler thunk as the combinator
        auto new_exci = get_coder()->generate_register();
        auto i = machine()->create_integer(4);
        auto d = get_coder()->emit_data(i);
        get_coder()->emit_op_data(new_exci, d);

        set_register_exc(exc);
        set_register_rt(new_exc);
        set_register_rti(new_exci);
        visit(c);

        // XXX: I am unsure after half a year whether to maintain an invariant
        // here
        set_register_rt(rt);
        set_register_rti(rti);
    }

    void visit_expr_throw(const Position &p, const ptr<Ast> &e) override {
        PANIC("throw is combinator");
    }

    void visit_directive_import(const Position &p,
                                const icu::UnicodeString &i) override {
    }

    // XXX: another means to translate data combinators (see emit_data)
    void visit_decl_data(const Position &p, const ptrs<Ast> &nn) override {
        for (auto n : nn) {
            switch (n->tag()) {
                case AST_EXPR_COMBINATOR: {
                    auto [p, ss, s] = AstExprCombinator::split(n);
                    auto d = VMObjectData::create(machine(), ss, s);
                    machine()->define_data(d);
                    _out.push_back(d);
                } break;
                default:
                    PANIC("combinator expected");
            }
        }
    }

    void visit_decl_definition(const Position &p, const ptr<Ast> &n,
                               const ptr<Ast> &e) override {
        auto frame = get_coder()->generate_register();

        auto l = get_coder()->generate_label();
        set_fail_label(l);

        auto rt = get_coder()->generate_register();
        auto rti = get_coder()->generate_register();
        auto k = get_coder()->generate_register();
        auto exc = get_coder()->generate_register();
        auto c = get_coder()->generate_register();

        set_register_frame(frame);
        set_register_rt(rt);
        set_register_rti(rti);
        set_register_k(k);
        set_register_exc(exc);
        set_arity(0);

        get_coder()->emit_op_takex(rt, c, frame, 0);
        get_coder()->emit_op_fail(l);
        set_state(EMIT_EXPR_ROOT);
        visit(e);
        get_coder()->emit_label(l);

        auto em = get_coder()->generate_register();
        auto r = get_coder()->generate_register();

        get_coder()->emit_op_array(em, rti, rt);  // gen an empty array
        get_coder()->emit_op_concatx(r, em, frame, 4);
        get_coder()->emit_op_set(rt, rti, r);
        get_coder()->emit_op_return(k);

        get_coder()->relabel();
        auto code = get_coder()->code();
        auto data = get_coder()->data();

        auto [p0, ss, s] = AstExprCombinator::split(n);
        auto b = VMObjectBytecode::create(machine(), code, data, ss, s);
        machine()->define_data(b);
        _out.push_back(b);

        get_coder()->reset();
    }

    // treat as a definition
    void visit_decl_value(const Position &p, const ptr<Ast> &o,
                          const ptr<Ast> &e) override {
        visit_decl_definition(p, o, e);
    }

    void visit_decl_operator(const Position &p, const ptr<Ast> &o,
                             const ptr<Ast> &e) override {
        auto frame = get_coder()->generate_register();

        auto l = get_coder()->generate_label();
        set_fail_label(l);

        auto rt = get_coder()->generate_register();
        auto rti = get_coder()->generate_register();
        auto k = get_coder()->generate_register();
        auto exc = get_coder()->generate_register();
        auto c = get_coder()->generate_register();

        set_register_frame(frame);
        set_register_rt(rt);
        set_register_rti(rti);
        set_register_k(k);
        set_register_exc(exc);
        set_arity(0);

        get_coder()->emit_op_takex(rt, c, frame, 0);
        get_coder()->emit_op_fail(l);
        set_state(EMIT_EXPR_ROOT);
        visit(e);
        get_coder()->emit_label(l);

        auto em = get_coder()->generate_register();
        auto r = get_coder()->generate_register();

        get_coder()->emit_op_array(em, rti, rt);  // gen an empty array
        get_coder()->emit_op_concatx(r, em, frame, 4);
        get_coder()->emit_op_set(rt, rti, r);
        get_coder()->emit_op_return(k);

        get_coder()->relabel();
        auto code = get_coder()->code();
        auto data = get_coder()->data();

        auto [p0, ss, s] = AstExprOperator::split(o);
        auto b = VMObjectBytecode::create(machine(), code, data, ss, s);
        machine()->define_data(b);
        _out.push_back(b);

        get_coder()->reset();
    }

private:
    emit_state_t _state;
    VM *_machine;

    reg_t _register_frame;

    reg_t _register_rt;
    reg_t _register_rti;
    reg_t _register_k;
    reg_t _register_exc;

    int _arity;
    reg_t _current_reg;
    label_t _fail;
    CoderPtr _coder;
    RegisterMap _variables;

    std::vector<VMObjectPtr> _out;
};

std::vector<VMObjectPtr> emit_code(VM *m, const ptr<Ast> &a) {
    EmitCode emit;
    return emit.emit(m, a);
}

}  // namespace egel
