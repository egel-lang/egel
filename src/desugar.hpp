#pragma once

#include "ast.hpp"
#include "desugar.hpp"
#include "environment.hpp"
#include "error.hpp"
#include "position.hpp"
#include "runtime.hpp"
#include "transform.hpp"

// XXX: merge these small passes when stable.

namespace egel {

class RewriteWildcard : public Rewrite {
public:
    ptr<Ast> wildcard(const ptr<Ast> &a) {
        _tick = 0;
        return rewrite(a);
    }

    int tick() {
        return _tick++;
    }

    ptr<Ast> rewrite_expr_wildcard(const Position &p,
                                   const icu::UnicodeString &v) override {
        icu::UnicodeString w = "WILD";
        w = w + VM::unicode_from_int(tick());
        return AstExprVariable::create(p, w);
    }

private:
    int _tick;
};

ptr<Ast> pass_wildcard(const ptr<Ast> &a) {
    RewriteWildcard wildcard;
    return wildcard.wildcard(a);
}

class RewriteCondition : public Rewrite {
public:
    ptr<Ast> condition(const ptr<Ast> &a) {
        return rewrite(a);
    }

    //  F(if i then t else e) -> ([ true -> F(t) | _ -> F(e) ] F(i))
    ptr<Ast> rewrite_expr_if(const Position &p, const ptr<Ast> &i,
                             const ptr<Ast> &t, const ptr<Ast> &e) override {
        auto i0 = rewrite(i);
        auto t0 = rewrite(t);
        auto e0 = rewrite(e);

        auto a0 = AstExprCombinator::create(p, STRING_SYSTEM, STRING_TRUE);
        ptrs<Ast> aa0;
        aa0.push_back(a0);
        auto a1 = AstExprMatch::create(p, aa0, AstEmpty::create(), t0);

        auto b0 = AstExprWildcard::create(p, "_");
        ptrs<Ast> bb0;
        bb0.push_back(b0);
        auto b1 = AstExprMatch::create(p, bb0, AstEmpty::create(), e0);

        ptrs<Ast> cc0;
        cc0.push_back(a1);
        cc0.push_back(b1);

        auto c = AstExprBlock::create(p, cc0);

        auto d = AstExprApplication::create(p, c, i0);

        return d;
    }
};

ptr<Ast> pass_condition(const ptr<Ast> &a) {
    RewriteCondition condition;
    return condition.condition(a);
}

class RewriteTuple : public Rewrite {
public:
    ptr<Ast> tuple(const ptr<Ast> &a) {
        return rewrite(a);
    }

    //  F( (e0, .., en) ) -> ( tuple F(e0) .. F(en) )
    ptr<Ast> rewrite_expr_tuple(const Position &p,
                                const ptrs<Ast> &ee) override {
        auto t = AstExprCombinator::create(p, STRING_SYSTEM, STRING_TUPLE);
        for (auto &e : ee) {
            auto e0 = rewrite(e);
            t = AstExprApplication::create(p, t, e0);
        }
        return t;
    }
};

ptr<Ast> pass_tuple(const ptr<Ast> &a) {
    RewriteTuple tuple;
    return tuple.tuple(a);
}

class RewriteList : public Rewrite {
public:
    ptr<Ast> list(const ptr<Ast> &a) {
        return rewrite(a);
    }

    //  F( {e0, .., en|ee} ) -> (cons F(e0) (.. (cons F(en) F(ee) ))) )
    ptr<Ast> rewrite_expr_list(const Position &p, const ptrs<Ast> &ee,
                               const ptr<Ast> &tl) override {
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

ptr<Ast> pass_list(const ptr<Ast> &a) {
    RewriteList list;
    return list.list(a);
}

class RewriteStatement : public Rewrite {
public:
    ptr<Ast> stat(const ptr<Ast> &a) {
        return rewrite(a);
    }

    //  F( r; l ) ) -> ( let _ = F(r) in F(l) )
    ptr<Ast> rewrite_expr_statement(const Position &p, const ptr<Ast> &r,
                                    const ptr<Ast> &l) override {
        auto r0 = rewrite(r);
        auto l0 = rewrite(l);

        auto k = AstExprCombinator::create(p, STRING_SYSTEM, STRING_K);

        auto b0 = AstExprWildcard::create(p, "_");
        ptrs<Ast> bb0;
        bb0.push_back(b0);

        return AstExprLet::create(p, bb0, r0, l0);
    }
};

ptr<Ast> pass_statement(const ptr<Ast> &a) {
    RewriteStatement stat;
    return stat.stat(a);
}

class RewriteLambda : public Rewrite {
public:
    ptr<Ast> lambda(const ptr<Ast> &a) {
        return rewrite(a);
    }

    //  F( (\v0, .., vn -> e) ) -> ( [ v0, .., vn ->  F(e) ] )
    ptr<Ast> rewrite_expr_lambda(const Position &p,
                                 const ptrs<Ast> &mm) override {
        auto mm0 = rewrites(mm);
        return AstExprBlock::create(p, mm0);
    }
};

ptr<Ast> pass_lambda(const ptr<Ast> &a) {
    RewriteLambda lambda;
    return lambda.lambda(a);
}

class RewriteLet : public Rewrite {
public:
    ptr<Ast> let(const ptr<Ast> &a) {
        return rewrite(a);
    }

    //  F( (let l = r in b) ) -> ( [ l -> F(b) ] (F(r)) )
    ptr<Ast> rewrite_expr_let(const Position &p, const ptrs<Ast> &ll,
                              const ptr<Ast> &r, const ptr<Ast> &b) override {
        auto r0 = rewrite(r);
        auto b0 = rewrite(b);

        auto m = AstExprMatch::create(p, ll, AstEmpty::create(), b0);

        ptrs<Ast> mm;
        mm.push_back(m);
        auto q = AstExprBlock::create(p, mm);

        return AstExprApplication::create(p, q, r0);
    }
};

ptr<Ast> pass_let(const ptr<Ast> &a) {
    RewriteLet let;
    return let.let(a);
}

class RewriteThrow : public Rewrite {
public:
    ptr<Ast> dethrow(const ptr<Ast> &a) {
        return rewrite(a);
    }

    //  F( throw e ) -> ( (System.throw (F e)) )
    ptr<Ast> rewrite_expr_throw(const Position &p, const ptr<Ast> &e) override {
        auto t0 = AstExprCombinator::create(p, STRING_SYSTEM, STRING_THROW);
        auto e0 = rewrite(e);

        return AstExprApplication::create(p, t0, e0);
    }
};

ptr<Ast> pass_throw(const ptr<Ast> &a) {
    RewriteThrow t;
    return t.dethrow(a);
}

class RewriteTry : public Rewrite {
public:
    ptr<Ast> try_to_handle(const ptr<Ast> &a) {
        return rewrite(a);
    }

    //  F( try f catch g ) -> ( (System.handle [_ -> F g] [_ -> F f]) )
    ptr<Ast> rewrite_expr_try(const Position &p, const ptr<Ast> &t,
                              const ptr<Ast> &c) override {
        auto t0 = rewrite(t);
        auto c0 = rewrite(c);
        auto handle =
            AstExprCombinator::create(p, STRING_SYSTEM, STRING_HANDLE);

        auto b0 = AstExprWildcard::create(p, "_");
        ptrs<Ast> bb0;
        bb0.push_back(b0);
        auto t1 = AstExprMatch::create(p, bb0, AstEmpty::create(), t0);
        ptrs<Ast> tt0;
        tt0.push_back(t1);
        auto t2 = AstExprBlock::create(p, tt0);
        auto c1 = AstExprMatch::create(p, bb0, AstEmpty::create(), c0);
        ptrs<Ast> cc0;
        cc0.push_back(c1);
        auto c2 = AstExprBlock::create(p, cc0);

        return AstExprApplication::create(p, handle, c2, t2);
    }
};

ptr<Ast> pass_try(const ptr<Ast> &a) {
    RewriteTry t;
    return t.try_to_handle(a);
}

class RewriteDo : public Rewrite {
public:
    ptr<Ast> do0(const ptr<Ast> &a) {
        _tick = 0;
        return rewrite(a);
    }

    int tick() {
        return _tick++;
    }

    ptr<Ast> fresh_do_var(const Position &p) {
        icu::UnicodeString w = "DOVAR";
        w = w + VM::unicode_from_int(tick());
        return AstExprVariable::create(p, w);
    }

    ptr<Ast> add_var(const ptr<Ast> &e, const ptr<Ast> &v) {
        if (e->tag() == AST_EXPR_APPLICATION) {
            auto [p, ee0] = AstExprApplication::split(e);
            if ((ee0[0]->tag() == AST_EXPR_COMBINATOR) &&
                (ee0[0]->to_text() == "System::|>") && (ee0.size() > 2)) {
                ptrs<Ast> ee1;
                ee1.push_back(ee0[0]);
                ee1.push_back(add_var(ee0[1], v));
                for (unsigned int i = 2; i < ee0.size(); i++) {
                    ee1.push_back(ee0[i]);
                }
                return AstExprApplication::create(e->position(), ee1);
            } else {
                return AstExprApplication::create(e->position(), e, v);
            }
        } else {
            return AstExprApplication::create(e->position(), e, v);
        }
    }

    //  F( do F |> G ) -> ( [X -> (X |> F) |> G] )
    ptr<Ast> rewrite_expr_do(const Position &p, const ptr<Ast> &e) override {
        auto e0 = rewrite(e);

        auto x = fresh_do_var(p);
        auto e1 = add_var(e0, x);
        ptrs<Ast> pp;
        pp.push_back(x);
        return AstExprBlock::create(
            p, AstExprMatch::create(p, pp, AstEmpty::create(), e1));
    }

private:
    int _tick;
};

ptr<Ast> pass_do(const ptr<Ast> &a) {
    RewriteDo d;
    return d.do0(a);
}

class RewriteMonmin : public Rewrite {
public:
    ptr<Ast> monmin(const ptr<Ast> &a) {
        return rewrite(a);
    }

    ptr<Ast> lambdify(const ptr<Ast> &e) {
        auto p = e->position();
        auto v = AstExprVariable::create(p, "WILD0");
        ptrs<Ast> vv;
        vv.push_back(v);
        auto m = AstExprMatch::create(p, vv, AstEmpty::create(), e);
        ptrs<Ast> mm;
        mm.push_back(m);
        return AstExprBlock::create(p, mm);
    }

    // (- x) -> -x
    ptr<Ast> rewrite_expr_application(const Position &p,
                                      const ptrs<Ast> &ee) override {
        if (ee.size() == 2) {
            auto op = ee[0];
            if (op->tag() == AST_EXPR_COMBINATOR) {
                auto s = op->to_text();
                auto arg0 = ee[1];
                if ((s == "System:!-") && (arg0->tag() == AST_EXPR_INTEGER)) {
                    auto [p, s] = AstExprInteger::split(arg0);
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

ptr<Ast> pass_monmin(const ptr<Ast> &a) {
    RewriteMonmin t;
    return t.monmin(a);
}

class RewriteLazyOp : public Rewrite {
public:
    ptr<Ast> lazyop(const ptr<Ast> &a) {
        return rewrite(a);
    }

    ptr<Ast> lambdify(const ptr<Ast> &e) {
        auto p = e->position();
        auto v = AstExprVariable::create(p, "WILD0");
        ptrs<Ast> vv;
        vv.push_back(v);
        auto m = AstExprMatch::create(p, vv, AstEmpty::create(), e);
        ptrs<Ast> mm;
        mm.push_back(m);
        return AstExprBlock::create(p, mm);
    }

    //  e0 || e1 -> e0 || [ _ -> e1 ] and e0 && e1 -> e0 && [ _ -> e1 ]
    ptr<Ast> rewrite_expr_application(const Position &p,
                                      const ptrs<Ast> &ee) override {
        if (ee.size() == 3) {
            auto op = ee[0];
            if (op->tag() == AST_EXPR_COMBINATOR) {
                auto s = op->to_text();
                if ((s == "System:&&") || (s == "System:||")) {
                    auto arg0 = rewrite(ee[1]);
                    auto arg1 = lambdify(rewrite(ee[2]));

                    ptrs<Ast> ff;
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

ptr<Ast> pass_lazyop(const ptr<Ast> &a) {
    RewriteLazyOp t;
    return t.lazyop(a);
}

ptr<Ast> desugar(const ptr<Ast> &a) {
    auto a0 = pass_condition(a);
    a0 = pass_wildcard(a0);
    a0 = pass_tuple(a0);
    a0 = pass_list(a0);
    a0 = pass_do(a0);
    a0 = pass_statement(a0);
    a0 = pass_let(a0);
    a0 = pass_lambda(a0);
    a0 = pass_throw(a0);
    a0 = pass_try(a0);
    a0 = pass_lazyop(a0);
    a0 = pass_monmin(a0);
    a0 = pass_wildcard(a0);
    return a0;
}

}  // namespace egel
