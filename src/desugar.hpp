#pragma once

#include "ast.hpp"
#include "desugar.hpp"
#include "environment.hpp"
#include "error.hpp"
#include "position.hpp"
#include "transform.hpp"

// XXX: merge these small passes when stable.

class RewriteWildcard : public Rewrite {
public:
    AstPtr wildcard(const AstPtr &a) {
        _tick = 0;
        return rewrite(a);
    }

    int tick() {
        return _tick++;
    }

    AstPtr rewrite_expr_wildcard(const Position &p,
                                 const icu::UnicodeString &v) override {
        icu::UnicodeString w = "WILD";
        w = w + unicode_convert_uint(tick());
        return AstExprVariable::create(p, w);
    }

private:
    int _tick;
};

AstPtr pass_wildcard(const AstPtr &a) {
    RewriteWildcard wildcard;
    return wildcard.wildcard(a);
}

class RewriteCondition : public Rewrite {
public:
    AstPtr condition(const AstPtr &a) {
        return rewrite(a);
    }

    //  F(if i then t else e) -> ([ true -> F(t) | _ -> F(e) ] F(i))
    AstPtr rewrite_expr_if(const Position &p, const AstPtr &i, const AstPtr &t,
                           const AstPtr &e) override {
        auto i0 = rewrite(i);
        auto t0 = rewrite(t);
        auto e0 = rewrite(e);

        auto a0 = AstExprCombinator::create(p, STRING_SYSTEM, STRING_TRUE);
        AstPtrs aa0;
        aa0.push_back(a0);
        auto a1 = AstExprMatch::create(p, aa0, AstEmpty::create(), t0);

        auto b0 = AstExprWildcard::create(p, "_");
        AstPtrs bb0;
        bb0.push_back(b0);
        auto b1 = AstExprMatch::create(p, bb0, AstEmpty::create(), e0);

        AstPtrs cc0;
        cc0.push_back(a1);
        cc0.push_back(b1);

        auto c = AstExprBlock::create(p, cc0);

        auto d = AstExprApplication::create(p, c, i0);

        return d;
    }
};

AstPtr pass_condition(const AstPtr &a) {
    RewriteCondition condition;
    return condition.condition(a);
}

class RewriteTuple : public Rewrite {
public:
    AstPtr tuple(const AstPtr &a) {
        return rewrite(a);
    }

    //  F( (e0, .., en) ) -> ( tuple F(e0) .. F(en) )
    AstPtr rewrite_expr_tuple(const Position &p, const AstPtrs &ee) override {
        auto t = AstExprCombinator::create(p, STRING_SYSTEM, STRING_TUPLE);
        for (auto &e : ee) {
            auto e0 = rewrite(e);
            t = AstExprApplication::create(p, t, e0);
        }
        return t;
    }
};

AstPtr pass_tuple(const AstPtr &a) {
    RewriteTuple tuple;
    return tuple.tuple(a);
}

class RewriteList : public Rewrite {
public:
    AstPtr list(const AstPtr &a) {
        return rewrite(a);
    }

    //  F( {e0, .., en|ee} ) -> (cons F(e0) (.. (cons F(en) F(ee) ))) )
    AstPtr rewrite_expr_list(const Position &p, const AstPtrs &ee,
                             const AstPtr &tl) override {
        auto nil = AstExprCombinator::create(p, STRING_SYSTEM, STRING_NIL);
        auto cons = AstExprCombinator::create(p, STRING_SYSTEM, STRING_CONS);
        auto l = nil;
        if (tl != nullptr) {
            l = rewrite(tl);
        }
        for (int i = ee.size() - 1; i >= 0; i--) {
            l = AstExprApplication::create(p, cons, ee[i], l);
        }
        return rewrite(l);
    }
};

AstPtr pass_list(const AstPtr &a) {
    RewriteList list;
    return list.list(a);
}

class RewriteStatement : public Rewrite {
public:
    AstPtr stat(const AstPtr &a) {
        return rewrite(a);
    }

    //  F( r; l ) ) -> ( let _ = F(r) in F(l) )
    AstPtr rewrite_expr_statement(const Position &p, const AstPtr &r,
                                  const AstPtr &l) override {
        auto r0 = rewrite(r);
        auto l0 = rewrite(l);

        auto k = AstExprCombinator::create(p, STRING_SYSTEM, STRING_K);

        auto b0 = AstExprWildcard::create(p, "_");
        AstPtrs bb0;
        bb0.push_back(b0);

        return AstExprLet::create(p, bb0, r0, l0);
    }
};

AstPtr pass_statement(const AstPtr &a) {
    RewriteStatement stat;
    return stat.stat(a);
}

class RewriteLambda : public Rewrite {
public:
    AstPtr lambda(const AstPtr &a) {
        return rewrite(a);
    }

    //  F( (\v0, .., vn -> e) ) -> ( [ v0, .., vn ->  F(e) ] )
    AstPtr rewrite_expr_lambda(const Position &p, const AstPtr &m) override {
        AstPtrs mm;
        auto m0 = rewrite(m);
        mm.push_back(m0);
        return AstExprBlock::create(p, mm);
    }
};

AstPtr pass_lambda(const AstPtr &a) {
    RewriteLambda lambda;
    return lambda.lambda(a);
}

class RewriteLet : public Rewrite {
public:
    AstPtr let(const AstPtr &a) {
        return rewrite(a);
    }

    //  F( (let l = r in b) ) -> ( [ l -> F(b) ] (F(r)) )
    AstPtr rewrite_expr_let(const Position &p, const AstPtrs &ll,
                            const AstPtr &r, const AstPtr &b) override {
        auto r0 = rewrite(r);
        auto b0 = rewrite(b);

        auto m = AstExprMatch::create(p, ll, AstEmpty::create(), b0);

        AstPtrs mm;
        mm.push_back(m);
        auto q = AstExprBlock::create(p, mm);

        return AstExprApplication::create(p, q, r0);
    }
};

AstPtr pass_let(const AstPtr &a) {
    RewriteLet let;
    return let.let(a);
}

class RewriteObject : public Rewrite {
public:
    AstPtr object(const AstPtr &a) {
        return rewrite(a);
    }

    AstPtr rewrite_decl_definition(const Position &p, const AstPtr &c,
                                   const AstPtr &e) override {
        return AstDeclDefinition::create(p, c, e);  // cut
    }

    AstPtr rewrite_decl_operator(const Position &p, const AstPtr &c,
                                 const AstPtr &e) override {
        return AstDeclOperator::create(p, c, e);  // cut
    }

    AstPtr rewrite_decl_object(const Position &p, const AstPtr &c,
                               const AstPtrs &vv, const AstPtrs &ff,
                               const AstPtrs &ee) override {
        AstPtrs oo;
        AstPtrs dd;
        oo.push_back(
            AstExprCombinator::create(p, STRING_SYSTEM, STRING_OBJECT));
        for (auto f : ff) {
            if (f->tag() == AST_DECL_DATA) {
                AST_DECL_DATA_SPLIT(f, p, dd0);
                oo.push_back(dd0[0]);
                oo.push_back(dd0[1]);
                dd.push_back(dd0[0]);
            } else if (f->tag() == AST_DECL_DEFINITION) {
                AST_DECL_DEFINITION_SPLIT(f, p, c, e);
                oo.push_back(c);
                oo.push_back(e);
                dd.push_back(c);
            } else {
                PANIC("failed to rewrite field");
            }
        }
        AstPtr body = AstExprApplication::create(p, oo);
        for (auto e : ee) {
            auto ex =
                AstExprCombinator::create(p, STRING_SYSTEM, STRING_EXTEND);
            AstPtrs bb;
            bb.push_back(ex);
            bb.push_back(e);
            bb.push_back(body);
            body = AstExprApplication::create(p, bb);
        }
        if (vv.size() > 0) {
            auto m = AstExprMatch::create(p, vv, AstEmpty::create(), body);
            auto l = AstExprBlock::create(p, m);
            body = l;
        }
        AstPtrs decls;
        decls.push_back(AstDeclData::create(p, dd));
        decls.push_back(AstDeclDefinition::create(p, c, body));
        return AstWrapper::create(p, decls);
    }
};

AstPtr pass_object(const AstPtr &a) {
    RewriteObject object;
    return object.object(a);
}

class RewriteThrow : public Rewrite {
public:
    AstPtr dethrow(const AstPtr &a) {
        return rewrite(a);
    }

    //  F( throw e ) -> ( (System.throw e) )
    AstPtr rewrite_expr_throw(const Position &p, const AstPtr &e) override {
        auto t0 = AstExprCombinator::create(p, STRING_SYSTEM, STRING_THROW);
        auto e0 = rewrite(e);

        return AstExprApplication::create(p, t0, e0);
    }
};

AstPtr pass_throw(const AstPtr &a) {
    RewriteThrow t;
    return t.dethrow(a);
}

class RewriteTry : public Rewrite {
public:
    AstPtr idtry(const AstPtr &a) {
        return rewrite(a);
    }

    //  F( throw e ) -> ( (System.throw e) )
    AstPtr rewrite_expr_try(const Position &p, const AstPtr &t,
                            const AstPtr &c) override {
        auto id = AstExprCombinator::create(p, STRING_SYSTEM, STRING_ID);
        auto t0 = rewrite(t);
        auto c0 = rewrite(c);

        auto e0 = AstExprTry::create(p, t0, c0);

        return AstExprApplication::create(p, id, e0);
    }
};

AstPtr pass_try(const AstPtr &a) {
    RewriteTry t;
    return t.idtry(a);
}

class RewriteMonmin : public Rewrite {
public:
    AstPtr monmin(const AstPtr &a) {
        return rewrite(a);
    }

    AstPtr lambdify(const AstPtr &e) {
        auto p = e->position();
        auto v = AstExprVariable::create(p, "WILD0");
        AstPtrs vv;
        vv.push_back(v);
        auto m = AstExprMatch::create(p, vv, AstEmpty::create(), e);
        AstPtrs mm;
        mm.push_back(m);
        return AstExprBlock::create(p, mm);
    }

    // (- x) -> -x
    AstPtr rewrite_expr_application(const Position &p,
                                    const AstPtrs &ee) override {
        if (ee.size() == 2) {
            auto op = ee[0];
            if (op->tag() == AST_EXPR_COMBINATOR) {
                auto s = op->to_text();
                auto arg0 = ee[1];
                if ((s == "System:!-") && (arg0->tag() == AST_EXPR_INTEGER)) {
                    AST_EXPR_INTEGER_SPLIT(arg0, p, s);
                    return AstExprInteger::create(p, "-" + s);
                } else {
                    return AstExprApplication::create(p, rewrites(ee));
                }
            } else {
                return AstExprApplication::create(p, rewrites(ee));
            }
        } else {
            return AstExprApplication::create(p, rewrites(ee));
        }
    }
};

AstPtr pass_monmin(const AstPtr &a) {
    RewriteMonmin t;
    return t.monmin(a);
}

class RewriteLazyOp : public Rewrite {
public:
    AstPtr lazyop(const AstPtr &a) {
        return rewrite(a);
    }

    AstPtr lambdify(const AstPtr &e) {
        auto p = e->position();
        auto v = AstExprVariable::create(p, "WILD0");
        AstPtrs vv;
        vv.push_back(v);
        auto m = AstExprMatch::create(p, vv, AstEmpty::create(), e);
        AstPtrs mm;
        mm.push_back(m);
        return AstExprBlock::create(p, mm);
    }

    //  e0 || e1 -> e0 || [ _ -> e1 ] and e0 && e1 -> e0 && [ _ -> e1 ]
    AstPtr rewrite_expr_application(const Position &p,
                                    const AstPtrs &ee) override {
        if (ee.size() == 3) {
            auto op = ee[0];
            if (op->tag() == AST_EXPR_COMBINATOR) {
                auto s = op->to_text();
                if ((s == "System:&&") || (s == "System:||")) {
                    auto arg0 = rewrite(ee[1]);
                    auto arg1 = lambdify(rewrite(ee[2]));

                    AstPtrs ff;
                    ff.push_back(op);
                    ff.push_back(arg0);
                    ff.push_back(arg1);

                    return AstExprApplication::create(p, ff);
                } else {
                    return AstExprApplication::create(p, rewrites(ee));
                }
            } else {
                return AstExprApplication::create(p, rewrites(ee));
            }
        } else {
            return AstExprApplication::create(p, rewrites(ee));
        }
    }
};

AstPtr pass_lazyop(const AstPtr &a) {
    RewriteLazyOp t;
    return t.lazyop(a);
}

AstPtr desugar(const AstPtr &a) {
    auto a0 = pass_condition(a);
    a0 = pass_wildcard(a0);
    a0 = pass_tuple(a0);
    a0 = pass_list(a0);
    a0 = pass_statement(a0);
    a0 = pass_let(a0);
    a0 = pass_lambda(a0);
    a0 = pass_object(a0);
    a0 = pass_throw(a0);
    a0 = pass_try(a0);
    a0 = pass_lazyop(a0);
    a0 = pass_monmin(a0);
    return a0;
}
