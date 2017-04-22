#ifndef EMIT_HPP
#define EMIT_HPP

#include <vector>
#include "error.hpp"
#include "ast.hpp"
#include "transform.hpp"
#include "runtime.hpp"
#include "bytecode.hpp"

void emit_data(VM* vm, const AstPtr& a);
void emit_code(VM* vm, const AstPtr& a);

#endif
