#include "runtime.hpp"
#include "builtin_process.hpp"

#include <stdlib.h>
#include <mutex>
#include <thread>
#include <chrono>
#include <queue>

/**
 * Egel's process implementation.
 **/

class Process : public Opaque {
public:
    OPAQUE_PREAMBLE(Process, "System", "process");

    Process(VM* vm, const VMObjectPtr& f)
        : Opaque(vm, "System", "process") {
        _program = f;
        _exception = nullptr;
        _state = RUNNING;
    }

    Process(const Process& proc): Opaque(proc.machine(), proc.symbol()) {
        _program = proc.program();
        _exception = nullptr;
        _state = RUNNING;
    }

    VMObjectPtr clone() const override {
        return VMObjectPtr(new Process(*this));
    }

    int compare(const VMObjectPtr& o) override {
        return -1; // XXX: fix this once
    }

    void set_program(VMObjectPtr& p) {
        _program = p;
    }

    VMObjectPtr program() const {
        return _program;
    }

    void in_push(const VMObjectPtr& o) {
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

    void out_push(const VMObjectPtr& o) {
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
        if (_state != HALTED) { // XXX: do I need to lock here?
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
                thunk.push_back(in); //NOTE: _program and in are reduced
                auto app = VMObjectArray(thunk).clone();

                in = nullptr;

                auto r = machine()->reduce(app, &_state);

                if (r.exception) {
                    _exception = r.result;
                    set_state(HALTED);
                } else {
                    auto t = r.result;
                    if (t->tag() == VM_OBJECT_ARRAY) {
                        auto ff = VM_OBJECT_ARRAY_VALUE(t);
                        if ((ff.size() == 3) && (ff[0]->symbol() == tup)) {
                            out_push(ff[1]);
                            _program = ff[2]; 
                        } else {
                            _exception = VMObjectText("no tuple").clone();
                            set_state(HALTED);
                        }
                    } else {
                        _exception = VMObjectText("no tuple").clone();
                        set_state(HALTED);
                    }
                }
            }
        }
    }

protected:
    VMObjectPtr             _program;
    std::queue<VMObjectPtr> _in_queue;
    std::queue<VMObjectPtr> _out_queue;
    VMObjectPtr             _exception;
    std::mutex              _lock;
    reducer_state_t         _state;
};

void run_process(const VMObjectPtr& o) {
    auto process = std::static_pointer_cast<Process>(o);
    process->run();
}

// System.proc f 
// Create a process object from f
class Proc: public Monadic {
public:
    MONADIC_PREAMBLE(Proc, "System", "proc");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto vm = machine();

        auto proc = Process(vm, arg0).clone();

        std::thread run(run_process, proc);

        run.detach();

        return proc;
    }
};

// System.send msg proc
// Send proc a message
class Send: public Dyadic {
public:
    DYADIC_PREAMBLE(Send, "System", "send");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        symbol_t pr = machine()->enter_symbol("System", "process");

        if ((arg1->tag() == VM_OBJECT_OPAQUE) && (arg1->symbol() == pr)) {
            auto process = std::static_pointer_cast<Process>(arg1);
            process->in_push(arg0);
            return machine()->get_data_string("System", "nop");
        } else {
            BADARGS;
        }
    }
};

// System.recv proc
// Receive a message from proc
class Recv: public Monadic {
public:
    MONADIC_PREAMBLE(Recv, "System", "recv");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        symbol_t pr = machine()->enter_symbol("System", "process");

        if ((arg0->tag() == VM_OBJECT_OPAQUE) && (arg0->symbol() == pr)) {
            auto process = std::static_pointer_cast<Process>(arg0);
            VMObjectPtr msg = nullptr;
            while ((msg = process->out_pop()) == nullptr) {
                std::this_thread::sleep_for(std::chrono::milliseconds(25));
            }
            return msg;
        } else {
            BADARGS;
        }
    }
};

// System.halt proc
// Halt process proc
class Halt: public Monadic {
public:
    MONADIC_PREAMBLE(Halt, "System", "halt");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        symbol_t pr = machine()->enter_symbol("System", "process");

        if ((arg0->tag() == VM_OBJECT_OPAQUE) && (arg0->symbol() == pr)) {
            auto process = std::static_pointer_cast<Process>(arg0);
            process->set_state(HALTED);
            return machine()->get_data_string("System", "nop");
        } else {
            BADARGS;
        }
    }
};

std::vector<VMObjectPtr> builtin_process(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Process(vm).clone()); // XXX: I always forget whether this is needed
    oo.push_back(Proc(vm).clone());
    oo.push_back(Send(vm).clone());
    oo.push_back(Recv(vm).clone());
    oo.push_back(Halt(vm).clone());

    return oo;
}
