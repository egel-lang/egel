#include "position.hpp"
#include "error.hpp"
#include "ast.hpp"
#include "transform.hpp"
#include "environment.hpp"
#include "semantical.hpp"

//XXX this is a concat we use pretty often and should be moved to a template?
UnicodeStrings concat(const UnicodeStrings& qq0, const UnicodeStrings& qq1) {
    UnicodeStrings qq;
    for (auto& q:qq0) {
        qq.push_back(q);
    }
    for (auto& q:qq1) {
        qq.push_back(q);
    }
    return qq;
}

typedef enum {
    STATE_DECLARE,
    STATE_DECLARE_IMPLICIT,
} declare_state_t;

class VisitDeclare: public Visit {
public:

    VisitDeclare() {
    }

    void declare(NamespacePtr& env, const AstPtr& a) {
        _spaces = env;
        visit(a);
    }

    // state manipulation
    void set_declare_state(declare_state_t s) {
        _declare_state = s;
    }

    declare_state_t get_declare_state() {
        return _declare_state;
    }

    void clear_qualifications() {
        _qualifications.clear();
    }

    UnicodeStrings get_qualifications() {
        return _qualifications;
    }

    void set_qualifications(UnicodeStrings qq) {
        _qualifications = qq;
    }

    // helper functions 
    UnicodeString qualified(const UnicodeStrings& nn, const UnicodeString n) {
        UnicodeString s;
        for (auto& n: nn) {
            s += n + '.';
        }
        s += n;
        return s;
    }

    // visits
    void visit_expr_combinator(const Position& p, const UnicodeStrings& nn, const UnicodeString& n) override {
        switch (get_declare_state()) {
        case STATE_DECLARE:
            try {
                auto nn0 = concat(_qualifications, nn);
                auto q = qualified(nn0, n);
                ::declare(_spaces, nn0, n, q);
            } catch (ErrorSemantical e) {
                throw ErrorSemantical(p, "redeclaration of " + n);
            }
            break;
        case STATE_DECLARE_IMPLICIT:
            try {
                auto nn0 = concat(_qualifications, nn);
                auto q = qualified(nn0, n);
                declare_implicit(_spaces, nn0, n, q);
            } catch (ErrorSemantical e) {
                throw ErrorSemantical(p, "redeclaration of " + n);
            }
            break;
        }
    }

    void visit_decl_data(const Position& p, const AstPtrs& nn) override {
        set_declare_state(STATE_DECLARE);
        visits(nn);
    }

    void visit_decl_definition(const Position& p, const AstPtr& n, const AstPtr& e) override {
        set_declare_state(STATE_DECLARE);
        visit(n);
    }

    void visit_decl_operator(const Position& p, const AstPtr& c, const AstPtr& a0, const AstPtr& a1, const AstPtr& e) override {
        set_declare_state(STATE_DECLARE_IMPLICIT);
        visit(c);
    }

    void visit_decl_namespace(const Position& p, const UnicodeStrings& nn, const AstPtrs& dd) override {
        auto nn0 = get_qualifications();
        auto nn1 = concat(nn0, nn);
        set_qualifications(nn1);
        visits(dd);
        set_qualifications(nn0);
    }

private:
    NamespacePtr    _spaces;
    UnicodeStrings  _qualifications;
    declare_state_t _declare_state;
};

void declare(NamespacePtr env, const AstPtr& a) {
    VisitDeclare declare;
    declare.declare(env, a);
}

typedef enum {
    STATE_IDENTIFY_USAGE,
    STATE_IDENTIFY_DECLARE,
    STATE_IDENTIFY_PATTERN,
} identify_state_t;

/*
 * Stuff this pass does:
 * + Check that all symbols have bindings.
 * + Fully qualify declarations.
 * + Flatten the namespace.
 */
class RewriteIdentify: public Rewrite {
public:
    AstPtr identify(NamespacePtr env, AstPtr a) {
        _identify_state = STATE_IDENTIFY_USAGE;
        _range = range_nil(env);
        return rewrite(a);
    }

    // state
    void set_identify_state(identify_state_t s) {
        _identify_state = s;
    }

    identify_state_t get_identify_state() {
        return _identify_state;
    }

    // range manipulation
    void declare(const Position& p, const UnicodeString& k, const UnicodeString& v) {
        try {
            ::declare(_range, k, v);
        } catch (ErrorSemantical e) {
            throw ErrorSemantical(p, "redeclaration of " + k);
        }
    }

    bool has(const UnicodeString& k) {
        UnicodeString tmp = ::get(_range, k);
        return (tmp != "");
    }

    UnicodeString get(const Position& p, const UnicodeString& k) {
        UnicodeString tmp = ::get(_range, k);
        if (tmp == "") {
            throw ErrorSemantical(p, "undeclared " + k);
        } else {
            return tmp;
        }
    }

    UnicodeString get(const Position& p, const UnicodeStrings& kk, const UnicodeString& k) {
        UnicodeString tmp = ::get(_range, kk, k);
        if (tmp == "") {
            throw ErrorSemantical(p, "undeclared " + k);
        } else {
            return tmp;
        }
    }

    void enter_range() {
        _range = ::enter_range(_range);
    }

    void leave_range() {
        _range = ::leave_range(_range);
    }

    void add_using(const UnicodeStrings& nn) {
        ::add_using(_range, nn);
    }

    // namespaces
    UnicodeStrings get_namespace() {
        return _namespace;
    }

    void set_namespace(UnicodeStrings n) {
        _namespace = n;
    }

    // push/pop declarations
    void push_declaration(const AstPtr& d) {
        _declarations.push_back(d);
    }

    AstPtrs pop_declarations() {
        return _declarations;
    }

    // rewrites
    AstPtr rewrite_expr_variable(const Position& p, const UnicodeString& v) override {
        switch (get_identify_state()) {
        case STATE_IDENTIFY_USAGE: {
                auto v1 = get(p, v);
                return AstExprVariable(p, v1).clone();
            }
            break;
        case STATE_IDENTIFY_PATTERN: {
                declare(p, v, v);
                return AstExprVariable(p, v).clone();
            }
            break;
        default:
            PANIC("unknown state");
            return nullptr;
        }
    }

    // XXX needs to be thought over..
    AstPtr rewrite_expr_combinator(const Position& p, const UnicodeStrings& nn, const UnicodeString& t) override {
        UnicodeStrings ee;
        switch (get_identify_state()) {
        case STATE_IDENTIFY_USAGE: {
                auto v = get(p, nn, t);
                auto c = AstExprCombinator(p, ee, v).clone();
                return c;
            }
            break;
        case STATE_IDENTIFY_DECLARE: {
                // XXX wait, wut?
                auto v = get(p, nn, t);
                auto c = AstExprCombinator(p, ee, v).clone();
                return c;
            }
            break;
        case STATE_IDENTIFY_PATTERN: {
                auto v = get(p, nn, t);
                auto c = AstExprCombinator(p, ee, v).clone();
                return c;
            }
            break;
        }
        return nullptr; // XXX: surpress warning?
    }

    // XXX needs to be thought over..
    AstPtr rewrite_expr_operator(const Position& p, const UnicodeStrings& nn, const UnicodeString& t) override {
        UnicodeStrings ee;
        switch (get_identify_state()) {
        case STATE_IDENTIFY_USAGE: {
                auto v = get(p, nn, t);
                auto c = AstExprOperator(p, ee, v).clone();
                return c;
            }
            break;
        case STATE_IDENTIFY_DECLARE: {
                auto v = get(p, nn, t);
                auto c = AstExprOperator(p, ee, v).clone();
                return c;
            }
            break;
        case STATE_IDENTIFY_PATTERN: {
                auto v = get(p, nn, t);
                auto c = AstExprOperator(p, ee, v).clone();
                return c;
            }
            break;
        }
        return nullptr; // XXX: surpress warning?
    }

    AstPtr rewrite_expr_match(const Position& p, const AstPtrs& mm, const AstPtr& g, const AstPtr& e) override {
        enter_range();
        set_identify_state(STATE_IDENTIFY_PATTERN);
        auto mm0 = rewrites(mm);
        set_identify_state(STATE_IDENTIFY_USAGE);
        auto g0 = rewrite(g);
        set_identify_state(STATE_IDENTIFY_USAGE);
        auto e0 = rewrite(e);
        leave_range();
        return AstExprMatch(p, mm0, g0, e0).clone();
    }

    AstPtr rewrite_expr_let(const Position& p, const AstPtr& lhs, const AstPtr& rhs, const AstPtr& body) override {
        enter_range();
        set_identify_state(STATE_IDENTIFY_PATTERN);
        auto lhs0 = rewrite(lhs);
        set_identify_state(STATE_IDENTIFY_USAGE);
        auto rhs0 = rewrite(rhs);
        set_identify_state(STATE_IDENTIFY_USAGE);
        auto body0 = rewrite(body);
        leave_range();
        return AstExprLet(p, lhs0, rhs0, body0).clone();
    }

    AstPtr rewrite_expr_tag(const Position& p, const AstPtr& e, const AstPtr& t) override {
        set_identify_state(STATE_IDENTIFY_PATTERN);
        auto e0 = rewrite(e);
        set_identify_state(STATE_IDENTIFY_USAGE);
        auto t0 = rewrite(t);
        set_identify_state(STATE_IDENTIFY_PATTERN);
        return AstExprTag(p, e0, t0).clone();
    }

    AstPtr rewrite_directive_using(const Position& p, const UnicodeStrings& nn) override {
        add_using(nn);
        return AstDirectUsing(p, nn).clone();
    }

    AstPtr rewrite_decl_data(const Position& p, const AstPtrs& ee) override {
        set_identify_state(STATE_IDENTIFY_DECLARE);
        AstPtrs ee0;
        for (auto& e:ee) {
            auto e0 = rewrite(e);
            ee0.push_back(e0);
        }
        auto a = AstDeclData(p, ee0).clone();
        push_declaration(a);
        return a;
    }

    AstPtr rewrite_decl_definition(const Position& p, const AstPtr& n, const AstPtr& e) override {
        set_identify_state(STATE_IDENTIFY_DECLARE);
        auto n0 = rewrite(n);
        set_identify_state(STATE_IDENTIFY_USAGE);
        auto e0 = rewrite(e);
        auto a = AstDeclDefinition(p, n0, e0).clone();
        push_declaration(a);
        return a;
    }

    AstPtr rewrite_decl_operator(const Position& p, const AstPtr& c, const AstPtr& a0, const AstPtr& a1, const AstPtr& e) override {
        set_identify_state(STATE_IDENTIFY_DECLARE);
        auto c0 = rewrite(c);
        set_identify_state(STATE_IDENTIFY_USAGE);
        auto b0 = rewrite(a0);
        auto b1 = rewrite(a1);
        auto e0 = rewrite(e);
        auto a = AstDeclOperator(p, c0, b0, b1, e0).clone();
        push_declaration(a);
        return a;
    }

    AstPtr rewrite_decl_namespace(const Position& p, const UnicodeStrings& nn, const AstPtrs& dd) override {
        auto nn0 = get_namespace();
        auto nn1 = concat(nn0, nn);
        set_namespace(nn1);
        enter_range();
        add_using(nn1);
        auto dd0 = rewrites(dd);
        leave_range();
        set_namespace(nn0);
        return AstDeclNamespace(p, nn, dd0).clone();
    }

    AstPtr rewrite_wrapper(const Position& p, const AstPtrs& dd) override {
        auto dd0 = rewrites(dd);
        auto aa = pop_declarations();
        return AstWrapper(p, aa).clone();
    }

private:
    identify_state_t    _identify_state;
    RangePtr            _range;
    AstPtrs             _declarations;
    UnicodeStrings      _namespace;
};

AstPtr identify(NamespacePtr env, const AstPtr& a) {
    RewriteIdentify identify;
    return identify.identify(env, a);
}

