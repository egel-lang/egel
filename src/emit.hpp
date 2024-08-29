#pragma once

#include <memory>
#include <vector>

#include "ast.hpp"
#include "bytecode.hpp"
#include "environment.hpp"
#include "runtime.hpp"
#include "transform.hpp"

namespace egel {

inline std::vector<VMObjectPtr> emit_data(VM *vm, const ptr<Ast> &a);
inline std::vector<VMObjectPtr> emit_code(VM *vm, const ptr<Ast> &a);

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

    void visit_decl_data(const Position &p, const ptr<Ast> &d, const ptrs<Ast> &nn) override {
        visit(d);
        visits(nn);
    }

    // cut
    void visit_decl_definition(const Position &p, const ptr<Ast> &n, const ptr<Ast> &d,
                               const ptr<Ast> &e) override {
    }

    // cut
    void visit_decl_value(const Position &p, const ptr<Ast> &n, const ptr<Ast> &d,
                          const ptr<Ast> &e) override {
    }

    // cut
    void visit_decl_operator(const Position &p, const ptr<Ast> &c, const ptr<Ast> &d,
                             const ptr<Ast> &e) override {
    }

private:
    VM *_machine;
    std::vector<VMObjectPtr> _out;
};

std::vector<VMObjectPtr> emit_data(VM *m, const ptr<Ast> &a) {
    EmitData emit;
    return emit.emit(m, a);
}

enum emit_state_t {
    EMIT_PATTERN,
    EMIT_EXPR,
    EMIT_EXPR_ROOT,
};

class EmitCode : public Visit {
public:
    std::vector<VMObjectPtr> emit(VM *vm, const ptr<Ast> &a) {
        _machine = vm;
        _coder = std::unique_ptr<Coder>(new Coder(vm));
        visit(a);
        return _out;
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

    void set_pattern_register(reg_t r) {
        _pattern_reg = r;
    }

    reg_t get_pattern_register() const {
        return _pattern_reg;
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

    // handle variables bound in patterns
    void reset_variables() {
        _variables.clear();
    }

    void add_variable_binding(const icu::UnicodeString &v, const reg_t t) {
        _variables[v] = t;
    }

    bool has_variable_binding(const icu::UnicodeString &v) {
        return _variables.count(v) > 0;
    }

    reg_t get_variable_binding(const icu::UnicodeString &v) {
        return _variables[v];
    }

    // handle variables bound in let expressions
    void reset_redexes() {
        _redexes.clear();
    }

    void add_redex_binding(const icu::UnicodeString &v, const reg_t t, int i) {
        _redexes[v] = std::make_tuple(t, i);
    }

    bool has_redex_binding(const icu::UnicodeString &v) {
        return _redexes.count(v) > 0;
    }

    std::tuple<reg_t, int> get_redex_binding(const icu::UnicodeString &v) {
        return _redexes[v];
    }

    void set_cursor(reg_t r, int i) {
        _cursor = std::make_tuple(r, i);
    }

    std::tuple<reg_t, int> get_cursor() {
        return _cursor;
    }

    /*
        The emit scheme revolves primarily about setting up thunks for redexes.

        *  Every combinator definitions is a pattern matching abstraction, so
           the thunk is destructed on inspection.
        *  Every root node might receive spurious arguments from the thunks so
           root nodes need to be handled as a separate case.
        *  Terms are built up from constants, variables, combinators, and
           applications. Every application is a possible redex, where a term
           at the front is in head-redex position.
           1. Constants never build thunks whether in head-redex position or
              not.
           2. Variables are bound to fully reduced terms because of eager
              semantics, therefor thunks are only set up for variables in
              head-redex position.
           3. Combinators can always rewrite so thunks are set up for them
              in all cases.
    */

    void visit_pattern_constant(const VMObjectPtr &o) {
        auto r = get_pattern_register();
        auto l = get_fail_label();
        auto ri = get_coder()->generate_register();

        auto d = get_coder()->emit_data(o);
        get_coder()->emit_op_data(ri, d);
        get_coder()->emit_op_test(r, ri);
        get_coder()->emit_op_fail(l);
    }

    void visit_pattern(const ptr<Ast> &e) {
        switch (e->tag()) {
            case AST_EXPR_INTEGER: {
                auto [p, v] = AstExprInteger::split(e);
                if (v.startsWith("0x")) {
                    auto i = VM::unicode_to_hexint(v);
                    auto o = machine()->create_integer(i);
                    visit_pattern_constant(o);
                } else {
                    auto i = VM::unicode_to_int(v);
                    auto o = machine()->create_integer(i);
                    visit_pattern_constant(o);
                }
            } break;
            case AST_EXPR_FLOAT: {
                auto [p, v] = AstExprFloat::split(e);
                auto f = VM::unicode_to_float(v);
                auto o = machine()->create_float(f);
                visit_pattern_constant(o);
            } break;
            case AST_EXPR_CHARACTER: {
                auto [p, v] = AstExprCharacter::split(e);
                auto c = VM::unicode_to_char(v);
                auto o = machine()->create_char(c);
                visit_pattern_constant(o);
            } break;
            case AST_EXPR_TEXT: {
                auto [p, v] = AstExprText::split(e);
                auto c = VM::unicode_to_text(v);
                auto o = machine()->create_text(c);
                visit_pattern_constant(o);
            } break;
            case AST_EXPR_COMBINATOR: {
                auto [p, nn, n] = AstExprCombinator::split(e);
                auto o = machine()->get_combinator(nn, n);
                visit_pattern_constant(o);
            } break;
            case AST_EXPR_TAG: {
                auto [p, v, t] = AstExprTag::split(e);
                auto r = get_pattern_register();
                auto l = get_fail_label();

                visit_pattern(v);

                if (t->tag() == AST_EXPR_COMBINATOR) {
                    auto [p, nn, n] = AstExprCombinator::split(t);
                    auto o = machine()->get_combinator(nn, n);
                    auto d = get_coder()->emit_data(o);

                    auto rt = get_coder()->generate_register();

                    get_coder()->emit_op_data(rt, d);
                    get_coder()->emit_op_tag(r, rt);
                    get_coder()->emit_op_fail(l);
                } else {
                    PANIC("combinator in tag expected");
                }
            } break;
            case AST_EXPR_VARIABLE: {
                auto [p, v] = AstExprVariable::split(e);
                auto r = get_pattern_register();
                add_variable_binding(v, r);
            } break;
            case AST_EXPR_APPLICATION: {
                auto [p, ee] = AstExprApplication::split(e);
                auto r = get_pattern_register();
                auto l = get_fail_label();

                reg_t x = 0, y = 0;
                for (size_t n = 0; n < ee.size(); n++) {
                    y = get_coder()->generate_register();
                    if (n == 0) x = y;
                }

                get_coder()->emit_op_split(x, y, r);
                get_coder()->emit_op_fail(l);

                reg_t n = x;
                for (auto &e : ee) {
                    set_pattern_register(n);
                    visit_pattern(e);
                    n++;
                }
            } break;
            default: {
                PANIC("pattern expected");
            };
        }
    }

    // unravel a tree (an expression not a redex)
    // sets reg_out to the tree, remembers redex variables
    reg_t visit_tree_constant(const VMObjectPtr &o) {
        auto r = get_coder()->generate_register();
        auto d = get_coder()->emit_data(o);
        get_coder()->emit_op_data(r, d);
        return r;
    }

    reg_t visit_tree(const ptr<Ast> &e) {
        switch (e->tag()) {
            case AST_EXPR_INTEGER: {
                auto [p, v] = AstExprInteger::split(e);
                if (v.startsWith("0x")) {
                    auto i = VM::unicode_to_hexint(v);
                    auto o = machine()->create_integer(i);
                    return visit_tree_constant(o);
                } else {
                    auto i = VM::unicode_to_int(v);
                    auto o = machine()->create_integer(i);
                    return visit_tree_constant(o);
                }
            } break;
            case AST_EXPR_FLOAT: {
                auto [p, v] = AstExprFloat::split(e);
                auto f = VM::unicode_to_float(v);
                auto o = machine()->create_float(f);
                return visit_tree_constant(o);
            } break;
            case AST_EXPR_CHARACTER: {
                auto [p, v] = AstExprCharacter::split(e);
                auto c = VM::unicode_to_char(v);
                auto o = machine()->create_char(c);
                return visit_tree_constant(o);
            } break;
            case AST_EXPR_TEXT: {
                auto [p, v] = AstExprText::split(e);
                auto c = VM::unicode_to_text(v);
                auto o = machine()->create_text(c);
                return visit_tree_constant(o);
            } break;
            case AST_EXPR_COMBINATOR: {
                auto [p, nn, n] = AstExprCombinator::split(e);
                auto o = machine()->get_combinator(nn, n);
                return visit_tree_constant(o);
            } break;
            case AST_EXPR_VARIABLE: {
                auto [p, v] = AstExprVariable::split(e);
                if (has_variable_binding(v)) {
                    auto r = get_variable_binding(v);
                    auto r0 = get_coder()->generate_register();
                    get_coder()->emit_op_mov(r0, r);
                    return r0;
                } else {
                    auto [t, i] = get_cursor();
                    add_redex_binding(v, t, i);
                    auto r = get_coder()->generate_register();
                    get_coder()->emit_op_nil(r);
                    return r;
                }
            } break;
            case AST_EXPR_APPLICATION: {
                auto [p, ee] = AstExprApplication::split(e);

                auto t = get_coder()->generate_register();
                int i = 0;

                std::queue<reg_t> children;
                // first unravel all child nodes
                for (auto &e : ee) {
                    set_cursor(t, i);
                    i++;
                    if (e->tag() == AST_EXPR_APPLICATION) {
                        auto c = visit_tree(e);
                        children.push(c);
                    }
                }

                // then set up the array
                bool f = true;
                reg_t first = 0;
                reg_t last = 0;
                i = 0;
                for (auto &e : ee) {
                    set_cursor(t, i);
                    i++;
                    if (e->tag() == AST_EXPR_APPLICATION) {
                        auto c = children.front();
                        children.pop();
                        auto r = get_coder()->generate_register();
                        if (f) {
                            f = false;
                            first = r;
                        }
                        last = r;
                        get_coder()->emit_op_mov(r, c);
                    } else {
                        auto r = visit_tree(e);
                        if (f) {
                            f = false;
                            first = r;
                        }
                        last = r;
                    }
                }
                get_coder()->emit_op_array(t, first, last);

                return t;
            } break;
            default: {
                PANIC("tree expected");
                return 0;
            };
        }
    }

    reg_t visit_redex_small(const reg_t r) {
        auto rt = get_coder()->generate_register();
        auto rti = get_coder()->generate_register();
        auto k = get_coder()->generate_register();
        auto exc = get_coder()->generate_register();
        auto c = get_coder()->generate_register();

        get_coder()->emit_op_mov(rt, get_register_rt());
        get_coder()->emit_op_mov(rti, get_register_rti());
        get_coder()->emit_op_mov(k, get_register_k());
        get_coder()->emit_op_mov(exc, get_register_exc());
        get_coder()->emit_op_mov(c, r);

        auto t = get_coder()->generate_register();
        get_coder()->emit_op_array(t, rt, c);
        set_register_k(t);
        return t;
    }

    // a redex is either a combinator, a variable, or an application
    // sets reg_k, the continuation
    reg_t visit_redex(const ptr<Ast> &e) {
        switch (e->tag()) {
            case AST_EXPR_COMBINATOR: {
                auto [p, nn, n] = AstExprCombinator::split(e);
                auto o = machine()->get_combinator(nn, n);
                auto d = get_coder()->emit_data(o);
                auto r = get_coder()->generate_register();
                get_coder()->emit_op_data(r, d);
                return visit_redex_small(r);
            } break;
            case AST_EXPR_VARIABLE: {
                auto [p, v] = AstExprVariable::split(e);
                if (has_variable_binding(v)) {
                    auto r = get_variable_binding(v);
                    return visit_redex_small(r);
                } else {
                    PANIC("redex variable in redex");
                    return 0;
                }
            } break;
            case AST_EXPR_APPLICATION: {
                auto [p, ee] = AstExprApplication::split(e);

                std::queue<reg_t> children;

                auto t = get_coder()->generate_register();
                int i = 4;
                // first unravel all child nodes
                for (auto &e : ee) {
                    set_cursor(t, i);
                    i++;
                    if (e->tag() == AST_EXPR_APPLICATION) {
                        auto c = visit_tree(e);
                        children.push(c);
                    }
                }

                // then set up the thunk
                auto rt = get_coder()->generate_register();
                auto rti = get_coder()->generate_register();
                auto k = get_coder()->generate_register();
                auto exc = get_coder()->generate_register();

                get_coder()->emit_op_mov(rt, get_register_rt());
                get_coder()->emit_op_mov(rti, get_register_rti());
                get_coder()->emit_op_mov(k, get_register_k());
                get_coder()->emit_op_mov(exc, get_register_exc());

                i = 4;
                reg_t last = 0;
                for (auto &e : ee) {
                    set_cursor(t, i);
                    i++;
                    if (e->tag() == AST_EXPR_APPLICATION) {
                        auto c = children.front();
                        children.pop();
                        auto r = get_coder()->generate_register();
                        last = r;
                        get_coder()->emit_op_mov(r, c);
                    } else {
                        auto r = visit_tree(e);
                        last = r;
                    }
                }

                get_coder()->emit_op_array(t, rt, last);
                set_register_k(t);
                return t;
            } break;
            default: {
                PANIC("redex expected");
                return 0;  // keep compiler happy
            };
        }
    }

    bool is_redex(const ptr<Ast> o) {
        auto t = o->tag();
        if ((t == AST_EXPR_INTEGER) || (t == AST_EXPR_HEXINTEGER) ||
            (t == AST_EXPR_FLOAT) || (t == AST_EXPR_CHARACTER) ||
            (t == AST_EXPR_TEXT)) {
            return false;
        } else if (t == AST_EXPR_VARIABLE) {
            return true;
        } else if (t == AST_EXPR_COMBINATOR) {
            auto [p, nn, n] = AstExprCombinator::split(o);
            if (machine()->has_combinator(nn, n)) {
                auto c = machine()->get_combinator(nn, n);
                return !(machine()->is_data(c) || machine()->is_opaque(c));
            } else {
                return true;
            }
        } else if (t == AST_EXPR_APPLICATION) {
            auto [p, ee] = AstExprApplication::split(o);
            return is_redex(ee[0]);
        } else {
            return false;
        }
    }

    void visit_root_tree(const ptr<Ast> &e) {
        auto c = visit_tree(e);
        auto f = get_register_frame();
        get_coder()->emit_op_concatx(c, c, f, 5 + get_arity());
        auto rt = get_register_rt();
        auto rti = get_register_rti();
        get_coder()->emit_op_set(rt, rti, c);
    }

    void visit_root_redex(const ptr<Ast> &e) {
        auto c = visit_redex(e);
        auto f = get_register_frame();
        get_coder()->emit_op_concatx(c, c, f, 5 + get_arity());
    }

    // don't know whether the root is a redex or a tree
    void visit_root(const ptr<Ast> &e) {
        switch (e->tag()) {
            case AST_EXPR_INTEGER:
            case AST_EXPR_FLOAT:
            case AST_EXPR_CHARACTER:
            case AST_EXPR_TEXT: {
                visit_root_tree(e);
            } break;
            case AST_EXPR_VARIABLE: {
                visit_root_redex(e);
            } break;
            case AST_EXPR_APPLICATION:
            case AST_EXPR_COMBINATOR: {
                if (is_redex(e)) {
                    visit_root_redex(e);
                } else {
                    visit_root_tree(e);
                }
            } break;
            case AST_EXPR_LET: {
                auto [p, ee0, e1, e2] = AstExprLet::split(e);
                auto e0 = ee0[0];
                visit_root(e2);
                if (e0->tag() == AST_EXPR_VARIABLE) {
                    auto [p, v] = AstExprVariable::split(e0);
                    auto [rt, i] = get_redex_binding(v);
                    auto o = machine()->create_integer(i);
                    auto d = get_coder()->emit_data(o);
                    auto rti = get_coder()->generate_register();
                    get_coder()->emit_op_data(rti, d);
                    set_register_rt(rt);
                    set_register_rti(rti);
                    visit_redex(e1);
                } else {
                    PANIC("variable in let expected");
                }
            } break;
            default: {
                PANIC("root expression expected");
            };
        }
    }

    // this is where the visitor is discarded for recursive descent
    void visit_expr_match(const Position &p, const ptrs<Ast> &mm,
                          const ptr<Ast> &g, const ptr<Ast> &e) override {
        // reset the registers for each match
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

        reg_t n = x;
        for (auto &m : mm) {
            set_pattern_register(n);
            n++;
            visit_pattern(m);
        }

        visit_root(e);

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

    void visit_directive_import(const Position &p,
                                const icu::UnicodeString &i) override {
        // cut
    }

    void visit_decl_data(const Position &p, const ptr<Ast> &d, const ptrs<Ast> &nn) override {
        // cut
    }

    void visit_decl_definition(const Position &p, const ptr<Ast> &n, const ptr<Ast> &d,
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

        auto [p1, doc] = AstDocstring::split(d);
        VMObjectBytecode::cast(b)->set_docstring(VM::unicode_to_text(doc));

        machine()->define_data(b);
        _out.push_back(b);

        get_coder()->reset();
    }

    // treat as a definition
    void visit_decl_value(const Position &p, const ptr<Ast> &o, const ptr<Ast> &d,
                          const ptr<Ast> &e) override {
        visit_decl_definition(p, o, d, e);
    }

    // treat as a definition
    void visit_decl_operator(const Position &p, const ptr<Ast> &o, const ptr<Ast> &d,
                             const ptr<Ast> &e) override {
        auto [p0, ss, s] = AstExprOperator::split(o);
        auto c = AstExprCombinator::create(p0, ss, s);
        visit_decl_definition(p, c, d, e);
    }

private:
    VM *_machine;

    reg_t _register_frame;

    reg_t _register_rt;
    reg_t _register_rti;
    reg_t _register_k;
    reg_t _register_exc;

    int _arity;
    reg_t _pattern_reg;
    label_t _fail;
    std::map<icu::UnicodeString, reg_t> _variables;
    std::map<icu::UnicodeString, std::tuple<reg_t, int>> _redexes;
    std::unique_ptr<Coder> _coder;
    std::tuple<reg_t, int> _cursor;

    std::vector<VMObjectPtr> _out;
};

std::vector<VMObjectPtr> emit_code(VM *m, const ptr<Ast> &a) {
    EmitCode emit;
    return emit.emit(m, a);
}

}  // namespace egel
