#include "builtin_eval.hpp"

#include "runtime.hpp"
#include "utils.hpp" 
#include "position.hpp" 
#include "reader.hpp" 
#include "lexical.hpp" 
#include "syntactical.hpp" 
#include "machine.hpp" 
#include "modules.hpp" 
#include "eval.hpp" 
#include <functional> 

//## namespace System - the `eval` combinator

//## System:eval text - evaluatate the expression in `text`
class Evaluate: public Unary {
public:
    UNARY_PREAMBLE(VM_SUB_BUILTIN, Evaluate, "System", "eval");

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
            THROW_BADARGS;
        }
    }

    UnicodeString result(const VMObjectPtr& o) {
        return o->to_text();
    }
};

std::vector<VMObjectPtr> builtin_eval(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Evaluate(vm).clone());

    return oo;
}
