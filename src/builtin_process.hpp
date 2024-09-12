#pragma once

#include <stdlib.h>

#include <chrono>
#include <mutex>
#include <queue>
#include <thread>

#include "runtime.hpp"

/**
 * Egel's process implementation.
 **/

namespace egel {

// DOCSTRING("namespace System - process support");

class Process : public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_BUILTIN, Process, "System", "process");

    DOCSTRING("System::process - opaque process object");
    Process(VM *vm, const VMObjectPtr &f)
        : Opaque(VM_SUB_BUILTIN, vm, "System", "process") {
        _program = f;
        _exception = nullptr;
        _state = RUNNING;
    }

    Process(const Process &proc)
        : Opaque(VM_SUB_BUILTIN, proc.machine(), proc.symbol()) {
        _program = proc.program();
        _exception = nullptr;
        _state = RUNNING;
    }

    static VMObjectPtr create(VM *vm, const VMObjectPtr &f) {
        return std::make_shared<Process>(vm, f);
    }

    int compare(const VMObjectPtr &o) override {
        return -1;  // XXX: fix this once
    }

    void set_program(VMObjectPtr &p) {
        _program = p;
    }

    VMObjectPtr program() const {
        return _program;
    }

    void in_push(const VMObjectPtr &o) {
        _lock.lock();
        _in_queue.push(o);
        _lock.unlock();
    }

    VMObjectPtr in_pop() {
        VMObjectPtr o = nullptr;
        _lock.lock();
        if (!_in_queue.empty()) {
            o = _in_queue.front();
            _in_queue.pop();
        }
        _lock.unlock();
        return o;
    }

    void out_push(const VMObjectPtr &o) {
        _lock.lock();
        _out_queue.push(o);
        _lock.unlock();
    }

    VMObjectPtr out_pop() {
        VMObjectPtr o = nullptr;
        _lock.lock();
        if (!_out_queue.empty()) {
            o = _out_queue.front();
            _out_queue.pop();
        }
        _lock.unlock();
        return o;
    }

    reducer_state_t get_state() const {
        return _state;
    }

    void set_state(reducer_state_t s) {
        if (_state != HALTED) {  // XXX: do I need to lock here?
            _state = s;
        }
    }

    VMObjectPtr get_exception() {
        _lock.lock();
        auto e = _exception;
        _lock.unlock();
        return e;
    }

    void set_exception(VMObjectPtr e) {
        _lock.lock();
        _exception = e;
        _lock.unlock();
    }

    void run() {
        symbol_t tup = machine()->enter_symbol("System", "tuple");

        _state = RUNNING;
        VMObjectPtr in = nullptr;

        while (_state != HALTED) {
            in = in_pop();
            if (in == nullptr) {
                std::this_thread::sleep_for(std::chrono::milliseconds(25));
            } else {
                VMObjectPtrs thunk;
                thunk.push_back(_program);
                thunk.push_back(in);  // NOTE: _program and in are reduced
                auto app = machine()->create_array(thunk);

                in = nullptr;

                auto r = machine()->reduce(app, &_state);

                if (r.exception) {
                    _exception = r.result;
                    set_state(HALTED);
                } else {
                    auto t = r.result;
                    if (machine()->is_array(t)) {
                        auto ff = machine()->get_array(t);
                        if ((ff.size() == 3) && (ff[0]->symbol() == tup)) {
                            out_push(ff[1]);
                            _program = ff[2];
                        } else {
                            _exception = VMObjectText::create("no tuple");
                            set_state(HALTED);
                        }
                    } else {
                        _exception = VMObjectText::create("no tuple");
                        set_state(HALTED);
                    }
                }
            }
        }
    }

protected:
    VMObjectPtr _program;
    std::queue<VMObjectPtr> _in_queue;
    std::queue<VMObjectPtr> _out_queue;
    VMObjectPtr _exception;
    std::mutex _lock;
    reducer_state_t _state;
};

void run_process(const VMObjectPtr &o) {
    auto process = std::static_pointer_cast<Process>(o);
    process->run();
}

class Proc : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Proc, "System", "proc");
    DOCSTRING("System::proc f - create a process object from f");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        auto vm = machine();

        auto proc = Process::create(vm, arg0);

        std::thread run(run_process, proc);

        run.detach();

        return proc;
    }
};

class Send : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Send, "System", "send");
    DOCSTRING("System::send proc msg - send message msg to proc");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        symbol_t pr = machine()->enter_symbol("System", "process");

        if ((arg0->tag() == VM_OBJECT_OPAQUE) && (arg0->symbol() == pr)) {
            auto process = std::static_pointer_cast<Process>(arg0);
            process->in_push(arg1);
            return machine()->create_none();
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class Recv : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Recv, "System", "recv");
    DOCSTRING("System::recv proc - receive a message from process proc");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        symbol_t pr = machine()->enter_symbol("System", "process");

        if ((machine()->is_opaque(arg0)) && (arg0->symbol() == pr)) {
            auto process = std::static_pointer_cast<Process>(arg0);
            VMObjectPtr msg = nullptr;
            while ((msg = process->out_pop()) == nullptr) {
                std::this_thread::sleep_for(std::chrono::milliseconds(25));
            }
            return msg;
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Halt : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Halt, "System", "halt");
    DOCSTRING("System::halt proc - halt process proc");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        symbol_t pr = machine()->enter_symbol("System", "process");

        if ((machine()->is_opaque(arg0)) && (arg0->symbol() == pr)) {
            auto process = std::static_pointer_cast<Process>(arg0);
            process->set_state(HALTED);
            return machine()->create_none();
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class ProcessModule : public CModule {
public:
    icu::UnicodeString name() const override {
        return "process";
    }

    icu::UnicodeString docstring() const override {
        return "The 'process' module defines a process abstraction.";
    }

    std::vector<VMObjectPtr> exports(VM *vm) override {
        std::vector<VMObjectPtr> oo;

        oo.push_back(Process::create(vm));
        oo.push_back(Proc::create(vm));
        oo.push_back(Send::create(vm));
        oo.push_back(Recv::create(vm));
        oo.push_back(Halt::create(vm));

        return oo;
    }
};

}  // namespace egel
