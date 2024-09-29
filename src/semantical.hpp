#pragma once

#include "ast.hpp"
#include "environment.hpp"
#include "error.hpp"
#include "position.hpp"
#include "runtime.hpp"
#include "semantical.hpp"
#include "transform.hpp"

namespace egel {

class VisitDeclare : public Visit {
public:
    VisitDeclare() {
    }

    void declare(ScopePtr env, const ptr<Ast> &a) {
        _spaces = env;
        visit(a);
    }

    // state manipulation
    void clear_path() {
        _path.clear();
    }

    UnicodeStrings get_path() {
        return _path;
    }

    void set_path(UnicodeStrings qq) {
        _path = qq;
    }

    // visits
    void visit_expr_combinator(const Position &p, const UnicodeStrings &nn,
                               const icu::UnicodeString &n) override {
        try {
            auto nn0 = VM::concat(_path, nn);
            auto q = VM::qualified(nn0, n);
            egel::declare_global(_spaces, q);
        } catch (ErrorSemantical &e) {
            throw ErrorSemantical(p, "redeclaration of " + n);
        }
    }

    void visit_decl_data(const Position &p, const ptr<Ast> &d,
                         const ptrs<Ast> &nn) override {
        visits(nn);
    }

    void visit_decl_definition(const Position &p, const ptr<Ast> &n,
                               const ptr<Ast> &d, const ptr<Ast> &e) override {
        visit(n);
    }

    void visit_decl_value(const Position &p, const ptr<Ast> &l,
                          const ptr<Ast> &d, const ptr<Ast> &r) override {
        visit(l);
    }

    void visit_decl_operator(const Position &p, const ptr<Ast> &c,
                             const ptr<Ast> &d, const ptr<Ast> &e) override {
        visit(c);
    }

    void visit_decl_namespace(const Position &p, const UnicodeStrings &nn,
                              const ptrs<Ast> &dd) override {
        auto nn0 = get_path();
        auto nn1 = VM::concat(nn0, nn);
        set_path(nn1);
        visits(dd);
        set_path(nn0);
    }

    void visit_directive_using(const Position &p, const ptrs<Ast> &nn) override {
    }

private:
    ScopePtr _spaces;
    UnicodeStrings _path;
};

inline void declare(ScopePtr env, const ptr<Ast> &a) {
    VisitDeclare declare;
    declare.declare(env, a);
}

enum identify_state_t {
    STATE_IDENTIFY_USE,
    STATE_IDENTIFY_DECLARE,
};

/*
 * Stuff this pass does:
 * + Check that all symbols have bindings.
 * + Fully qualify declarations.
 * + Flatten the namespace.
 */
class RewriteIdentify : public Rewrite {
public:
    ptr<Ast> identify(ScopePtr env, ptr<Ast> a) {
        Position p = a->position();
        _identify_state = STATE_IDENTIFY_USE;
        _scope = egel::enter_scope(env);
        _counter = 0;
        rewrite(a);
        auto aa = pop_declarations();
        return AstWrapper::create(p, aa);
    }

    // variable
    icu::UnicodeString fresh_variable() {
        auto v = icu::UnicodeString("V_") + VM::unicode_from_int(_counter);
        _counter++;
        return v;
    }

    // state
    void set_identify_state(identify_state_t s) {
        _identify_state = s;
    }

    identify_state_t get_identify_state() {
        return _identify_state;
    }

    // range manipulation
    void declare(const Position &p, const icu::UnicodeString &k,
                 const icu::UnicodeString &v) {
        try {
            egel::declare_local(_scope, k, v);
        } catch (ErrorSemantical &e) {
            throw ErrorSemantical(p, "redeclaration of " + k);
        }
    }

    bool has(const icu::UnicodeString &k) {
        icu::UnicodeString tmp = egel::get_local(_scope, k);
        return (tmp != "");
    }

    icu::UnicodeString get(const Position &p, const icu::UnicodeString &k) {
        icu::UnicodeString tmp = egel::get_local(_scope, k);
        if (tmp == "") {
            //_scope->render(std::cout, 0);
            throw ErrorSemantical(p, "undeclared " + k);
        } else {
            return tmp;
        }
    }

    icu::UnicodeString get(const Position &p, const UnicodeStrings &kk,
                           const icu::UnicodeString &k) {
        return get(p, VM::qualified(kk,k));
    }

    void enter_scope() {
        _scope = egel::enter_scope(_scope);
    }

    void leave_scope() {
        _scope = egel::leave_scope(_scope);
    }

    void enter_namespace(const icu::UnicodeString &s) {
        _scope = egel::enter_namespace(_scope, s);
    }

    void leave_namespace() {
        _scope = egel::leave_namespace(_scope);
    }

    // namespaces
    UnicodeString get_namespace() {
        return _scope->get_namespace();
    }

    void push_declaration(const ptr<Ast> &d) {
        _declarations.push_back(d);
    }

    ptrs<Ast> pop_declarations() {
        return _declarations;
    }

    // rewrites
    ptr<Ast> rewrite_expr_variable(const Position &p,
                                   const icu::UnicodeString &v) override {
        switch (get_identify_state()) {
            case STATE_IDENTIFY_USE: {
                auto v1 = get(p, v);
                return AstExprVariable::create(p, v1);
            } break;
            case STATE_IDENTIFY_DECLARE: {
                auto fv = fresh_variable();
                declare(p, v, fv);
                return AstExprVariable::create(p, fv);
            } break;
            default:
                PANIC("unknown state");
                return nullptr;
                break;
        }
    }

    ptr<Ast> rewrite_expr_combinator(const Position &p,
                                     const UnicodeStrings &nn,
                                     const icu::UnicodeString &t) override {
        UnicodeStrings ee;
        switch (get_identify_state()) {
            case STATE_IDENTIFY_DECLARE:
            case STATE_IDENTIFY_USE: {
                auto v = get(p, nn, t);
                auto c = AstExprCombinator::create(p, ee, v);
                return c;
            } break;
            default:
                PANIC("unknown state");
                return nullptr;
                break;
        }
        return nullptr;  // play nice with -Wall
    }

    ptr<Ast> rewrite_expr_operator(const Position &p, const UnicodeStrings &nn,
                                   const icu::UnicodeString &t) override {
        UnicodeStrings ee;
        switch (get_identify_state()) {
            case STATE_IDENTIFY_DECLARE:
            case STATE_IDENTIFY_USE: {
                auto v = get(p, nn, t);
                auto c = AstExprOperator::create(p, ee, v);
                return c;
            } break;
            default:
                PANIC("unknown state");
                return nullptr;
                break;
        }
        return nullptr;  // play nice with -Wall
    }

    ptr<Ast> rewrite_expr_match(const Position &p, const ptrs<Ast> &mm,
                                const ptr<Ast> &g, const ptr<Ast> &e) override {
        enter_scope();
        set_identify_state(STATE_IDENTIFY_DECLARE);
        auto mm0 = rewrites(mm);
        set_identify_state(STATE_IDENTIFY_USE);
        auto g0 = rewrite(g);
        set_identify_state(STATE_IDENTIFY_USE);
        auto e0 = rewrite(e);
        leave_scope();
        return AstExprMatch::create(p, mm0, g0, e0);
    }

    ptr<Ast> rewrite_expr_let(const Position &p, const ptrs<Ast> &lhs,
                              const ptr<Ast> &rhs,
                              const ptr<Ast> &body) override {
        set_identify_state(STATE_IDENTIFY_USE);
        auto rhs0 = rewrite(rhs);
        enter_scope();
        set_identify_state(STATE_IDENTIFY_DECLARE);
        auto lhs0 = rewrites(lhs);
        set_identify_state(STATE_IDENTIFY_USE);
        auto body0 = rewrite(body);
        leave_scope();
        return AstExprLet::create(p, lhs0, rhs0, body0);
    }

    ptr<Ast> rewrite_expr_tag(const Position &p, const ptr<Ast> &e,
                              const ptr<Ast> &t) override {
        set_identify_state(STATE_IDENTIFY_DECLARE);
        auto e0 = rewrite(e);
        set_identify_state(STATE_IDENTIFY_USE);
        auto t0 = rewrite(t);
        set_identify_state(STATE_IDENTIFY_DECLARE);
        return AstExprTag::create(p, e0, t0);
    }

    ptr<Ast> rewrite_rename(const Position &p,
                                     const ptr<Ast>& n,
                                     const ptrs<Ast> &nn) override {
        // add an alias for the namespace when not present
        auto n0 = n;
        if (n0->tag() != AST_ALIAS) {
            auto p = n0->position();
            n0 = AstAlias::create(p, AstPath::create(p, UnicodeStrings()), n);
        }
        // add combinators when not present
        auto nn0 = nn;
        if (nn0.size() == 0) {
            auto uu = AstPath::cast(AstAlias::cast(n0)->right_hand_side())->path();
            auto dd = egel::get_namespace(_scope, VM::path(uu));
            auto len = VM::path(uu).countChar32() + 2;
            for (auto &d: dd) {
                nn0.push_back(AstExprCombinator::create(p, UnicodeStrings(), d.removeBetween(0, len)));
            }
        }
        // add aliases for combinators when not present
        ptrs<Ast> nn1;
        for (const auto &n:nn0) {
            if (n->tag() != AST_ALIAS) {
                nn1.push_back(AstAlias::create(p, n, n));
            } else {
                nn1.push_back(n);
            }
        }
        //std::cout << "YYY REWRITTEN" << AstRename::create(p, n0, nn1) << std::endl;
        // declare everything
        auto p0 = AstAlias::cast(n0)->left_hand_side();
        auto p1 = AstAlias::cast(n0)->right_hand_side();
        auto uu0 = AstPath::cast(p0)->path();
        auto uu1 = AstPath::cast(p1)->path();
        for (const auto &a:nn1) {
            auto a0 = AstAlias::cast(a)->left_hand_side()->to_text();
            auto a1 = AstAlias::cast(a)->right_hand_side()->to_text();
            auto s0 = VM::qualified(uu0, a0);
            auto s1 = VM::qualified(uu1, a1);
            //std::cerr << "declare: " << s0 << " -> " << s1 << std::endl;
            try {
                egel::declare_local(_scope, s0, s1);
            } catch (ErrorSemantical &e) {
                throw ErrorSemantical(p, "redeclaration of " + s0);
            }
        }
        //egel::debug_scope(_scope);
        return AstRename::create(p, n0, nn1);
    }

    ptr<Ast> rewrite_decl_data(const Position &p, const ptr<Ast> &d,
                               const ptrs<Ast> &ee) override {
        set_identify_state(STATE_IDENTIFY_USE);
        auto ee0 = rewrites(ee);
        auto a = AstDeclData::create(p, d, ee0);
        push_declaration(a);
        return a;
    }

    ptr<Ast> rewrite_decl_definition(const Position &p, const ptr<Ast> &n,
                                     const ptr<Ast> &d,
                                     const ptr<Ast> &e) override {
        set_identify_state(STATE_IDENTIFY_USE);
        auto n0 = rewrite(n);
        auto e0 = rewrite(e);
        auto a = AstDeclDefinition::create(p, n0, d, e0);
        push_declaration(a);
        return a;
    }

    ptr<Ast> rewrite_decl_value(const Position &p, const ptr<Ast> &l,
                                const ptr<Ast> &d, const ptr<Ast> &r) override {
        set_identify_state(STATE_IDENTIFY_USE);
        auto l0 = rewrite(l);
        auto r0 = rewrite(r);
        auto a = AstDeclValue::create(p, l0, d, r0);
        push_declaration(a);
        return a;
    }

    ptr<Ast> rewrite_decl_operator(const Position &p, const ptr<Ast> &c,
                                   const ptr<Ast> &d,
                                   const ptr<Ast> &e) override {
        set_identify_state(STATE_IDENTIFY_USE);
        auto c0 = rewrite(c);
        auto e0 = rewrite(e);
        auto a = AstDeclOperator::create(p, c0, d, e0);
        push_declaration(a);
        return a;
    }

    ptr<Ast> rewrite_decl_namespace(const Position &p, const UnicodeStrings &nn,
                                    const ptrs<Ast> &dd) override {
        enter_namespace(VM::path(nn));
        auto dd0 = rewrites(dd);
        leave_namespace();
        return AstDeclNamespace::create(p, nn, dd0);
    }

private:
    identify_state_t _identify_state;
    ScopePtr _scope;
    ptrs<Ast> _declarations;
    UnicodeStrings _namespace;
    int _counter;
};

ptr<Ast> identify(ScopePtr env, const ptr<Ast> &a) {
    RewriteIdentify identify;
    return identify.identify(env, a);
}

};  // namespace egel
