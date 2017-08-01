#include "../../src/runtime.hpp"

#include <stdlib.h>
#include <math.h>
#include <thread>

/**
 * Egel's par construct. 'par f g' starts two computations in parallel and returns
 * a tuple.
 *
 * It's a simplistic construct which doesn't handle exception handling well. I.e.,
 * when a thread throws an exception it places it in the resulting tuple; the 
 * other thread is allowed to continue to run.
 **/


void runthread(VM* vm, const VMObjectPtr& e, const VMObjectPtr& ret, const VMObjectPtr& exc) {
    vm->reduce(e, ret, exc);
}

class VMObjectThreadResult : public VMObjectCombinator {
public:
    VMObjectThreadResult(VM* m, const symbol_t s, const VMObjectPtr& tuple, int pos)
        : VMObjectCombinator(VM_OBJECT_FLAG_COMBINATOR, m, s), _tuple(tuple), _pos(pos) {
    };

    VMObjectThreadResult(const VMObjectThreadResult& d)
        : VMObjectThreadResult(d.machine(), d.symbol(), d._tuple, d._pos) {
    }

    VMObjectPtr clone() const {
        return VMObjectPtr(new VMObjectThreadResult(*this));
    }

    VMObjectPtr reduce(const VMObjectPtr& thunk) const override {
        auto tt  = VM_OBJECT_ARRAY_VALUE(thunk);
        auto arg0   = tt[5];

        auto t = VM_OBJECT_ARRAY_CAST(_tuple);
        t->set(_pos, arg0);

        return nullptr;
    }
protected:
    VMObjectPtr _tuple;
    int         _pos;
};

class VMObjectThreadException : public VMObjectCombinator {
public:
    VMObjectThreadException(VM* m, const symbol_t s, const VMObjectPtr& tuple, int pos)
        : VMObjectCombinator(VM_OBJECT_FLAG_COMBINATOR, m, s), _tuple(tuple), _pos(pos) {
    };

    VMObjectThreadException(const VMObjectThreadException& d)
        : VMObjectThreadException(d.machine(), d.symbol(), d._tuple, d._pos) {
    }

    VMObjectPtr clone() const {
        return VMObjectPtr(new VMObjectThreadException(*this));
    }

    VMObjectPtr reduce(const VMObjectPtr& thunk) const override {
        auto tt    = VM_OBJECT_ARRAY_VALUE(thunk);
        auto arg0  = tt[5];

        auto t = VM_OBJECT_ARRAY_CAST(_tuple);
        t->set(_pos, arg0);

        return nullptr;
    }
protected:
    VMObjectPtr _tuple;
    int         _pos;
};


// System.par f g
// Concurrently evaluate 'f nop' and 'g nop'
class Par: public Dyadic {
public:
    DYADIC_PREAMBLE(Par, "System", "par");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        static VMObjectPtr _tuple = nullptr;
        if (_tuple == nullptr) _tuple = machine()->get_data_string("System", "tuple");

        static VMObjectPtr _nop = nullptr;
        if (_nop == nullptr) _nop = machine()->get_data_string("System", "nop");

        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("System", "thread");


        VMObjectPtrs tt;
        tt.push_back(_tuple);
        tt.push_back(nullptr);
        tt.push_back(nullptr);
        auto result = VMObjectArray(tt).clone();

        VMObjectPtrs ll;
        ll.push_back(arg0);
        ll.push_back(_nop);
        auto left = VMObjectArray(ll).clone();

        VMObjectPtrs rr;
        rr.push_back(arg1);
        rr.push_back(_nop);
        auto right = VMObjectArray(rr).clone();

        auto vm = machine();

        std::thread first (runthread, vm, left, 
                            VMObjectThreadResult(vm, sym, result, 1).clone(),
                            VMObjectThreadException(vm, sym, result, 1).clone() );
        std::thread second (runthread, vm, sym, right, 
                            VMObjectThreadResult(vm, sym, result, 2).clone(),
                            VMObjectThreadException(vm, sym, result, 2).clone() );

        first.join();
        second.join();

        return result;
    }
};

extern "C" std::vector<UnicodeString> egel_imports() {
    return std::vector<UnicodeString>();
}

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(VMObjectData(vm, "System", "par").clone());

    return oo;

}
