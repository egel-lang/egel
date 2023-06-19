#pragma once

#include <stdlib.h>

#include <chrono>
#include <future>

#include "runtime.hpp"

/**
 * Egel's async tasks implementation.
 **/

namespace egel {

// ## namespace System - async tasks support

// ## System::future - opaque future object
class Future : public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_BUILTIN, Future, "System", "future");

    int compare(const VMObjectPtr &o) override {
        return -1;  // XXX: fix this once
    }

    static VMReduceResult reduce(VM* vm, const VMObjectPtr &o) {
        return vm->reduce(o);
    }

    void async(const VMObjectPtr& o) {
        VMObjectPtrs thunk;
        thunk.push_back(o);
        thunk.push_back(machine()->create_none());
        auto app = machine()->create_array(thunk);

        _future = std::async(std::launch::async, reduce, machine(), app);
    }

    VMReduceResult await() {
        return _future.get();
    }


    bool wait_for(int n) {
        std::chrono::milliseconds ms(n);
        auto status = _future.wait_for(ms);
        return (status == std::future_status::ready);
    }

    bool valid() {
        return _future.valid();
    }

protected:
    std::future<VMReduceResult> _future;
};

// ## System::async f - create a task
class Async : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Async, "System", "async");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        auto vm = machine();
        auto o = Future::create(vm);
        auto f = std::static_pointer_cast<Future>(o);
        f->async(arg0);
        return o;
    }
};

// ## System::await f - wait for async task
class Await : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Await, "System", "await");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        symbol_t s = machine()->enter_symbol("System", "future");

        if ((machine()->is_opaque(arg0)) && (arg0->symbol() == s)) {
            auto f = std::static_pointer_cast<Future>(arg0);
            auto r = f->await();
            if (r.exception) {
                throw r.result;
            } else {
                return r.result;
            }
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## System::wait_for f n - check whether f reduced in n milliseconds
class WaitFor : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, WaitFor, "System", "wait_for");

    VMObjectPtr apply(const VMObjectPtr &arg0, const VMObjectPtr &arg1) const override {
        symbol_t s = machine()->enter_symbol("System", "future");

        if ((machine()->is_opaque(arg0)) && (arg0->symbol() == s) && (machine()->is_integer(arg1))) {
            auto f = std::static_pointer_cast<Future>(arg0);
            auto n = machine()->get_integer(arg1);
            auto b = f->wait_for(n);
            return machine()->create_bool(b);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## System::is_valid f - check whether f reduced
class IsValid : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsValid, "System", "is_valid");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        symbol_t s = machine()->enter_symbol("System", "future");

        if ((machine()->is_opaque(arg0)) && (arg0->symbol() == s)) {
            auto f = std::static_pointer_cast<Future>(arg0);
            auto b = f->valid();
            return machine()->create_bool(b);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

inline std::vector<VMObjectPtr> builtin_async(VM *vm) {
    std::vector<VMObjectPtr> oo;

    //oo.push_back(Future::create(vm)); // XXX: I always forget whether this
    // is needed
    oo.push_back(VMObjectStub::create(
        vm, "System::future"));  // XXX: I always forget whether this is needed
    oo.push_back(Async::create(vm));
    oo.push_back(Await::create(vm));
    oo.push_back(WaitFor::create(vm));
    oo.push_back(IsValid::create(vm));

    return oo;
}

}  // namespace egel
