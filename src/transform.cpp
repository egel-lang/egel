#include "ast.hpp"
#include "transform.hpp"

class Occurs: public Visit {
public:
    bool occurs(const AstPtr& t0, const AstPtr& t1) {
        _term = t0;
        _found = false;
        visit(t1);
        return _found;
    }

    void visit(const AstPtr& t) override {
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
    AstPtr  _term;
    bool    _found;
};

bool occurs(const AstPtr& t0, const AstPtr& t1) {
    Occurs occurs;
    return occurs.occurs(t0, t1);
}

class Substitute: public Rewrite {
public:

    AstPtr substitute(const AstPtr& term, const AstPtr& s0, const AstPtr& s1) {
        _source = s0;
        _target = s1;
        return rewrite(term);
    }

    AstPtr rewrite_expr_match(const Position& p, const AstPtrs& mm, const AstPtr& g, const AstPtr& e) override {
        for (auto m:mm) {
            if (occurs(_source, m)) {
                return AstExprMatch(p, mm, g, e).clone();
            }
        }
        auto g0 = rewrite(g);
        auto e0 = rewrite(e);
        return AstExprMatch(p, mm, g0, e0).clone();
    }

    AstPtr rewrite_expr_let(const Position& p, const AstPtr& lhs, const AstPtr& rhs, const AstPtr& body) override {
        // XXX: rewrite this once that the lhs may be a pattern
        if (occurs(_source, lhs)) {
            return AstExprLet(p, lhs, rhs, body).clone();
        }
        auto rhs0 = rewrite(rhs);
        auto body0 = rewrite(body);
        return AstExprLet(p, lhs, rhs0, body0).clone();
    }

    AstPtr rewrite(const AstPtr& a) override {
        if (a == _source) {
            return _target;
        } else {
            return Rewrite::rewrite(a);
        }
    }

private:
    AstPtr  _source;
    AstPtr  _target;
};

AstPtr substitute(const AstPtr& term, const AstPtr& s0, const AstPtr& s1) {
    Substitute subs;
    return subs.substitute(term, s0, s1);
}

typedef enum {
    FREEVARS_INSERT,
    FREEVARS_REMOVE,
} freevars_state_t;

class FreeVars: public Visit {
public:
    AstPtrSet freevars(const AstPtr& a) {
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

    void insert(const AstPtr& a) {
        _fv.insert(a);
    }

    void remove(const AstPtr& a) {
        _fv.erase(a);
    }

    void visit_expr_variable(const Position& p, const UnicodeString& n) override {
        switch (get_state()) {
        case FREEVARS_INSERT:
            insert(AstExprVariable(p, n).clone());
            break;
        case FREEVARS_REMOVE:
            remove(AstExprVariable(p, n).clone());
            break;
        }
    }

    void visit_expr_match(const Position& p, const AstPtrs& mm, const AstPtr& g, const AstPtr& e) override {
        visit(g);
        visit(e);
        set_state(FREEVARS_REMOVE);
        visits(mm);
        set_state(FREEVARS_INSERT);
    }

    void visit_expr_let(const Position& p, const AstPtr& lhs, const AstPtr& rhs, const AstPtr& body) override {
        visit(rhs); // XXX: shouldn't introduce freevars?
        visit(body);
        set_state(FREEVARS_REMOVE);
        visit(lhs);
        set_state(FREEVARS_INSERT);
    }

    void visit_decl_object(const Position& p, const AstPtr& c, const AstPtrs& vv, const AstPtrs& ff, const AstPtrs& ee) override {
        visits(ff);
        visits(ee);
        set_state(FREEVARS_REMOVE);
        visits(vv);
        set_state(FREEVARS_INSERT);
    }

private:
    AstPtrSet           _fv;
    freevars_state_t    _state;
};

AstPtrSet freevars(const AstPtr& t) {
    FreeVars freevars;
    return freevars.freevars(t);
}
