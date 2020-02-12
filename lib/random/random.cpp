#include "../../src/runtime.hpp"

#include <stdlib.h>
#include <math.h>
#include <thread>
#include <random>
#include <mutex>

/**
 * Start of a simplistic uniform random prng library which synchronizes
 * on a thread-safe singleton.
 */

class random {
private:
    random() {}

public:
    static random& get() {
        static random instance;
        return instance;
    }

    vm_int_t between(const vm_int_t& min, const vm_int_t& max) {
        std::uniform_int_distribution<vm_int_t> distribution(min,max);
        std::lock_guard<std::mutex> lock(_lock);
        return distribution(_generator);
    }

private:
    std::mutex      _lock;
    std::mt19937    _generator;
};

// Math.between min max
class Random: public Dyadic {
public:
    DYADIC_PREAMBLE(Random, "Math", "between");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_INTEGER) && (arg1->tag() == VM_OBJECT_INTEGER)) {
            auto i0 = VM_OBJECT_INTEGER_VALUE(arg0);
            auto i1 = VM_OBJECT_INTEGER_VALUE(arg1);
            return VMObjectInteger(random::get().between(i0, i1)).clone();
        } else {
            // XXX: extend once with two float values
            return nullptr;
        }
    }
};

extern "C" std::vector<icu::UnicodeString> egel_imports() {
    return std::vector<icu::UnicodeString>();
}

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Random(vm).clone());

    return oo;

}
