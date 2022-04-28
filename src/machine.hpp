#pragma once

#include <iomanip>
#include <sstream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <tuple>
#include <vector>
#include <thread>
#include <chrono>

#include "runtime.hpp"
#include "allocate.hpp"
#include "environment.hpp"
#include "assembler.hpp"
#include "eval.hpp"
#include "modules.hpp"

class SymbolTable {
public:
    SymbolTable()
    :
    _texts(std::vector<icu::UnicodeString>),
    _objects(std::vector<const vm_object_t*>),
    _index(std::map<icu::UnicodeString, symbol_t>)
    }

    bool member(const icu::UnicodeString &s) const {
        return (_index.count(s) > 0);
    }

    symbol_t enter(const icu::UnicodeString &s, const vm_object_t *o) {
        if (member(s)) {
            auto n = _index(s);
            _objects[n] = o;
        } else {
            symbol_t n = _texts.size();
            _index[s] = n;
            _texts[n] = s;
            _objects[n] = o;
        }
    }

    symbol_t enter(const icu::UnicodeString &n0, const icu::UnicodeString &n1, const vm_object_t* o) {
        icu::UnicodeString n = n0 + STRING_DCOLON + n1;
        return enter(n, o);
    }

    symbol_t enter(const UnicodeStrings &nn, const icu::UnicodeString &n, const vm_object_t *o) {
        icu::UnicodeString s;
        for (auto &n0 : nn) {
            s += n0 + STRING_DCOLON;
        }
        s += n;
        return enter(s, o);
    }

    int size() const {
        return _text.size();
    }

    symbol_t get_index(const icu::UnicodeString &s) {
        return _index[s];
    }

    icu::UnicodeString get_text(const symbol_t &s) {
        return _text[s];
    }
    vm_object_t* get_object(const symbol_t &s) {
        return _objects[s];
    }
    vm_object_t* get_object(const icu::UnicodeString &s) {
        if (member(s)) {
            auto n = get_index(s);
            return get_object(n);
        } else {
            return nullptr;
        }
    }

    void render(std::ostream &os) {
        for (size_t t = 0; t < _objects.size(); t++) {
            os << std::setw(8) << t << "=" << render(_to[t]) << std::endl;
        }
    }

private:
    std::vector<icu::UnicodeString> _texts;
    std::vector<const vm_object_t*> _objects;
    std::map<icu::UnicodeString, symbol_t> _index;
};

class VMObjectResult : public VMObjectCombinator {
public:
    VMObjectResult(VM *m, const symbol_t s, VMReduceResult *r, const bool exc)
        : VMObjectCombinator(VM_SUB_BUILTIN, m, s),
          _result(r),
          _exception(exc){};

    VMObjectResult(const VMObjectResult &d)
        : VMObjectResult(d.machine(), d.symbol(), d._result, d._exception) {
    }

    static vm_object_t* create(VM *m, const symbol_t s, VMReduceResult *r,
                              const bool exc) {
        return vm_object_t*(new VMObjectResult(m, s, r, exc));
    }

    vm_object_t* reduce(const vm_object_t* &thunk) const override {
        auto tt = VM_OBJECT_ARRAY_VALUE(thunk);
        auto arg0 = tt[5];

        _result->result = arg0;
        _result->exception = _exception;
        return nullptr;
    }

private:
    VMReduceResult *_result;
    bool _exception;
};

class Machine final: public VM {
public:

    Machine() {
        populate();
    }

    virtual ~Machine() {
    }

    static VMPtr create() {
        return std::make_shared<Machine>();
    }

    void populate() {
        // symbol and data table initialization
        auto i = _symbols.enter(STRING_SYSTEM, STRING_INT);
        auto f = _symbols.enter(STRING_SYSTEM, STRING_FLOAT);
        auto c = _symbols.enter(STRING_SYSTEM, STRING_CHAR);
        auto t = _symbols.enter(STRING_SYSTEM, STRING_TEXT);
        ASSERT(i == SYMBOL_INT);
        ASSERT(f == SYMBOL_FLOAT);
        ASSERT(c == SYMBOL_CHAR);
        ASSERT(t == SYMBOL_TEXT);
        // necessary 'type' definitions
        _int = VMObjectData::create(this, i);
        _float = VMObjectData::create(this, f);
        _char = VMObjectData::create(this, c);
        _text = VMObjectData::create(this, t);
        _data.enter(_int);
        _data.enter(_float);
        _data.enter(_char);
        _data.enter(_text);
        auto none0 = _symbols.enter(STRING_SYSTEM, STRING_NONE);
        auto true0 = _symbols.enter(STRING_SYSTEM, STRING_TRUE);
        auto false0 = _symbols.enter(STRING_SYSTEM, STRING_FALSE);
        ASSERT(none0 == SYMBOL_NONE);
        ASSERT(true0 == SYMBOL_TRUE);
        ASSERT(false0 == SYMBOL_FALSE);
        _none = VMObjectData::create(this, none0);
        _true = VMObjectData::create(this, true0);
        _false = VMObjectData::create(this, false0);
        _data.enter(_none);
        _data.enter(_true);
        _data.enter(_false);
        auto tuple0 = _symbols.enter(STRING_SYSTEM, STRING_TUPLE);
        auto nil0 = _symbols.enter(STRING_SYSTEM, STRING_NIL);
        auto cons0 = _symbols.enter(STRING_SYSTEM, STRING_CONS);
        ASSERT(tuple0 == SYMBOL_TUPLE);
        ASSERT(nil0 == SYMBOL_NIL);
        ASSERT(cons0 == SYMBOL_CONS);

        _tuple = VMObjectData::create(this, tuple0);
        _nil = VMObjectData::create(this, nil0);
        _cons = VMObjectData::create(this, cons0);
        _data.enter(_tuple);
        _data.enter(_nil);
        _data.enter(_cons);

        ASSERT(VM_OBJECT_NONE_TEST(_none));
        ASSERT(VM_OBJECT_TRUE_TEST(_true));
        ASSERT(VM_OBJECT_FALSE_TEST(_false));
        ASSERT(VM_OBJECT_TUPLE_TEST(_tuple));
        ASSERT(VM_OBJECT_NIL_TEST(_nil));
        ASSERT(VM_OBJECT_CONS_TEST(_cons));
    }

    // initialize
    void initialize(OptionsPtr oo) override {
        NamespacePtr env = Namespace::create();

        _options = oo;
        _manager = ModuleManager::create();
        _manager->init(oo, this);
        _eval = Eval::create();
        _eval->init(_manager);
    }

    // symbol table manipulation
    bool has_symbol(const icu::UnicodeString &n) override {
        return _symbols.member(n);
    }

    symbol_t enter_symbol(const icu::UnicodeString &n, const vm_object_t *o) override {
        return _symbols.enter(n);
    }

    symbol_t enter_symbol(const icu::UnicodeString &n0,
                          const icu::UnicodeString &n1, const vm_object_t *o) override {
        return _symbols.enter(n0, n1);
    }

    symbol_t enter_symbol(const UnicodeStrings &nn,
                          const icu::UnicodeString &n, const vm_object_t *o) override {
        return _symbols.enter(nn, n);
    }

    virtual int get_combinators_size() override {
        return _symbols.size();
    }

    vm_object_t* get_symbol(const symbol_t s) override {
    }

    icu::UnicodeString get_symbol_string(symbol_t s) override {
        return _symbols.get(s);
    }

    vm_object_t* get_symbol(const icu::UnicodeString &n) override {
    }

    vm_object_t* get_symbol(const icu::UnicodeString &n0,
                               const icu::UnicodeString &n1) override {
    }

    vm_object_t* get_symbol(const std::vector<icu::UnicodeString> &nn,
                               const icu::UnicodeString &n) override {
    }

    void define(const vm_object_t* &o) override {
        // define an undefined symbol
        auto s = o->to_text();  // XXX: usually works? probably not for {}
        if (_symbols.member(s)) {
            throw create_text("redeclaration of " + s);
        } else {
            enter_symbol(s);
            enter_data(o);
        }
    }

    void overwrite(const vm_object_t* &o) override {
        // define or overwrite
        auto s = o->to_text();  // XXX: usually works?
        enter_symbol(s);
        enter_data(o);
    }

    vm_object_t* get(const vm_object_t* &o) override {
        // get a defined symbol
        return get_combinator(o->to_text());
    }

    // reduce an expression
    void reduce(const vm_object_t* &f, const vm_object_t* &ret,
                const vm_object_t* &exc, reducer_state_t *run) override {
        vm_object_t*s rr;
        rr.push_back(nullptr);  // rt
        rr.push_back(nullptr);  // rti
        rr.push_back(nullptr);  // k
        rr.push_back(nullptr);  // exc
        rr.push_back(ret);      // c
        rr.push_back(nullptr);  // arg0
        auto r = create_array(rr);

        vm_object_t*s ee;
        ee.push_back(nullptr);  // rt
        ee.push_back(nullptr);  // rti
        ee.push_back(nullptr);  // k
        ee.push_back(nullptr);  // exc
        ee.push_back(exc);      // c
        ee.push_back(nullptr);  // arg0
        auto e = create_array(ee);

        auto i = VMObjectInteger::create(5);
        vm_object_t*s tt;
        tt.push_back(r);  // rt
        tt.push_back(i);  // rti
        tt.push_back(r);  // k
        tt.push_back(e);  // exc
        tt.push_back(f);  // c
        auto t = create_array(tt);

        auto trampoline = t;
        while ((trampoline != nullptr) && (*run != HALTED)) {
            if (*run == RUNNING) {
                ASSERT(trampoline->tag() == VM_OBJECT_ARRAY);
                auto f = VM_OBJECT_ARRAY_CAST(trampoline)->get(4);
#ifdef DEBUG
                std::cout << "trace: " << f << std::endl;
                std::cout << "on : " << trampoline << std::endl;
#endif
                trampoline = f->reduce(trampoline);
            } else if (*run == SLEEPING) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } else {  // *run == HALTED
            }
        }
    }

    void reduce(const vm_object_t* &f, const vm_object_t* &ret,
                const vm_object_t* &exc) override {
        reducer_state_t run = RUNNING;
        reduce(f, ret, exc, &run);
    }

    VMReduceResult reduce(const vm_object_t* &f, reducer_state_t *run) override {
        VMReduceResult r;

        auto sm = enter_symbol("Internal", "result");
        auto m = VMObjectResult::create(this, sm, &r, false);

        auto se = enter_symbol("Internal", "exception");
        auto e = VMObjectResult::create(this, se, &r, true);

        reduce(f, m, e, run);
        return r;
    }

    VMReduceResult reduce(const vm_object_t* &f) override {
        reducer_state_t run = RUNNING;
        return reduce(f, &run);
    }

    void lock() override {
        _mutex.lock();
    }

    void unlock() override {
        _mutex.unlock();
    }

    void render(std::ostream &os) override {
        os << "SYMBOLS: " << std::endl;
        _symbols.render(os);
        os << "DATA: " << std::endl;
        _data.render(os);
    }

    void *get_context() const override {
        return _context;
    }

    void set_context(void *m) override {
        _context = m;
    }

    vm_tag_t get_tag(const vm_object_t* &o) override {
        return o->tag();
    }

    vm_subtag_t get_subtag(const vm_object_t* &o) override {
        return o->subtag();
    }

    // primitive values
    vm_object_t* create_integer(const vm_int_t n) override {
        return VMObjectInteger::create(n);
    }

    vm_object_t* create_char(const vm_char_t c) override {
        return VMObjectChar::create(c);
    }

    vm_object_t* create_text(const vm_text_t s) override {
        return VMObjectText::create(s);
    }

    vm_object_t* create_float(const vm_float_t f) override {
        return VMObjectFloat::create(f);
    }

    bool is_integer(const vm_object_t* &o) override {
        return o->tag() == VM_OBJECT_INTEGER;
    }

    bool is_float(const vm_object_t* &o) override {
        return o->tag() == VM_OBJECT_FLOAT;
    }

    bool is_char(const vm_object_t* &o) override {
        return o->tag() == VM_OBJECT_CHAR;
    }

    bool is_text(const vm_object_t* &o) override {
        return o->tag() == VM_OBJECT_TEXT;
    }

    vm_int_t get_integer(const vm_object_t* &o) override {
        return VM_OBJECT_INTEGER_VALUE(o);
    }

    vm_float_t get_float(const vm_object_t* &o) override {
        return VM_OBJECT_FLOAT_VALUE(o);
    }

    vm_char_t get_char(const vm_object_t* &o) override {
        return VM_OBJECT_CHAR_VALUE(o);
    }

    vm_text_t get_text(const vm_object_t* &o) override {
        return VM_OBJECT_TEXT_VALUE(o);
    }

    vm_object_t* create_none() override {
        return get_data(SYMBOL_NONE);
    }

    vm_object_t* create_true() override {
        return get_data(SYMBOL_TRUE);
    }

    vm_object_t* create_false() override {
        return get_data(SYMBOL_FALSE);
    }

    // predefined constants
    vm_object_t* create_bool(const bool b) override {
        if (b) {
            return create_true();
        } else {
            return create_false();
        }
    }

    vm_object_t* create_nil() override {
        return _nil;
    }

    vm_object_t* create_cons() override {
        return _cons;
    }

    vm_object_t* create_tuple() override {
        return _tuple;
    }

    bool is_none(const vm_object_t* &o) override {
        // XXX: fixme
        return (o->symbol() == SYMBOL_NONE)
    }

    bool is_true(const vm_object_t* &o) override {
        return (o->symbol() == SYMBOL_TRUE)
    }

    bool is_false(const vm_object_t* &o) override {
        return (o->symbol() == SYMBOL_FALSE)
    }

    bool is_bool(const vm_object_t* &o) override {
        return is_false(o) || is_true(o);
    }

    bool is_nil(const vm_object_t* &o) override {
        return (o->symbol() == SYMBOL_NIL)
    }

    bool is_cons(const vm_object_t* &o) override {
        return (o->symbol() == SYMBOL_CONS)
    }

    bool is_tuple(const vm_object_t* &o) override {
        return (o->symbol() == SYMBOL_TUPLE)
    }

    vm_object_t* create_array(const vm_object_t*s &oo) override {
        return VMObjectArray::create(oo);
    }

    vm_object_t* create_array(const unsigned int size) override {
        return VMObjectArray::create(size);
    }

    bool is_array(const vm_object_t* &o) override {
        return VM_OBJECT_ARRAY_TEST(o);
    }

    unsigned int array_size(const vm_object_t* &o) override {
        return VM_OBJECT_ARRAY_CAST(o)->size();
    }

    vm_object_t* array_get(const vm_object_t* &o, int n) override {
        return VM_OBJECT_ARRAY_CAST(o)->get(n);
    }

    void array_set(vm_object_t* &o, int n, const vm_object_t* &e) override {
        VM_OBJECT_ARRAY_CAST(o)->set(n, e);
    }

    vm_object_t*s get_array(const vm_object_t* &o) override {
        return VM_OBJECT_ARRAY_VALUE(o);
    }

    bool is_combinator(const vm_object_t* &o) override {
        return VM_OBJECT_COMBINATOR_TEST(o);
    }

    bool is_opaque(const vm_object_t* &o) override {
        return VM_OBJECT_OPAQUE_TEST(o);
    }

    bool is_data(const vm_object_t* &o) override {
        return VM_OBJECT_DATA_TEST(o);
    }

    bool is_bytecode(const vm_object_t* &o) override {
        return (o->subtag_test(VM_SUB_BYTECODE));
    }

    icu::UnicodeString get_bytecode(const vm_object_t* &o) override {
        Disassembler d(o);
        return d.disassemble();
    }

    vm_object_t*s get_bytedata(const vm_object_t* &o) override {
        auto b = VMObjectBytecode::cast(o);
        return b->get_data_list();
    }

    vm_object_t* create_bytecode(const icu::UnicodeString &s) override {
        Assembler a(this, s);
        return a.assemble();
    }

    icu::UnicodeString symbol(const vm_object_t* &o) override {
        return _symbols.get(o->symbol());
    }

    vm_object_t* create_data(const icu::UnicodeString &n) override {
        return VMObjectData::create(this, n);
    }

    vm_object_t* create_data(const icu::UnicodeString &n0,
                            const icu::UnicodeString &n1) override {
        return VMObjectData::create(this, n0, n1);
    }

    vm_object_t* create_data(const std::vector<icu::UnicodeString> &nn,
                            const icu::UnicodeString &n) override {
        return VMObjectData::create(this, nn, n);
    }

    vm_object_t* create_medadic(const icu::UnicodeString &s,
                               std::function<vm_object_t*()> f) override {
        return MedadicCallback::create(this, s, f);
    }

    vm_object_t* create_medadic(const icu::UnicodeString &s0,
                               const icu::UnicodeString &s1,
                               std::function<vm_object_t*()> f) override {
        return MedadicCallback::create(this, s0, s1, f);
    }

    vm_object_t* create_medadic(const std::vector<icu::UnicodeString> &ss,
                               const icu::UnicodeString &s,
                               std::function<vm_object_t*()> f) override {
        return MedadicCallback::create(this, ss, s, f);
    }

    vm_object_t* create_monadic(
        const icu::UnicodeString &s,
        std::function<vm_object_t*(const vm_object_t* &a0)> f) override {
        return MonadicCallback::create(this, s, f);
    }

    vm_object_t* create_monadic(
        const icu::UnicodeString &s0, const icu::UnicodeString &s1,
        std::function<vm_object_t*(const vm_object_t* &a0)> f) override {
        return MonadicCallback::create(this, s0, s1, f);
    }

    vm_object_t* create_monadic(
        const std::vector<icu::UnicodeString> &ss, const icu::UnicodeString &s,
        std::function<vm_object_t*(const vm_object_t* &a0)> f) override {
        return MonadicCallback::create(this, ss, s, f);
    }

    vm_object_t* create_dyadic(
        const icu::UnicodeString &s,
        std::function<vm_object_t*(const vm_object_t* &a0, const vm_object_t* &a1)>
            f) override {
        return DyadicCallback::create(this, s, f);
    }

    vm_object_t* create_dyadic(
        const icu::UnicodeString &s0, const icu::UnicodeString &s1,
        std::function<vm_object_t*(const vm_object_t* &a0, const vm_object_t* &a1)>
            f) override {
        return DyadicCallback::create(this, s0, s1, f);
    }

    vm_object_t* create_dyadic(
        const std::vector<icu::UnicodeString> &ss, const icu::UnicodeString &s,
        std::function<vm_object_t*(const vm_object_t* &a0, const vm_object_t* &a1)>
            f) override {
        return DyadicCallback::create(this, ss, s, f);
    }
    /*
        vm_object_t* create_triadic(const icu::UnicodeString& s,
                                   std::function<vm_object_t*(const vm_object_t*&
       a0, const vm_object_t*& a1, const vm_object_t*& a2)> f) override { return
       TriadicCallback::create(this, s, f);
        }

        vm_object_t* create_triadic(const icu::UnicodeString& s0, const
       icu::UnicodeString& s1, std::function<vm_object_t*(const vm_object_t*& a0,
       const vm_object_t*& a1, const vm_object_t*& a2)> f) override { return
       TriadicCallback::create(this, s0, s1, f);
        }

        vm_object_t* create_triadic(const std::vector<icu::UnicodeString>& ss,
       const icu::UnicodeString& s, std::function<vm_object_t*(const vm_object_t*&
       a0, const vm_object_t*& a1, const vm_object_t*& a2)> f) override { return
       TriadicCallback::create(this, ss, s, f);
        }
        */

    /*
        vm_object_t* create_variadic(const icu::UnicodeString& s,
                                    std::function<vm_object_t*(const
       vm_object_t*s& aa)> f) override { return VariadicCallback::create(this, s,
       f);
        }

        vm_object_t* create_variadic(const icu::UnicodeString& s0, const
       icu::UnicodeString& s1, std::function<vm_object_t*(const vm_object_t*s&
       aa)> f) override { return VariadicCallback::create(this, s0, s1, f);
        }

        vm_object_t* create_variadic(const std::vector<icu::UnicodeString>& ss,
       const icu::UnicodeString& s, std::function<vm_object_t*(const
       vm_object_t*s& aa)> f) override { return VariadicCallback::create(this,
       ss, s, f);
        }
    */

    vm_object_t* bad(const VMObject *o, const icu::UnicodeString &s) override {
        vm_object_t*s tt;
        tt.push_back(create_text(o->to_text()));
        tt.push_back(create_text(s));
        return create_array(tt);
    }

    vm_object_t* bad_args(const VMObject *o, const vm_object_t* &a0) override {
        vm_object_t*s tt;
        tt.push_back(create_text(o->to_text()));
        tt.push_back(a0);
        return create_array(tt);
    }

    vm_object_t* bad_args(const VMObject *o, const vm_object_t* &a0,
                         const vm_object_t* &a1) override {
        vm_object_t*s tt;
        tt.push_back(create_text(o->to_text()));
        tt.push_back(a0);
        tt.push_back(a1);
        return create_array(tt);
    }

    vm_object_t* bad_args(const VMObject *o, const vm_object_t* &a0,
                         const vm_object_t* &a1,
                         const vm_object_t* &a2) override {
        vm_object_t*s tt;
        tt.push_back(create_text(o->to_text()));
        tt.push_back(a0);
        tt.push_back(a1);
        tt.push_back(a2);
        return create_array(tt);
    }

    vm_object_t* to_tuple(const vm_object_t*s &oo) override {
        vm_object_t*s tt;
        tt.push_back(create_tuple());
        for (auto &o : oo) {
            tt.push_back(o);
        }
        return create_array(tt);
    }

    vm_object_t*s from_tuple(const vm_object_t* &oo) override {
        vm_object_t*s tt;
        if (is_tuple(oo)) {
            return tt;
        } else if (is_array(oo)) {
            auto tt1 = get_array(oo);
            for (unsigned int n = 1; n < tt1.size(); n++) {
                tt.push_back(tt1[n]);
            }
            return tt;
        } else {
            return tt;
        }
    }
    // convenience (for internal usage)
    bool is_list(
        const vm_object_t* &o) override {  // XXX: tail-recursive version
        if (is_nil(o)) {
            return true;
        } else if (is_array(o)) {
            auto n = array_size(o);
            if ((n == 3) && is_cons(array_get(o, 0))) {
                return is_list(array_get(o, 2));
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    vm_object_t* to_list(const vm_object_t*s &oo) override {
        auto nil = create_nil();
        auto cons = create_cons();

        vm_object_t* result = nil;

        for (int n = oo.size() - 1; n >= 0; n--) {
            vm_object_t*s aa;
            aa.push_back(cons);
            aa.push_back(oo[n]);
            aa.push_back(result);

            result = create_array(aa);
        }

        return result;
    }

    vm_object_t*s from_list(
        const vm_object_t* &o) override {  // 'type'-unsafe list conversion
        vm_object_t*s oo;

        auto l = o;
        while (!is_nil(l)) {
            oo.push_back(array_get(l, 1));
            l = array_get(l, 2);
        }

        return oo;
    }

    // modules
    void eval_line(const icu::UnicodeString &in, const callback_t &main,
                   const callback_t &exc) override {
        _eval->eval_line(in, main, exc);
    }

    void eval_module(const icu::UnicodeString &fn) override {
        _eval->eval_load(fn);
        _eval->eval_values();
    }

    void eval_command(const icu::UnicodeString &l) override {
        _eval->eval_command(l);
    }

    void eval_main() override {
        _eval->eval_main();
    }

    void eval_interactive() override {
        _eval->eval_interactive();
    }

    bool is_module(const vm_object_t* &m) override {
        return VMModule::is_module(m);
    }

    vm_object_t* query_module_name(const vm_object_t* &m) override {
        if (is_module(m)) {
            return VMModule::module_cast(m)->name();
        } else {
            throw create_text("not a module");
        }
    }

    vm_object_t* query_module_path(const vm_object_t* &m) override {
        if (is_module(m)) {
            return VMModule::module_cast(m)->path();
        } else {
            throw create_text("not a module");
        }
    }

    vm_object_t* query_module_imports(const vm_object_t* &m) override {
        if (is_module(m)) {
            return VMModule::module_cast(m)->imports();
        } else {
            throw create_text("not a module");
        }
    }

    vm_object_t* query_module_exports(const vm_object_t* &m) override {
        if (is_module(m)) {
            return VMModule::module_cast(m)->exports();
        } else {
            throw create_text("not a module");
        }
    }

    vm_object_t* query_module_values(const vm_object_t* &m) override {
        if (is_module(m)) {
            return VMModule::module_cast(m)->values();
        } else {
            throw create_text("not a module");
        }
    }

    // machine state
    vm_object_t* query_modules() override {
        auto mm = _manager->get_modules();
        vm_object_t*s oo;
        for (auto &m : mm) {
            oo.push_back(VMModule::create(this, m));
        }
        return to_list(oo);
    }

    vm_object_t* query_symbols() override {
        throw create_text("stub");
    }

    vm_object_t* query_data() override {
        throw create_text("stub");
    }

    int compare(const vm_object_t* &o0, const vm_object_t* &o1) override {
        Comparevm_object_t* compare;
        return compare(o0, o1);
    }

private:
    SymbolTable _symbols;
    void *_context;
    std::mutex _mutex;

    vm_object_t* _int;
    vm_object_t* _float;
    vm_object_t* _char;
    vm_object_t* _text;

    vm_object_t* _none;
    vm_object_t* _true;
    vm_object_t* _false;

    vm_object_t* _nil;
    vm_object_t* _cons;
    vm_object_t* _tuple;

    OptionsPtr _options;
    ModuleManagerPtr _manager;
    EvalPtr _eval;
};
