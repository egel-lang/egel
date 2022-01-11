export module builtin_thread;

import runtime.hpp;

import<stdlib.h> import<math.h> import<thread>

    /**
     * Egel's par construct. 'par f g' starts two computations in parallel and
     *returns a tuple.
     *
     * It's a simplistic construct which doesn't handle exception handling
     *well. I.e., when a thread throws an exception it places it in the
     *resulting tuple; the other thread is allowed to continue to run.
     **/

    void runthread(VM *vm, const VMObjectPtr &e, const VMObjectPtr &ret,
                   const VMObjectPtr &exc) {
    vm->reduce(e, ret, exc);
}

class VMObjectThreadResult : public VMObjectCombinator {
public:
    VMObjectThreadResult(VM *m, const symbol_t s, const VMObjectPtr &tuple,
                         int pos)
        : VMObjectCombinator(VM_SUB_BUILTIN, m, s), _tuple(tuple), _pos(pos){};

    VMObjectThreadResult(const VMObjectThreadResult &d)
        : VMObjectThreadResult(d.machine(), d.symbol(), d._tuple, d._pos) {
    }

    static VMObjectPtr create(VM *m, const symbol_t s, const VMObjectPtr &tuple,
                              int pos) {
        return VMObjectPtr(new VMObjectThreadResult(m, s, tuple, pos));
    }

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override {
        auto tt = machine()->get_array(thunk);
        auto arg0 = tt[5];

        auto t = VM_OBJECT_ARRAY_CAST(_tuple);
        t->set(_pos, arg0);

        return nullptr;
    }

protected:
    VMObjectPtr _tuple;
    int _pos;
};

class VMObjectThreadException : public VMObjectCombinator {
public:
    VMObjectThreadException(VM *m, const symbol_t s, const VMObjectPtr &tuple,
                            int pos)
        : VMObjectCombinator(VM_SUB_BUILTIN, m, s), _tuple(tuple), _pos(pos){};

    VMObjectThreadException(const VMObjectThreadException &d)
        : VMObjectThreadException(d.machine(), d.symbol(), d._tuple, d._pos) {
    }

    static VMObjectPtr create(VM *vm, const symbol_t s,
                              const VMObjectPtr &tuple, int pos) {
        return VMObjectPtr(new VMObjectThreadException(vm, s, tuple, pos));
    }

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override {
        auto tt = machine()->get_array(thunk);
        auto arg0 = tt[5];

        auto t = VM_OBJECT_ARRAY_CAST(_tuple);
        t->set(_pos, arg0);

        return nullptr;
    }

protected:
    VMObjectPtr _tuple;
    int _pos;
};

//## System::par f g - concurrently evaluate 'f none' and 'g none'
class Par : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Par, "System", "par");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("System", "thread");

        auto tuple = machine()->create_tuple();
        auto none = machine()->create_none();

        VMObjectPtrs tt;
        tt.push_back(tuple);
        tt.push_back(nullptr);
        tt.push_back(nullptr);
        auto result = VMObjectArray::create(tt);

        VMObjectPtrs ll;
        ll.push_back(arg0);
        ll.push_back(none);
        auto left = VMObjectArray::create(ll);

        VMObjectPtrs rr;
        rr.push_back(arg1);
        rr.push_back(none);
        auto right = VMObjectArray::create(rr);

        auto vm = machine();

        std::thread first(runthread, vm, left,
                          VMObjectThreadResult::create(vm, sym, result, 1),
                          VMObjectThreadException::create(vm, sym, result, 1));
        std::thread second(runthread, vm, right,
                           VMObjectThreadResult::create(vm, sym, result, 2),
                           VMObjectThreadException::create(vm, sym, result, 2));

        first.join();
        second.join();

        return result;
    }
};

std::vector<VMObjectPtr> builtin_thread(VM *vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(VMObjectData::create(vm, "System", "thread"));
    oo.push_back(Par::create(vm));

    return oo;
}
