#pragma once

#include <vector>

#include "ast.hpp"
#include "error.hpp"
#include "runtime.hpp"

namespace egel {

class Transform {
public:
    Transform() {
    }

    virtual ptrs<Ast> transforms(const ptrs<Ast> &aa) {
        ptrs<Ast> _aa;
        for (auto &a : aa) {
            ptr<Ast> _a = transform(a);
            _aa.push_back(_a);
        }
        return _aa;
    }

    virtual void transform_pre(const ptr<Ast> &a) {
    }

    virtual ptr<Ast> transform_docstring(const ptr<Ast> &a, const Position &p,
                                         const icu::UnicodeString &v) {
        return a;
    }

    virtual ptr<Ast> transform_expr_integer(const ptr<Ast> &a,
                                            const Position &p,
                                            const icu::UnicodeString &v) {
        return a;
    }

    virtual ptr<Ast> transform_expr_hexinteger(const ptr<Ast> &a,
                                               const Position &p,
                                               const icu::UnicodeString &v) {
        return a;
    }

    virtual ptr<Ast> transform_expr_float(const ptr<Ast> &a, const Position &p,
                                          const icu::UnicodeString &v) {
        return a;
    }

    virtual ptr<Ast> transform_expr_complex(const ptr<Ast> &a,
                                            const Position &p,
                                            const icu::UnicodeString &v) {
        return a;
    }

    virtual ptr<Ast> transform_expr_character(const ptr<Ast> &a,
                                              const Position &p,
                                              const icu::UnicodeString &v) {
        return a;
    }

    virtual ptr<Ast> transform_expr_text(const ptr<Ast> &a, const Position &p,
                                         const icu::UnicodeString &v) {
        return a;
    }

    virtual ptr<Ast> transform_expr_variable(const ptr<Ast> &a,
                                             const Position &p,
                                             const icu::UnicodeString &n) {
        return a;
    }

    virtual ptr<Ast> transform_expr_wildcard(const ptr<Ast> &a,
                                             const Position &p,
                                             const icu::UnicodeString &n) {
        return a;
    }

    virtual ptr<Ast> transform_expr_combinator(const ptr<Ast> &a,
                                               const Position &p,
                                               const UnicodeStrings &nn,
                                               const icu::UnicodeString &n) {
        return a;
    }

    virtual ptr<Ast> transform_expr_operator(const ptr<Ast> &a,
                                             const Position &p,
                                             const UnicodeStrings &nn,
                                             const icu::UnicodeString &n) {
        return a;
    }

    virtual ptr<Ast> transform_path(const ptr<Ast> &a,
                                               const Position &p,
                                               const UnicodeStrings &nn) {
        return a;
    }

    virtual ptr<Ast> transform_alias(const ptr<Ast> &a, const Position &p, const ptr<Ast> &l, const ptr<Ast> &r) {
        auto l0 = transform(l);
        auto r0 = transform(r);
        return AstAlias::create(p, l0, r0);
    }

    virtual ptr<Ast> transform_expr_tuple(const ptr<Ast> &a, const Position &p,
                                          const ptrs<Ast> &tt) {
        auto tt0 = transforms(tt);
        return AstExprTuple::create(p, tt0);
    }

    virtual ptr<Ast> transform_expr_list(const ptr<Ast> &a, const Position &p,
                                         const ptrs<Ast> &tt,
                                         const ptr<Ast> &tl) {
        auto tt0 = transforms(tt);
        if (tl == nullptr) {
            return AstExprList::create(p, tt0);
        } else {
            auto tl0 = transform(tl);
            return AstExprList::create(p, tt0, tl0);
        }
    }

    virtual ptr<Ast> transform_expr_application(const ptr<Ast> &a,
                                                const Position &p,
                                                const ptrs<Ast> &tt) {
        auto tt0 = transforms(tt);
        return AstExprApplication::create(p, tt0);
    }

    virtual ptr<Ast> transform_expr_match(const ptr<Ast> &a, const Position &p,
                                          const ptrs<Ast> &mm,
                                          const ptr<Ast> &g,
                                          const ptr<Ast> &e) {
        auto mm0 = transforms(mm);
        auto g0 = transform(g);
        auto e0 = transform(e);
        return AstExprMatch::create(p, mm0, g0, e0);
    }

    virtual ptr<Ast> transform_expr_block(const ptr<Ast> &a, const Position &p,
                                          const ptrs<Ast> &alts) {
        auto alts0 = transforms(alts);
        return AstExprBlock::create(p, alts0);
    }

    virtual ptr<Ast> transform_expr_lambda(const ptr<Ast> &a, const Position &p,
                                          const ptrs<Ast> &alts) {
        auto alts0 = transforms(alts);
        return AstExprLambda::create(p, alts0);
    }

    virtual ptr<Ast> transform_expr_let(const ptr<Ast> &a, const Position &p,
                                        const ptrs<Ast> &lhs,
                                        const ptr<Ast> &rhs,
                                        const ptr<Ast> &body) {
        auto lhs0 = transforms(lhs);
        auto rhs0 = transform(rhs);
        auto body0 = transform(body);
        return AstExprLet::create(p, lhs0, rhs0, body0);
    }

    virtual ptr<Ast> transform_expr_tag(const ptr<Ast> &a, const Position &p,
                                        const ptr<Ast> &e, const ptr<Ast> &t) {
        auto e0 = transform(e);
        auto t0 = transform(t);
        return AstExprTag::create(p, e0, t0);
    }

    virtual ptr<Ast> transform_expr_if(const ptr<Ast> &a, const Position &p,
                                       const ptr<Ast> &i, const ptr<Ast> &t,
                                       const ptr<Ast> &e) {
        auto i0 = transform(i);
        auto t0 = transform(t);
        auto e0 = transform(e);
        return AstExprIf::create(p, i0, t0, e0);
    }

    virtual ptr<Ast> transform_expr_statement(const ptr<Ast> &a,
                                              const Position &p,
                                              const ptr<Ast> &l,
                                              const ptr<Ast> &r) {
        auto r0 = transform(r);
        auto l0 = transform(l);
        return AstExprStatement::create(p, r0, l0);
    }

    virtual ptr<Ast> transform_expr_try(const ptr<Ast> &a, const Position &p,
                                        const ptr<Ast> &t, const ptr<Ast> &c) {
        auto t0 = transform(t);
        auto c0 = transform(c);
        return AstExprTry::create(p, t0, c0);
    }

    virtual ptr<Ast> transform_expr_throw(const ptr<Ast> &a, const Position &p,
                                          const ptr<Ast> &e) {
        auto e0 = transform(e);
        return AstExprThrow::create(p, e0);
    }

    virtual ptr<Ast> transform_expr_do(const ptr<Ast> &a, const Position &p,
                                       const ptr<Ast> &e) {
        auto e0 = transform(e);
        return AstExprDo::create(p, e0);
    }

    virtual ptr<Ast> transform_directive_import(const ptr<Ast> &a,
                                                const Position &p,
                                                const icu::UnicodeString &i) {
        return a;
    }

    virtual ptr<Ast> transform_directive_using(const ptr<Ast> &a,
                                               const Position &p,
                                               const UnicodeStrings &uu) {
        return a;
    }

    virtual ptr<Ast> transform_decl_data(const ptr<Ast> &a, const Position &p,
                                         const ptr<Ast> &d,
                                         const ptrs<Ast> &nn) {
        auto d0 = transform(d);
        auto nn0 = transforms(nn);
        return AstDeclData::create(p, d0, nn0);
    }

    virtual ptr<Ast> transform_decl_definition(const ptr<Ast> &a,
                                               const Position &p,
                                               const ptr<Ast> &n,
                                               const ptr<Ast> &d,
                                               const ptr<Ast> &e) {
        auto n0 = transform(n);
        auto d0 = transform(d);
        auto e0 = transform(e);
        return AstDeclDefinition::create(p, n0, d0, e0);
    }

    virtual ptr<Ast> transform_decl_value(const ptr<Ast> &a, const Position &p,
                                          const ptr<Ast> &l, const ptr<Ast> &d,
                                          const ptr<Ast> &r) {
        auto l0 = transform(l);
        auto d0 = transform(d);
        auto r0 = transform(r);
        return AstDeclValue::create(p, l0, d0, r0);
    }

    virtual ptr<Ast> transform_decl_operator(const ptr<Ast> &a,
                                             const Position &p,
                                             const ptr<Ast> &c,
                                             const ptr<Ast> &d,
                                             const ptr<Ast> &e) {
        auto c0 = transform(c);
        auto d0 = transform(d);
        auto e0 = transform(e);
        return AstDeclOperator::create(p, c0, d0, e0);
    }

    virtual ptr<Ast> transform_decl_namespace(const ptr<Ast> &a,
                                              const Position &p,
                                              const UnicodeStrings &nn,
                                              const ptrs<Ast> &dd) {
        auto dd0 = transforms(dd);
        return AstDeclNamespace::create(p, nn, dd0);
    }

    virtual ptr<Ast> transform_wrapper(const ptr<Ast> &a, const Position &p,
                                       const ptrs<Ast> &dd) {
        auto dd0 = transforms(dd);
        return AstWrapper::create(p, dd0);
    }

    ptr<Ast> transform(const ptr<Ast> &a) {
        transform_pre(a);

        switch (a->tag()) {
            case AST_EMPTY: {
                return a;
                break;
            }
            case AST_DOCSTRING: {
                auto [p, t] = AstDocstring::split(a);
                return transform_docstring(a, p, t);
                break;
            }
            // literals
            case AST_EXPR_INTEGER: {
                auto [p, t] = AstExprInteger::split(a);
                return transform_expr_integer(a, p, t);
                break;
            }
            case AST_EXPR_HEXINTEGER: {
                auto [p, t] = AstExprHexInteger::split(a);
                return transform_expr_hexinteger(a, p, t);
                break;
            }
            case AST_EXPR_FLOAT: {
                auto [p, t] = AstExprFloat::split(a);
                return transform_expr_float(a, p, t);
                break;
            }
            case AST_EXPR_COMPLEX: {
                auto [p, t] = AstExprComplex::split(a);
                return transform_expr_complex(a, p, t);
                break;
            }
            case AST_EXPR_CHARACTER: {
                auto [p, t] = AstExprCharacter::split(a);
                return transform_expr_character(a, p, t);
                break;
            }
            case AST_EXPR_TEXT: {
                auto [p, t] = AstExprText::split(a);
                return transform_expr_text(a, p, t);
                break;
            }
            // variables and constants
            case AST_EXPR_VARIABLE: {
                auto [p, t] = AstExprVariable::split(a);
                return transform_expr_variable(a, p, t);
                break;
            }
            case AST_EXPR_WILDCARD: {
                auto [p, t] = AstExprWildcard::split(a);
                return transform_expr_wildcard(a, p, t);
                break;
            }
            case AST_EXPR_COMBINATOR: {
                auto [p, tt, t] = AstExprCombinator::split(a);
                return transform_expr_combinator(a, p, tt, t);
                break;
            }
            case AST_EXPR_OPERATOR: {
                auto [p, tt, t] = AstExprOperator::split(a);
                return transform_expr_operator(a, p, tt, t);
                break;
            }
            case AST_PATH: {
                auto [p, tt] = AstPath::split(a);
                return transform_path(a, p, tt);
                break;
            }
            case AST_ALIAS: {
                auto [p, l, r] = AstAlias::split(a);
                return transform_alias(a, p, l, r);
                break;
            }
            // tuple
            case AST_EXPR_TUPLE: {
                auto [p, tt] = AstExprTuple::split(a);
                return transform_expr_tuple(a, p, tt);
                break;
            }
            case AST_EXPR_LIST: {
                auto [p, tt, tl] = AstExprList::split(a);
                return transform_expr_list(a, p, tt, tl);
                break;
            }
            // compound statements
            case AST_EXPR_APPLICATION: {
                auto [p, ee] = AstExprApplication::split(a);
                return transform_expr_application(a, p, ee);
                break;
            }
            case AST_EXPR_BLOCK: {
                auto [p, mm] = AstExprBlock::split(a);
                return transform_expr_block(a, p, mm);
                break;
            }
            case AST_EXPR_LAMBDA: {
                auto [p, mm] = AstExprLambda::split(a);
                return transform_expr_lambda(a, p, mm);
                break;
            }
            case AST_EXPR_MATCH: {
                auto [p, mm, g, e] = AstExprMatch::split(a);
                return transform_expr_match(a, p, mm, g, e);
                break;
            }
            case AST_EXPR_LET: {
                auto [p, l, r, e] = AstExprLet::split(a);
                return transform_expr_let(a, p, l, r, e);
                break;
            }
            case AST_EXPR_TAG: {
                auto [p, e, t] = AstExprTag::split(a);
                return transform_expr_tag(a, p, e, t);
                break;
            }
            case AST_EXPR_IF: {
                auto [p, i, t, e] = AstExprIf::split(a);
                return transform_expr_if(a, p, i, t, e);
                break;
            }
            case AST_EXPR_STATEMENT: {
                auto [p, r, l] = AstExprStatement::split(a);
                return transform_expr_statement(a, p, r, l);
                break;
            }
            case AST_EXPR_TRY: {
                auto [p, t, c] = AstExprTry::split(a);
                return transform_expr_try(a, p, t, c);
                break;
            }
            case AST_EXPR_THROW: {
                auto [p, exc] = AstExprThrow::split(a);
                return transform_expr_throw(a, p, exc);
                break;
            }
            case AST_EXPR_DO: {
                auto [p, e] = AstExprDo::split(a);
                return transform_expr_do(a, p, e);
                break;
            }
            // directives
            case AST_DIRECT_IMPORT: {
                auto [p, i] = AstDirectImport::split(a);
                return transform_directive_import(a, p, i);
                break;
            }
            case AST_DIRECT_USING: {
                auto [p, uu] = AstDirectUsing::split(a);
                return transform_directive_using(a, p, uu);
                break;
            }
            // declarations
            case AST_DECL_DATA: {
                auto [p, d, nn] = AstDeclData::split(a);
                return transform_decl_data(a, p, d, nn);
                break;
            }
            case AST_DECL_DEFINITION: {
                auto [p, n, d, e] = AstDeclDefinition::split(a);
                return transform_decl_definition(a, p, n, d, e);
                break;
            }
            case AST_DECL_VALUE: {
                auto [p, l, d, r] = AstDeclValue::split(a);
                return transform_decl_value(a, p, l, d, r);
            }
            case AST_DECL_OPERATOR: {
                auto [p, c, d, e] = AstDeclOperator::split(a);
                return transform_decl_operator(a, p, c, d, e);
                break;
            }
            case AST_DECL_NAMESPACE: {
                auto [p, nn, dd] = AstDeclNamespace::split(a);
                return transform_decl_namespace(a, p, nn, dd);
                break;
            }
            // wrapper
            case AST_WRAPPER: {
                auto [p, dd] = AstWrapper::split(a);
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

    virtual ptrs<Ast> rewrites(const ptrs<Ast> &aa) {
        ptrs<Ast> _aa;
        for (auto &a : aa) {
            ptr<Ast> _a = rewrite(a);
            _aa.push_back(_a);
        }
        return _aa;
    }

    virtual void rewrite_pre(const ptr<Ast> &a) {
    }

    virtual ptr<Ast> rewrite_docstring(const Position &p,
                                       const icu::UnicodeString &v) {
        return AstDocstring::create(p, v);
    }

    // literals
    virtual ptr<Ast> rewrite_expr_integer(const Position &p,
                                          const icu::UnicodeString &v) {
        return AstExprInteger::create(p, v);
    }

    virtual ptr<Ast> rewrite_expr_hexinteger(const Position &p,
                                             const icu::UnicodeString &v) {
        return AstExprHexInteger::create(p, v);
    }

    virtual ptr<Ast> rewrite_expr_float(const Position &p,
                                        const icu::UnicodeString &v) {
        return AstExprFloat::create(p, v);
    }

    virtual ptr<Ast> rewrite_expr_complex(const Position &p,
                                          const icu::UnicodeString &v) {
        return AstExprComplex::create(p, v);
    }

    virtual ptr<Ast> rewrite_expr_character(const Position &p,
                                            const icu::UnicodeString &v) {
        return AstExprCharacter::create(p, v);
    }

    virtual ptr<Ast> rewrite_expr_text(const Position &p,
                                       const icu::UnicodeString &v) {
        return AstExprText::create(p, v);
    }

    // variables and constants
    virtual ptr<Ast> rewrite_expr_variable(const Position &p,
                                           const icu::UnicodeString &n) {
        return AstExprVariable::create(p, n);
    }

    virtual ptr<Ast> rewrite_expr_wildcard(const Position &p,
                                           const icu::UnicodeString &n) {
        return AstExprWildcard::create(p, n);
    }

    virtual ptr<Ast> rewrite_expr_combinator(const Position &p,
                                             const UnicodeStrings &nn,
                                             const icu::UnicodeString &n) {
        return AstExprCombinator::create(p, nn, n);
    }

    virtual ptr<Ast> rewrite_expr_operator(const Position &p,
                                           const UnicodeStrings &nn,
                                           const icu::UnicodeString &n) {
        return AstExprOperator::create(p, nn, n);
    }

    virtual ptr<Ast> rewrite_path(const Position &p,
                                             const UnicodeStrings &nn) {
        return AstPath::create(p, nn);
    }

    virtual ptr<Ast> rewrite_alias(const Position &p,
                                           const ptr<Ast> &l,
                                           const ptr<Ast> &r) {
        auto l0 = rewrite(l);
        auto r0 = rewrite(r);
        return AstAlias::create(p, l, r);
    }

    // tuple and list
    virtual ptr<Ast> rewrite_expr_tuple(const Position &p,
                                        const ptrs<Ast> &tt) {
        auto tt0 = rewrites(tt);
        return AstExprTuple::create(p, tt0);
    }

    virtual ptr<Ast> rewrite_expr_list(const Position &p, const ptrs<Ast> &tt,
                                       const ptr<Ast> &tl) {
        auto tt0 = rewrites(tt);
        if (tl == nullptr) {
            return AstExprList::create(p, tt0);
        } else {
            auto tl0 = rewrite(tl);
            return AstExprList::create(p, tt0, tl0);
        }
    }

    // compound statements
    virtual ptr<Ast> rewrite_expr_application(const Position &p,
                                              const ptrs<Ast> &aa) {
        auto aa0 = rewrites(aa);
        return AstExprApplication::create(p, aa0);
    }

    virtual ptr<Ast> rewrite_expr_match(const Position &p, const ptrs<Ast> &mm,
                                        const ptr<Ast> &g, const ptr<Ast> &e) {
        auto mm0 = rewrites(mm);
        auto g0 = rewrite(g);
        auto e0 = rewrite(e);
        return AstExprMatch::create(p, mm0, g0, e0);
    }

    virtual ptr<Ast> rewrite_expr_block(const Position &p,
                                        const ptrs<Ast> &alts) {
        auto alts0 = rewrites(alts);
        return AstExprBlock::create(p, alts0);
    }

    virtual ptr<Ast> rewrite_expr_lambda(const Position &p,
                                        const ptrs<Ast> &alts) {
        auto alts0 = rewrites(alts);
        return AstExprLambda::create(p, alts0);
    }

    virtual ptr<Ast> rewrite_expr_let(const Position &p, const ptrs<Ast> &lhs,
                                      const ptr<Ast> &rhs,
                                      const ptr<Ast> &body) {
        auto lhs0 = rewrites(lhs);
        auto rhs0 = rewrite(rhs);
        auto body0 = rewrite(body);
        return AstExprLet::create(p, lhs0, rhs0, body0);
    }

    virtual ptr<Ast> rewrite_expr_tag(const Position &p, const ptr<Ast> &e,
                                      const ptr<Ast> &t) {
        auto e0 = rewrite(e);
        auto t0 = rewrite(t);
        return AstExprTag::create(p, e0, t0);
    }

    virtual ptr<Ast> rewrite_expr_if(const Position &p, const ptr<Ast> &i,
                                     const ptr<Ast> &t, const ptr<Ast> &e) {
        auto i0 = rewrite(i);
        auto t0 = rewrite(t);
        auto e0 = rewrite(e);
        return AstExprIf::create(p, i0, t0, e0);
    }

    virtual ptr<Ast> rewrite_expr_statement(const Position &p,
                                            const ptr<Ast> &r,
                                            const ptr<Ast> &l) {
        auto r0 = rewrite(r);
        auto l0 = rewrite(l);
        return AstExprStatement::create(p, r0, l0);
    }

    virtual ptr<Ast> rewrite_expr_try(const Position &p, const ptr<Ast> &t,
                                      const ptr<Ast> &c) {
        auto t0 = rewrite(t);
        auto c0 = rewrite(c);
        return AstExprTry::create(p, t0, c0);
    }

    virtual ptr<Ast> rewrite_expr_throw(const Position &p, const ptr<Ast> &e) {
        auto e0 = rewrite(e);
        return AstExprThrow::create(p, e0);
    }

    virtual ptr<Ast> rewrite_expr_do(const Position &p, const ptr<Ast> &e) {
        auto e0 = rewrite(e);
        return AstExprDo::create(p, e0);
    }

    virtual ptr<Ast> rewrite_directive_import(const Position &p,
                                              const icu::UnicodeString &i) {
        return AstDirectImport::create(p, i);
    }

    virtual ptr<Ast> rewrite_directive_using(const Position &p,
                                             const UnicodeStrings &nn) {
        return AstDirectUsing::create(p, nn);
    }

    virtual ptr<Ast> rewrite_decl_data(const Position &p, const ptr<Ast> &d,
                                       const ptrs<Ast> &nn) {
        auto d0 = rewrite(d);
        auto nn0 = rewrites(nn);
        return AstDeclData::create(p, d, nn0);
    }

    virtual ptr<Ast> rewrite_decl_definition(const Position &p,
                                             const ptr<Ast> &n,
                                             const ptr<Ast> &d,
                                             const ptr<Ast> &e) {
        auto n0 = rewrite(n);
        auto d0 = rewrite(d);
        auto e0 = rewrite(e);
        return AstDeclDefinition::create(p, n0, d0, e0);
    }

    virtual ptr<Ast> rewrite_decl_value(const Position &p, const ptr<Ast> &l,
                                        const ptr<Ast> &d, const ptr<Ast> &r) {
        auto l0 = rewrite(l);
        auto d0 = rewrite(d);
        auto r0 = rewrite(r);
        return AstDeclValue::create(p, l0, d0, r0);
    }

    virtual ptr<Ast> rewrite_decl_operator(const Position &p, const ptr<Ast> &c,
                                           const ptr<Ast> &d,
                                           const ptr<Ast> &e) {
        auto c0 = rewrite(c);
        auto d0 = rewrite(d);
        auto e0 = rewrite(e);
        return AstDeclOperator::create(p, c0, d0, e0);
    }

    virtual ptr<Ast> rewrite_decl_namespace(const Position &p,
                                            const UnicodeStrings &nn,
                                            const ptrs<Ast> &dd) {
        auto dd0 = rewrites(dd);
        return AstDeclNamespace::create(p, nn, dd0);
    }

    virtual ptr<Ast> rewrite_wrapper(const Position &p, const ptrs<Ast> &dd) {
        auto dd0 = rewrites(dd);
        return AstWrapper::create(p, dd0);
    }

    virtual ptr<Ast> rewrite(const ptr<Ast> &a) {
        rewrite_pre(a);

        switch (a->tag()) {
            case AST_EMPTY: {
                return a;
                break;
            }
            case AST_DOCSTRING: {
                auto [p, t] = AstDocstring::split(a);
                return rewrite_docstring(p, t);
                break;
            }
            // literals
            case AST_EXPR_INTEGER: {
                auto [p, t] = AstExprInteger::split(a);
                return rewrite_expr_integer(p, t);
                break;
            }
            case AST_EXPR_HEXINTEGER: {
                auto [p, t] = AstExprHexInteger::split(a);
                return rewrite_expr_hexinteger(p, t);
                break;
            }
            case AST_EXPR_FLOAT: {
                auto [p, t] = AstExprFloat::split(a);
                return rewrite_expr_float(p, t);
                break;
            }
            case AST_EXPR_COMPLEX: {
                auto [p, t] = AstExprComplex::split(a);
                return rewrite_expr_complex(p, t);
                break;
            }
            case AST_EXPR_CHARACTER: {
                auto [p, t] = AstExprCharacter::split(a);
                return rewrite_expr_character(p, t);
                break;
            }
            case AST_EXPR_TEXT: {
                auto [p, t] = AstExprText::split(a);
                return rewrite_expr_text(p, t);
                break;
            }
            // variables and constants
            case AST_EXPR_VARIABLE: {
                auto [p, t] = AstExprVariable::split(a);
                return rewrite_expr_variable(p, t);
                break;
            }
            case AST_EXPR_WILDCARD: {
                auto [p, t] = AstExprWildcard::split(a);
                return rewrite_expr_wildcard(p, t);
                break;
            }
            case AST_EXPR_COMBINATOR: {
                auto [p, nn, n] = AstExprCombinator::split(a);
                return rewrite_expr_combinator(p, nn, n);
                break;
            }
            case AST_EXPR_OPERATOR: {
                auto [p, nn, n] = AstExprOperator::split(a);
                return rewrite_expr_operator(p, nn, n);
                break;
            }
            case AST_PATH: {
                auto [p, nn] = AstPath::split(a);
                return rewrite_path(p, nn);
                break;
            }
            case AST_ALIAS: {
                auto [p, l, r] = AstAlias::split(a);
                return rewrite_alias(p, l, r);
                break;
            }
            // tuple and list
            case AST_EXPR_TUPLE: {
                auto [p, tt] = AstExprTuple::split(a);
                return rewrite_expr_tuple(p, tt);
                break;
            }
            case AST_EXPR_LIST: {
                auto [p, tt, tl] = AstExprList::split(a);
                return rewrite_expr_list(p, tt, tl);
                break;
            }
            // compound statements
            case AST_EXPR_APPLICATION: {
                auto [p, aa] = AstExprApplication::split(a);
                return rewrite_expr_application(p, aa);
                break;
            }
            case AST_EXPR_BLOCK: {
                auto [p, mm] = AstExprBlock::split(a);
                return rewrite_expr_block(p, mm);
                break;
            }
            case AST_EXPR_LAMBDA: {
                auto [p, mm] = AstExprLambda::split(a);
                return rewrite_expr_lambda(p, mm);
                break;
            }
            case AST_EXPR_MATCH: {
                auto [p, mm, g, e] = AstExprMatch::split(a);
                return rewrite_expr_match(p, mm, g, e);
                break;
            }
            case AST_EXPR_LET: {
                auto [p, l, r, e] = AstExprLet::split(a);
                return rewrite_expr_let(p, l, r, e);
                break;
            }
            case AST_EXPR_TAG: {
                auto [p, e, t] = AstExprTag::split(a);
                return rewrite_expr_tag(p, e, t);
                break;
            }
            case AST_EXPR_IF: {
                auto [p, i, t, e] = AstExprIf::split(a);
                return rewrite_expr_if(p, i, t, e);
                break;
            }
            case AST_EXPR_STATEMENT: {
                auto [p, r, l] = AstExprStatement::split(a);
                return rewrite_expr_statement(p, r, l);
                break;
            }
            case AST_EXPR_TRY: {
                auto [p, t, c] = AstExprTry::split(a);
                return rewrite_expr_try(p, t, c);
                break;
            }
            case AST_EXPR_THROW: {
                auto [p, exc] = AstExprThrow::split(a);
                return rewrite_expr_throw(p, exc);
                break;
            }
            case AST_EXPR_DO: {
                auto [p, e] = AstExprDo::split(a);
                return rewrite_expr_do(p, e);
                break;
            }
            // directives
            case AST_DIRECT_IMPORT: {
                auto [p, i] = AstDirectImport::split(a);
                return rewrite_directive_import(p, i);
                break;
            }
            case AST_DIRECT_USING: {
                auto [p, i] = AstDirectUsing::split(a);
                return rewrite_directive_using(p, i);
                break;
            }
            // declarations
            case AST_DECL_DATA: {
                auto [p, d, nn] = AstDeclData::split(a);
                return rewrite_decl_data(p, d, nn);
                break;
            }
            case AST_DECL_DEFINITION: {
                auto [p, n, d, e] = AstDeclDefinition::split(a);
                return rewrite_decl_definition(p, n, d, e);
                break;
            }
            case AST_DECL_VALUE: {
                auto [p, l, d, r] = AstDeclValue::split(a);
                return rewrite_decl_value(p, l, d, r);
                break;
            }
            case AST_DECL_OPERATOR: {
                auto [p, c, d, e] = AstDeclOperator::split(a);
                return rewrite_decl_operator(p, c, d, e);
                break;
            }
            case AST_DECL_NAMESPACE: {
                auto [p, nn, dd] = AstDeclNamespace::split(a);
                return rewrite_decl_namespace(p, nn, dd);
                break;
            }
            // wrapper
            case AST_WRAPPER: {
                auto [p, dd] = AstWrapper::split(a);
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

    virtual void visit_pre(const ptr<Ast> &a) {
    }

    void visits(const ptrs<Ast> &dd) {
        for (auto &d : dd) {
            visit(d);
        }
    }

    virtual void visit_docstring(const Position &p,
                                 const icu::UnicodeString &v) {
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

    virtual void visit_expr_complex(const Position &p,
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

    virtual void visit_path(const Position &p,
                                       const UnicodeStrings &nn) {
    }

    virtual void visit_path(const Position &p,
                                     const ptr<Ast> &l,
                                     const ptr<Ast> &r) {
        visit(l);
        visit(r);
    }

    virtual void visit_expr_tuple(const Position &p, const ptrs<Ast> &tt) {
        visits(tt);
    }

    virtual void visit_expr_list(const Position &p, const ptrs<Ast> &tt,
                                 const ptr<Ast> &tl) {
        if (tl == nullptr) {
            visits(tt);
        } else {
            visits(tt);
            visit(tl);
        }
    }

    virtual void visit_expr_application(const Position &p,
                                        const ptrs<Ast> &tt) {
        visits(tt);
    }

    virtual void visit_expr_match(const Position &p, const ptrs<Ast> &mm,
                                  const ptr<Ast> &g, const ptr<Ast> &e) {
        visits(mm);
        visit(g);
        visit(e);
    }

    virtual void visit_expr_block(const Position &p, const ptrs<Ast> &alts) {
        visits(alts);
    }

    virtual void visit_expr_lambda(const Position &p, const ptrs<Ast> &alts) {
        visits(alts);
    }

    virtual void visit_expr_let(const Position &p, const ptrs<Ast> &lhs,
                                const ptr<Ast> &rhs, const ptr<Ast> &body) {
        visits(lhs);
        visit(rhs);
        visit(body);
    }

    virtual void visit_expr_tag(const Position &p, const ptr<Ast> &e,
                                const ptr<Ast> &t) {
        visit(e);
        visit(t);
    }

    virtual void visit_expr_if(const Position &p, const ptr<Ast> &i,
                               const ptr<Ast> &t, const ptr<Ast> &e) {
        visit(i);
        visit(t);
        visit(e);
    }

    virtual void visit_expr_statement(const Position &p, const ptr<Ast> &r,
                                      const ptr<Ast> &l) {
        visit(r);
        visit(l);
    }

    virtual void visit_expr_try(const Position &p, const ptr<Ast> &t,
                                const ptr<Ast> &c) {
        visit(t);
        visit(c);
    }

    virtual void visit_expr_throw(const Position &p, const ptr<Ast> &e) {
        visit(e);
    }

    virtual void visit_expr_do(const Position &p, const ptr<Ast> &e) {
        visit(e);
    }

    virtual void visit_directive_import(const Position &p,
                                        const icu::UnicodeString &i) {
    }

    virtual void visit_directive_using(const Position &p,
                                       const UnicodeStrings &nn) {
    }

    virtual void visit_decl_data(const Position &p, const ptr<Ast> &d,
                                 const ptrs<Ast> &nn) {
        visit(d);
        visits(nn);
    }

    virtual void visit_decl_definition(const Position &p, const ptr<Ast> &n,
                                       const ptr<Ast> &d, const ptr<Ast> &e) {
        visit(n);
        visit(d);
        visit(e);
    }

    virtual void visit_decl_value(const Position &p, const ptr<Ast> &l,
                                  const ptr<Ast> &d, const ptr<Ast> &r) {
        visit(l);
        visit(d);
        visit(r);
    }

    virtual void visit_decl_operator(const Position &p, const ptr<Ast> &c,
                                     const ptr<Ast> &d, const ptr<Ast> &e) {
        visit(c);
        visit(d);
        visit(e);
    }

    virtual void visit_decl_namespace(const Position &p,
                                      const UnicodeStrings &nn,
                                      const ptrs<Ast> &dd) {
        visits(dd);
    }

    virtual void visit_wrapper(const Position &p, const ptrs<Ast> &dd) {
        visits(dd);
    }

    virtual void visit(const ptr<Ast> &a) {
        visit_pre(a);

        switch (a->tag()) {
            case AST_EMPTY: {
                return;
                break;
            }
            case AST_DOCSTRING: {
                auto [p, t] = AstDocstring::split(a);
                return visit_docstring(p, t);
                break;
            }
            // literals
            case AST_EXPR_INTEGER: {
                auto [p, t] = AstExprInteger::split(a);
                return visit_expr_integer(p, t);
                break;
            }
            case AST_EXPR_HEXINTEGER: {
                auto [p, t] = AstExprHexInteger::split(a);
                return visit_expr_hexinteger(p, t);
                break;
            }
            case AST_EXPR_FLOAT: {
                auto [p, t] = AstExprFloat::split(a);
                return visit_expr_float(p, t);
                break;
            }
            case AST_EXPR_COMPLEX: {
                auto [p, t] = AstExprComplex::split(a);
                return visit_expr_complex(p, t);
                break;
            }
            case AST_EXPR_CHARACTER: {
                auto [p, t] = AstExprCharacter::split(a);
                return visit_expr_character(p, t);
                break;
            }
            case AST_EXPR_TEXT: {
                auto [p, t] = AstExprText::split(a);
                return visit_expr_text(p, t);
                break;
            }
            // variables and constants
            case AST_EXPR_VARIABLE: {
                auto [p, t] = AstExprVariable::split(a);
                return visit_expr_variable(p, t);
                break;
            }
            case AST_EXPR_WILDCARD: {
                auto [p, t] = AstExprWildcard::split(a);
                return visit_expr_wildcard(p, t);
                break;
            }
            case AST_EXPR_COMBINATOR: {
                auto [p, nn, n] = AstExprCombinator::split(a);
                return visit_expr_combinator(p, nn, n);
                break;
            }
            case AST_EXPR_OPERATOR: {
                auto [p, nn, n] = AstExprOperator::split(a);
                return visit_expr_operator(p, nn, n);
                break;
            }
            case AST_PATH: {
                auto [p, nn] = AstPath::split(a);
                return visit_path(p, nn);
                break;
            }
            case AST_ALIAS: {
                auto [p, l, r] = AstExprOperator::split(a);
                return visit_expr_operator(p, l, r);
                break;
            }
            // tuple
            case AST_EXPR_TUPLE: {
                auto [p, tt] = AstExprTuple::split(a);
                return visit_expr_tuple(p, tt);
                break;
            }
            case AST_EXPR_LIST: {
                auto [p, tt, tl] = AstExprList::split(a);
                return visit_expr_list(p, tt, tl);
                break;
            }
            // compound statements
            case AST_EXPR_APPLICATION: {
                auto [p, tt] = AstExprApplication::split(a);
                return visit_expr_application(p, tt);
                break;
            }
            case AST_EXPR_BLOCK: {
                auto [p, mm] = AstExprBlock::split(a);
                return visit_expr_block(p, mm);
                break;
            }
            case AST_EXPR_LAMBDA: {
                auto [p, mm] = AstExprLambda::split(a);
                return visit_expr_lambda(p, mm);
                break;
            }
            case AST_EXPR_MATCH: {
                auto [p, mm, g, e] = AstExprMatch::split(a);
                return visit_expr_match(p, mm, g, e);
                break;
            }
            case AST_EXPR_LET: {
                auto [p, l, r, e] = AstExprLet::split(a);
                return visit_expr_let(p, l, r, e);
                break;
            }
            case AST_EXPR_TAG: {
                auto [p, e, t] = AstExprTag::split(a);
                return visit_expr_tag(p, e, t);
                break;
            }
            case AST_EXPR_IF: {
                auto [p, i, t, e] = AstExprIf::split(a);
                return visit_expr_if(p, i, t, e);
                break;
            }
            case AST_EXPR_STATEMENT: {
                auto [p, r, l] = AstExprStatement::split(a);
                return visit_expr_statement(p, r, l);
                break;
            }
            case AST_EXPR_TRY: {
                auto [p, t, c] = AstExprTry::split(a);
                return visit_expr_try(p, t, c);
                break;
            }
            case AST_EXPR_THROW: {
                auto [p, exc] = AstExprThrow::split(a);
                return visit_expr_throw(p, exc);
                break;
            }
            case AST_EXPR_DO: {
                auto [p, e] = AstExprDo::split(a);
                return visit_expr_do(p, e);
                break;
            }
            // directives
            case AST_DIRECT_IMPORT: {
                auto [p, i] = AstDirectImport::split(a);
                return visit_directive_import(p, i);
                break;
            }
            case AST_DIRECT_USING: {
                auto [p, uu] = AstDirectUsing::split(a);
                return visit_directive_using(p, uu);
                break;
            }
            // declarations
            case AST_DECL_DATA: {
                auto [p, d, nn] = AstDeclData::split(a);
                return visit_decl_data(p, d, nn);
                break;
            }
            case AST_DECL_DEFINITION: {
                auto [p, n, d, e] = AstDeclDefinition::split(a);
                return visit_decl_definition(p, n, d, e);
                break;
            }
            case AST_DECL_VALUE: {
                auto [p, r, d, l] = AstDeclValue::split(a);
                return visit_decl_value(p, r, d, l);
                break;
            }
            case AST_DECL_OPERATOR: {
                auto [p, c, d, e] = AstDeclOperator::split(a);
                return visit_decl_operator(p, c, d, e);
                break;
            }
            case AST_DECL_NAMESPACE: {
                auto [p, nn, dd] = AstDeclNamespace::split(a);
                return visit_decl_namespace(p, nn, dd);
                break;
            }
            // wrapper
            case AST_WRAPPER: {
                auto [p, dd] = AstWrapper::split(a);
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
    bool occurs(const ptr<Ast> &t0, const ptr<Ast> &t1) {
        _term = t0;
        _found = false;
        visit(t1);
        return _found;
    }

    void visit(const ptr<Ast> &t) override {
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
    ptr<Ast> _term;
    bool _found;
};

inline bool occurs(const ptr<Ast> &t0, const ptr<Ast> &t1) {
    Occurs occurs;
    return occurs.occurs(t0, t1);
}

class Substitute : public Rewrite {
public:
    ptr<Ast> substitute(const ptr<Ast> &term, const ptr<Ast> &s0,
                        const ptr<Ast> &s1) {
        _source = s0;
        _target = s1;
        return rewrite(term);
    }

    ptr<Ast> rewrite_expr_match(const Position &p, const ptrs<Ast> &mm,
                                const ptr<Ast> &g, const ptr<Ast> &e) override {
        for (auto m : mm) {
            if (occurs(_source, m)) {
                return AstExprMatch::create(p, mm, g, e);
            }
        }
        auto g0 = rewrite(g);
        auto e0 = rewrite(e);
        return AstExprMatch::create(p, mm, g0, e0);
    }

    ptr<Ast> rewrite_expr_let(const Position &p, const ptrs<Ast> &lhs,
                              const ptr<Ast> &rhs,
                              const ptr<Ast> &body) override {
        for (auto m : lhs) {
            if (occurs(_source, m)) {
                return AstExprLet::create(p, lhs, rhs, body);
            }
        }
        auto rhs0 = rewrite(rhs);
        auto body0 = rewrite(body);
        return AstExprLet::create(p, lhs, rhs0, body0);
    }

    ptr<Ast> rewrite(const ptr<Ast> &a) override {
        if (a == _source) {
            return _target;
        } else {
            return Rewrite::rewrite(a);
        }
    }

private:
    ptr<Ast> _source;
    ptr<Ast> _target;
};

inline ptr<Ast> substitute(const ptr<Ast> &term, const ptr<Ast> &s0,
                           const ptr<Ast> &s1) {
    Substitute subs;
    return subs.substitute(term, s0, s1);
}

enum freevars_state_t {
    FREEVARS_INSERT,
    FREEVARS_REMOVE,
};

class FreeVars : public Visit {
public:
    std::set<ptr<Ast>, LessAst> freevars(const ptr<Ast> &a) {
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

    void insert(const ptr<Ast> &a) {
        _fv.insert(a);
    }

    void remove(const ptr<Ast> &a) {
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

    void visit_expr_match(const Position &p, const ptrs<Ast> &mm,
                          const ptr<Ast> &g, const ptr<Ast> &e) override {
        visit(g);
        visit(e);
        set_state(FREEVARS_REMOVE);
        visits(mm);
        set_state(FREEVARS_INSERT);
    }

    void visit_expr_let(const Position &p, const ptrs<Ast> &lhs,
                        const ptr<Ast> &rhs, const ptr<Ast> &body) override {
        visit(rhs);  // XXX: shouldn't introduce freevars?
        visit(body);
        set_state(FREEVARS_REMOVE);
        visits(lhs);
        set_state(FREEVARS_INSERT);
    }

private:
    std::set<ptr<Ast>, LessAst> _fv;
    freevars_state_t _state;
};

inline std::set<ptr<Ast>, LessAst> freevars(const ptr<Ast> &t) {
    FreeVars freevars;
    return freevars.freevars(t);
}

};  // namespace egel
