#include <memory>
#include "ast.hpp"
#include "transform.hpp"
#include "environment.hpp"
#include "bytecode.hpp"
#include "emit.hpp"

typedef std::map<UnicodeString, reg_t> RegisterMap;
typedef std::unique_ptr<Coder>         CoderPtr;

class EmitData: public Visit {
public:
    void emit(VM* m, const AstPtr& a) {
        _machine = m;
        visit(a);
    }

    void visit_directive_import(const Position& p, const UnicodeString& i) override {
    }

    void visit_expr_combinator(const Position& p, const UnicodeStrings& nn, const UnicodeString& n) override {
        auto c = VMObjectData(_machine, nn, n).clone();
        _machine->define_data(c);
    }

    void visit_decl_data(const Position& p, const AstPtrs& nn) override {
        visits(nn);
    }

    void visit_decl_definition(const Position& p, const AstPtr& n, const AstPtr& e) override {
    }

    void visit_decl_operator(const Position& p, const AstPtr& c, const AstPtr& a0, const AstPtr& a1, const AstPtr& e) override {
    }

private:
    VM*             _machine;
};

void emit_data(VM* m, const AstPtr& a) {
    EmitData emit;
    emit.emit(m, a);
}

typedef enum {
    EMIT_PATTERN,
    EMIT_EXPR,
    EMIT_EXPR_ROOT,
//    EMIT_EXPR_CONSTANT, // XXX constant optimization not implemented yet
} emit_state_t;

class EmitCode: public Visit {
public:
    void emit(VM* vm, const AstPtr& a) {
        _machine = vm;
        _coder = std::unique_ptr<Coder>(new Coder());
        visit(a);
    }

    void set_state(const emit_state_t s) {
        _state = s;
    }

    emit_state_t get_state() const {
        return _state;
    }

    VM* get_machine() const {
        return _machine;
    }

    Coder* get_coder() {
        return _coder.get();
    }

    void set_register_frame(reg_t r) {
        _register_frame = r;
    }

    reg_t get_register_frame() const {
        return _register_frame;
    }

    void set_register_rt(reg_t r) {
        _register_rt = r;
    }

    reg_t get_register_rt() const {
        return _register_rt;
    }

    void set_register_rti(reg_t r) {
        _register_rti = r;
    }

    reg_t get_register_rti() const {
        return _register_rti;
    }

    void set_register_k(reg_t r) {
        _register_k = r;
    }

    reg_t get_register_k() const {
        return _register_k;
    }

    void set_register_exc(reg_t r) {
        _register_exc = r;
    }

    reg_t get_register_exc() const {
        return _register_exc;
    }

    void set_current_register(reg_t r) {
        _current_reg = r;
    }

    reg_t get_current_register() const {
        return _current_reg;
    }

    void set_fail_label(label_t l) {
        _fail = l;
    }

    label_t get_fail_label() const {
        return _fail;
    }

    void set_arity(uint_t a) {
        _arity = a;
    }

    uint_t get_arity() {
        return _arity;
    }

    void add_variable_binding(const UnicodeString& v, const reg_t t) {
        _variables[v] = t;
    }

    reg_t get_variable_binding(const UnicodeString& v) {
        return _variables[v];
    }


    void visit_expr_integer(const Position& p, const UnicodeString& v) override {
        switch(get_state()) {
        case EMIT_PATTERN: {
            auto r = get_current_register();
            auto l = get_fail_label();
            auto i = VMObjectInteger(convert_to_int(v)).clone();
            auto d = get_machine()->enter_data(i);

            auto ri = get_coder()->generate_register();

            get_coder()->emit_op_data(ri, d);
            get_coder()->emit_op_test(r, ri);
            get_coder()->emit_op_fail(l);
            }
            break;

        case EMIT_EXPR_ROOT:
        case EMIT_EXPR: {
            auto i = VMObjectInteger(convert_to_int(v)).clone();
            auto d = get_machine()->enter_data(i);

            auto rt  = get_register_rt();
            auto rti = get_register_rti();

            auto x   = get_coder()->generate_register();

            get_coder()->emit_op_data(x, d);
            get_coder()->emit_op_set(rt, rti, x);

            if (get_state() == EMIT_EXPR_ROOT) {
                auto k = get_register_k();
                get_coder()->emit_op_return(k);
            }
            }
            break;
        }
    }

    void visit_expr_float(const Position& p, const UnicodeString& v) override {
        switch(get_state()) {
        case EMIT_PATTERN: {
            auto r = get_current_register();
            auto l = get_fail_label();
            auto i = VMObjectFloat(convert_to_float(v)).clone();
            auto d = get_machine()->enter_data(i);

            auto ri = get_coder()->generate_register();

            get_coder()->emit_op_data(ri, d);
            get_coder()->emit_op_test(r, ri);
            get_coder()->emit_op_fail(l);
            }

            break;
        case EMIT_EXPR_ROOT:
        case EMIT_EXPR: {
            auto i = VMObjectFloat(convert_to_float(v)).clone();
            auto d = get_machine()->enter_data(i);

            auto rt  = get_register_rt();
            auto rti = get_register_rti();

            auto x   = get_coder()->generate_register();

            get_coder()->emit_op_data(x, d);
            get_coder()->emit_op_set(rt, rti, x);

            if (get_state() == EMIT_EXPR_ROOT) {
                auto k = get_register_k();
                get_coder()->emit_op_return(k);
            }
            }
            break;
        }
    }

    void visit_expr_character(const Position& p, const UnicodeString& v) override {
        switch(get_state()) {
        case EMIT_PATTERN: {
            auto r = get_current_register();
            auto l = get_fail_label();
            auto i = VMObjectChar(convert_to_char(v)).clone();
            auto d = get_machine()->enter_data(i);

            auto ri = get_coder()->generate_register();

            get_coder()->emit_op_data(ri, d);
            get_coder()->emit_op_test(r, ri);
            get_coder()->emit_op_fail(l);
            }
            break;

        case EMIT_EXPR_ROOT:
        case EMIT_EXPR: {
            auto i = VMObjectChar(convert_to_char(v)).clone();
            auto d = get_machine()->enter_data(i);

            auto rt  = get_register_rt();
            auto rti = get_register_rti();

            auto x   = get_coder()->generate_register();

            get_coder()->emit_op_data(x, d);
            get_coder()->emit_op_set(rt, rti, x);

            if (get_state() == EMIT_EXPR_ROOT) {
                auto k = get_register_k();
                get_coder()->emit_op_return(k);
            }
            }
            break;
        }
    }

    void visit_expr_text(const Position& p, const UnicodeString& v) override {
        switch(get_state()) {
        case EMIT_PATTERN: {
            auto r = get_current_register();
            auto l = get_fail_label();
            auto i = VMObjectText(convert_to_text(v)).clone();
            auto d = get_machine()->enter_data(i);

            auto ri = get_coder()->generate_register();

            get_coder()->emit_op_data(ri, d);
            get_coder()->emit_op_test(r, ri);
            get_coder()->emit_op_fail(l);
            }
            break;
        case EMIT_EXPR_ROOT:
        case EMIT_EXPR: {
            auto i = VMObjectText(convert_to_text(v)).clone();
            auto d = get_machine()->enter_data(i);

            auto rt  = get_register_rt();
            auto rti = get_register_rti();

            auto x   = get_coder()->generate_register();

            get_coder()->emit_op_data(x, d);
            get_coder()->emit_op_set(rt, rti, x);

            if (get_state() == EMIT_EXPR_ROOT) {
                auto k = get_register_k();
                get_coder()->emit_op_return(k);
            }
            }
            break;
        }
    }

    void visit_expr_variable(const Position& p, const UnicodeString& n) override {
        switch(get_state()) {
        case EMIT_PATTERN: {
            auto r = get_current_register();
            add_variable_binding(n, r);
            }
            break;
        case EMIT_EXPR_ROOT: {
            set_state(EMIT_EXPR);
            auto r = get_variable_binding(n);

            auto rt  = get_coder()->generate_register();
            auto rti = get_coder()->generate_register();
            auto k   = get_coder()->generate_register();
            auto exc = get_coder()->generate_register();
            auto c   = get_coder()->generate_register();

            auto t   = get_coder()->generate_register();

            get_coder()->emit_op_mov(rt, get_register_rt());
            get_coder()->emit_op_mov(rti, get_register_rti());
            get_coder()->emit_op_mov(k, get_register_k());
            get_coder()->emit_op_mov(exc, get_register_exc());
            get_coder()->emit_op_mov(c, r);
            get_coder()->emit_op_array(t, rt, c);

            auto x   = get_coder()->generate_register();
            auto f   = get_register_frame();

            get_coder()->emit_op_concatx(x, t, f, 5 + get_arity());
            set_register_k(x);
            get_coder()->emit_op_return(x);

            }
            break;
        case EMIT_EXPR: {
            auto r = get_variable_binding(n);

            auto rt  = get_register_rt();
            auto rti = get_register_rti();
            get_coder()->emit_op_set(rt, rti, r);
            }
            break;
        }
    }

    void visit_expr_combinator(const Position& p, const UnicodeStrings& nn, const UnicodeString& n) override {
        switch(get_state()) {
        case EMIT_PATTERN: {
            auto r = get_current_register();
            auto l = get_fail_label();
            auto c = get_machine()->get_data_string(nn, n);
            auto d = get_machine()->enter_data(c);

            auto ri = get_coder()->generate_register();

            get_coder()->emit_op_data(ri, d);
            get_coder()->emit_op_test(r, ri);
            get_coder()->emit_op_fail(l);
            }
            break;
        case EMIT_EXPR_ROOT:
        case EMIT_EXPR: {
            auto z = get_machine()->get_data_string(nn, n);
            auto d = get_machine()->enter_data(z);

            auto rt  = get_coder()->generate_register();
            auto rti = get_coder()->generate_register();
            auto k   = get_coder()->generate_register();
            auto exc = get_coder()->generate_register();
            auto c   = get_coder()->generate_register();

            auto t   = get_coder()->generate_register();

            get_coder()->emit_op_mov(rt, get_register_rt());
            get_coder()->emit_op_mov(rti, get_register_rti());
            get_coder()->emit_op_mov(k, get_register_k());
            get_coder()->emit_op_mov(exc, get_register_exc());
            get_coder()->emit_op_data(c, d);

            get_coder()->emit_op_array(t, rt, c);

            if (get_state() == EMIT_EXPR_ROOT) {
                set_state(EMIT_EXPR);
                auto x   = get_coder()->generate_register();
                auto f   = get_register_frame();

                get_coder()->emit_op_concatx(x, t, f, 5 + get_arity());
                get_coder()->emit_op_return(x);
            } else {
                set_register_k(t);
            }

            }
            break;
        }
    }

    void visit_expr_operator(const Position& p, const UnicodeStrings& nn, const UnicodeString& n) override {
        visit_expr_combinator(p, nn, n); // XXX: for now
    }

    void visit_expr_application(const Position& p, const AstPtrs& aa) override {
        switch(get_state()) {
        case EMIT_PATTERN: {
            auto r = get_current_register();
            auto l = get_fail_label();

            reg_t x = 0, y = 0;
            for (uint_t n = 0; n < aa.size(); n++) {
                y = get_coder()->generate_register();
                if (n == 0) x = y;
            }

            get_coder()->emit_op_split(x, y, r);
            get_coder()->emit_op_fail(l);

            reg_t n = x;
            for (auto& a:aa) {
                set_current_register(n);
                visit(a);
                n++;
            }
            }
            break;

        case EMIT_EXPR_ROOT:
        case EMIT_EXPR: { //XXX

            // generate labels rt, rti, k, exc, c, x .. y
            auto rt  = get_coder()->generate_register();
            auto rti = get_coder()->generate_register();
            auto k   = get_coder()->generate_register();
            auto exc = get_coder()->generate_register();
            auto c   = get_coder()->generate_register();

            reg_t x = 0, y = 0;
            uint_t sz = aa.size();
            for (uint_t n = 1; n < sz; n++) {
                y = get_coder()->generate_register();
                if (n==1) x = y;
            }

            // generate thunk label
            auto t   = get_coder()->generate_register();

            // fill rt, rti, k, exc, c, x .. y
            get_coder()->emit_op_mov(rt, get_register_rt());
            get_coder()->emit_op_mov(rti, get_register_rti());
            get_coder()->emit_op_mov(k, get_register_k());
            get_coder()->emit_op_mov(exc, get_register_exc());

            auto a = aa[0];
            bool head_flag;
            if (a->tag() == AST_EXPR_VARIABLE) {
                AST_EXPR_VARIABLE_SPLIT(a, p, n);
                auto r = get_variable_binding(n);
                get_coder()->emit_op_mov(c, r);
                head_flag = true;
            } else if (a->tag() == AST_EXPR_COMBINATOR) {
                AST_EXPR_COMBINATOR_SPLIT(a, p, nn, n);
                auto v = get_machine()->get_data_string(nn, n);
                auto d = get_machine()->enter_data(v);
                get_coder()->emit_op_data(c, d);
                head_flag = true;
            } else {
                get_coder()->emit_op_nil(c);
                head_flag = false;
            }

            reg_t z = x;
            for (uint_t n = 1; n < sz; n++) {
                get_coder()->emit_op_nil(z);
                z++;
            }
            get_coder()->emit_op_array(t, rt, y);

            // adjust for root
            auto root   = get_coder()->generate_register();
            auto state  = get_state();
            if (state == EMIT_EXPR_ROOT) {
                set_state(EMIT_EXPR);
                auto f   = get_register_frame();

                get_coder()->emit_op_concatx(root, t, f, 5 + get_arity());
            } else {
                root = t; // XXX: no mov?
            }
            k = root; set_register_k(k);
            rt = root; set_register_rt(rt);

            // generate thunks for nil fields
            if (!head_flag) {
                auto i = VMObjectInteger(0).clone();
                auto d = get_machine()->enter_data(i);
                get_coder()->emit_op_data(rti, d);

                set_register_rt(rt);
                set_register_rti(rti);

                visit(aa[0]);
            }

            for (uint_t n = 1; n < sz; n++) {
                auto i = VMObjectInteger(n+4).clone();
                auto d = get_machine()->enter_data(i);
                reg_t q = get_coder()->generate_register();
                get_coder()->emit_op_data(q, d);

                set_register_rt(rt);
                set_register_rti(q);

                visit(aa[n]);
            }

            // return k if root
            if (state == EMIT_EXPR_ROOT) {
                k = get_register_k();
                get_coder()->emit_op_return(k);
            }

            break;
            }
        }
    }

    void visit_expr_tag(const Position& p, const AstPtr& v, const AstPtr& t) override {
        switch(get_state()) {
        case EMIT_PATTERN: {
            auto r = get_current_register();
            auto l = get_fail_label();

            if (v->tag() == AST_EXPR_VARIABLE) {
                visit(v); // register the binding
            } else {
                PANIC("variable expected"); // XXX: turn into asserts
            }

            if (t->tag() == AST_EXPR_COMBINATOR) {
                AST_EXPR_COMBINATOR_SPLIT(t, p, nn, n);
                auto c = get_machine()->get_data_string(nn, n);
                auto d = get_machine()->enter_data(c);

                auto rt = get_coder()->generate_register();

                get_coder()->emit_op_data(rt, d);
                get_coder()->emit_op_tag(r, rt);
                get_coder()->emit_op_fail(l);
            } else {
                PANIC("combinator expected");
            }

            }
            break;
        case EMIT_EXPR_ROOT: 
        case EMIT_EXPR: {
            PANIC("tag in expression");
            }
            break;
        }
    }

    void visit_expr_match(const Position& p, const AstPtrs& mm, const AstPtr& g, const AstPtr& e) override {
        // we have memberberries
        auto member = get_coder()->peek_register();
        auto r = get_register_frame();

        auto l  = get_coder()->generate_label();
        set_fail_label(l);

        uint_t arity = mm.size();
        set_arity(arity);
        reg_t x = 0, y = 0;
        for (uint_t n = 0; n < arity; n++) {
            y = get_coder()->generate_register();
            if (n==0) x = y;
        }

        get_coder()->emit_op_takex(x, y, r, 5);
        get_coder()->emit_op_fail(l);

        set_state(EMIT_PATTERN);
        reg_t n = x;
        for (auto& m:mm) {
            set_current_register(n);
            n++;
            visit(m);
        }

        set_state(EMIT_EXPR_ROOT);
        visit(e);

        get_coder()->emit_label(l);
        get_coder()->restore_register(member);
    }

    void visit_expr_block(const Position& p, const AstPtrs& alts) override {
        // keep link registers invariant
        auto rt = get_register_rt();
        auto rti = get_register_rti();
        auto k = get_register_k();
        auto exc = get_register_exc();

        for (auto& a:alts) {
            set_register_rt(rt);
            set_register_rti(rti);
            set_register_k(k);
            set_register_exc(exc);
            visit(a);
        }
    }

    void visit_expr_try(const Position& p, const AstPtr& t, const AstPtr& c) override {
        auto rt  = get_register_rt();
        auto rti = get_register_rti();
        auto k   = get_register_k();
        auto exc = get_register_exc();
        if (c->tag() == AST_EXPR_COMBINATOR) {
            AST_EXPR_COMBINATOR_SPLIT(c, p, nn, n);
            auto c = get_machine()->get_data_string(nn, n);
            auto d = get_machine()->enter_data(c);

            auto rt0 = get_coder()->generate_register();
            auto rti0= get_coder()->generate_register();
            auto k0  = get_coder()->generate_register();
            auto exc0= get_coder()->generate_register();
            auto c0  = get_coder()->generate_register();

            auto exc1= get_coder()->generate_register();

            get_coder()->emit_op_mov(rt0, rt);
            get_coder()->emit_op_mov(rti0, rti);
            get_coder()->emit_op_mov(k0, k);
            get_coder()->emit_op_mov(exc0, exc);
            get_coder()->emit_op_data(c0, d);
            get_coder()->emit_op_array(exc1, rt0, c0);

            set_register_exc(exc1);

            visit(t);

            set_register_exc(exc);
        } else {
            PANIC("combinator expected");
        }
    }

    void visit_expr_throw(const Position& p, const AstPtr& e) override {
        auto exc = get_register_exc();

        auto rt0 = get_coder()->generate_register();
        /* auto rti0= */ get_coder()->generate_register();
        /* auto k0  = */ get_coder()->generate_register();
        /* auto exc0= */ get_coder()->generate_register();
        auto c0  = get_coder()->generate_register();

        get_coder()->emit_op_split(rt0, c0, exc);

        auto nil0= get_coder()->generate_register();
        auto exc1 = get_coder()->generate_register();

        get_coder()->emit_op_nil(nil0);
        get_coder()->emit_op_array(exc1, rt0, nil0);

        auto exc1i= get_coder()->generate_register();
        auto i = VMObjectInteger(5).clone();
        auto d = get_machine()->enter_data(i);
        get_coder()->emit_op_data(exc1i, d);

        set_register_k(exc1);
        set_register_rt(exc1);
        set_register_rti(exc1i);

        visit(e);
    }

    void visit_directive_import(const Position& p, const UnicodeString& i) override {
    }

    void visit_decl_data(const Position& p, const AstPtrs& nn) override {
        for (auto n:nn) {
            switch (n->tag()) {
            case AST_EXPR_COMBINATOR: {
                    AST_EXPR_COMBINATOR_SPLIT(n, p, ss, s);
                    auto d = VMObjectData(get_machine(), ss, s).clone();
                    get_machine()->define_data(d);
                }
                break;
            default:
                PANIC("combinator expected");
            }
        }
    }

    void visit_decl_definition(const Position& p, const AstPtr& n, const AstPtr& e) override {
        auto frame = get_coder()->generate_register();

        auto l = get_coder()->generate_label();
        set_fail_label(l);

        auto rt  = get_coder()->generate_register();
        auto rti = get_coder()->generate_register();
        auto k   = get_coder()->generate_register();
        auto exc = get_coder()->generate_register();
        auto c   = get_coder()->generate_register();

        set_register_frame(frame);
        set_register_rt(rt);
        set_register_rti(rti);
        set_register_k(k);
        set_register_exc(exc);
        set_arity(0);

        get_coder()->emit_op_takex(rt, c, frame, 0);
        get_coder()->emit_op_fail(l);
        set_state(EMIT_EXPR_ROOT);
        visit(e);
        get_coder()->emit_label(l);

        auto em   = get_coder()->generate_register();
        auto r    = get_coder()->generate_register();

        get_coder()->emit_op_array(em, rti, rt); // gen an empty array
        get_coder()->emit_op_concatx(r, em, frame, 4);
        get_coder()->emit_op_set(rt, rti, r);
        get_coder()->emit_op_return(k);

        auto code = get_coder()->code();
        AST_EXPR_COMBINATOR_SPLIT(n, p0, ss, s);
        auto b = VMObjectBytecode(get_machine(), code, ss, s).clone();

        get_coder()->reset();
        get_machine()->define_data(b);
    }

    void visit_decl_operator(const Position& p, const AstPtr& c, const AstPtr& a0, const AstPtr& a1, const AstPtr& e) override {
        if (a1->tag() == AST_EMPTY) {
            AST_EXPR_COMBINATOR_SPLIT(c,  p0, nn0, n0);
            AST_EXPR_COMBINATOR_SPLIT(a0, p1, nn1, n1);
            AST_EXPR_COMBINATOR_SPLIT(e,  p2, nn2, n2);

            auto s0 = get_machine()->enter_symbol(nn0,n0);
            auto s1 = get_machine()->enter_symbol(nn1,n1);
            auto s2 = get_machine()->enter_symbol(nn2,n2);

            get_machine()->enter_binding(s0, s1, s2);

            auto o = VMObjectPrefix(get_machine(), s0).clone();
            get_machine()->define_data(o);
        } else {
            AST_EXPR_COMBINATOR_SPLIT(c,  p0, nn0, n0);
            AST_EXPR_COMBINATOR_SPLIT(a0, p1, nn1, n1);
            AST_EXPR_COMBINATOR_SPLIT(a1, p2, nn2, n2);
            AST_EXPR_COMBINATOR_SPLIT(e,  p3, nn3, n3);

            auto s0 = get_machine()->enter_symbol(nn0,n0);
            auto s1 = get_machine()->enter_symbol(nn1,n1);
            auto s2 = get_machine()->enter_symbol(nn2,n2);
            auto s3 = get_machine()->enter_symbol(nn3,n3);

            get_machine()->enter_binding(s0, s1, s2, s3);

            auto o = VMObjectInfix(get_machine(), s0).clone();
            get_machine()->define_data(o);
        }
    }

private:
    emit_state_t    _state;
    VM*             _machine;

    reg_t           _register_frame;

    reg_t           _register_rt;
    reg_t           _register_rti;
    reg_t           _register_k;
    reg_t           _register_exc;

    uint_t          _arity;
    reg_t           _current_reg;
    label_t         _fail;
    CoderPtr        _coder;
    RegisterMap     _variables;
};

void emit_code(VM* m, const AstPtr& a) {
    EmitCode emit;
    emit.emit(m, a);
}

