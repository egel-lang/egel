#ifndef BUILTIN_SYSTEM_HPP
#define BUILTIN_SYSTEM_HPP

#include "runtime.hpp"

extern int     application_argc;
extern char**  application_argv;

std::vector<VMObjectPtr> builtin_system(VM* vm);

#endif
