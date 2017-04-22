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
        return l;
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

    //  F( (l = r; b) ) -> ( [ l -> F(b) ] F(r) )
    AstPtr rewrite_expr_let(const Position& p, const AstPtr& l, const AstPtr& r, const AstPtr& b) override {
        auto r0 = rewrite(r);
        auto b0 = rewrite(b);

        AstPtrs gg;
        gg.push_back(l);
        auto m = AstExprMatch(p, gg, AstEmpty().clone(), b0).clone();

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

class RewriteGuard: public Rewrite {
public:
    AstPtr guard(const AstPtr& a) {
        return rewrite(a);
    }

    uint_t tick() {
        uint_t r = newvar;
        newvar++;
        return r;
    }

    AstPtr generate_variable(const Position& p) {
        auto s0 = UnicodeString("_#");
        auto s1 = unicode_convert_uint(tick());
        auto s2 = unicode_concat(s0, s1);

        return AstExprVariable(p, s2).clone();
    }

    AstPtrs generate_variables_for_pattern(const Position& p, const AstPtrs& pp) {
        AstPtrs vv;
        uint_t a = AST_EXPR_MATCH_CAST(pp[0])->arity();
        for (uint_t n = 0; n < a; n++) {
            auto v = generate_variable(p);
            vv.push_back(v);
        }
        return vv;
    }


    bool match_is_guarded(const AstPtr& m) {
        switch (m->tag()) {
        case AST_EXPR_MATCH: {
                auto m0 = AST_EXPR_MATCH_CAST(m);
                if (m0->guard()->tag() != AST_EMPTY) return true;
            }
            break;
        default:
            break;
        }
        return false;
    }

    bool matches_are_guarded(const AstPtrs& mm) {
        for (auto& m: mm) {
            if (match_is_guarded(m)) return true;
        }
        return false;
    }

    /**
     *     F( [ pp0 -> e0 | pp1 ? g -> e1 | pp2 -> e2 ] )
     *   ->
     *     F( [ vv ->
     *          [ pp0 -> e0
     *          | pp1 ->    [ true -> e1
     *                      | _    -> [ pp2 -> e2 ] vv ] g ] vv ] )
     *
     *   The algorithm works from the last to the first match 
     *   constructing a new set of matches.
     *
     *   If the current match is unguarded, it can simply be prepended
     *   to the matches being constructed.
     *
     *   If the current match is guarded, the matches being 
     *   constructed are compiled to a block and wrapped inside the 
     *   body of a match which evaluates the guard.
     **/
    AstPtr rewrite_expr_block(const Position& p, const AstPtrs& mm) override {
        if (matches_are_guarded(mm)) {
            auto vv = generate_variables_for_pattern(p, mm);

            // we're going to create a set of new matches to compile to a block
            AstPtrs mm_new;

            // work from the last to the first match, start with the last
            auto last = mm[mm.size() -1];

            // the last match being inserted is a special case
            if (match_is_guarded(last)) {
                // [ pp -> [ true -> e ] g ] 
                AST_EXPR_MATCH_SPLIT(last, p0, pp, g, e);
                auto t = AstExprCombinator(p0, STRING_SYSTEM, STRING_TRUE).clone();
                AstPtrs tt0;
                tt0.push_back(t);
                auto m0 = AstExprMatch(p, tt0, AstEmpty().clone(), e).clone();

                AstPtrs mm0;
                mm0.push_back(m0);
                auto b0 = AstExprBlock(p, mm0).clone();

                auto a0 = AstExprApplication(p, b0, g).clone();
                auto m1 = AstExprMatch(p, pp, AstEmpty().clone(), a0).clone();
                mm_new.push_back(m1);
            } else {
                // [ pp -> e ]
                mm_new.push_back(last);
            }

            // now process the remaining matches in reverse order
            for (int n = mm.size() - 2; n >= 0; n--) {
                auto m = mm[n];
                if (match_is_guarded(m)) {
                    AST_EXPR_MATCH_SPLIT(m, p0, pp, g, e);
                    // [ pp -> [ true -> e
                    //         | _    -> [ ... ] vv ] g ]

                    // the true match
                    auto t = AstExprCombinator(p0, STRING_SYSTEM, STRING_TRUE).clone();
                    AstPtrs tt0;
                    tt0.push_back(t);
                    auto m0 = AstExprMatch(p, tt0, AstEmpty().clone(), e).clone();

                    // the other match
                    auto b0 = AstExprBlock(p, mm_new).clone();
                    auto a = b0; 
                    for (auto& v: vv) {
                        a = AstExprApplication(p, a, v).clone();
                    };
                    auto v = generate_variable(p);
                    AstPtrs vv0;
                    vv0.push_back(v);
                    auto m1 = AstExprMatch(p, vv0, AstEmpty().clone(), a).clone();

                    // the body which evaluates the guard
                    AstPtrs mm0;
                    mm0.push_back(m0);
                    mm0.push_back(m1);
                    auto b1 = AstExprBlock(p, mm0).clone();
                    auto b2 = AstExprApplication(p, b1, g).clone();
                    
                    // the new match
                    auto m_new = AstExprMatch(p, pp, AstEmpty().clone(), b2).clone();
                    mm_new.clear();
                    mm_new.push_back(m_new);
                } else {
                    // [ pp -> e | ... ]
                    mm_new.insert(mm_new.begin(), m);
                }
            }
            
            // wrap the matches in an eta expansion
            auto a = AstExprBlock(p, mm_new).clone();
            for (auto& v: vv) {
                a = AstExprApplication(p, a, v).clone();
            }
            
            auto m = AstExprMatch(p, vv, AstEmpty().clone(), a).clone();
            AstPtrs mm0;
            mm0.push_back(m);
            // XXX: forgot to call super
            return AstExprBlock(p, mm0).clone();
        } else {
            // XXX: forgot to call super
            return AstExprBlock(p, mm).clone();
        }
    }

private:
    uint_t  newvar = 0;
};

AstPtr pass_guard(const AstPtr& a) {
    RewriteGuard guard;
    return guard.guard(a);
}

AstPtr desugar(const AstPtr& a) {
    auto a0 = pass_condition(a);
    a0 = pass_wildcard(a0);
    a0 = pass_tuple(a0);
    a0 = pass_list(a0);
    a0 = pass_let(a0);
    a0 = pass_lambda(a0);
    a0 = pass_guard(a0);
    return a0;
}
