#pragma once

#include <stdlib.h>

#include <chrono>
#include <future>

#include "runtime.hpp"

/**
 * Egel's async tasks implementation.
 **/

namespace egel {

// DOCSTRING("namespace System - async tasks support");

class Future : public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_BUILTIN, Future, "System", "future");

    DOCSTRING("System::future - opaque future object");
    int compare(const VMObjectPtr &o) override {
        return -1;  // XXX: fix this once
    }

    static VMReduceResult reduce(VM *vm, const VMObjectPtr &o) {
        return vm->reduce(o);
    }

    void async(const VMObjectPtr &o) {
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

class Async : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Async, "System", "async");

    DOCSTRING("System::async f - create a task");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        auto vm = machine();
        auto o = Future::create(vm);
        auto f = std::static_pointer_cast<Future>(o);
        f->async(arg0);
        return o;
    }
};

class Await : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Await, "System", "await");

    DOCSTRING("System::await f - wait for async task");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (Future::is_type(arg0)) {
            auto f = Future::cast(arg0);
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

class WaitFor : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, WaitFor, "System", "wait_for");

    DOCSTRING(
        "System::wait_for f n - check whether future reduced during "
        "milliseconds");
    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if (Future::is_type(arg0) && (machine()->is_integer(arg1))) {
            auto f = Future::cast(arg0);
            auto n = machine()->get_integer(arg1);
            auto b = f->wait_for(n);
            return machine()->create_bool(b);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class IsValid : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsValid, "System", "is_valid");

    DOCSTRING("System::is_valid f - check whether future is reduced");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (Future::is_type(arg0)) {
            auto f = Future::cast(arg0);
            auto b = f->valid();
            return machine()->create_bool(b);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Sleep : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Sleep, "System", "sleep");

    DOCSTRING("System::sleep n - sleep for a number of milliseconds");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto n = machine()->get_integer(arg0);
            std::this_thread::sleep_for(std::chrono::milliseconds(n));
            return machine()->create_none();
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class AsyncModule : public CModule {
public:
    icu::UnicodeString name() const override {
        return "async";
    }

    icu::UnicodeString docstring() const override {
        return "The 'async' module defines concurrency combinators.";
    }

    std::vector<VMObjectPtr> exports(VM *vm) override {
        std::vector<VMObjectPtr> oo;

        // oo.push_back(Future::create(vm)); // XXX: I always forget whether
        // this
        //  is needed
        oo.push_back(VMObjectStub::create(vm, "System::future"));
        oo.push_back(Async::create(vm));
        oo.push_back(Await::create(vm));
        oo.push_back(WaitFor::create(vm));
        oo.push_back(IsValid::create(vm));
        oo.push_back(Sleep::create(vm));

        return oo;
    }
};

}  // namespace egel
