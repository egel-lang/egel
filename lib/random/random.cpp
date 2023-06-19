#include <math.h>
#include <stdlib.h>

#include <mutex>
#include <random>
#include <thread>

#include "../../src/runtime.hpp"

using namespace egel;

/**
 * Start of a simplistic uniform random prng library which synchronizes
 * on a thread-safe singleton.
 */

class random {
private:
    random() {
    }

public:
    static random& get() {
        static random instance;
        return instance;
    }

    vm_int_t between(const vm_int_t& min, const vm_int_t& max) {
        std::uniform_int_distribution<vm_int_t> distribution(min, max);
        std::lock_guard<std::mutex> lock(_lock);
        return distribution(_generator);
    }

private:
    std::mutex _lock;
    std::mt19937 _generator;
};

// ## Math::between min max - return a random number between min and max
class Random : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, Random, "Math", "between");

    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
        if ((machine()->is_integer(arg0)) && (machine()->is_integer(arg1))) {
            auto i0 = machine()->get_integer(arg0);
            auto i1 = machine()->get_integer(arg1);
            return machine()->create_integer(random::get().between(i0, i1));
        } else {
            // XXX: extend once with two float values
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

extern "C" std::vector<icu::UnicodeString> egel_imports() {
    return std::vector<icu::UnicodeString>();
}

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Random::create(vm));

    return oo;
}
