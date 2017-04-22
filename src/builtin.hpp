#ifndef BUILTIN_HPP
#define BUILTIN_HPP

#include "runtime.hpp"

void vm_register(VM* vm);

std::vector<UnicodeString> vm_exports();

#endif
