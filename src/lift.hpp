#pragma once

#include "runtime.hpp"
#include "ast.hpp"
#include "environment.hpp"
#include "error.hpp"
#include "position.hpp"
#include "transform.hpp"

namespace egel {

// combinator lifting
class RewriteEta : public Rewrite {
public:
    ptr<Ast> eta(const ptr<Ast> &a) {
        return rewrite(a);
    }

    ptr<Ast> push_fv_front(const ptrs<Ast> &fv, const ptr<Ast> &m) {
        switch (m->tag()) {
            case AST_EXPR_MATCH: {
                auto [p, pp, g, e] = AstExprMatch::split(m);
                ptrs<Ast> pp0;
                for (auto &v : fv) {
                    pp0.push_back(v);
                }
                for (auto &p0 : pp) {
                    pp0.push_back(p0);
                }
                return AstExprMatch::create(p, pp0, g, e);
            } break;
            default:
                PANIC("match expected");
                return nullptr;
                break;
        }
    }

    /**
     * For corner cases this transform relies on a bit of `magic`.
     *
     * [ X -> [ X -> X | -> X ] 0 ] 1 rewrites to
     * [ X -> [ X X -> X | X -> X ] 0 ] 1 ] which is only correct
     * since the backend will correctly choose the second `X` in
     * the inner lambda.
     */
    // XXX: if FV = 0 do nothing
    ptr<Ast> rewrite_expr_block(const Position &p,
                                const ptrs<Ast> &mm) override {
        // rewrite the matches
        auto mm0 = rewrites(mm);
        // determine the freevars
        auto e0 = AstExprBlock::create(p, mm0);
        auto fv0 = freevars(e0);
        // this conversion is actually completely unnecessary but better safe
        // than sorry
        ptrs<Ast> fv;
        for (auto &v : fv0) {
            fv.push_back(v);
        }
        // push freevars to the front of matches
        ptrs<Ast> mm1;
        for (auto &m : mm0) {
            mm1.push_back(push_fv_front(fv, m));
        }
        // apply the bloch to the freevars
        auto e1 = AstExprBlock::create(p, mm1);
        ptrs<Ast> aa;
        aa.push_back(e1);
        for (auto &v : fv) {
            aa.push_back(v);
        }
        return AstExprApplication::create(p, aa);
    }
};

ptr<Ast> pass_eta(const ptr<Ast> &a) {
    RewriteEta eta;
    return eta.eta(a);
}

// spurious applications are introduced, lets remove them all
// but what a horrible name
class RewriteDeapply : public Rewrite {
public:
    ptr<Ast> deapply(const ptr<Ast> &a) {
        return rewrite(a);
    }

    ptr<Ast> rewrite_expr_application(const Position &p,
                                      const ptrs<Ast> &aa) override {
        auto &a = aa[0];
        if (a->tag() == AST_EXPR_APPLICATION) {
            auto [p, aa0] = AstExprApplication::split(a);
            ptrs<Ast> aa1;
            for (auto &a0 : aa0) aa1.push_back(a0);
            for (size_t n = 1; n < aa.size(); n++) aa1.push_back(aa[n]);
            return rewrite(AstExprApplication::create(p, aa1));
        } else if (aa.size() == 1) {
            return rewrite(aa[0]);
        } else {
            auto aa0 = rewrites(aa);
            return AstExprApplication::create(p, aa0);
        }
    }
};

ptr<Ast> pass_deapply(const ptr<Ast> &a) {
    RewriteDeapply deapply;
    return deapply.deapply(a);
}

class RewriteLift : public Rewrite {
public:
    ptr<Ast> lift(const ptr<Ast> &a) {
        return rewrite(a);
    }

    void set_scope(const ptr<Ast> &a) {
        ptrs<Ast> aa;
        _scope = a;
        _counter = 0;
        _lifted = aa;
    }

    ptr<Ast> get_scope() {
        return _scope;
    }

    void add_lifted(const ptr<Ast> &a) {
        _lifted.push_back(a);
    }

    ptrs<Ast> get_lifted() {
        return _lifted;
    }

    int tick() {
        return _counter++;
    }

    UnicodeStrings localize(UnicodeStrings ss) {
        UnicodeStrings ss0;
        for (auto &s : ss) {
            ss0.push_back(s);
        }
        ss0.push_back(STRING_LOCAL);
        return ss0;
    }

    ptr<Ast> combinator_expand(const ptr<Ast> &c, icu::UnicodeString s) {
        if (c->tag() == AST_EXPR_COMBINATOR) {
            auto [p, nn, n] = AstExprCombinator::split(c);
            icu::UnicodeString n0 = n + s;
            auto nn0 = localize(nn);
            return AstExprCombinator::create(p, nn0, n0);
        } else if (c->tag() == AST_EXPR_OPERATOR) {
            auto [p, nn, n] = AstExprOperator::split(
                c);  // XXX: keep source to source correct
            icu::UnicodeString n0 = n + s;
            auto nn0 = localize(nn);
            return AstExprOperator::create(p, nn0, n0);
        } else {
            PANIC("combinator expected");
            return nullptr;
        }
    }

    ptr<Ast> fresh_combinator() {
        auto c = get_scope();
        auto n = VM::unicode_from_int(tick());
        return combinator_expand(c, n);
    }

    ptr<Ast> rewrite_expr_block(const Position &p,
                                const ptrs<Ast> &mm) override {
        auto mm0 = rewrites(mm);
        auto e = AstExprBlock::create(p, mm0);
        auto c = fresh_combinator();
        auto decl = AstDeclDefinition::create(p, c, e);
        add_lifted(decl);
        return c;
    }

    /*
    ptr<Ast> rewrite_expr_try(const Position& p, const ptr<Ast>& t, const
    ptr<Ast>& c) override { ptr<Ast> t0; if (t->tag() == AST_EXPR_COMBINATOR) {
    // keep combinator definitions t0 = t; } else { auto t1   = rewrite(t); auto
    tnew = fresh_combinator(); auto td   = AstDeclDefinition::create(p, tnew,
    t1); add_lifted(td); t0 = tnew;
        }

        ptr<Ast> c0;
        if (c->tag() == AST_EXPR_COMBINATOR) { // keep combinator definitions
            c0 = c;
        } else {
            auto c1   = rewrite(c);
            auto cnew = fresh_combinator();
            auto cd   = AstDeclDefinition::create(p, cnew, c1);
            add_lifted(cd);
            c0 = cnew;
        }

        return AstExprTry::create(p, t0, c0);
    }
    */

    ptr<Ast> rewrite_decl_definition(const Position &p, const ptr<Ast> &c,
                                     const ptr<Ast> &e) override {
        set_scope(c);
        ptr<Ast> e0;
        if (e->tag() == AST_EXPR_BLOCK) {  // keep direct block definitions
            auto [p0, mm] = AstExprBlock::split(e);
            auto mm0 = rewrites(mm);
            e0 = AstExprBlock::create(p0, mm0);
        } else {
            e0 = rewrite(e);
        }
        auto e1 = AstDeclDefinition::create(p, c, e0);
        if (get_lifted().size() == 0) {
            return e1;
        } else {
            add_lifted(e1);
            return AstWrapper::create(p, get_lifted());
        }
    }

    ptr<Ast> rewrite_decl_operator(const Position &p, const ptr<Ast> &c,
                                   const ptr<Ast> &e) override {
        set_scope(c);
        ptr<Ast> e0;
        if (e->tag() == AST_EXPR_BLOCK) {  // keep direct block definitions
            auto [p0, mm] = AstExprBlock::split(e);
            auto mm0 = rewrites(mm);
            e0 = AstExprBlock::create(p0, mm0);
        } else {
            e0 = rewrite(e);
        }
        auto e1 = AstDeclOperator::create(p, c, e0);
        if (get_lifted().size() == 0) {
            return e1;
        } else {
            add_lifted(e1);
            return AstWrapper::create(p, get_lifted());
        }
    }

    ptr<Ast> rewrite_decl_value(const Position &p, const ptr<Ast> &c,
                                const ptr<Ast> &e) override {
        set_scope(c);
        ptr<Ast> e0;
        if (e->tag() == AST_EXPR_BLOCK) {  // keep direct block definitions
            auto [p0, mm] = AstExprBlock::split(e);
            auto mm0 = rewrites(mm);
            e0 = AstExprBlock::create(p0, mm0);
        } else {
            e0 = rewrite(e);
        }
        auto e1 = AstDeclValue::create(p, c, e0);
        if (get_lifted().size() == 0) {
            return e1;
        } else {
            add_lifted(e1);
            return AstWrapper::create(p, get_lifted());
        }
    }

private:
    ptr<Ast> _scope;
    int _counter;
    ptrs<Ast> _lifted;
};

inline ptr<Ast> pass_lift(const ptr<Ast> &a) {
    RewriteLift lift;
    return lift.lift(a);
}

class RewriteRelambda : public Rewrite {
public:
    ptr<Ast> relambda(const ptr<Ast> &a) {
        return rewrite(a);
    }

    ptr<Ast> rewrite_decl_definition(const Position &p, const ptr<Ast> &c,
                                     const ptr<Ast> &e) override {
        ptr<Ast> e0;
        if (e->tag() == AST_EXPR_BLOCK) {  // keep direct block definitions
            e0 = e;
        } else {  // wrap all expressions in a nullary lambda to simplify code
            // generation
            // (i.e., generate a return instruction at the end of each match)
            ptrs<Ast> vv;
            auto m = AstExprMatch::create(p, vv, AstEmpty::create(), e);
            ptrs<Ast> mm;
            mm.push_back(m);
            e0 = AstExprBlock::create(p, mm);
        }
        return AstDeclDefinition::create(p, c, e0);
    }

    // treat as a definition
    ptr<Ast> rewrite_decl_operator(const Position &p, const ptr<Ast> &c,
                                   const ptr<Ast> &e) override {
        auto e0 = rewrite_decl_definition(p, c, e);
        if (e0->tag() == AST_DECL_DEFINITION) {
            auto [p1, c1, e1] = AstDeclDefinition::split(e0);
            return AstDeclOperator::create(p1, c1, e1);
        } else {
            PANIC("didn't find a definition");
            return nullptr;
        }
    }

    // treat as a definition
    ptr<Ast> rewrite_decl_value(const Position &p, const ptr<Ast> &c,
                                const ptr<Ast> &e) override {
        auto e0 = rewrite_decl_definition(p, c, e);
        if (e0->tag() == AST_DECL_DEFINITION) {
            auto [p1, c1, e1] = AstDeclDefinition::split(e0);
            return AstDeclValue::create(p1, c1, e1);
        } else {
            PANIC("didn't find a definition");
            return nullptr;
        }
    }
};

inline ptr<Ast> pass_relambda(const ptr<Ast> &a) {
    RewriteRelambda relambda;
    return relambda.relambda(a);
}

// NOTE: this pass is run after data combinators are added to the machine
class RewriteRedex : public Rewrite {
public:
    ptr<Ast> reredex(const ptr<Ast> &a, VM *m) {
        _machine = m;
        return rewrite(a);
    }

    VM* machine() {
        return _machine;
    }

    int tick() {
        return _tick++;
    }

    void reset() {
        _tick = 0;
        _redexes = std::vector<std::tuple<ptr<Ast>,ptr<Ast>>>();
    }
        
    ptr<Ast> fresh_variable(const Position &p) {
        auto v = icu::UnicodeString("_REDEX") + VM::unicode_from_int(tick());
        return AstExprVariable::create(p, v);
    }

    void redexes_push(const ptr<Ast> &r, const ptr<Ast> e) {
        _redexes.push_back(std::make_tuple(r, e));
    }

    bool redexes_has() {
        return _redexes.size() > 0;
    }

    std::tuple<ptr<Ast>,ptr<Ast>> redexes_pop() {
        auto n = _redexes.size() - 1;
        auto t = _redexes[n];
        _redexes.erase(_redexes.begin()+n);
        return t;
    }

    bool is_combinator_redex(const ptr<Ast> &e) {
        if (e->tag() == AST_EXPR_COMBINATOR) {
           auto [p, nn, n] = AstExprCombinator::split(e);
           if (machine()->has_combinator(nn, n)) {
                auto c = machine()->get_combinator(nn, n);
                return !(machine()->is_data(c) || machine()->is_opaque(c));
           } else {
                return true; // XXX: undeclared combinators are redexes, not always true
           }
        } else {
            return false;
        }
    }

    bool is_head_redex(const ptr<Ast> &e) {
        if (e->tag() == AST_EXPR_VARIABLE) {
            return true;
        } else if (e->tag() == AST_EXPR_COMBINATOR) {
            return is_combinator_redex(e);
        } else {
            return false;
        }
    }
    
    ptr<Ast> rewrite_expr_combinator(const Position &p,
                                             const UnicodeStrings &nn,
                                             const icu::UnicodeString &n) override {
        auto c = AstExprCombinator::create(p, nn, n);
        if (is_combinator_redex(c)) {
            auto v = fresh_variable(p);
            redexes_push(v, c);
            return v;
        } else {
            return c;
        }
    }

    ptr<Ast> rewrite_expr_operator(const Position &p,
                                           const UnicodeStrings &nn,
                                           const icu::UnicodeString &n) override {
        // NOTE: a single operators is unlikely to be a redex
        auto c = AstExprOperator::create(p, nn, n);
        auto v = fresh_variable(p);
        redexes_push(v, c);
        return v;
    }

    ptr<Ast> rewrite_expr_application(const Position &p,
                                      const ptrs<Ast> &aa) override {
        // var or combinator
        if (is_head_redex(aa[0])) {
            ptrs<Ast> aa0;
            aa0.push_back(aa[0]);
            for (unsigned int i = 1; i < aa.size(); i++) {
                auto a = rewrite(aa[i]);
                aa0.push_back(a);
            }
            auto r = AstExprApplication::create(p, aa0);
            auto v = fresh_variable(p);
            redexes_push(v, r);
            return v;
        } else {
            auto aa0 = rewrites(aa);
            return AstExprApplication::create(p, aa0);
        }
    }

    ptr<Ast> rewrite_expr_match(const Position &p, const ptrs<Ast> &mm,
                                        const ptr<Ast> &g, const ptr<Ast> &e) override {
        reset();
        auto e0 = rewrite(e);
        while (redexes_has()) {
            auto t = redexes_pop();
            ptrs<Ast> gg;
            gg.push_back(std::get<0>(t));
            e0 = AstExprLet::create(p, gg, std::get<1>(t), e0);
        }
        return AstExprMatch::create(p, mm, g, e0);
    }

    ptr<Ast> rewrite_decl_definition(const Position &p, const ptr<Ast> &c,
                                     const ptr<Ast> &e) override {
        auto e0 = rewrite(e);
        return AstDeclDefinition::create(p, c, e0);
    }

    // treat as a definition
    ptr<Ast> rewrite_decl_operator(const Position &p, const ptr<Ast> &c,
                                   const ptr<Ast> &e) override {
        auto e0 = rewrite_decl_definition(p, c, e);
        if (e0->tag() == AST_DECL_DEFINITION) {
            auto [p1, c1, e1] = AstDeclDefinition::split(e0);
            return AstDeclOperator::create(p1, c1, e1);
        } else {
            PANIC("didn't find a definition");
            return nullptr;
        }
    }

    // treat as a definition
    ptr<Ast> rewrite_decl_value(const Position &p, const ptr<Ast> &c,
                                const ptr<Ast> &e) override {
        auto e0 = rewrite_decl_definition(p, c, e);
        if (e0->tag() == AST_DECL_DEFINITION) {
            auto [p1, c1, e1] = AstDeclDefinition::split(e0);
            return AstDeclValue::create(p1, c1, e1);
        } else {
            PANIC("didn't find a definition");
            return nullptr;
        }
    }
private:
    VM*         _machine;
    std::vector<std::tuple<ptr<Ast>,ptr<Ast>>>   _redexes;
    int         _tick;
};

inline ptr<Ast> pass_reredex(const ptr<Ast> &a, VM *m) {
    RewriteRedex reredex;
    return reredex.reredex(a, m);
}

inline ptr<Ast> lift(const ptr<Ast> &a, VM *m) {
    ptr<Ast> a0;
    a0 = pass_eta(a);
    a0 = pass_deapply(a0);
    a0 = pass_lift(a0);
    a0 = pass_relambda(a0);
    //a0 = pass_reredex(a0, m);
    return a0;
}
}  // namespace egel
