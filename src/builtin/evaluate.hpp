#ifndef BUILTIN_EVAL_HPP
#define BUILTIN_EVAL_HPP

#include "../runtime.hpp"

#include "../utils.hpp"
#include "../position.hpp"
#include "../reader.hpp"
#include "../lexical.hpp"
#include "../syntactical.hpp"
#include "../machine.hpp"
#include "../modules.hpp"
#include "../eval.hpp"

#include <functional>

std::vector<VMObjectPtr> builtin_eval(VM* vm); // XXX: forward declared in modules

#endif
