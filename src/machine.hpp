#ifndef MACHINE_HPP
#define MACHINE_HPP

#include <memory>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <iomanip>
#include <tuple>

#include "runtime.hpp"

class SymbolTable {
public:
    SymbolTable():
            _to(std::vector<UnicodeString>()),
            _from(std::map<UnicodeString, symbol_t>()) {
    }

    SymbolTable(const SymbolTable& other): 
        _to(other._to), _from(other._from) {
    }

    void initialize() {
        auto i = enter(STRING_SYSTEM, STRING_INT);
        auto f = enter(STRING_SYSTEM, STRING_FLOAT);
        auto c = enter(STRING_SYSTEM, STRING_CHAR);
        auto t = enter(STRING_SYSTEM, STRING_TEXT);
        ASSERT(i == SYMBOL_INT);
        ASSERT(f == SYMBOL_FLOAT);
        ASSERT(c == SYMBOL_CHAR);
        ASSERT(t == SYMBOL_TEXT);
    }

    symbol_t enter(const UnicodeString& s) {
        if (_from.count(s) == 0) {
            symbol_t n = _to.size();
            _to.push_back(s);
            _from[s] = n;
            return n;
        } else {
            return _from[s];
        }
    }

    symbol_t enter(const UnicodeString& n0, const UnicodeString& n1) {
        UnicodeString n = n0 + '.' + n1;
        return enter(n);
    }

    symbol_t enter(const UnicodeStrings& nn, const UnicodeString& n) {
        UnicodeString s;
        for (auto& n0: nn) {
            s += n0 + '.';
        }
        s += n;
        return enter(s);
    }

    UnicodeString get(const symbol_t& s) {
        return _to[s];
    }

    void render(std::ostream& os) {
        for (uint_t t = 0; t < _to.size(); t++) {
            os << std::setw(8) << t << ":" << _to[t] << std::endl;
        }
    }

private:
    std::vector<UnicodeString>          _to;
    std::map<UnicodeString, symbol_t>   _from;
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

    void render(std::ostream& os) {
        for (uint_t t = 0; t < _to.size(); t++) {
            os << std::setw(8) << t << ":";
            _to[t]->debug(os);
            os << std::endl;
        }
    }
            
private:
    std::vector<VMObjectPtr>                        _to;
    std::map<VMObjectPtr, data_t, LessVMObjectPtr>   _from;
};


class Machine: public VM {
public:
    Machine() {
        _symbols.initialize();
    }

    // import table manipulation
    bool has_import(const UnicodeString& i) override {
        return false;
	}

    void add_import(const UnicodeString& i) override {
	}


    // symbol table manipulation
    symbol_t enter_symbol(const UnicodeString& n) override {
        return _symbols.enter(n);
	}

    symbol_t enter_symbol(const UnicodeString& n0, const UnicodeString& n1) override {
        return _symbols.enter(n0, n1);
	}

    symbol_t enter_symbol(const UnicodeStrings& nn, const UnicodeString& n) override {
        return _symbols.enter(nn, n);
	}

    UnicodeString get_symbol(symbol_t s) override {
        return _symbols.get(s);
	}


    // data table manipulation
    data_t enter_data(const VMObjectPtr& o) override {
        return _data.enter(o);
	}

    data_t define_data(const VMObjectPtr& o) override {
        return _data.define(o);
	}

    VMObjectPtr get_data(const data_t d) override {
        return _data.get(d);
	}

    // reduce an expression
    void result_handler(const VMObjectPtr& r) override {
        _result = r;
	}

    void exception_handler(const VMObjectPtr& e) override {
        _exception = e;
	}

    void reduce(const VMObjectPtr& f, const VMObjectPtr& ret, const VMObjectPtr& exc) override {
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
        while (trampoline != nullptr) {
            ASSERT(trampoline->tag() == VM_OBJECT_ARRAY);
            auto f = VM_OBJECT_ARRAY_CAST(trampoline)->get(4);
#ifdef DEBUG
            std::cout << "trace: " << f << std::endl;
            std::cout << "on : " << trampoline << std::endl;
#endif
            trampoline = f->reduce(trampoline);
        }
    }

    void reduce(const VMObjectPtr& f) override {
        reduce(f, _result, _exception);
	}

	void add_lock() override {
        std::cerr << "warning: threads not implemented yet" << std::endl;
	}

	void release_lock() override {
	}
                    
	void render(std::ostream& os) override {
        os << "SYMBOLS: " << std::endl;
        _symbols.render(os);
        os << "DATA: " << std::endl;
        _data.render(os);
	}

private:
    SymbolTable     _symbols;
    DataTable       _data;
    VMObjectPtr     _result;
    VMObjectPtr     _exception;
};

#endif
