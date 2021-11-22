#ifndef MACHINE_HPP
#define MACHINE_HPP

#include <memory>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <iomanip>
#include <tuple>
#include <mutex>

#include "runtime.hpp"
#include "modules.hpp"
#include "eval.hpp"

class SymbolTable {
public:
    SymbolTable():
            _to(std::vector<icu::UnicodeString>()),
            _from(std::map<icu::UnicodeString, symbol_t>()) {
    }

    SymbolTable(const SymbolTable& other): 
        _to(other._to), _from(other._from) {
    }

    bool member(const icu::UnicodeString& s) const {
        return (_from.count(s) > 0);
    }

    symbol_t enter(const icu::UnicodeString& s) {
        if (_from.count(s) == 0) {
            symbol_t n = _to.size();
            _to.push_back(s);
            _from[s] = n;
            return n;
        } else {
            return _from[s];
        }
    }

    symbol_t enter(const icu::UnicodeString& n0, const icu::UnicodeString& n1) {
        icu::UnicodeString n = n0 + STRING_DCOLON + n1;
        return enter(n);
    }

    symbol_t enter(const UnicodeStrings& nn, const icu::UnicodeString& n) {
        icu::UnicodeString s;
        for (auto& n0: nn) {
            s += n0 + STRING_DCOLON;
        }
        s += n;
        return enter(s);
    }

    int size() const {
        return _to.size();
    }

    icu::UnicodeString get(const symbol_t& s) {
        return _to[s];
    }

    void render(std::ostream& os) {
        for (uint_t t = 0; t < _to.size(); t++) {
            os << std::setw(8) << t << "=" << _to[t] << std::endl;
        }
    }

private:
    std::vector<icu::UnicodeString>          _to;
    std::map<icu::UnicodeString, symbol_t>   _from;
};

class DataTable {
public:
    DataTable():
            _to(std::vector<VMObjectPtr>()) {
           // _from(std::map<VMObjectPtr, data_t, LessVMObjectPtr>()) {
    }

    DataTable(const DataTable& other): 
        _to(other._to), _from(other._from) {
    }

    void initialize() {
    }

    data_t enter(const VMObjectPtr& s) {
        if (_from.count(s) == 0) {
            data_t n = _to.size();
            _to.push_back(s);
            _from[s] = n;
            return n;
        } else {
            return _from[s];
        }
    }

    data_t size() {
        return _to.size();
    }

    data_t define(const VMObjectPtr& s) {
        if (_from.count(s) == 0) {
            return enter(s);
        } else {
            data_t n = _from[s];
            _to[n] = s;
            return n;
        }
    }

    VMObjectPtr get(const data_t& s) {
        return _to[s];
    }

    data_t get(const VMObjectPtr& o) {
        return _from[o];
    }

    void render(std::ostream& os) {
        for (uint_t t = 0; t < _to.size(); t++) {
            os << std::setw(8) << t << ":";
            _to[t]->debug(os);
            os << std::endl;
        }
    }
            
private:
    std::vector<VMObjectPtr>                        _to;
    std::map<VMObjectPtr, data_t, LessVMObjectPtr>  _from;
};

class VMObjectResult : public VMObjectCombinator {
public:
    VMObjectResult(VM* m, const symbol_t s, VMReduceResult* r, const bool exc)
        : VMObjectCombinator(VM_SUB_BUILTIN, m, s), _result(r), _exception(exc) {
    };

    VMObjectResult(const VMObjectResult& d)
        : VMObjectResult(d.machine(), d.symbol(), d._result, d._exception) {
    }

    VMObjectPtr clone() const override {
        return VMObjectPtr( (VMObject*) new VMObjectResult(*this));
    }

    VMObjectPtr reduce(const VMObjectPtr& thunk) const override {
        auto tt   = VM_OBJECT_ARRAY_VALUE(thunk);
        auto arg0 = tt[5];

        _result->result    = arg0;
        _result->exception = _exception;
        return nullptr;
    }

private:
    VMReduceResult* _result;
    bool            _exception;
};

class Machine: public VM {
public:
    Machine() {
        populate();
    }

    virtual ~Machine() {
    }

    VMPtr clone() const override {
        return VMPtr(new Machine()); // XXX: use a copy constructor once
    }

    void populate() {
        // symbol and data table initialization
        auto i = _symbols.enter(STRING_SYSTEM, STRING_INT);
        auto f = _symbols.enter(STRING_SYSTEM, STRING_FLOAT);
        auto c = _symbols.enter(STRING_SYSTEM, STRING_CHAR);
        auto t = _symbols.enter(STRING_SYSTEM, STRING_TEXT);
        auto p = _symbols.enter(STRING_SYSTEM, STRING_PTR);
        ASSERT(i == SYMBOL_INT);
        ASSERT(f == SYMBOL_FLOAT);
        ASSERT(c == SYMBOL_CHAR);
        ASSERT(t == SYMBOL_TEXT);
        ASSERT(p == SYMBOL_POINTER);
        // necessary 'type' definitions
        _int   = VMObjectData::create(this, i);
        _float = VMObjectData::create(this, f);
        _char  = VMObjectData::create(this, c);
        _text  = VMObjectData::create(this, t);
        _ptr   = VMObjectData::create(this, p);
        _data.enter(_int);
        _data.enter(_float);
        _data.enter(_char);
        _data.enter(_text);
        _data.enter(_ptr);
        auto none0  = _symbols.enter(STRING_SYSTEM, STRING_NONE);
        auto true0  = _symbols.enter(STRING_SYSTEM, STRING_TRUE);
        auto false0 = _symbols.enter(STRING_SYSTEM, STRING_FALSE);
        ASSERT(none0  == SYMBOL_NONE);
        ASSERT(true0  == SYMBOL_TRUE);
        ASSERT(false0 == SYMBOL_FALSE);
        _none  = VMObjectData::create(this, none0);
        _true  = VMObjectData::create(this, true0);
        _false = VMObjectData::create(this, false0);
        _data.enter(_none);
        _data.enter(_true);
        _data.enter(_false);
        auto tuple0 = _symbols.enter(STRING_SYSTEM, STRING_TUPLE);
        auto nil0   = _symbols.enter(STRING_SYSTEM, STRING_NIL);
        auto cons0  = _symbols.enter(STRING_SYSTEM, STRING_CONS);
        ASSERT(tuple0  == SYMBOL_TUPLE);
        ASSERT(nil0    == SYMBOL_NIL);
        ASSERT(cons0   == SYMBOL_CONS);

        _tuple = VMObjectData::create(this, tuple0);
        _nil   = VMObjectData::create(this, nil0);
        _cons  = VMObjectData::create(this, cons0);
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
        NamespacePtr env = Namespace().clone();

        _options = oo;
        _manager = ModuleManager().clone();
        _manager->init(oo, this);
        _eval = Eval().clone();
        _eval->init(_manager);
    }

    // symbol table manipulation
    symbol_t enter_symbol(const icu::UnicodeString& n) override {
        return _symbols.enter(n);
    }

    symbol_t enter_symbol(const icu::UnicodeString& n0, const icu::UnicodeString& n1) override {
        return _symbols.enter(n0, n1);
    }

    symbol_t enter_symbol(const UnicodeStrings& nn, const icu::UnicodeString& n) override {
        return _symbols.enter(nn, n);
    }

    virtual int get_symbols_size() override {
        return _symbols.size();
    }

    icu::UnicodeString get_symbol_string(symbol_t s) override {
        return _symbols.get(s);
    }

    // data table manipulation
    data_t enter_data(const VMObjectPtr& o) override {
        return _data.enter(o);
    }

    data_t define_data(const VMObjectPtr& o) override {
        return _data.define(o);
    }

    data_t get_data(const VMObjectPtr& o) override {
        return _data.get(o);
    }

    VMObjectPtr get_data(const data_t d) override {
        return _data.get(d);
    }

    // convenience
    VMObjectPtr get_symbol(const symbol_t s) override {
        auto o = VMObjectStub(this, s).clone();
        auto d = enter_data(o);
        return get_data(d);
    }

    VMObjectPtr get_symbol(const icu::UnicodeString& n) override {
        auto i = enter_symbol(n);
        return get_symbol(i);
    }

    VMObjectPtr get_symbol(const icu::UnicodeString& n0, const icu::UnicodeString& n1) override {
        auto i = enter_symbol(n0, n1);
        return get_symbol(i);
    }

    VMObjectPtr get_symbol(const std::vector<icu::UnicodeString>& nn, const icu::UnicodeString& n) override {
        auto i = enter_symbol(nn, n);
        return get_symbol(i);
    }

    void define(const VMObjectPtr& o) override {
        // define an undefined symbol
        auto s = o->to_text();  // XXX: usually works?
        if (_symbols.member(s)) {
            throw create_text("redeclaration of " + s);

        } else {
            enter_symbol(s);
            enter_data(o);
        }
    }

    void overwrite(const VMObjectPtr& o) override {
        // define or overwrite
        auto s = o->to_text();  // XXX: usually works?
        enter_symbol(s);
        enter_data(o);
    }

    VMObjectPtr get(const VMObjectPtr& o) override {
        // get a defined symbol
        return get_symbol(o->to_text());
    }

    // reduce an expression
    void reduce(const VMObjectPtr& f, const VMObjectPtr& ret, const VMObjectPtr& exc, reducer_state_t* run) override {
        VMObjectPtrs rr;
        rr.push_back(nullptr); // rt
        rr.push_back(nullptr); // rti
        rr.push_back(nullptr); // k
        rr.push_back(nullptr); // exc
        rr.push_back(ret); // c
        rr.push_back(nullptr); // arg0
        auto r = VMObjectArray(rr).clone();

        VMObjectPtrs ee;
        ee.push_back(nullptr); // rt
        ee.push_back(nullptr); // rti
        ee.push_back(nullptr); // k
        ee.push_back(nullptr); // exc
        ee.push_back(exc); // c
        ee.push_back(nullptr); // arg0
        auto e = VMObjectArray(ee).clone();

        auto i = VMObjectInteger(5).clone();
        VMObjectPtrs tt;
        tt.push_back(r); // rt
        tt.push_back(i); // rti
        tt.push_back(r); // k
        tt.push_back(e); // exc
        tt.push_back(f); // c
        auto t = VMObjectArray(tt).clone();

        auto trampoline = t;
        while ( (trampoline != nullptr) && (*run != HALTED) ) {
            if (*run == RUNNING) {
                ASSERT(trampoline->tag() == VM_OBJECT_ARRAY);
                auto f = VM_OBJECT_ARRAY_CAST(trampoline)->get(4);
#ifdef DEBUG
                std::cout << "trace: " << f << std::endl;
                std::cout << "on : " << trampoline << std::endl;
#endif
                trampoline = f->reduce(trampoline);
            } else if (*run == SLEEPING) {
                usleep(100); // sleep for 100ms
            } else { // *run == HALTED
            }
        }
    }

    void reduce(const VMObjectPtr& f, const VMObjectPtr& ret, const VMObjectPtr& exc) override {
        reducer_state_t run = RUNNING;
        reduce(f, ret, exc, &run);
    }

    VMReduceResult reduce(const VMObjectPtr& f, reducer_state_t* run) override {
        VMReduceResult r;

        auto sm = enter_symbol("Internal", "result");
        auto m  = VMObjectResult(this, sm, &r, false).clone();

        auto se = enter_symbol("Internal", "exception");
        auto e  = VMObjectResult(this, se, &r, true).clone();

        reduce(f, m, e, run);
        return r;
    }

    VMReduceResult reduce(const VMObjectPtr& f) override {
        reducer_state_t run = RUNNING;
        return reduce(f, &run);
    }

    void lock() override {
        _mutex.lock();
    }

    void unlock() override {
        _mutex.unlock();
    }
                    
    void render(std::ostream& os) override {
        os << "SYMBOLS: " << std::endl;
        _symbols.render(os);
        os << "DATA: " << std::endl;
        _data.render(os);
    }

    void* get_context() const override {
        return _context;
    }

    void set_context(void* m) override {
        _context = m;
    }

    // primitive values
    VMObjectPtr create_integer(const vm_int_t n) override {
        return VMObjectInteger::create(n);
    }

    VMObjectPtr create_char(const vm_char_t c) override {
        return VMObjectChar::create(c);
    }

    VMObjectPtr create_text(const vm_text_t s) override {
        return VMObjectText::create(s);
    }

    VMObjectPtr create_float(const vm_float_t f) override {
        return VMObjectFloat::create(f);
    }

    bool is_integer(const VMObjectPtr& o) override {
        return o->tag() == VM_OBJECT_INTEGER;
    }

    bool is_float(const VMObjectPtr& o) override {
        return o->tag() == VM_OBJECT_FLOAT;
    }

    bool is_char(const VMObjectPtr& o) override {
        return o->tag() == VM_OBJECT_CHAR;
    }

    bool is_text(const VMObjectPtr& o) override {
        return o->tag() == VM_OBJECT_TEXT;
    }

    vm_int_t value_integer(const VMObjectPtr& o) override {
        return VM_OBJECT_INTEGER_VALUE(o);
    }

    vm_float_t value_float(const VMObjectPtr& o) override {
        return VM_OBJECT_FLOAT_VALUE(o);
    }

    vm_char_t value_char(const VMObjectPtr& o) override {
        return VM_OBJECT_CHAR_VALUE(o);
    }

    vm_text_t value_text(const VMObjectPtr& o) override {
        return VM_OBJECT_TEXT_VALUE(o);
    }

    VMObjectPtr create_none() override {
        return get_data(SYMBOL_NONE);
    }

    VMObjectPtr create_true() override {
        return get_data(SYMBOL_TRUE);
    }

    VMObjectPtr create_false() override {
        return get_data(SYMBOL_FALSE);
    }

    // predefined constants
    VMObjectPtr create_bool(const bool b) override {
        if (b) {
            return create_true();
        } else {
            return create_false();
        }
    }

    VMObjectPtr create_nil() override {
        return _nil;
    }

    VMObjectPtr create_cons() override {
        return _cons;
    }

    VMObjectPtr create_tuple() override {
        return _tuple;
    }

    bool is_none(const VMObjectPtr& o) override {
        return (VM_OBJECT_NONE_TEST(_none));
    }

    bool is_true(const VMObjectPtr& o) override {
        return (VM_OBJECT_TRUE_TEST(_true));

    }

    bool is_false(const VMObjectPtr& o) override {
        return (VM_OBJECT_FALSE_TEST(_false));
    }

    bool is_bool(const VMObjectPtr& o) override {
        return is_false(o) || is_true(o);
    }

    bool is_nil(const VMObjectPtr& o) override {
        return (VM_OBJECT_NIL_TEST(_nil));
    }

    bool is_cons(const VMObjectPtr& o) override {
        return (VM_OBJECT_CONS_TEST(_cons));
    }

    bool is_tuple(const VMObjectPtr& o) override {
        return (VM_OBJECT_TUPLE_TEST(_tuple));
    }

    VMObjectPtr create_array() override {
        return VMObjectArray::create();
    }

    void array_append(VMObjectPtr aa, const VMObjectPtr a) override {
        if (VM_OBJECT_ARRAY_TEST(aa)) {
            auto aa0 = VM_OBJECT_ARRAY_CAST(aa);
            aa0->push_back(a);
        } else {
            throw ErrorInternal("push to array failed");
        }
    }

    bool is_array(const VMObjectPtr& o) override {
        return VM_OBJECT_ARRAY_TEST(o);
    }

    int array_size(const VMObjectPtr& o) override {
        return VM_OBJECT_ARRAY_CAST(o)->size();
    }

    VMObjectPtr array_get(const VMObjectPtr& o, int n) override {
        return VM_OBJECT_ARRAY_CAST(o)->get(n);
    }

    void array_set(VMObjectPtr& o, int n, const VMObjectPtr& e) override {
        VM_OBJECT_ARRAY_CAST(o)->set(n, e);
    }

    VMObjectPtrs array_vector(const VMObjectPtr& o) override {
        return VM_OBJECT_ARRAY_VALUE(o);
    }

    VMObjectPtr create_data(const icu::UnicodeString& n) override {
        return VMObjectData::create(this, n);
    }

    VMObjectPtr create_data(const icu::UnicodeString& n0, const icu::UnicodeString& n1) override {
        return VMObjectData::create(this, n0, n1);
    }

    VMObjectPtr create_data(const std::vector<icu::UnicodeString>& nn, const icu::UnicodeString& n) override {
        return VMObjectData::create(this, nn, n);
    }

    VMObjectPtr create_medadic(const icu::UnicodeString& s, 
                               std::function<VMObjectPtr()> f) override {
        return MedadicCallback::create(this, s, f);
    }

    VMObjectPtr create_medadic(const icu::UnicodeString& s0, const icu::UnicodeString& s1,
                               std::function<VMObjectPtr()> f) override {
        return MedadicCallback::create(this, s0, s1, f);
    }

    VMObjectPtr create_medadic(const std::vector<icu::UnicodeString>& ss, const icu::UnicodeString& s,
                               std::function<VMObjectPtr()> f) override {
        return MedadicCallback::create(this, ss, s, f);
    }

    VMObjectPtr create_monadic(const icu::UnicodeString& s, 
                               std::function<VMObjectPtr(const VMObjectPtr& a0)> f) override {
        return MonadicCallback::create(this, s, f);
    }

    VMObjectPtr create_monadic(const icu::UnicodeString& s0, const icu::UnicodeString& s1,
                               std::function<VMObjectPtr(const VMObjectPtr& a0)> f) override {
        return MonadicCallback::create(this, s0, s1, f);
    }

    VMObjectPtr create_monadic(const std::vector<icu::UnicodeString>& ss, const icu::UnicodeString& s,
                               std::function<VMObjectPtr(const VMObjectPtr& a0)> f) override {
        return MonadicCallback::create(this, ss, s, f);
    }

    VMObjectPtr create_dyadic(const icu::UnicodeString& s, 
                              std::function<VMObjectPtr(const VMObjectPtr& a0, const VMObjectPtr& a1)> f) override {
        return DyadicCallback::create(this, s, f);
    }

    VMObjectPtr create_dyadic(const icu::UnicodeString& s0, const icu::UnicodeString& s1,
                              std::function<VMObjectPtr(const VMObjectPtr& a0, const VMObjectPtr& a1)> f) override {
        return DyadicCallback::create(this, s0, s1, f);
    }

    VMObjectPtr create_dyadic(const std::vector<icu::UnicodeString>& ss, const icu::UnicodeString& s,
                              std::function<VMObjectPtr(const VMObjectPtr& a0, const VMObjectPtr& a1)> f) override {
        return DyadicCallback::create(this, ss, s, f);
    }

    VMObjectPtr create_triadic(const icu::UnicodeString& s, 
                               std::function<VMObjectPtr(const VMObjectPtr& a0, const VMObjectPtr& a1, const VMObjectPtr& a2)> f) override {
        return TriadicCallback::create(this, s, f);
    }

    VMObjectPtr create_triadic(const icu::UnicodeString& s0, const icu::UnicodeString& s1,
                               std::function<VMObjectPtr(const VMObjectPtr& a0, const VMObjectPtr& a1, const VMObjectPtr& a2)> f) override {
        return TriadicCallback::create(this, s0, s1, f);
    }

    VMObjectPtr create_triadic(const std::vector<icu::UnicodeString>& ss, const icu::UnicodeString& s,
                               std::function<VMObjectPtr(const VMObjectPtr& a0, const VMObjectPtr& a1, const VMObjectPtr& a2)> f) override {
        return TriadicCallback::create(this, ss, s, f);
    }

    VMObjectPtr create_variadic(const icu::UnicodeString& s, 
                                std::function<VMObjectPtr(const VMObjectPtrs& aa)> f) override {
        return VariadicCallback::create(this, s, f);
    }

    VMObjectPtr create_variadic(const icu::UnicodeString& s0, const icu::UnicodeString& s1,
                                std::function<VMObjectPtr(const VMObjectPtrs& aa)> f) override {
        return VariadicCallback::create(this, s0, s1, f);
    }

    VMObjectPtr create_variadic(const std::vector<icu::UnicodeString>& ss, const icu::UnicodeString& s,
                                std::function<VMObjectPtr(const VMObjectPtrs& aa)> f) override {
        return VariadicCallback::create(this, ss, s, f);
    }

    // convenience (for internal usage)
    bool         is_list(const VMObjectPtr& o) override { // XXX: tail-recursive version
        if (is_nil(o)) {
            return true;
        } else if (is_array(o)) {
            auto n = array_size(o);
            if ( (n == 3) && is_cons(array_get(o, 0)) ) {
                return is_list(array_get(o,2));
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    VMObjectPtr  to_list(const VMObjectPtrs& oo) override {
        auto nil  = create_nil();
        auto cons = create_cons();

        VMObjectPtr result = nil;

        for (int n = oo.size() - 1; n >= 0; n--) {
            VMObjectPtr aa = create_array();
            array_append(aa, cons);
            array_append(aa, oo[n]);
            array_append(aa, result);

            result = aa;
        }

        return result;
    }

    VMObjectPtrs from_list(const VMObjectPtr& o) override { // 'type'-unsafe list conversion
        VMObjectPtrs oo;

        auto l = o;
        while (!is_nil(l)) {
            oo.push_back(array_get(l, 1));
            l = array_get(l,2);
        }

        return oo;
    }

    // modules
    void eval_line(const icu::UnicodeString& in, const callback_t& main, const callback_t& exc) override {
        _eval->eval_line(in, main, exc);
    }

    void eval_module(const icu::UnicodeString& fn) override {
        _eval->eval_load(fn);
        _eval->eval_values();
    }

    void eval_command(const icu::UnicodeString& l) override {
        _eval->eval_command(l);
    }

    void eval_main() override {
        _eval->eval_main();
    }

    void eval_interactive() override {
        _eval->eval_interactive();
    }

    bool is_module(const VMObjectPtr& m) override {
        return VMModule::is_module(m);
    }

    VMObjectPtr query_module_name(const VMObjectPtr& m) override {
        if (is_module(m)) {
            return VMModule::module_cast(m)->name();
        } else {
            throw create_text("not a module");
        }
    }

    VMObjectPtr query_module_path(const VMObjectPtr& m) override {
        if (is_module(m)) {
            return VMModule::module_cast(m)->path();
        } else {
            throw create_text("not a module");
        }
    }

    VMObjectPtr query_module_imports(const VMObjectPtr& m) override {
        if (is_module(m)) {
            return VMModule::module_cast(m)->imports();
        } else {
            throw create_text("not a module");
        }
    }

    VMObjectPtr query_module_exports(const VMObjectPtr& m) override {
        if (is_module(m)) {
            return VMModule::module_cast(m)->exports();
        } else {
            throw create_text("not a module");
        }
    }

    VMObjectPtr query_module_values(const VMObjectPtr& m) override {
        if (is_module(m)) {
            return VMModule::module_cast(m)->values();
        } else {
            throw create_text("not a module");
        }
    }

    // machine state
    VMObjectPtr query_modules() override {
        auto mm = _manager->get_modules();
        VMObjectPtrs oo;
        for (auto& m:mm) {
            oo.push_back(VMModule(this, m).clone());
        }
        return to_list(oo);
    }

    VMObjectPtr query_symbols() override {
        throw create_text("stub");
    }

    VMObjectPtr query_data()    override {
        throw create_text("stub");
    }

private:
    SymbolTable     _symbols;
    DataTable       _data;
    void*           _context;
    std::mutex      _mutex;

    VMObjectPtr     _int;
    VMObjectPtr     _float;
    VMObjectPtr     _char;
    VMObjectPtr     _text;
    VMObjectPtr     _ptr;

    VMObjectPtr     _none;
    VMObjectPtr     _true;
    VMObjectPtr     _false;

    VMObjectPtr     _nil;
    VMObjectPtr     _cons;
    VMObjectPtr     _tuple;

    OptionsPtr       _options;
    ModuleManagerPtr _manager;
    EvalPtr          _eval;
};

#endif
