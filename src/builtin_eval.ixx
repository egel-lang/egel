module;

#include <functional>

export module buitin_eval;

import runtime;

import utils;
import position;
import reader;
import lexical;
import syntactical;
import machine;
import modules;
import eval;


//## namespace System - the `eval` combinator

//## System::eval text - evaluatate the expression in `text`
class Evaluate : public Unary {
public:
    UNARY_PREAMBLE(VM_SUB_BUILTIN, Evaluate, "System", "eval");

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
	    throw badargs(this, arg0);
        }
    }

    UnicodeString result(const VMObjectPtr &o) {
        return o->to_text();
    }
};

std::vector<VMObjectPtr> builtin_eval(VM *vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Evaluate::create(vm));

    return oo;
};
