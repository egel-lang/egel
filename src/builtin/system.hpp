#ifndef BUILTIN_SYSTEM_HPP
#define BUILTIN_SYSTEM_HPP

#include "../runtime.hpp"

extern "C" char** application_argv;
extern "C" int    application_argc;

std::vector<VMObjectPtr> builtin_system(VM* vm);

#endif
