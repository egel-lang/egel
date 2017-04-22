#ifndef SEMANTICAL_HPP
#define SEMANTICAL_HPP

#include "ast.hpp"
#include "environment.hpp"

void    declare(NamespacePtr env, const AstPtr& a);
AstPtr  identify(NamespacePtr env, const AstPtr& a);

#endif
