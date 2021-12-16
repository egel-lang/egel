#include "../../src/runtime.hpp"

#include <stdlib.h>
#include <iostream>
#include <fstream>

#include <utility>
#include <string>
#include <map>

typedef std::vector<std::pair<VMObjectPtr, VMObjectPtr>> pqueue_t;

//## System::pqueue - a pqueue
class PQueue: public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_EGO, PQueue, "System", "pqueue");

    PQueue(VM* m, const pqueue_t& d): PQueue(m) {
        _value = d;
    }

    PQueue(const PQueue& d): PQueue(d.machine(), d.value()) {
    }

    static VMObjectPtr create(VM* m, const pqueue_t& d) {
        return VMObjectPtr(new PQueue(m, d));
    }

    static std::shared_ptr<PQueue> cast(const VMObjectPtr& o) {
        return std::static_pointer_cast<PQueue>(o);
    }

    int compare(const VMObjectPtr& o) override {
        return -1; // XXX: for later
    }

    pqueue_t value() const {
        return _value;
    }

    size_t size() const {
        return _value.size();
    }

    bool empty() const {
         return _value.size() == 0;
    }

    VMObjectPtr top() {
        auto p = _value[0];

        VMObjectPtrs tt;
        tt.push_back(p.first);
        tt.push_back(p.second);

        return machine()->to_tuple(tt);
    }

    void pop() {
        _value.erase(_value.begin());
    }

    void push(const VMObjectPtr& k, const VMObjectPtr& v) {
        auto it = _value.begin();

        bool found = false;
        unsigned int n = 0;
        while(!found && (n < _value.size())) {
            auto key = _value[n].first;
            if (machine()->compare(k, key) == -1) {
                found = true;
            } else {
                n++;
            }
        }

        _value.insert(it+n, std::pair<VMObjectPtr, VMObjectPtr>(k, v));
    }

protected:
    pqueue_t _value;
};

//## System::pqueue - create a pqueue object
class APQueue: public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_EGO, APQueue, "System", "pqueue");

    VMObjectPtr apply() const override {
        return PQueue::create(machine(), pqueue_t());
    }
};

//## System::pqueue_has d k - check for key
class PQueueEmpty: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, PQueueEmpty, "System", "pqueue_empty");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (m->is_opaque(arg0) && m->symbol(arg0) == "System::pqueue") {
            auto d = PQueue::cast(arg0);
            return m->create_bool(d->empty());
        } else {
            THROW_BADARGS;
        }
    }
};

//## System::pqueue_get d k - get a value by key
class PQueueTop: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, PQueueTop, "System", "pqueue_top");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (m->is_opaque(arg0) && m->symbol(arg0) == "System::pqueue") {
            auto d = PQueue::cast(arg0);
            return d->top();
        } else {
            THROW_BADARGS;
        }
    }
};

//## System::pqueue_keys d - pqueue keys as list
class PQueuePop: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, PQueuePop, "System", "pqueue_pop");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (m->is_opaque(arg0) && m->symbol(arg0) == "System::pqueue") {
            auto d = PQueue::cast(arg0);
            d->pop();
            return m->create_none();
        } else {
            THROW_BADARGS;
        }
    }
};

//## System::pqueue_set d k v - set a value by key
class PQueuePush: public Ternary {
public:
    TERNARY_PREAMBLE(VM_SUB_EGO, PQueuePush, "System", "pqueue_push");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        auto m = machine();
        if (m->is_opaque(arg0) && m->symbol(arg0) == "System::pqueue") {
            auto d = PQueue::cast(arg0);
            d->push(arg1, arg2);
            return arg0;
        } else {
            THROW_BADARGS;
        }
    }
};


extern "C" std::vector<icu::UnicodeString> egel_imports() {
    return std::vector<icu::UnicodeString>();
}

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(APQueue::create(vm));
    oo.push_back(PQueueEmpty::create(vm));
    oo.push_back(PQueueTop::create(vm));
    oo.push_back(PQueuePop::create(vm));
    oo.push_back(PQueuePush::create(vm));

    return oo;
}
