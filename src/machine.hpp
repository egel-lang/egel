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

class SymbolTable {
public:
    SymbolTable():
            _to(std::vector<icu::UnicodeString>()),
            _from(std::map<icu::UnicodeString, symbol_t>()) {
    }

    SymbolTable(const SymbolTable& other): 
        _to(other._to), _from(other._from) {
    }

    void initialize() {
        auto i = enter(STRING_SYSTEM, STRING_INT);
        auto f = enter(STRING_SYSTEM, STRING_FLOAT);
        auto c = enter(STRING_SYSTEM, STRING_CHAR);
        auto t = enter(STRING_SYSTEM, STRING_TEXT);
        auto p = enter(STRING_SYSTEM, STRING_PTR);
        ASSERT(i == SYMBOL_INT);
        ASSERT(f == SYMBOL_FLOAT);
        ASSERT(c == SYMBOL_CHAR);
        ASSERT(t == SYMBOL_TEXT);
        ASSERT(p == SYMBOL_POINTER);
        auto _nop   = enter(STRING_SYSTEM, STRING_NOP);
        auto _true  = enter(STRING_SYSTEM, STRING_TRUE);
        auto _false = enter(STRING_SYSTEM, STRING_FALSE);
        ASSERT(_nop   == SYMBOL_NOP);
        ASSERT(_true  == SYMBOL_TRUE);
        ASSERT(_false == SYMBOL_FALSE);
        auto _tuple = enter(STRING_SYSTEM, STRING_TUPLE);
        auto _nil   = enter(STRING_SYSTEM, STRING_NIL);
        auto _cons  = enter(STRING_SYSTEM, STRING_CONS);
        ASSERT(_tuple  == SYMBOL_TUPLE);
        ASSERT(_nil    == SYMBOL_NIL);
        ASSERT(_cons   == SYMBOL_CONS);
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
        icu::UnicodeString n = n0 + STRING_COLON + n1;
        return enter(n);
    }

    symbol_t enter(const UnicodeStrings& nn, const icu::UnicodeString& n) {
        icu::UnicodeString s;
        for (auto& n0: nn) {
            s += n0 + STRING_COLON;
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
        return VMObjectPtr(new VMObjectResult(*this));
    }

    VMObjectPtr reduce(const VMObjectPtr& thunk) const override {
        auto tt  = VM_OBJECT_ARRAY_VALUE(thunk);
        auto arg0   = tt[5];

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
        initialize();
    }

    void initialize() {
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
        auto nop0   = _symbols.enter(STRING_SYSTEM, STRING_NOP);
        auto true0  = _symbols.enter(STRING_SYSTEM, STRING_TRUE);
        auto false0 = _symbols.enter(STRING_SYSTEM, STRING_FALSE);
        ASSERT(nop0   == SYMBOL_NOP);
        ASSERT(true0  == SYMBOL_TRUE);
        ASSERT(false0 == SYMBOL_FALSE);
        _nop   = VMObjectData::create(this, nop0);
        _true  = VMObjectData::create(this, true0);
        _false = VMObjectData::create(this, false0);
        _data.enter(_nop);
        _data.enter(_true);
        _data.enter(_false);
        auto _tuple0 = _symbols.enter(STRING_SYSTEM, STRING_TUPLE);
        auto _nil0   = _symbols.enter(STRING_SYSTEM, STRING_NIL);
        auto _cons0  = _symbols.enter(STRING_SYSTEM, STRING_CONS);
        ASSERT(_tuple0  == SYMBOL_TUPLE);
        ASSERT(_nil0    == SYMBOL_NIL);
        ASSERT(_cons0   == SYMBOL_CONS);
        _tuple = VMObjectData::create(this, nop0);
        _nil   = VMObjectData::create(this, true0);
        _cons  = VMObjectData::create(this, false0);
        _data.enter(_tuple);
        _data.enter(_nil);
        _data.enter(_cons);
    }
    // import table manipulation
    bool has_import(const icu::UnicodeString& i) override {
        return false;
    }

    void add_import(const icu::UnicodeString& i) override {
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

    VMObjectPtr create_nop() override {
        return get_data(SYMBOL_NOP);
    }

    VMObjectPtr create_true() override {
        return get_data(SYMBOL_TRUE);
    }

    VMObjectPtr create_false() override {
        return get_data(SYMBOL_FALSE);
    }

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

    VMObjectPtr create_array() override {
        return VMObjectArray::create();
    }

    void append(VMObjectPtr aa, const VMObjectPtr a) override {
        if (VM_OBJECT_ARRAY_TEST(aa)) {
            auto aa0 = VM_OBJECT_ARRAY_CAST(aa);
            aa0->push_back(a);
        } else {
            throw ErrorInternal("push to array failed");
        }
    }

    VMObjectPtr create_data(const icu::UnicodeString& n) {
        return get_symbol(n);
        
    }

    VMObjectPtr create_data(const icu::UnicodeString& n0, const icu::UnicodeString& n1) {
        return get_symbol(n0, n1);
    }

    VMObjectPtr create_data(const std::vector<icu::UnicodeString>& nn, const icu::UnicodeString& n) {
        return get_symbol(nn, n);
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

    VMObjectPtr     _nop;
    VMObjectPtr     _true;
    VMObjectPtr     _false;

    VMObjectPtr     _nil;
    VMObjectPtr     _cons;
    VMObjectPtr     _tuple;
};

#endif
