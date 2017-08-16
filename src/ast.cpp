#include "utils.hpp"
#include "ast.hpp"

uint_t Ast::line_length = 80;

int compare_ast_tag(ast_tag_t t, const AstPtr& a0, const AstPtr& a1);

int compare_ast(const AstPtr& a0, const AstPtr& a1) {
    ast_tag_t t0 = a0->tag();
    ast_tag_t t1 = a1->tag();
    if (t0 < t1) {
        return -1;
    } else if (t1 < t0) {
        return 1;
    } else {
        return compare_ast_tag(t0, a0, a1);
    }
}

int equal_ast(const AstPtr& a0, const AstPtr& a1) {
    return compare_ast(a0, a1) == 0;
}

int compare_asts(const AstPtrs& aa0, const AstPtrs& aa1) {
    uint_t sz0 = aa0.size();
    uint_t sz1 = aa1.size();
    if (sz0 < sz1) {
        return -1;
    } else if (sz1 < sz0) {
        return 1;
    } else {
        uint_t sz = sz0;
        for (uint_t i = 0; i < sz; i++) {
            auto a0 = aa0[i];
            auto a1 = aa1[i];
            int c = compare_ast(a0, a1);
            if (c != 0) return c;
        }
        return 0;
    }
}

int compare_text(const UnicodeString& t0, const UnicodeString& t1) {
    return t0.compare(t1);
}

int compare_texts(const UnicodeStrings& aa0, const UnicodeStrings& aa1) {
    uint_t sz0 = aa0.size();
    uint_t sz1 = aa1.size();
    if (sz0 < sz1) {
        return -1;
    } else if (sz1 < sz0) {
        return 1;
    } else {
        uint_t sz = sz0;
        for (uint_t i = 0; i < sz; i++) {
            auto a0 = aa0[i];
            auto a1 = aa1[i];
            int c = compare_text(a0, a1);
            if (c != 0) return c;
        }
        return 0;
    }
}

int compare_ast2(const AstPtr& a0, const AstPtr& a1, const AstPtr& a2, const AstPtr& a3) {
    uint_t c = compare_ast(a0, a1);
    if (c != 0) return c;
    return compare_ast(a2, a3);
}

int compare_ast3(const AstPtr& a0, const AstPtr& a1, const AstPtr& a2, const AstPtr& a3, const AstPtr& a4, const AstPtr& a5) {
    uint_t c = compare_ast(a0, a1);
    if (c != 0) return c;
    c = compare_ast(a2, a3);
    if (c != 0) return c;
    return compare_ast(a4, a5);
}

int compare_ast_tag(ast_tag_t t, const AstPtr& a0, const AstPtr& a1) {
    int c;

    switch(t) {
    case AST_EMPTY: {
        return 0;
    }
    // literals
    case AST_EXPR_INTEGER: {
        AST_EXPR_INTEGER_SPLIT(a0, p0, t0);
        AST_EXPR_INTEGER_SPLIT(a1, p1, t1);
        return compare_text(t0, t1);
        break;
    }
    case AST_EXPR_HEXINTEGER: {
        AST_EXPR_HEXINTEGER_SPLIT(a0, p0, t0);
        AST_EXPR_HEXINTEGER_SPLIT(a1, p1, t1);
        return compare_text(t0, t1);
        break;
    }
    case AST_EXPR_FLOAT: {
        AST_EXPR_FLOAT_SPLIT(a0, p0, t0);
        AST_EXPR_FLOAT_SPLIT(a1, p1, t1);
        return compare_text(t0, t1);
        break;
    }
    case AST_EXPR_CHARACTER: {
        AST_EXPR_CHARACTER_SPLIT(a0, p0, t0);
        AST_EXPR_CHARACTER_SPLIT(a1, p1, t1);
        return compare_text(t0, t1);
        break;
    }
    case AST_EXPR_TEXT: {
        AST_EXPR_TEXT_SPLIT(a0, p0, t0);
        AST_EXPR_TEXT_SPLIT(a1, p1, t1);
        return compare_text(t0, t1);
        break;
    }
    // variables and constants
    case AST_EXPR_VARIABLE: {
        AST_EXPR_VARIABLE_SPLIT(a0, p0, t0);
        AST_EXPR_VARIABLE_SPLIT(a1, p1, t1);
        return compare_text(t0, t1);
        break;
    }
    case AST_EXPR_WILDCARD: {
        AST_EXPR_WILDCARD_SPLIT(a0, p0, t0);
        AST_EXPR_WILDCARD_SPLIT(a1, p1, t1);
        return compare_text(t0, t1);
        break;
    }
    case AST_EXPR_COMBINATOR: {
        AST_EXPR_COMBINATOR_SPLIT(a0, p0, q0, t0);
        AST_EXPR_COMBINATOR_SPLIT(a1, p1, q1, t1);
        c = compare_texts(q0, q1);
        if (c != 0) return c;
        return compare_text(t0, t1);
        break;
    }
    case AST_EXPR_OPERATOR: {
        AST_EXPR_OPERATOR_SPLIT(a0, p0, q0, t0);
        AST_EXPR_OPERATOR_SPLIT(a1, p1, q1, t1);
        c = compare_texts(q0, q1);
        if (c != 0) return c;
        return compare_text(t0, t1);
        break;
    }
    //  list and tuple
    case AST_EXPR_LIST: {
        AST_EXPR_LIST_SPLIT(a0, p0, tt0);
        AST_EXPR_LIST_SPLIT(a1, p1, tt1);
        return compare_asts(tt0, tt1);
        break;
    }
    case AST_EXPR_TUPLE: {
        AST_EXPR_TUPLE_SPLIT(a0, p0, tt0);
        AST_EXPR_TUPLE_SPLIT(a1, p1, tt1);
        return compare_asts(tt0, tt1);
        break;
    }
    // compound statements
    case AST_EXPR_APPLICATION: {
        AST_EXPR_APPLICATION_SPLIT(a0, p0, aa0);
        AST_EXPR_APPLICATION_SPLIT(a1, p1, aa1);
        return compare_asts(aa0, aa1);
        break;
    }
    case AST_EXPR_LAMBDA: {
        AST_EXPR_LAMBDA_SPLIT(a0, p0, m0);
        AST_EXPR_LAMBDA_SPLIT(a1, p1, m1);
        return compare_ast(m0, m1);
        break;
    }
    case AST_EXPR_BLOCK: {
        AST_EXPR_BLOCK_SPLIT(a0, p0, mm0);
        AST_EXPR_BLOCK_SPLIT(a1, p1, mm1);
        return compare_asts(mm0, mm1);
        break;
    }
    case AST_EXPR_MATCH: {
        AST_EXPR_MATCH_SPLIT(a0, p0, mm0, g0, e0);
        AST_EXPR_MATCH_SPLIT(a1, p1, mm1, g1, e1);
        c = compare_asts(mm0, mm1);
        if (c != 0) return c;
        return compare_ast2(g0, g1, e0, e1);
        break;
    }
    case AST_EXPR_LET: {
        AST_EXPR_LET_SPLIT(a0, p0, l0, r0, e0);
        AST_EXPR_LET_SPLIT(a1, p1, l1, r1, e1);
        return compare_ast3(l0, l1, r0, r1, e0, e1);
        break;
    }
    case AST_EXPR_TAG: {
        AST_EXPR_TAG_SPLIT(a0, p0, e0, t0);
        AST_EXPR_TAG_SPLIT(a1, p1, e1, t1);
        return compare_ast2(e0, e1, t0, t1);
        break;
    }
    case AST_EXPR_IF: {
        AST_EXPR_IF_SPLIT(a0, p0, i0, t0, e0);
        AST_EXPR_IF_SPLIT(a1, p1, i1, t1, e1);
        return compare_ast3(i0, i1, t0, t1, e0, e1);
        break;
    }
    case AST_EXPR_TRY: {
        AST_EXPR_TRY_SPLIT(a0, p0, t0, c0);
        AST_EXPR_TRY_SPLIT(a1, p1, t1, c1);
        break;
    }
    case AST_EXPR_THROW: {
        AST_EXPR_THROW_SPLIT(a0, p0, exc0);
        AST_EXPR_THROW_SPLIT(a1, p1, exc1);
        break;
    }
    // directives
    case AST_DIRECT_IMPORT: {
        AST_DIRECT_IMPORT_SPLIT(a0, p0, n0);
        AST_DIRECT_IMPORT_SPLIT(a1, p1, n1);
        return compare_text(n0, n1);
        break;
    }
    case AST_DIRECT_USING: {
        AST_DIRECT_USING_SPLIT(a0, p0, pp0);
        AST_DIRECT_USING_SPLIT(a1, p1, pp1);
        return compare_texts(pp0, pp1);
        break;
    }
    // declarations
    case AST_DECL_DATA: {
        AST_DECL_DATA_SPLIT(a0, p0, nn0);
        AST_DECL_DATA_SPLIT(a1, p1, nn1);
        return compare_asts(nn0, nn1);
        break;
    }
    case AST_DECL_DEFINITION: {
        AST_DECL_DEFINITION_SPLIT(a0, p0, n0, e0);
        AST_DECL_DEFINITION_SPLIT(a1, p1, n1, e1);
        return compare_ast2(n0, n1, e0, e1);
        break;
    }
    case AST_DECL_OPERATOR: {
        AST_DECL_OPERATOR_SPLIT(a0, p0, c0, e0);
        AST_DECL_OPERATOR_SPLIT(a1, p1, c1, e1);
        return compare_ast2(c0, c1, e0, e1);
        break;
    }
    case AST_DECL_OBJECT: {
        AST_DECL_OBJECT_SPLIT(a0, p0, c0, vv0, ff0, ee0);
        AST_DECL_OBJECT_SPLIT(a1, p1, c1, vv1, ff1, ee1);
        c = compare_ast(c0, c1);
        if (c != 0) return c;
        c = compare_asts(vv0, vv1);
        if (c != 0) return c;
        c = compare_asts(ff0, ff1);
        if (c != 0) return c;
        return compare_asts(ee0, ee1);
        break;
    }
    case AST_DECL_NAMESPACE: {
        AST_DECL_NAMESPACE_SPLIT(a0, p0, nn0, dd0);
        AST_DECL_NAMESPACE_SPLIT(a1, p1, nn1, dd1);
        c = compare_texts(nn0, nn1);
        if (c != 0) return c;
        return compare_asts(dd0, dd1);
        break;
    }
    // wrapper
    case AST_WRAPPER: {
        AST_WRAPPER_SPLIT(a0, p0, dd0);
        AST_WRAPPER_SPLIT(a1, p1, dd1);
        return compare_asts(dd0, dd1);
        break;
    }
    case AST_VAR: {
        AST_VAR_SPLIT(a0, p0, l0, r0);
        AST_VAR_SPLIT(a1, p1, l1, r1);
        return compare_ast2(l0, l1, r0, r1);
        break;
    }
    default:
        PANIC("compare ast failed");
    }
    return 0;
}

