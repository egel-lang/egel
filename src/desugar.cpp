#include "position.hpp"
#include "error.hpp"
#include "ast.hpp"
#include "transform.hpp"
#include "environment.hpp"
#include "desugar.hpp"

// XXX: merge these small passes when stable.

class RewriteWildcard: public Rewrite {
public:
    AstPtr wildcard(const AstPtr& a) {
        _tick = 0;
        return rewrite(a);
    }

    uint_t tick() {
        return _tick++;
    }

    AstPtr rewrite_expr_wildcard(const Position& p, const UnicodeString& v) override {
        UnicodeString w = "WILD";
        w = w + unicode_convert_uint(tick());
        return AstExprVariable(p, w).clone();
    }

private:
    uint_t  _tick;
};

AstPtr pass_wildcard(const AstPtr& a) {
    RewriteWildcard wildcard;
    return wildcard.wildcard(a);
}


class RewriteCondition: public Rewrite {
public:
    AstPtr condition(const AstPtr& a) {
        return rewrite(a);
    }

    //  F(if i then t else e) -> ([ true -> F(t) | _ -> F(e) ] F(i))
    AstPtr rewrite_expr_if(const Position& p, const AstPtr& i, const AstPtr& t, const AstPtr& e) override {
        auto i0 = rewrite(i);
        auto t0 = rewrite(t);
        auto e0 = rewrite(e);

        auto a0 = AstExprCombinator(p, STRING_SYSTEM, STRING_TRUE).clone();
        AstPtrs aa0;
        aa0.push_back(a0);
        auto a1 = AstExprMatch(p, aa0, AstEmpty().clone(), t0).clone();

        auto b0 = AstExprWildcard(p, "_").clone();
        AstPtrs bb0;
        bb0.push_back(b0);
        auto b1 = AstExprMatch(p, bb0, AstEmpty().clone(), e0).clone();

        AstPtrs cc0;
        cc0.push_back(a1);
        cc0.push_back(b1);

        auto c = AstExprBlock(p, cc0).clone();

        auto d = AstExprApplication(p, c, i0).clone();

        return d;
    }
};

AstPtr pass_condition(const AstPtr& a) {
    RewriteCondition condition;
    return condition.condition(a);
}


class RewriteTuple: public Rewrite {
public:
    AstPtr tuple(const AstPtr& a) {
        return rewrite(a);
    }

    //  F( (e0, .., en) ) -> ( tuple F(e0) .. F(en) )
    AstPtr rewrite_expr_tuple(const Position& p, const AstPtrs& ee) override {
        auto t = AstExprCombinator(p, STRING_SYSTEM, STRING_TUPLE).clone();
        for (auto& e: ee) {
            auto e0 = rewrite(e);
            t = AstExprApplication(p, t, e0).clone();
        }
        return t;
    }
};

AstPtr pass_tuple(const AstPtr& a) {
    RewriteTuple tuple;
    return tuple.tuple(a);
}


class RewriteList: public Rewrite {
public:
    AstPtr list(const AstPtr& a) {
        return rewrite(a);
    }

    //  F( {e0, .., en} ) -> (cons F(e0) (.. (cons F(en) nil ))) )
    AstPtr rewrite_expr_list(const Position& p, const AstPtrs& ee) override {
        auto nil = AstExprCombinator(p, STRING_SYSTEM, STRING_NIL).clone();
        auto cons = AstExprCombinator(p, STRING_SYSTEM, STRING_CONS).clone();
        auto l = nil;
        for (int i = ee.size() - 1; i >= 0; i--) {
            l = AstExprApplication(p, cons, ee[i], l).clone();
        }
        return rewrite(l);
    }
};

AstPtr pass_list(const AstPtr& a) {
    RewriteList list;
    return list.list(a);
}


class RewriteLambda: public Rewrite {
public:
    AstPtr lambda(const AstPtr& a) {
        return rewrite(a);
    }

    //  F( (\v0, .., vn -> e) ) -> ( [ v0, .., vn ->  F(e) ] )
    AstPtr rewrite_expr_lambda(const Position& p, const AstPtr& m) override {
        AstPtrs mm;
        auto m0 = rewrite(m);
        mm.push_back(m0);
        return AstExprBlock(p, mm).clone();
    }
};

AstPtr pass_lambda(const AstPtr& a) {
    RewriteLambda lambda;
    return lambda.lambda(a);
}

class RewriteLet: public Rewrite {
public:
    AstPtr let(const AstPtr& a) {
        return rewrite(a);
    }

    //  F( (let l = r in b) ) -> ( [ l -> F(b) ] F(r) )
    AstPtr rewrite_expr_let(const Position& p, const AstPtrs& ll, const AstPtr& r, const AstPtr& b) override {
        auto r0 = rewrite(r);
        auto b0 = rewrite(b);

        auto m = AstExprMatch(p, ll, AstEmpty().clone(), b0).clone();

        AstPtrs mm;
        mm.push_back(m);
        auto q =  AstExprBlock(p, mm).clone();

        return AstExprApplication(p, q, r0).clone();
    }
};

AstPtr pass_let(const AstPtr& a) {
    RewriteLet let;
    return let.let(a);
}

class RewriteStatement: public Rewrite {
public:
    AstPtr stat(const AstPtr& a) {
        return rewrite(a);
    }

    //  F( r; l ) ) -> ( (System.k) F(l) F(r) )
    AstPtr rewrite_expr_statement(const Position& p, const AstPtr& r, const AstPtr& l) override {
        auto r0 = rewrite(r);
        auto l0 = rewrite(l);

        auto k = AstExprCombinator(p, STRING_SYSTEM, STRING_K).clone();

        AstPtrs ee;
        ee.push_back(k);
        ee.push_back(l0);
        ee.push_back(r0);

        return AstExprApplication(p, ee).clone();
    }
};

AstPtr pass_statement(const AstPtr& a) {
    RewriteStatement stat;
    return stat.stat(a);
}

class RewriteObject: public Rewrite {
public:
    AstPtr object(const AstPtr& a) {
        return rewrite(a);
    }

    AstPtr rewrite_decl_definition(const Position& p, const AstPtr& c, const AstPtr& e) override {
        return AstDeclDefinition(p, c, e).clone(); // cut
    }

    AstPtr rewrite_decl_operator(const Position& p, const AstPtr& c, const AstPtr& e) override {
        return AstDeclOperator(p, c, e).clone(); // cut
    }

    AstPtr rewrite_decl_object(const Position& p, const AstPtr& c, const AstPtrs& vv, const AstPtrs& ff, const AstPtrs& ee) override {
        AstPtrs oo;
        AstPtrs dd;
        oo.push_back(AstExprCombinator(p, STRING_SYSTEM, STRING_OBJECT).clone());
        for (auto f:ff) {
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
        AstPtr body = AstExprApplication(p, oo).clone();
        for (auto e:ee) {
            auto ex = AstExprCombinator(p, STRING_SYSTEM, STRING_EXTEND).clone();
            AstPtrs bb;
            bb.push_back(ex);
            bb.push_back(e);
            bb.push_back(body);
            body = AstExprApplication(p, bb).clone();
        }
        if (vv.size() > 0) {
            auto m = AstExprMatch(p, vv, AstEmpty().clone(), body).clone();
            auto l = AstExprBlock(p, m).clone();
            body = l;
        }
        AstPtrs decls;
        decls.push_back(AstDeclData(p, dd).clone());
        decls.push_back(AstDeclDefinition(p, c, body).clone());
        return AstWrapper(p, decls).clone();
    }
};

AstPtr pass_object(const AstPtr& a) {
    RewriteObject object;
    return object.object(a);
}

class RewriteThrow: public Rewrite {
public:
    AstPtr dethrow(const AstPtr& a) {
        return rewrite(a);
    }

    //  F( throw e ) -> ( (System.throw e) )
    AstPtr rewrite_expr_throw(const Position& p, const AstPtr& e) override {
        auto t0 = AstExprCombinator(p, STRING_SYSTEM, STRING_THROW).clone();
        auto e0 = rewrite(e);

        return AstExprApplication(p, t0, e0).clone();
    }
};

AstPtr pass_throw(const AstPtr& a) {
    RewriteThrow t;
    return t.dethrow(a);
}

AstPtr desugar(const AstPtr& a) {
    auto a0 = pass_condition(a);
    a0 = pass_wildcard(a0);
    a0 = pass_tuple(a0);
    a0 = pass_list(a0);
    a0 = pass_let(a0);
    a0 = pass_statement(a0);
    a0 = pass_lambda(a0);
    a0 = pass_object(a0);
    a0 = pass_throw(a0);
    return a0;
}
