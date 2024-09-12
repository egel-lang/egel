#pragma once

#include <functional>

#include "runtime.hpp"

namespace egel {

class Evaluate : public Unary {
public:
    UNARY_PREAMBLE(VM_SUB_BUILTIN, Evaluate, "System", "eval");

    DOCSTRING("System::eval text - evaluatate the expression in `text`");
    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);

            VMObjectPtr r = nullptr;
            VMObjectPtr e = nullptr;

            callback_t main = [&r](VM *vm, const VMObjectPtr &o) { r = o; };
            callback_t exc = [&e](VM *vm, const VMObjectPtr &o) { e = o; };

            try {
                machine()->eval_line(s, main, exc);
            } catch (Error &e) {
                auto s = e.message();
                throw VMObjectText::create(s);
            }

            if (e != nullptr) {
                throw e;
            } else if (r != nullptr) {
                return r;
            } else {
                return nullptr;
            }
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }

    UnicodeString result(const VMObjectPtr &o) {
        return o->to_text();
    }
};

class EvalModule : public CModule {
public:
    icu::UnicodeString name() const override {
        return "eval";
    }

    icu::UnicodeString docstring() const override {
        return "The 'eval' module defines the eval combinator.";
    }

    std::vector<VMObjectPtr> exports(VM *vm) override {
        std::vector<VMObjectPtr> oo;
        oo.push_back(Evaluate::create(vm));
        return oo;
    }
};

}  // namespace egel
