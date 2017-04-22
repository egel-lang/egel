#include "position.hpp"
#include "error.hpp"
#include "ast.hpp"
#include "transform.hpp"
#include "environment.hpp"

typedef enum {
    STATE_PATTERN_IN,
    STATE_PATTERN_OUT,
} pattern_state_t;

class RewritePattern: public Rewrite {
public:
    AstPtr pattern(const AstPtr& a) {
        return rewrite(a);
    }

    // state
    void set_pattern_state(pattern_state_t s) {
        _pattern_state = s;
    }

    pattern_state_t get_pattern_state() {
        return _pattern_state;
    }

    void enter_pattern() {
        set_pattern_state(STATE_PATTERN_IN);
    }

    void leave_pattern() {
        set_pattern_state(STATE_PATTERN_OUT);
    }

    bool in_pattern() {
        return _pattern_state == STATE_PATTERN_IN;
    }

    // helper functions
    void application_to_apps(const AstPtr& a, AstPtrs& aa) {
        switch (a->tag()) {
        case AST_EXPR_APPLICATION: {
                AST_EXPR_APPLICATION_SPLIT(a, p, l, r);
                application_to_apps(l, aa);
                aa.push_back(r);
            }
            break;
        case AST_NAME: {
                aa.push_back(a);
            }
            break;
        default:
            throw ErrorIdentification(a->position(), "expression name expected in pattern");
            break;
        }
    }

    AstPtr apps_to_record(const Position& p, const AstPtrs& aa) {
        AstPtr f;
        AstPtrs args;
        bool first = true;
        for (auto a:aa) {
            if (first) {
                first = false;
                f = a;
            } else {
                args.push_back(a);
            }
        }
        return AstExprRecord(p, f, args).clone();
    }

    // rewrites
    AstPtr rewrite_expr_application(const Position& p, const AstPtr& l, const AstPtr& r) override {
        if (in_pattern()) {
            AstPtrs apps;
            application_to_apps(l, apps);
            apps.push_back(r);
            auto a = apps_to_record(p, apps);
            return rewrite(a);
        } else {
            auto l0 = rewrite(l);
            auto r0 = rewrite(r);
            return AstExprApplication(p, l0, r0).clone();
        }
    }

    AstPtr rewrite_expr_lambda(const Position& p, const AstPtr& m) override {
        if (in_pattern()) throw ErrorIdentification(p, "lambda expression not allowed in pattern");
        auto m0 = rewrite(m);
        return AstExprLambda(p, m0).clone();
    }

    AstPtr rewrite_expr_match(const Position& p, const AstPtrs& mm, const AstPtr& g, const AstPtr& e) override {
        if (in_pattern()) throw ErrorIdentification(p, "match expression not allowed in pattern");
        enter_pattern();
        auto mm0 = rewrites(mm);
        leave_pattern();
        auto g0 = rewrite(g);
        auto e0 = rewrite(e);
        return AstExprMatch(p, mm0, g0, e0).clone();
    }

    AstPtr rewrite_expr_block(const Position& p, const AstPtrs& alts) override {
        if (in_pattern()) throw ErrorIdentification(p, "block expression not allowed in pattern");
        auto alts0 = rewrites(alts);
        return AstExprBlock(p, alts0).clone();
    }

    AstPtr rewrite_expr_let(const Position& p, const AstPtr& lhs, const AstPtr& rhs, const AstPtr& body) override {
        if (in_pattern()) throw ErrorIdentification(p, "let expression not allowed in pattern");
        enter_pattern();
        auto lhs0 = rewrite(lhs);
        leave_pattern();
        auto rhs0 = rewrite(rhs);
        auto body0 = rewrite(body);
        return AstExprLet(p, lhs0, rhs0, body0).clone();
    }

    AstPtr rewrite_expr_select(const Position& p, const AstPtr& e, const AstPtr& m) override {
        if (in_pattern()) throw ErrorIdentification(p, "select expression not allowed in pattern");
        auto e0 = rewrite(e);
        auto m0 = rewrite(m);
        return AstExprSelect(p, e0, m0).clone();
    }

    AstPtr rewrite_expr_tag(const Position& p, const AstPtr& e, const AstPtr& t) override {
        auto e0 = rewrite(e);
        auto t0 = rewrite(t);
        return AstExprTag(p, e0, t0).clone();
    }

    AstPtr rewrite_expr_if(const Position& p, const AstPtr& i, const AstPtr& t, const AstPtr& e) override {
        if (in_pattern()) throw ErrorIdentification(p, "if expression not allowed in pattern");
        auto i0 = rewrite(i);
        auto t0 = rewrite(t);
        auto e0 = rewrite(e);
        return AstExprLet(p, i0, t0, e0).clone();
    }

    AstPtr rewrite_expr_try(const Position& p, const AstPtr& t, const AstPtr& c) override {
        if (in_pattern()) throw ErrorIdentification(p, "try expression not allowed in pattern");
        auto t0 = rewrite(t);
        auto c0 = rewrite(c);
        return AstExprTry(p, t0, c0).clone();
    }

    AstPtr rewrite_expr_throw(const Position& p, const AstPtr& e) override {
        if (in_pattern()) throw ErrorIdentification(p, "throw expression not allowed in pattern");
        auto e0 = rewrite(e);
        return AstExprThrow(p, e0).clone();
    }

    AstPtr rewrite_decl_data(const Position& p, const AstPtrs& aa) override {
        return AstDeclData(p, aa).clone();
    }

    AstPtr rewrite_decl_definition(const Position& p, const AstPtr& n, const AstPtr& e) override {
        auto e0 = rewrite(e);
        return AstDeclDefinition(p, n, e0).clone();
    }

    AstPtr rewrite_decl_rule(const Position& p, const AstPtr& n, const AstPtr& e) override {
        auto e0 = rewrite(e);
        return AstDeclRule(p, n, e0).clone();
    }

private:
    pattern_state_t _pattern_state;
};

AstPtr pass_pattern(const AstPtr& a) {
    RewritePattern pattern;
    return pattern.pattern(a);
}


class RewriteQualify: public Rewrite {
public:
    RewriteQualify() {
    }

    // state manipulation
    void clear_qualifications() {
        _qualifications.clear();
    }

    AstPtrs get_qualifications() {
        return _qualifications;
    }

    void set_qualifications(AstPtrs qq) {
        _qualifications = qq;
    }

    // helper functions
    AstPtrs add_qualifications(const AstPtrs& qq0, const AstPtrs& qq1) {
        AstPtrs qq;
        for (auto& q:qq0) {
            qq.push_back(q);
        }
        for (auto& q:qq1) {
            qq.push_back(q);
        }
        return qq;
    }

    AstPtr add_qualification(const AstPtr& n) {
        if (n->tag() == AST_NAME) {
            AST_NAME_SPLIT(n, p, qq, s);
            return AstName(p, add_qualifications(get_qualifications(), qq), s).clone();
        } else {
            PANIC("name expected");
            return nullptr;
        }
    }

    AstPtrs name_to_qualification(const AstPtr& n) {
        if (n->tag() == AST_NAME) {
            AST_NAME_SPLIT(n, p, qq, s);
            qq.push_back(AstName(p, s).clone());
            return qq;
        } else {
            PANIC("name expected");
            return nullptr;
        }
    }

    AstPtr qualified_name_to_expression_constant(const AstPtr& n) {
        if (n->tag() != AST_NAME) {
            PANIC("name expected");
            return nullptr;
        }
        auto n0 = AST_NAME_CAST(n);
        return AstExprConstant(n0->position(), n0->qualified_name()).clone();
    }

    // rewrite rules
    AstPtr rewrite_decl_data(const Position& p, const AstPtrs& ee) override {
        AstPtrs ee0;
        for (auto& e: ee) {
            auto e0 = add_qualification(e);
            auto e1 = qualified_name_to_expression_constant(e0);
            ee0.push_back(e1);
        }
        return AstDeclData(p, ee0).clone();
    }

    AstPtr rewrite_decl_definition(const Position& p, const AstPtr& n, const AstPtr& e) override {
        auto n0 = add_qualification(n);
        auto n1 = qualified_name_to_expression_constant(n0);
        return AstDeclDefinition(p, n1, e).clone();
    }

    AstPtr rewrite_decl_rule(const Position& p, const AstPtr& n, const AstPtr& e) override {
        auto n0 = add_qualification(n);
        auto n1 = qualified_name_to_expression_constant(n0);
        return AstDeclRule(p, n1, e).clone();
    }

    AstPtr rewrite_decl_namespace(const Position& p, const AstPtr& n, const AstPtrs& dd) override {
        auto qq0 = get_qualifications();
        auto qq1 = name_to_qualification(n);
        auto qq2 = add_qualifications(qq0, qq1);
        set_qualifications(qq2);
        auto dd0 = rewrites(dd);
        set_qualifications(qq0);
        return AstDeclNamespace(p, n, dd0).clone();
    }

    AstPtr rewrite_wrapper(const Position& p, const AstPtrs& dd) override {
        auto dd0 = rewrites(dd);
        clear_qualifications();
        return AstWrapper(p, dd0).clone();
    }

private:
    AstPtrs     _qualifications;
};

AstPtr pass_qualify(const AstPtr& a) {
    RewriteQualify qualify;
    return qualify.rewrite(a);
}

AstPtr qualify(const AstPtr& a0) {
    auto a = pass_qualify(a0);
    return a;
}
