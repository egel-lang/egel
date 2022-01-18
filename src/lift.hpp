#pragma once

#include "ast.hpp"
#include "environment.hpp"
#include "error.hpp"
#include "position.hpp"
#include "transform.hpp"

// combinator lifting
export AstPtr lift(const AstPtr &a);

class RewriteEta : public Rewrite {
public:
    AstPtr eta(const AstPtr &a) {
        return rewrite(a);
    }

    AstPtr push_fv_front(const AstPtrs &fv, const AstPtr &m) {
        switch (m->tag()) {
            case AST_EXPR_MATCH: {
                AST_EXPR_MATCH_SPLIT(m, p, pp, g, e);
                AstPtrs pp0;
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
    AstPtr rewrite_expr_block(const Position &p, const AstPtrs &mm) override {
        // rewrite the matches
        auto mm0 = rewrites(mm);
        // determine the freevars
        auto e0 = AstExprBlock::create(p, mm0);
        auto fv0 = freevars(e0);
        // this conversion is actually completely unnecessary but better safe
        // than sorry
        AstPtrs fv;
        for (auto &v : fv0) {
            fv.push_back(v);
        }
        // push freevars to the front of matches
        AstPtrs mm1;
        for (auto &m : mm0) {
            mm1.push_back(push_fv_front(fv, m));
        }
        // apply the bloch to the freevars
        auto e1 = AstExprBlock::create(p, mm1);
        AstPtrs aa;
        aa.push_back(e1);
        for (auto &v : fv) {
            aa.push_back(v);
        }
        return AstExprApplication::create(p, aa);
    }
};

AstPtr pass_eta(const AstPtr &a) {
    RewriteEta eta;
    return eta.eta(a);
}

// spurious applications are introduced, lets remove them all
// but what a horrible name
class RewriteDeapply : public Rewrite {
public:
    AstPtr deapply(const AstPtr &a) {
        return rewrite(a);
    }

    AstPtr rewrite_expr_application(const Position &p,
                                    const AstPtrs &aa) override {
        auto &a = aa[0];
        if (a->tag() == AST_EXPR_APPLICATION) {
            AST_EXPR_APPLICATION_SPLIT(a, p, aa0);
            AstPtrs aa1;
            for (auto &a0 : aa0) aa1.push_back(a0);
            for (uint_t n = 1; n < aa.size(); n++) aa1.push_back(aa[n]);
            return rewrite(AstExprApplication::create(p, aa1));
        } else if (aa.size() == 1) {
            return rewrite(aa[0]);
        } else {
            auto aa0 = rewrites(aa);
            return AstExprApplication::create(p, aa0);
        }
    }
};

AstPtr pass_deapply(const AstPtr &a) {
    RewriteDeapply deapply;
    return deapply.deapply(a);
}

class RewriteLift : public Rewrite {
public:
    AstPtr lift(const AstPtr &a) {
        return rewrite(a);
    }

    void set_scope(const AstPtr &a) {
        AstPtrs aa;
        _scope = a;
        _counter = 0;
        _lifted = aa;
    }

    AstPtr get_scope() {
        return _scope;
    }

    void add_lifted(const AstPtr &a) {
        _lifted.push_back(a);
    }

    AstPtrs get_lifted() {
        return _lifted;
    }

    uint_t tick() {
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

    AstPtr combinator_expand(const AstPtr &c, icu::UnicodeString s) {
        if (c->tag() == AST_EXPR_COMBINATOR) {
            AST_EXPR_COMBINATOR_SPLIT(c, p, nn, n);
            icu::UnicodeString n0 = n + s;
            auto nn0 = localize(nn);
            return AstExprCombinator::create(p, nn0, n0);
        } else if (c->tag() == AST_EXPR_OPERATOR) {
            AST_EXPR_OPERATOR_SPLIT(c, p, nn,
                                    n);  // XXX: keep source to source correct
            icu::UnicodeString n0 = n + s;
            auto nn0 = localize(nn);
            return AstExprOperator::create(p, nn0, n0);
        } else {
            PANIC("combinator expected");
            return nullptr;
        }
    }

    AstPtr fresh_combinator() {
        auto c = get_scope();
        auto n = unicode_convert_uint(tick());
        return combinator_expand(c, n);
    }

    AstPtr rewrite_expr_block(const Position &p, const AstPtrs &mm) override {
        auto mm0 = rewrites(mm);
        auto e = AstExprBlock::create(p, mm0);
        auto c = fresh_combinator();
        auto decl = AstDeclDefinition::create(p, c, e);
        add_lifted(decl);
        return c;
    }

    /*
    AstPtr rewrite_expr_try(const Position& p, const AstPtr& t, const AstPtr& c)
    override { AstPtr t0; if (t->tag() == AST_EXPR_COMBINATOR) { // keep
    combinator definitions t0 = t; } else { auto t1   = rewrite(t); auto tnew =
    fresh_combinator(); auto td   = AstDeclDefinition::create(p, tnew, t1);
            add_lifted(td);
            t0 = tnew;
        }

        AstPtr c0;
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

    AstPtr rewrite_decl_definition(const Position &p, const AstPtr &c,
                                   const AstPtr &e) override {
        set_scope(c);
        AstPtr e0;
        if (e->tag() == AST_EXPR_BLOCK) {  // keep direct block definitions
            AST_EXPR_BLOCK_SPLIT(e, p0, mm);
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

    AstPtr rewrite_decl_operator(const Position &p, const AstPtr &c,
                                 const AstPtr &e) override {
        set_scope(c);
        AstPtr e0;
        if (e->tag() == AST_EXPR_BLOCK) {  // keep direct block definitions
            AST_EXPR_BLOCK_SPLIT(e, p0, mm);
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

    AstPtr rewrite_decl_value(const Position &p, const AstPtr &c,
                              const AstPtr &e) override {
        set_scope(c);
        AstPtr e0;
        if (e->tag() == AST_EXPR_BLOCK) {  // keep direct block definitions
            AST_EXPR_BLOCK_SPLIT(e, p0, mm);
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
    AstPtr _scope;
    uint_t _counter;
    AstPtrs _lifted;
};

AstPtr pass_lift(const AstPtr &a) {
    RewriteLift lift;
    return lift.lift(a);
}

class RewriteRelambda : public Rewrite {
public:
    AstPtr relambda(const AstPtr &a) {
        return rewrite(a);
    }

    AstPtr rewrite_decl_definition(const Position &p, const AstPtr &c,
                                   const AstPtr &e) override {
        AstPtr e0;
        if (e->tag() == AST_EXPR_BLOCK) {  // keep direct block definitions
            e0 = e;
        } else {  // wrap all expressions in a nullary lambda to simplify code
            // generation
            // (i.e., generate a return instruction at the end of each match)
            AstPtrs vv;
            auto m = AstExprMatch::create(p, vv, AstEmpty::create(), e);
            AstPtrs mm;
            mm.push_back(m);
            e0 = AstExprBlock::create(p, mm);
        }
        return AstDeclDefinition::create(p, c, e0);
    }

    // treat as a definition
    AstPtr rewrite_decl_operator(const Position &p, const AstPtr &c,
                                 const AstPtr &e) override {
        auto e0 = rewrite_decl_definition(p, c, e);
        if (e0->tag() == AST_DECL_DEFINITION) {
            AST_DECL_DEFINITION_SPLIT(e0, p1, c1, e1);
            return AstDeclOperator::create(p1, c1, e1);
        } else {
            PANIC("didn't find a definition");
            return nullptr;
        }
    }

    // treat as a definition
    AstPtr rewrite_decl_value(const Position &p, const AstPtr &c,
                              const AstPtr &e) override {
        auto e0 = rewrite_decl_definition(p, c, e);
        if (e0->tag() == AST_DECL_DEFINITION) {
            AST_DECL_DEFINITION_SPLIT(e0, p1, c1, e1);
            return AstDeclValue::create(p1, c1, e1);
        } else {
            PANIC("didn't find a definition");
            return nullptr;
        }
    }
};

AstPtr pass_relambda(const AstPtr &a) {
    RewriteRelambda relambda;
    return relambda.relambda(a);
}

AstPtr lift(const AstPtr &a) {
    AstPtr a0;
    a0 = pass_eta(a);
    a0 = pass_deapply(a0);
    a0 = pass_lift(a0);
    a0 = pass_relambda(a0);
    return a0;
}
