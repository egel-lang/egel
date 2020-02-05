#include "evaluate.hpp"

// System.eval text
class Evaluate: public Unary {
public:
    UNARY_PREAMBLE(Evaluate, "System", "eval");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);

            auto eval = (Eval*) machine()->get_context();

            VMObjectPtr r = nullptr;
            VMObjectPtr e = nullptr;

            callback_t main = [&r](VM* vm, const VMObjectPtr& o) { r = o; };
            callback_t exc  = [&e](VM* vm, const VMObjectPtr& o) { e = o; };

            try {
                eval->eval_line(s, main, exc);
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
            return nullptr;
        }
    }

    UnicodeString result(const VMObjectPtr& o) {
        return o->to_text();
    }
};

// System.blip expr
class Blip: public Monadic {
public:
    MONADIC_PREAMBLE(Blip, "System", "blip");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        UnicodeString t = arg0->to_text();
        return VMObjectText::create(t);
    }
};

std::vector<VMObjectPtr> builtin_eval(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Evaluate(vm).clone());
    oo.push_back(Blip(vm).clone());

    return oo;
}