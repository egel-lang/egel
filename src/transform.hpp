#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <vector>
#include "error.hpp"
#include "ast.hpp"

// a file full of bloody big case switches.
// anyone who proposes that I should use the visitor pattern can talk to Stroustrup.

// first a standard template for handling a transform with a local shared state

class Transform {
public:
    Transform() {}

    virtual AstPtrs transforms(const AstPtrs& aa) {
        AstPtrs _aa;
        for (auto& a: aa) {
            AstPtr _a = transform(a);
            _aa.push_back(_a);
        }
        return _aa;
    }

    virtual void transform_pre(const AstPtr& a) {
    }

    virtual AstPtr transform_expr_integer(const AstPtr& a, const Position& p, const UnicodeString& v) {
        return a;
    }

    virtual AstPtr transform_expr_hexinteger(const AstPtr& a, const Position& p, const UnicodeString& v) {
        return a;
    }

    virtual AstPtr transform_expr_float(const AstPtr& a, const Position& p, const UnicodeString& v) {
        return a;
    }

    virtual AstPtr transform_expr_character(const AstPtr& a, const Position& p, const UnicodeString& v) {
        return a;
    }

    virtual AstPtr transform_expr_text(const AstPtr& a, const Position& p, const UnicodeString& v) {
        return a;
    }

    virtual AstPtr transform_expr_variable(const AstPtr& a, const Position& p, const UnicodeString& n) {
        return a;
    }

    virtual AstPtr transform_expr_wildcard(const AstPtr& a, const Position& p, const UnicodeString& n) {
        return a;
    }

    virtual AstPtr transform_expr_combinator(const AstPtr& a, const Position& p, const UnicodeStrings& nn, const UnicodeString& n) {
        return a;
    }

    virtual AstPtr transform_expr_operator(const AstPtr& a, const Position& p, const UnicodeStrings& nn, const UnicodeString& n) {
        return a;
    }

    virtual AstPtr transform_expr_tuple(const AstPtr& a, const Position& p, const AstPtrs& tt) {
        auto tt0 = transforms(tt);
        return AstExprTuple(p, tt0).clone();
    }

    virtual AstPtr transform_expr_list(const AstPtr& a, const Position& p, const AstPtrs& tt) {
        auto tt0 = transforms(tt);
        return AstExprList(p, tt0).clone();
    }

    virtual AstPtr transform_expr_application(const AstPtr& a, const Position& p, const AstPtrs& tt) {
        auto tt0 = transforms(tt);
        return AstExprApplication(p, tt0).clone();
    }

    virtual AstPtr transform_expr_lambda(const AstPtr& a, const Position& p, const AstPtr& m) {
        auto m0 = transform(m);
        return AstExprLambda(p, m0).clone();
    }

    virtual AstPtr transform_expr_match(const AstPtr& a, const Position& p, const AstPtrs& mm, const AstPtr& g, const AstPtr& e) {
        auto mm0 = transforms(mm);
        auto g0 = transform(g);
        auto e0 = transform(e);
        return AstExprMatch(p, mm0, g0, e0).clone();
    }

    virtual AstPtr transform_expr_block(const AstPtr& a, const Position& p, const AstPtrs& alts) {
        auto alts0 = transforms(alts);
        return AstExprBlock(p, alts0).clone();
    }

    virtual AstPtr transform_expr_let(const AstPtr& a, const Position& p, const AstPtr& lhs, const AstPtr& rhs, const AstPtr& body) {
        auto lhs0 = transform(lhs);
        auto rhs0 = transform(rhs);
        auto body0 = transform(body);
        return AstExprLet(p, lhs0, rhs0, body0).clone();
    }

    virtual AstPtr transform_expr_tag(const AstPtr& a, const Position& p, const AstPtr& e, const AstPtr& t) {
        auto e0 = transform(e);
        auto t0 = transform(t);
        return AstExprTag(p, e0, t0).clone();
    }

    virtual AstPtr transform_expr_if(const AstPtr& a, const Position& p, const AstPtr& i, const AstPtr& t, const AstPtr& e) {
        auto i0 = transform(i);
        auto t0 = transform(t);
        auto e0 = transform(e);
        return AstExprIf(p, i0, t0, e0).clone();
    }

    virtual AstPtr transform_expr_try(const AstPtr& a, const Position& p, const AstPtr& t, const AstPtr& c) {
        auto t0 = transform(t);
        auto c0 = transform(c);
        return AstExprTry(p, t0, c0).clone();
    }

    virtual AstPtr transform_expr_throw(const AstPtr& a, const Position& p, const AstPtr& e) {
        auto e0 = transform(e);
        return AstExprThrow(p, e0).clone();
    }

    virtual AstPtr transform_directive_import(const AstPtr& a, const Position& p, const UnicodeString& i) {
        return a;
    }

    virtual AstPtr transform_directive_using(const AstPtr& a, const Position& p, const UnicodeStrings& uu) {
        return a;
    }

    virtual AstPtr transform_decl_data(const AstPtr& a, const Position& p, const AstPtrs& nn) {
        auto nn0 = transforms(nn);
        return AstDeclData(p, nn0).clone();
    }

    virtual AstPtr transform_decl_definition(const AstPtr& a, const Position& p, const AstPtr& n, const AstPtr& e) {
        auto n0 = transform(n);
        auto e0 = transform(e);
        return AstDeclDefinition(p, n0, e0).clone();
    }

    virtual AstPtr transform_decl_operator(const AstPtr& a, const Position& p, const AstPtr& c, const AstPtr& e) {
        auto c0 = transform(c);
        auto e0 = transform(e);
        return AstDeclOperator(p, c0, e0).clone();
    }

    virtual AstPtr transform_decl_object(const AstPtr& a, const Position& p, const AstPtr& c, const AstPtrs& vv, const AstPtrs& ff) {
        auto c0 = transform(c);
        auto vv0 = transforms(vv);
        auto ff0 = transforms(ff);
        return AstDeclObject(p, c0, vv0, ff0).clone();
    }

    virtual AstPtr transform_decl_namespace(const AstPtr& a, const Position& p, const UnicodeStrings& nn, const AstPtrs& dd) {
        auto dd0 = transforms(dd);
        return AstDeclNamespace(p, nn, dd0).clone();
    }

    virtual AstPtr transform_wrapper(const AstPtr& a, const Position& p, const AstPtrs& dd) {
        auto dd0 = transforms(dd);
        return AstWrapper(p, dd0).clone();
    }

    AstPtr transform(const AstPtr& a) {
        transform_pre(a);

        switch(a->tag()) {
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
            AST_EXPR_LIST_SPLIT(a, p, tt);
            return transform_expr_list(a, p, tt);
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
        case AST_DECL_OPERATOR: {
            AST_DECL_OPERATOR_SPLIT(a, p, c, e);
            return transform_decl_operator(a, p, c, e);
            break;
        }
        case AST_DECL_OBJECT: {
            AST_DECL_OBJECT_SPLIT(a, p, c, vv, ff);
            return transform_decl_object(a, p, c, vv, ff);
            break;
        }
        case AST_DECL_NAMESPACE: {
            AST_DECL_NAMESPACE_SPLIT(a, p, nn, dd);
            return transform_decl_namespace(a, p, nn, dd);
            break;
        }
        // wrapper
        case AST_WRAPPER:
            AST_WRAPPER_SPLIT(a, p, dd);
            return transform_wrapper(a, p, dd);
        }
        PANIC("transform exhausted");
        return nullptr;
    }
};


class Rewrite {
public:
    Rewrite() {}

    virtual AstPtrs rewrites(const AstPtrs& aa) {
        AstPtrs _aa;
        for (auto& a: aa) {
            AstPtr _a = rewrite(a);
            _aa.push_back(_a);
        }
        return _aa;
    }

    virtual void rewrite_pre(const AstPtr& a) {
    }

    // literals
    virtual AstPtr rewrite_expr_integer(const Position& p, const UnicodeString& v) {
        return AstExprInteger(p, v).clone();
    }

    virtual AstPtr rewrite_expr_hexinteger(const Position& p, const UnicodeString& v) {
        return AstExprHexInteger(p, v).clone();
    }

    virtual AstPtr rewrite_expr_float(const Position& p, const UnicodeString& v) {
        return AstExprFloat(p, v).clone();
    }

    virtual AstPtr rewrite_expr_character(const Position& p, const UnicodeString& v) {
        return AstExprCharacter(p, v).clone();
    }

    virtual AstPtr rewrite_expr_text(const Position& p, const UnicodeString& v) {
        return AstExprText(p, v).clone();
    }

    // variables and constants
    virtual AstPtr rewrite_expr_variable(const Position& p, const UnicodeString& n) {
        return AstExprVariable(p, n).clone();
    }

    virtual AstPtr rewrite_expr_wildcard(const Position& p, const UnicodeString& n) {
        return AstExprWildcard(p, n).clone();
    }

    virtual AstPtr rewrite_expr_combinator(const Position& p, const UnicodeStrings& nn, const UnicodeString& n) {
        return AstExprCombinator(p, nn, n).clone();
    }

    virtual AstPtr rewrite_expr_operator(const Position& p, const UnicodeStrings& nn, const UnicodeString& n) {
        return AstExprOperator(p, nn, n).clone();
    }

    // tuple and list
    virtual AstPtr rewrite_expr_tuple(const Position& p, const AstPtrs& tt) {
        auto tt0 = rewrites(tt);
        return AstExprTuple(p, tt0).clone();
    }

    virtual AstPtr rewrite_expr_list(const Position& p, const AstPtrs& tt) {
        auto tt0 = rewrites(tt);
        return AstExprList(p, tt0).clone();
    }

    // compound statements
    virtual AstPtr rewrite_expr_application(const Position& p, const AstPtrs& aa) {
        auto aa0 = rewrites(aa);
        return AstExprApplication(p, aa0).clone();
    }

    virtual AstPtr rewrite_expr_lambda(const Position& p, const AstPtr& m) {
        auto m0 = rewrite(m);
        return AstExprLambda(p, m0).clone();
    }

    virtual AstPtr rewrite_expr_match(const Position& p, const AstPtrs& mm, const AstPtr& g, const AstPtr& e) {
        auto mm0 = rewrites(mm);
        auto g0 = rewrite(g);
        auto e0 = rewrite(e);
        return AstExprMatch(p, mm0, g0, e0).clone();
    }

    virtual AstPtr rewrite_expr_block(const Position& p, const AstPtrs& alts) {
        auto alts0 = rewrites(alts);
        return AstExprBlock(p, alts0).clone();
    }

    virtual AstPtr rewrite_expr_let(const Position& p, const AstPtr& lhs, const AstPtr& rhs, const AstPtr& body) {
        auto lhs0 = rewrite(lhs);
        auto rhs0 = rewrite(rhs);
        auto body0 = rewrite(body);
        return AstExprLet(p, lhs0, rhs0, body0).clone();
    }

    virtual AstPtr rewrite_expr_tag(const Position& p, const AstPtr& e, const AstPtr& t) {
        auto e0 = rewrite(e);
        auto t0 = rewrite(t);
        return AstExprTag(p, e0, t0).clone();
    }

    virtual AstPtr rewrite_expr_if(const Position& p, const AstPtr& i, const AstPtr& t, const AstPtr& e) {
        auto i0 = rewrite(i);
        auto t0 = rewrite(t);
        auto e0 = rewrite(e);
        return AstExprIf(p, i0, t0, e0).clone();
    }

    virtual AstPtr rewrite_expr_try(const Position& p, const AstPtr& t, const AstPtr& c) {
        auto t0 = rewrite(t);
        auto c0 = rewrite(c);
        return AstExprTry(p, t0, c0).clone();
    }

    virtual AstPtr rewrite_expr_throw(const Position& p, const AstPtr& e) {
        auto e0 = rewrite(e);
        return AstExprThrow(p, e0).clone();
    }

    virtual AstPtr rewrite_directive_import(const Position& p, const UnicodeString& i) {
        return AstDirectImport(p, i).clone();
    }

    virtual AstPtr rewrite_directive_using(const Position& p, const UnicodeStrings& nn) {
        return AstDirectUsing(p, nn).clone();
    }

    virtual AstPtr rewrite_decl_data(const Position& p, const AstPtrs& nn) {
        auto nn0 = rewrites(nn);
        return AstDeclData(p, nn0).clone();
    }

    virtual AstPtr rewrite_decl_definition(const Position& p, const AstPtr& n, const AstPtr& e) {
        auto n0 = rewrite(n);
        auto e0 = rewrite(e);
        return AstDeclDefinition(p, n0, e0).clone();
    }

    virtual AstPtr rewrite_decl_operator(const Position& p, const AstPtr& c, const AstPtr& e) {
        auto c0 = rewrite(c);
        auto e0 = rewrite(e);
        return AstDeclOperator(p, c0, e0).clone();
    }

    virtual AstPtr rewrite_decl_object(const Position& p, const AstPtr& c, const AstPtrs& vv, const AstPtrs& ff) {
        auto c0 = rewrite(c);
        auto vv0 = rewrites(vv);
        auto ff0 = rewrites(ff);
        return AstDeclObject(p, c0, vv0, ff0).clone();
    }

    virtual AstPtr rewrite_decl_namespace(const Position& p, const UnicodeStrings& nn, const AstPtrs& dd) {
        auto dd0 = rewrites(dd);
        return AstDeclNamespace(p, nn, dd0).clone();
    }

    virtual AstPtr rewrite_wrapper(const Position& p, const AstPtrs& dd) {
        auto dd0 = rewrites(dd);
        return AstWrapper(p, dd0).clone();
    }

    virtual AstPtr rewrite(const AstPtr& a) {
        rewrite_pre(a);

        switch(a->tag()) {
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
            AST_EXPR_LIST_SPLIT(a, p, tt);
            return rewrite_expr_list(p, tt);
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
        case AST_DECL_OPERATOR: {
            AST_DECL_OPERATOR_SPLIT(a, p, c, e);
            return rewrite_decl_operator(p, c, e);
            break;
        }
        case AST_DECL_OBJECT: {
            AST_DECL_OBJECT_SPLIT(a, p, c, vv, ff);
            return rewrite_decl_object(p, c, vv, ff);
            break;
        }
        case AST_DECL_NAMESPACE: {
            AST_DECL_NAMESPACE_SPLIT(a, p, nn, dd);
            return rewrite_decl_namespace(p, nn, dd);
            break;
        }
        // wrapper
        case AST_WRAPPER:
            AST_WRAPPER_SPLIT(a, p, dd);
            return rewrite_wrapper(p, dd);
        }
        PANIC("rewrite exhausted");
        return nullptr;
    }
};


class Visit {
public:
    Visit() {}

    virtual void visit_pre(const AstPtr& a) {
    }

    void visits(const AstPtrs& dd) {
        for(auto& d: dd) {
            visit(d);
        }
    }

    virtual void visit_expr_integer(const Position& p, const UnicodeString& v) {
    }

    virtual void visit_expr_hexinteger(const Position& p, const UnicodeString& v) {
    }

    virtual void visit_expr_float(const Position& p, const UnicodeString& v) {
    }

    virtual void visit_expr_character(const Position& p, const UnicodeString& v) {
    }

    virtual void visit_expr_text(const Position& p, const UnicodeString& v) {
    }

    virtual void visit_expr_variable(const Position& p, const UnicodeString& n) {
    }

    virtual void visit_expr_wildcard(const Position& p, const UnicodeString& n) {
    }

    virtual void visit_expr_combinator(const Position& p, const UnicodeStrings& nn, const UnicodeString& n) {
    }

    virtual void visit_expr_operator(const Position& p, const UnicodeStrings& nn, const UnicodeString& n) {
    }

    virtual void visit_expr_tuple(const Position& p, const AstPtrs& tt) {
        visits(tt);
    }

    virtual void visit_expr_list(const Position& p, const AstPtrs& tt) {
        visits(tt);
    }

    virtual void visit_expr_application(const Position& p, const AstPtrs& tt) {
        visits(tt);
    }

    virtual void visit_expr_lambda(const Position& p, const AstPtr& m) {
        visit(m);
    }

    virtual void visit_expr_match(const Position& p, const AstPtrs& mm, const AstPtr& g, const AstPtr& e) {
        visits(mm);
        visit(g);
        visit(e);
    }

    virtual void visit_expr_block(const Position& p, const AstPtrs& alts) {
        visits(alts);
    }

    virtual void visit_expr_let(const Position& p, const AstPtr& lhs, const AstPtr& rhs, const AstPtr& body) {
        visit(lhs);
        visit(rhs);
        visit(body);
    }

    virtual void visit_expr_tag(const Position& p, const AstPtr& e, const AstPtr& t) {
        visit(e);
        visit(t);
    }

    virtual void visit_expr_if(const Position& p, const AstPtr& i, const AstPtr& t, const AstPtr& e) {
        visit(i);
        visit(t);
        visit(e);
    }

    virtual void visit_expr_try(const Position& p, const AstPtr& t, const AstPtr& c) {
        visit(t);
        visit(c);
    }

    virtual void visit_expr_throw(const Position& p, const AstPtr& e) {
        visit(e);
    }

    virtual void visit_directive_import(const Position& p, const UnicodeString& i) {
    }

    virtual void visit_directive_using(const Position& p, const UnicodeStrings& nn) {
    }

    virtual void visit_decl_data(const Position& p, const AstPtrs& nn) {
        visits(nn);
    }

    virtual void visit_decl_definition(const Position& p, const AstPtr& n, const AstPtr& e) {
        visit(n);
        visit(e);
    }

    virtual void visit_decl_operator(const Position& p, const AstPtr& c, const AstPtr& e) {
        visit(c);
        visit(e);
    }

    virtual void visit_decl_object(const Position& p, const AstPtr& c, const AstPtrs& vv, const AstPtrs& ff) {
        visit(c);
        visits(vv);
        visits(ff);
    }

    virtual void visit_decl_namespace(const Position& p, const UnicodeStrings& nn, const AstPtrs& dd) {
        visits(dd);
    }

    virtual void visit_wrapper(const Position& p, const AstPtrs& dd) {
        visits(dd);
    }

    virtual void visit(const AstPtr& a) {
        visit_pre(a);

        switch(a->tag()) {
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
            AST_EXPR_LIST_SPLIT(a, p, tt);
            return visit_expr_list(p, tt);
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
        case AST_DECL_OPERATOR: {
            AST_DECL_OPERATOR_SPLIT(a, p, c, e);
            return visit_decl_operator(p, c, e);
            break;
        }
        case AST_DECL_OBJECT: {
            AST_DECL_OBJECT_SPLIT(a, p, c, vv, ff);
            return visit_decl_object(p, c, vv, ff);
            break;
        }
        case AST_DECL_NAMESPACE: {
            AST_DECL_NAMESPACE_SPLIT(a, p, nn, dd);
            return visit_decl_namespace(p, nn, dd);
            break;
        }
        // wrapper
        case AST_WRAPPER:
            AST_WRAPPER_SPLIT(a, p, dd);
            return visit_wrapper(p, dd);
        }
        PANIC("visit exhausted");
    }

};

bool occurs(const AstPtr& t0, const AstPtr& t1);

AstPtr substitute(const AstPtr& term, const AstPtr& s0, const AstPtr& s1);

AstPtrSet freevars(const AstPtr& a);

#endif
