#pragma once

#include <vector>

#include "ast.hpp"
#include "error.hpp"

class Transform {
public:
    Transform() {
    }

    virtual AstPtrs transforms(const AstPtrs &aa) {
        AstPtrs _aa;
        for (auto &a : aa) {
            AstPtr _a = transform(a);
            _aa.push_back(_a);
        }
        return _aa;
    }

    virtual void transform_pre(const AstPtr &a) {
    }

    virtual AstPtr transform_expr_integer(const AstPtr &a, const Position &p,
                                          const icu::UnicodeString &v) {
        return a;
    }

    virtual AstPtr transform_expr_hexinteger(const AstPtr &a, const Position &p,
                                             const icu::UnicodeString &v) {
        return a;
    }

    virtual AstPtr transform_expr_float(const AstPtr &a, const Position &p,
                                        const icu::UnicodeString &v) {
        return a;
    }

    virtual AstPtr transform_expr_character(const AstPtr &a, const Position &p,
                                            const icu::UnicodeString &v) {
        return a;
    }

    virtual AstPtr transform_expr_text(const AstPtr &a, const Position &p,
                                       const icu::UnicodeString &v) {
        return a;
    }

    virtual AstPtr transform_expr_variable(const AstPtr &a, const Position &p,
                                           const icu::UnicodeString &n) {
        return a;
    }

    virtual AstPtr transform_expr_wildcard(const AstPtr &a, const Position &p,
                                           const icu::UnicodeString &n) {
        return a;
    }

    virtual AstPtr transform_expr_combinator(const AstPtr &a, const Position &p,
                                             const UnicodeStrings &nn,
                                             const icu::UnicodeString &n) {
        return a;
    }

    virtual AstPtr transform_expr_operator(const AstPtr &a, const Position &p,
                                           const UnicodeStrings &nn,
                                           const icu::UnicodeString &n) {
        return a;
    }

    virtual AstPtr transform_expr_tuple(const AstPtr &a, const Position &p,
                                        const AstPtrs &tt) {
        auto tt0 = transforms(tt);
        return AstExprTuple::create(p, tt0);
    }

    virtual AstPtr transform_expr_list(const AstPtr &a, const Position &p,
                                       const AstPtrs &tt, const AstPtr &tl) {
        auto tt0 = transforms(tt);
        if (tl == nullptr) {
            return AstExprList::create(p, tt0);
        } else {
            auto tl0 = transform(tl);
            return AstExprList::create(p, tt0, tl0);
        }
    }

    virtual AstPtr transform_expr_application(const AstPtr &a,
                                              const Position &p,
                                              const AstPtrs &tt) {
        auto tt0 = transforms(tt);
        return AstExprApplication::create(p, tt0);
    }

    virtual AstPtr transform_expr_lambda(const AstPtr &a, const Position &p,
                                         const AstPtr &m) {
        auto m0 = transform(m);
        return AstExprLambda::create(p, m0);
    }

    virtual AstPtr transform_expr_match(const AstPtr &a, const Position &p,
                                        const AstPtrs &mm, const AstPtr &g,
                                        const AstPtr &e) {
        auto mm0 = transforms(mm);
        auto g0 = transform(g);
        auto e0 = transform(e);
        return AstExprMatch::create(p, mm0, g0, e0);
    }

    virtual AstPtr transform_expr_block(const AstPtr &a, const Position &p,
                                        const AstPtrs &alts) {
        auto alts0 = transforms(alts);
        return AstExprBlock::create(p, alts0);
    }

    virtual AstPtr transform_expr_let(const AstPtr &a, const Position &p,
                                      const AstPtrs &lhs, const AstPtr &rhs,
                                      const AstPtr &body) {
        auto lhs0 = transforms(lhs);
        auto rhs0 = transform(rhs);
        auto body0 = transform(body);
        return AstExprLet::create(p, lhs0, rhs0, body0);
    }

    virtual AstPtr transform_expr_tag(const AstPtr &a, const Position &p,
                                      const AstPtr &e, const AstPtr &t) {
        auto e0 = transform(e);
        auto t0 = transform(t);
        return AstExprTag::create(p, e0, t0);
    }

    virtual AstPtr transform_expr_if(const AstPtr &a, const Position &p,
                                     const AstPtr &i, const AstPtr &t,
                                     const AstPtr &e) {
        auto i0 = transform(i);
        auto t0 = transform(t);
        auto e0 = transform(e);
        return AstExprIf::create(p, i0, t0, e0);
    }

    virtual AstPtr transform_expr_statement(const AstPtr &a, const Position &p,
                                            const AstPtr &l, const AstPtr &r) {
        auto r0 = transform(r);
        auto l0 = transform(l);
        return AstExprStatement::create(p, r0, l0);
    }

    virtual AstPtr transform_expr_try(const AstPtr &a, const Position &p,
                                      const AstPtr &t, const AstPtr &c) {
        auto t0 = transform(t);
        auto c0 = transform(c);
        return AstExprTry::create(p, t0, c0);
    }

    virtual AstPtr transform_expr_throw(const AstPtr &a, const Position &p,
                                        const AstPtr &e) {
        auto e0 = transform(e);
        return AstExprThrow::create(p, e0);
    }

    virtual AstPtr transform_directive_import(const AstPtr &a,
                                              const Position &p,
                                              const icu::UnicodeString &i) {
        return a;
    }

    virtual AstPtr transform_directive_using(const AstPtr &a, const Position &p,
                                             const UnicodeStrings &uu) {
        return a;
    }

    virtual AstPtr transform_decl_data(const AstPtr &a, const Position &p,
                                       const AstPtrs &nn) {
        auto nn0 = transforms(nn);
        return AstDeclData::create(p, nn0);
    }

    virtual AstPtr transform_decl_definition(const AstPtr &a, const Position &p,
                                             const AstPtr &n, const AstPtr &e) {
        auto n0 = transform(n);
        auto e0 = transform(e);
        return AstDeclDefinition::create(p, n0, e0);
    }

    virtual AstPtr transform_decl_value(const AstPtr &a, const Position &p,
                                        const AstPtr &l, const AstPtr &r) {
        auto l0 = transform(l);
        auto r0 = transform(r);
        return AstDeclValue::create(p, l0, r0);
    }

    virtual AstPtr transform_decl_operator(const AstPtr &a, const Position &p,
                                           const AstPtr &c, const AstPtr &e) {
        auto c0 = transform(c);
        auto e0 = transform(e);
        return AstDeclOperator::create(p, c0, e0);
    }

    virtual AstPtr transform_decl_object(const AstPtr &a, const Position &p,
                                         const AstPtr &c, const AstPtrs &vv,
                                         const AstPtrs &ff, const AstPtrs &ee) {
        auto c0 = transform(c);
        auto vv0 = transforms(vv);
        auto ff0 = transforms(ff);
        auto ee0 = transforms(ee);
        return AstDeclObject::create(p, c0, vv0, ff0, ee0);
    }

    virtual AstPtr transform_decl_namespace(const AstPtr &a, const Position &p,
                                            const UnicodeStrings &nn,
                                            const AstPtrs &dd) {
        auto dd0 = transforms(dd);
        return AstDeclNamespace::create(p, nn, dd0);
    }

    virtual AstPtr transform_wrapper(const AstPtr &a, const Position &p,
                                     const AstPtrs &dd) {
        auto dd0 = transforms(dd);
        return AstWrapper::create(p, dd0);
    }

    AstPtr transform(const AstPtr &a) {
        transform_pre(a);

        switch (a->tag()) {
            case AST_EMPTY: {
                return a;
                break;
            }
            // literals
            case AST_EXPR_INTEGER: {
                AST_EXPR_INTEGER_SPLIT(a, p, t);
                return transform_expr_integer(a, p, t);
                break;
            }
            case AST_EXPR_HEXINTEGER: {
                AST_EXPR_HEXINTEGER_SPLIT(a, p, t);
                return transform_expr_hexinteger(a, p, t);
                break;
            }
            case AST_EXPR_FLOAT: {
                AST_EXPR_FLOAT_SPLIT(a, p, t);
                return transform_expr_float(a, p, t);
                break;
            }
            case AST_EXPR_CHARACTER: {
                AST_EXPR_CHARACTER_SPLIT(a, p, t);
                return transform_expr_character(a, p, t);
                break;
            }
            case AST_EXPR_TEXT: {
                AST_EXPR_TEXT_SPLIT(a, p, t);
                return transform_expr_text(a, p, t);
                break;
            }
            // variables and constants
            case AST_EXPR_VARIABLE: {
                AST_EXPR_VARIABLE_SPLIT(a, p, t);
                return transform_expr_variable(a, p, t);
                break;
            }
            case AST_EXPR_WILDCARD: {
                AST_EXPR_WILDCARD_SPLIT(a, p, t);
                return transform_expr_wildcard(a, p, t);
                break;
            }
            case AST_EXPR_COMBINATOR: {
                AST_EXPR_COMBINATOR_SPLIT(a, p, tt, t);
                return transform_expr_combinator(a, p, tt, t);
                break;
            }
            case AST_EXPR_OPERATOR: {
                AST_EXPR_OPERATOR_SPLIT(a, p, tt, t);
                return transform_expr_operator(a, p, tt, t);
                break;
            }
            // tuple
            case AST_EXPR_TUPLE: {
                AST_EXPR_TUPLE_SPLIT(a, p, tt);
                return transform_expr_tuple(a, p, tt);
                break;
            }
            case AST_EXPR_LIST: {
                AST_EXPR_LIST_SPLIT(a, p, tt, tl);
                return transform_expr_list(a, p, tt, tl);
                break;
            }
            // compound statements
            case AST_EXPR_APPLICATION: {
                AST_EXPR_APPLICATION_SPLIT(a, p, ee);
                return transform_expr_application(a, p, ee);
                break;
            }
            case AST_EXPR_LAMBDA: {
                AST_EXPR_LAMBDA_SPLIT(a, p, m);
                return transform_expr_lambda(a, p, m);
                break;
            }
            case AST_EXPR_BLOCK: {
                AST_EXPR_BLOCK_SPLIT(a, p, mm);
                return transform_expr_block(a, p, mm);
                break;
            }
            case AST_EXPR_MATCH: {
                AST_EXPR_MATCH_SPLIT(a, p, mm, g, e);
                return transform_expr_match(a, p, mm, g, e);
                break;
            }
            case AST_EXPR_LET: {
                AST_EXPR_LET_SPLIT(a, p, l, r, e);
                return transform_expr_let(a, p, l, r, e);
                break;
            }
            case AST_EXPR_TAG: {
                AST_EXPR_TAG_SPLIT(a, p, e, t);
                return transform_expr_tag(a, p, e, t);
                break;
            }
            case AST_EXPR_IF: {
                AST_EXPR_IF_SPLIT(a, p, i, t, e);
                return transform_expr_if(a, p, i, t, e);
                break;
            }
            case AST_EXPR_STATEMENT: {
                AST_EXPR_STATEMENT_SPLIT(a, p, r, l);
                return transform_expr_statement(a, p, r, l);
                break;
            }
            case AST_EXPR_TRY: {
                AST_EXPR_TRY_SPLIT(a, p, t, c);
                return transform_expr_try(a, p, t, c);
                break;
            }
            case AST_EXPR_THROW: {
                AST_EXPR_THROW_SPLIT(a, p, exc);
                return transform_expr_throw(a, p, exc);
                break;
            }
            // directives
            case AST_DIRECT_IMPORT: {
                AST_DIRECT_IMPORT_SPLIT(a, p, i);
                return transform_directive_import(a, p, i);
                break;
            }
            case AST_DIRECT_USING: {
                AST_DIRECT_USING_SPLIT(a, p, uu);
                return transform_directive_using(a, p, uu);
                break;
            }
            // declarations
            case AST_DECL_DATA: {
                AST_DECL_DATA_SPLIT(a, p, nn);
                return transform_decl_data(a, p, nn);
                break;
            }
            case AST_DECL_DEFINITION: {
                AST_DECL_DEFINITION_SPLIT(a, p, n, e);
                return transform_decl_definition(a, p, n, e);
                break;
            }
            case AST_DECL_VALUE: {
                AST_DECL_VALUE_SPLIT(a, p, l, r);
                return transform_decl_value(a, p, l, r);
            }
            case AST_DECL_OPERATOR: {
                AST_DECL_OPERATOR_SPLIT(a, p, c, e);
                return transform_decl_operator(a, p, c, e);
                break;
            }
            case AST_DECL_OBJECT: {
                AST_DECL_OBJECT_SPLIT(a, p, c, vv, ff, ee);
                return transform_decl_object(a, p, c, vv, ff, ee);
                break;
            }
            case AST_DECL_NAMESPACE: {
                AST_DECL_NAMESPACE_SPLIT(a, p, nn, dd);
                return transform_decl_namespace(a, p, nn, dd);
                break;
            }
            // wrapper
            case AST_WRAPPER: {
                AST_WRAPPER_SPLIT(a, p, dd);
                return transform_wrapper(a, p, dd);
            }
            default:
                PANIC("transform exhausted");
                return nullptr;
        }
    }
};

class Rewrite {
public:
    Rewrite() {
    }

    virtual AstPtrs rewrites(const AstPtrs &aa) {
        AstPtrs _aa;
        for (auto &a : aa) {
            AstPtr _a = rewrite(a);
            _aa.push_back(_a);
        }
        return _aa;
    }

    virtual void rewrite_pre(const AstPtr &a) {
    }

    // literals
    virtual AstPtr rewrite_expr_integer(const Position &p,
                                        const icu::UnicodeString &v) {
        return AstExprInteger::create(p, v);
    }

    virtual AstPtr rewrite_expr_hexinteger(const Position &p,
                                           const icu::UnicodeString &v) {
        return AstExprHexInteger::create(p, v);
    }

    virtual AstPtr rewrite_expr_float(const Position &p,
                                      const icu::UnicodeString &v) {
        return AstExprFloat::create(p, v);
    }

    virtual AstPtr rewrite_expr_character(const Position &p,
                                          const icu::UnicodeString &v) {
        return AstExprCharacter::create(p, v);
    }

    virtual AstPtr rewrite_expr_text(const Position &p,
                                     const icu::UnicodeString &v) {
        return AstExprText::create(p, v);
    }

    // variables and constants
    virtual AstPtr rewrite_expr_variable(const Position &p,
                                         const icu::UnicodeString &n) {
        return AstExprVariable::create(p, n);
    }

    virtual AstPtr rewrite_expr_wildcard(const Position &p,
                                         const icu::UnicodeString &n) {
        return AstExprWildcard::create(p, n);
    }

    virtual AstPtr rewrite_expr_combinator(const Position &p,
                                           const UnicodeStrings &nn,
                                           const icu::UnicodeString &n) {
        return AstExprCombinator::create(p, nn, n);
    }

    virtual AstPtr rewrite_expr_operator(const Position &p,
                                         const UnicodeStrings &nn,
                                         const icu::UnicodeString &n) {
        return AstExprOperator::create(p, nn, n);
    }

    // tuple and list
    virtual AstPtr rewrite_expr_tuple(const Position &p, const AstPtrs &tt) {
        auto tt0 = rewrites(tt);
        return AstExprTuple::create(p, tt0);
    }

    virtual AstPtr rewrite_expr_list(const Position &p, const AstPtrs &tt,
                                     const AstPtr &tl) {
        auto tt0 = rewrites(tt);
        if (tl == nullptr) {
            return AstExprList::create(p, tt0);
        } else {
            auto tl0 = rewrite(tl);
            return AstExprList::create(p, tt0, tl0);
        }
    }

    // compound statements
    virtual AstPtr rewrite_expr_application(const Position &p,
                                            const AstPtrs &aa) {
        auto aa0 = rewrites(aa);
        return AstExprApplication::create(p, aa0);
    }

    virtual AstPtr rewrite_expr_lambda(const Position &p, const AstPtr &m) {
        auto m0 = rewrite(m);
        return AstExprLambda::create(p, m0);
    }

    virtual AstPtr rewrite_expr_match(const Position &p, const AstPtrs &mm,
                                      const AstPtr &g, const AstPtr &e) {
        auto mm0 = rewrites(mm);
        auto g0 = rewrite(g);
        auto e0 = rewrite(e);
        return AstExprMatch::create(p, mm0, g0, e0);
    }

    virtual AstPtr rewrite_expr_block(const Position &p, const AstPtrs &alts) {
        auto alts0 = rewrites(alts);
        return AstExprBlock::create(p, alts0);
    }

    virtual AstPtr rewrite_expr_let(const Position &p, const AstPtrs &lhs,
                                    const AstPtr &rhs, const AstPtr &body) {
        auto lhs0 = rewrites(lhs);
        auto rhs0 = rewrite(rhs);
        auto body0 = rewrite(body);
        return AstExprLet::create(p, lhs0, rhs0, body0);
    }

    virtual AstPtr rewrite_expr_tag(const Position &p, const AstPtr &e,
                                    const AstPtr &t) {
        auto e0 = rewrite(e);
        auto t0 = rewrite(t);
        return AstExprTag::create(p, e0, t0);
    }

    virtual AstPtr rewrite_expr_if(const Position &p, const AstPtr &i,
                                   const AstPtr &t, const AstPtr &e) {
        auto i0 = rewrite(i);
        auto t0 = rewrite(t);
        auto e0 = rewrite(e);
        return AstExprIf::create(p, i0, t0, e0);
    }

    virtual AstPtr rewrite_expr_statement(const Position &p, const AstPtr &r,
                                          const AstPtr &l) {
        auto r0 = rewrite(r);
        auto l0 = rewrite(l);
        return AstExprStatement::create(p, r0, l0);
    }

    virtual AstPtr rewrite_expr_try(const Position &p, const AstPtr &t,
                                    const AstPtr &c) {
        auto t0 = rewrite(t);
        auto c0 = rewrite(c);
        return AstExprTry::create(p, t0, c0);
    }

    virtual AstPtr rewrite_expr_throw(const Position &p, const AstPtr &e) {
        auto e0 = rewrite(e);
        return AstExprThrow::create(p, e0);
    }

    virtual AstPtr rewrite_directive_import(const Position &p,
                                            const icu::UnicodeString &i) {
        return AstDirectImport::create(p, i);
    }

    virtual AstPtr rewrite_directive_using(const Position &p,
                                           const UnicodeStrings &nn) {
        return AstDirectUsing::create(p, nn);
    }

    virtual AstPtr rewrite_decl_data(const Position &p, const AstPtrs &nn) {
        auto nn0 = rewrites(nn);
        return AstDeclData::create(p, nn0);
    }

    virtual AstPtr rewrite_decl_definition(const Position &p, const AstPtr &n,
                                           const AstPtr &e) {
        auto n0 = rewrite(n);
        auto e0 = rewrite(e);
        return AstDeclDefinition::create(p, n0, e0);
    }

    virtual AstPtr rewrite_decl_value(const Position &p, const AstPtr &l,
                                      const AstPtr &r) {
        auto l0 = rewrite(l);
        auto r0 = rewrite(r);
        return AstDeclValue::create(p, l0, r0);
    }

    virtual AstPtr rewrite_decl_operator(const Position &p, const AstPtr &c,
                                         const AstPtr &e) {
        auto c0 = rewrite(c);
        auto e0 = rewrite(e);
        return AstDeclOperator::create(p, c0, e0);
    }

    virtual AstPtr rewrite_decl_object(const Position &p, const AstPtr &c,
                                       const AstPtrs &vv, const AstPtrs &ff,
                                       const AstPtrs &ee) {
        auto c0 = rewrite(c);
        auto vv0 = rewrites(vv);
        auto ff0 = rewrites(ff);
        auto ee0 = rewrites(ee);
        return AstDeclObject::create(p, c0, vv0, ff0, ee0);
    }

    virtual AstPtr rewrite_decl_namespace(const Position &p,
                                          const UnicodeStrings &nn,
                                          const AstPtrs &dd) {
        auto dd0 = rewrites(dd);
        return AstDeclNamespace::create(p, nn, dd0);
    }

    virtual AstPtr rewrite_wrapper(const Position &p, const AstPtrs &dd) {
        auto dd0 = rewrites(dd);
        return AstWrapper::create(p, dd0);
    }

    virtual AstPtr rewrite(const AstPtr &a) {
        rewrite_pre(a);

        switch (a->tag()) {
            case AST_EMPTY: {
                return a;
                break;
            }
            // literals
            case AST_EXPR_INTEGER: {
                AST_EXPR_INTEGER_SPLIT(a, p, t);
                return rewrite_expr_integer(p, t);
                break;
            }
            case AST_EXPR_HEXINTEGER: {
                AST_EXPR_HEXINTEGER_SPLIT(a, p, t);
                return rewrite_expr_hexinteger(p, t);
                break;
            }
            case AST_EXPR_FLOAT: {
                AST_EXPR_FLOAT_SPLIT(a, p, t);
                return rewrite_expr_float(p, t);
                break;
            }
            case AST_EXPR_CHARACTER: {
                AST_EXPR_CHARACTER_SPLIT(a, p, t);
                return rewrite_expr_character(p, t);
                break;
            }
            case AST_EXPR_TEXT: {
                AST_EXPR_TEXT_SPLIT(a, p, t);
                return rewrite_expr_text(p, t);
                break;
            }
            // variables and constants
            case AST_EXPR_VARIABLE: {
                AST_EXPR_VARIABLE_SPLIT(a, p, t);
                return rewrite_expr_variable(p, t);
                break;
            }
            case AST_EXPR_WILDCARD: {
                AST_EXPR_WILDCARD_SPLIT(a, p, t);
                return rewrite_expr_wildcard(p, t);
                break;
            }
            case AST_EXPR_COMBINATOR: {
                AST_EXPR_COMBINATOR_SPLIT(a, p, nn, n);
                return rewrite_expr_combinator(p, nn, n);
                break;
            }
            case AST_EXPR_OPERATOR: {
                AST_EXPR_OPERATOR_SPLIT(a, p, nn, n);
                return rewrite_expr_operator(p, nn, n);
                break;
            }
            // tuple and list
            case AST_EXPR_TUPLE: {
                AST_EXPR_TUPLE_SPLIT(a, p, tt);
                return rewrite_expr_tuple(p, tt);
                break;
            }
            case AST_EXPR_LIST: {
                AST_EXPR_LIST_SPLIT(a, p, tt, tl);
                return rewrite_expr_list(p, tt, tl);
                break;
            }
            // compound statements
            case AST_EXPR_APPLICATION: {
                AST_EXPR_APPLICATION_SPLIT(a, p, aa);
                return rewrite_expr_application(p, aa);
                break;
            }
            case AST_EXPR_LAMBDA: {
                AST_EXPR_LAMBDA_SPLIT(a, p, m);
                return rewrite_expr_lambda(p, m);
                break;
            }
            case AST_EXPR_BLOCK: {
                AST_EXPR_BLOCK_SPLIT(a, p, mm);
                return rewrite_expr_block(p, mm);
                break;
            }
            case AST_EXPR_MATCH: {
                AST_EXPR_MATCH_SPLIT(a, p, mm, g, e);
                return rewrite_expr_match(p, mm, g, e);
                break;
            }
            case AST_EXPR_LET: {
                AST_EXPR_LET_SPLIT(a, p, l, r, e);
                return rewrite_expr_let(p, l, r, e);
                break;
            }
            case AST_EXPR_TAG: {
                AST_EXPR_TAG_SPLIT(a, p, e, t);
                return rewrite_expr_tag(p, e, t);
                break;
            }
            case AST_EXPR_IF: {
                AST_EXPR_IF_SPLIT(a, p, i, t, e);
                return rewrite_expr_if(p, i, t, e);
                break;
            }
            case AST_EXPR_STATEMENT: {
                AST_EXPR_STATEMENT_SPLIT(a, p, r, l);
                return rewrite_expr_statement(p, r, l);
                break;
            }
            case AST_EXPR_TRY: {
                AST_EXPR_TRY_SPLIT(a, p, t, c);
                return rewrite_expr_try(p, t, c);
                break;
            }
            case AST_EXPR_THROW: {
                AST_EXPR_THROW_SPLIT(a, p, exc);
                return rewrite_expr_throw(p, exc);
                break;
            }
            // directives
            case AST_DIRECT_IMPORT: {
                AST_DIRECT_IMPORT_SPLIT(a, p, i);
                return rewrite_directive_import(p, i);
                break;
            }
            case AST_DIRECT_USING: {
                AST_DIRECT_USING_SPLIT(a, p, i);
                return rewrite_directive_using(p, i);
                break;
            }
            // declarations
            case AST_DECL_DATA: {
                AST_DECL_DATA_SPLIT(a, p, nn);
                return rewrite_decl_data(p, nn);
                break;
            }
            case AST_DECL_DEFINITION: {
                AST_DECL_DEFINITION_SPLIT(a, p, n, e);
                return rewrite_decl_definition(p, n, e);
                break;
            }
            case AST_DECL_VALUE: {
                AST_DECL_VALUE_SPLIT(a, p, l, r);
                return rewrite_decl_value(p, l, r);
            }
            case AST_DECL_OPERATOR: {
                AST_DECL_OPERATOR_SPLIT(a, p, c, e);
                return rewrite_decl_operator(p, c, e);
                break;
            }
            case AST_DECL_OBJECT: {
                AST_DECL_OBJECT_SPLIT(a, p, c, vv, ff, ee);
                return rewrite_decl_object(p, c, vv, ff, ee);
                break;
            }
            case AST_DECL_NAMESPACE: {
                AST_DECL_NAMESPACE_SPLIT(a, p, nn, dd);
                return rewrite_decl_namespace(p, nn, dd);
                break;
            }
            // wrapper
            case AST_WRAPPER: {
                AST_WRAPPER_SPLIT(a, p, dd);
                return rewrite_wrapper(p, dd);
            }
            default:
                PANIC("rewrite exhausted");
                return nullptr;
        }
    }
};

class Visit {
public:
    Visit() {
    }

    virtual void visit_pre(const AstPtr &a) {
    }

    void visits(const AstPtrs &dd) {
        for (auto &d : dd) {
            visit(d);
        }
    }

    virtual void visit_expr_integer(const Position &p,
                                    const icu::UnicodeString &v) {
    }

    virtual void visit_expr_hexinteger(const Position &p,
                                       const icu::UnicodeString &v) {
    }

    virtual void visit_expr_float(const Position &p,
                                  const icu::UnicodeString &v) {
    }

    virtual void visit_expr_character(const Position &p,
                                      const icu::UnicodeString &v) {
    }

    virtual void visit_expr_text(const Position &p,
                                 const icu::UnicodeString &v) {
    }

    virtual void visit_expr_variable(const Position &p,
                                     const icu::UnicodeString &n) {
    }

    virtual void visit_expr_wildcard(const Position &p,
                                     const icu::UnicodeString &n) {
    }

    virtual void visit_expr_combinator(const Position &p,
                                       const UnicodeStrings &nn,
                                       const icu::UnicodeString &n) {
    }

    virtual void visit_expr_operator(const Position &p,
                                     const UnicodeStrings &nn,
                                     const icu::UnicodeString &n) {
    }

    virtual void visit_expr_tuple(const Position &p, const AstPtrs &tt) {
        visits(tt);
    }

    virtual void visit_expr_list(const Position &p, const AstPtrs &tt,
                                 const AstPtr &tl) {
        if (tl == nullptr) {
            visits(tt);
        } else {
            visits(tt);
            visit(tl);
        }
    }

    virtual void visit_expr_application(const Position &p, const AstPtrs &tt) {
        visits(tt);
    }

    virtual void visit_expr_lambda(const Position &p, const AstPtr &m) {
        visit(m);
    }

    virtual void visit_expr_match(const Position &p, const AstPtrs &mm,
                                  const AstPtr &g, const AstPtr &e) {
        visits(mm);
        visit(g);
        visit(e);
    }

    virtual void visit_expr_block(const Position &p, const AstPtrs &alts) {
        visits(alts);
    }

    virtual void visit_expr_let(const Position &p, const AstPtrs &lhs,
                                const AstPtr &rhs, const AstPtr &body) {
        visits(lhs);
        visit(rhs);
        visit(body);
    }

    virtual void visit_expr_tag(const Position &p, const AstPtr &e,
                                const AstPtr &t) {
        visit(e);
        visit(t);
    }

    virtual void visit_expr_if(const Position &p, const AstPtr &i,
                               const AstPtr &t, const AstPtr &e) {
        visit(i);
        visit(t);
        visit(e);
    }

    virtual void visit_expr_statement(const Position &p, const AstPtr &r,
                                      const AstPtr &l) {
        visit(r);
        visit(l);
    }

    virtual void visit_expr_try(const Position &p, const AstPtr &t,
                                const AstPtr &c) {
        visit(t);
        visit(c);
    }

    virtual void visit_expr_throw(const Position &p, const AstPtr &e) {
        visit(e);
    }

    virtual void visit_directive_import(const Position &p,
                                        const icu::UnicodeString &i) {
    }

    virtual void visit_directive_using(const Position &p,
                                       const UnicodeStrings &nn) {
    }

    virtual void visit_decl_data(const Position &p, const AstPtrs &nn) {
        visits(nn);
    }

    virtual void visit_decl_definition(const Position &p, const AstPtr &n,
                                       const AstPtr &e) {
        visit(n);
        visit(e);
    }

    virtual void visit_decl_value(const Position &p, const AstPtr &l,
                                  const AstPtr &r) {
        visit(l);
        visit(r);
    }

    virtual void visit_decl_operator(const Position &p, const AstPtr &c,
                                     const AstPtr &e) {
        visit(c);
        visit(e);
    }

    virtual void visit_decl_object(const Position &p, const AstPtr &c,
                                   const AstPtrs &vv, const AstPtrs &ff,
                                   const AstPtrs &ee) {
        visit(c);
        visits(vv);
        visits(ff);
        visits(ee);
    }

    virtual void visit_decl_namespace(const Position &p,
                                      const UnicodeStrings &nn,
                                      const AstPtrs &dd) {
        visits(dd);
    }

    virtual void visit_wrapper(const Position &p, const AstPtrs &dd) {
        visits(dd);
    }

    virtual void visit(const AstPtr &a) {
        visit_pre(a);

        switch (a->tag()) {
            case AST_EMPTY: {
                return;
                break;
            }
            // literals
            case AST_EXPR_INTEGER: {
                AST_EXPR_INTEGER_SPLIT(a, p, t);
                return visit_expr_integer(p, t);
                break;
            }
            case AST_EXPR_HEXINTEGER: {
                AST_EXPR_HEXINTEGER_SPLIT(a, p, t);
                return visit_expr_hexinteger(p, t);
                break;
            }
            case AST_EXPR_FLOAT: {
                AST_EXPR_FLOAT_SPLIT(a, p, t);
                return visit_expr_float(p, t);
                break;
            }
            case AST_EXPR_CHARACTER: {
                AST_EXPR_CHARACTER_SPLIT(a, p, t);
                return visit_expr_character(p, t);
                break;
            }
            case AST_EXPR_TEXT: {
                AST_EXPR_TEXT_SPLIT(a, p, t);
                return visit_expr_text(p, t);
                break;
            }
            // variables and constants
            case AST_EXPR_VARIABLE: {
                AST_EXPR_VARIABLE_SPLIT(a, p, t);
                return visit_expr_variable(p, t);
                break;
            }
            case AST_EXPR_WILDCARD: {
                AST_EXPR_WILDCARD_SPLIT(a, p, t);
                return visit_expr_wildcard(p, t);
                break;
            }
            case AST_EXPR_COMBINATOR: {
                AST_EXPR_COMBINATOR_SPLIT(a, p, nn, n);
                return visit_expr_combinator(p, nn, n);
                break;
            }
            case AST_EXPR_OPERATOR: {
                AST_EXPR_OPERATOR_SPLIT(a, p, nn, n);
                return visit_expr_operator(p, nn, n);
                break;
            }
            // tuple
            case AST_EXPR_TUPLE: {
                AST_EXPR_TUPLE_SPLIT(a, p, tt);
                return visit_expr_tuple(p, tt);
                break;
            }
            case AST_EXPR_LIST: {
                AST_EXPR_LIST_SPLIT(a, p, tt, tl);
                return visit_expr_list(p, tt, tl);
                break;
            }
            // compound statements
            case AST_EXPR_APPLICATION: {
                AST_EXPR_APPLICATION_SPLIT(a, p, tt);
                return visit_expr_application(p, tt);
                break;
            }
            case AST_EXPR_LAMBDA: {
                AST_EXPR_LAMBDA_SPLIT(a, p, m);
                return visit_expr_lambda(p, m);
                break;
            }
            case AST_EXPR_BLOCK: {
                AST_EXPR_BLOCK_SPLIT(a, p, mm);
                return visit_expr_block(p, mm);
                break;
            }
            case AST_EXPR_MATCH: {
                AST_EXPR_MATCH_SPLIT(a, p, mm, g, e);
                return visit_expr_match(p, mm, g, e);
                break;
            }
            case AST_EXPR_LET: {
                AST_EXPR_LET_SPLIT(a, p, l, r, e);
                return visit_expr_let(p, l, r, e);
                break;
            }
            case AST_EXPR_TAG: {
                AST_EXPR_TAG_SPLIT(a, p, e, t);
                return visit_expr_tag(p, e, t);
                break;
            }
            case AST_EXPR_IF: {
                AST_EXPR_IF_SPLIT(a, p, i, t, e);
                return visit_expr_if(p, i, t, e);
                break;
            }
            case AST_EXPR_STATEMENT: {
                AST_EXPR_STATEMENT_SPLIT(a, p, r, l);
                return visit_expr_statement(p, r, l);
                break;
            }
            case AST_EXPR_TRY: {
                AST_EXPR_TRY_SPLIT(a, p, t, c);
                return visit_expr_try(p, t, c);
                break;
            }
            case AST_EXPR_THROW: {
                AST_EXPR_THROW_SPLIT(a, p, exc);
                return visit_expr_throw(p, exc);
                break;
            }
            // directives
            case AST_DIRECT_IMPORT: {
                AST_DIRECT_IMPORT_SPLIT(a, p, i);
                return visit_directive_import(p, i);
                break;
            }
            case AST_DIRECT_USING: {
                AST_DIRECT_USING_SPLIT(a, p, uu);
                return visit_directive_using(p, uu);
                break;
            }
            // declarations
            case AST_DECL_DATA: {
                AST_DECL_DATA_SPLIT(a, p, nn);
                return visit_decl_data(p, nn);
                break;
            }
            case AST_DECL_DEFINITION: {
                AST_DECL_DEFINITION_SPLIT(a, p, n, e);
                return visit_decl_definition(p, n, e);
                break;
            }
            case AST_DECL_VALUE: {
                AST_DECL_VALUE_SPLIT(a, p, r, l);
                return visit_decl_value(p, r, l);
                break;
            }
            case AST_DECL_OPERATOR: {
                AST_DECL_OPERATOR_SPLIT(a, p, c, e);
                return visit_decl_operator(p, c, e);
                break;
            }
            case AST_DECL_OBJECT: {
                AST_DECL_OBJECT_SPLIT(a, p, c, vv, ff, ee);
                return visit_decl_object(p, c, vv, ff, ee);
                break;
            }
            case AST_DECL_NAMESPACE: {
                AST_DECL_NAMESPACE_SPLIT(a, p, nn, dd);
                return visit_decl_namespace(p, nn, dd);
                break;
            }
            // wrapper
            case AST_WRAPPER: {
                AST_WRAPPER_SPLIT(a, p, dd);
                return visit_wrapper(p, dd);
                break;
            }
            default:
                PANIC("visit exhausted");
        }
    }
};

class Occurs : public Visit {
public:
    bool occurs(const AstPtr &t0, const AstPtr &t1) {
        _term = t0;
        _found = false;
        visit(t1);
        return _found;
    }

    void visit(const AstPtr &t) override {
        if (_found) {
            return;
        } else if (_term == t) {
            _found = true;
            return;
        } else {
            Visit::visit(t);
        }
    }

private:
    AstPtr _term;
    bool _found;
};

inline bool occurs(const AstPtr &t0, const AstPtr &t1) {
    Occurs occurs;
    return occurs.occurs(t0, t1);
}

class Substitute : public Rewrite {
public:
    AstPtr substitute(const AstPtr &term, const AstPtr &s0, const AstPtr &s1) {
        _source = s0;
        _target = s1;
        return rewrite(term);
    }

    AstPtr rewrite_expr_match(const Position &p, const AstPtrs &mm,
                              const AstPtr &g, const AstPtr &e) override {
        for (auto m : mm) {
            if (occurs(_source, m)) {
                return AstExprMatch::create(p, mm, g, e);
            }
        }
        auto g0 = rewrite(g);
        auto e0 = rewrite(e);
        return AstExprMatch::create(p, mm, g0, e0);
    }

    AstPtr rewrite_expr_let(const Position &p, const AstPtrs &lhs,
                            const AstPtr &rhs, const AstPtr &body) override {
        for (auto m : lhs) {
            if (occurs(_source, m)) {
                return AstExprLet::create(p, lhs, rhs, body);
            }
        }
        auto rhs0 = rewrite(rhs);
        auto body0 = rewrite(body);
        return AstExprLet::create(p, lhs, rhs0, body0);
    }

    AstPtr rewrite(const AstPtr &a) override {
        if (a == _source) {
            return _target;
        } else {
            return Rewrite::rewrite(a);
        }
    }

private:
    AstPtr _source;
    AstPtr _target;
};

inline AstPtr substitute(const AstPtr &term, const AstPtr &s0, const AstPtr &s1) {
    Substitute subs;
    return subs.substitute(term, s0, s1);
}

enum freevars_state_t {
    FREEVARS_INSERT,
    FREEVARS_REMOVE,
};

class FreeVars : public Visit {
public:
    AstPtrSet freevars(const AstPtr &a) {
        set_state(FREEVARS_INSERT);
        visit(a);
        return _fv;
    }

    void set_state(freevars_state_t s) {
        _state = s;
    }

    freevars_state_t get_state() {
        return _state;
    }

    void insert(const AstPtr &a) {
        _fv.insert(a);
    }

    void remove(const AstPtr &a) {
        _fv.erase(a);
    }

    void visit_expr_variable(const Position &p,
                             const icu::UnicodeString &n) override {
        switch (get_state()) {
            case FREEVARS_INSERT:
                insert(AstExprVariable::create(p, n));
                break;
            case FREEVARS_REMOVE:
                remove(AstExprVariable::create(p, n));
                break;
        }
    }

    void visit_expr_match(const Position &p, const AstPtrs &mm, const AstPtr &g,
                          const AstPtr &e) override {
        visit(g);
        visit(e);
        set_state(FREEVARS_REMOVE);
        visits(mm);
        set_state(FREEVARS_INSERT);
    }

    void visit_expr_let(const Position &p, const AstPtrs &lhs,
                        const AstPtr &rhs, const AstPtr &body) override {
        visit(rhs);  // XXX: shouldn't introduce freevars?
        visit(body);
        set_state(FREEVARS_REMOVE);
        visits(lhs);
        set_state(FREEVARS_INSERT);
    }

    void visit_decl_object(const Position &p, const AstPtr &c,
                           const AstPtrs &vv, const AstPtrs &ff,
                           const AstPtrs &ee) override {
        visits(ff);
        visits(ee);
        set_state(FREEVARS_REMOVE);
        visits(vv);
        set_state(FREEVARS_INSERT);
    }

private:
    AstPtrSet _fv;
    freevars_state_t _state;
};

inline AstPtrSet freevars(const AstPtr &t) {
    FreeVars freevars;
    return freevars.freevars(t);
}
