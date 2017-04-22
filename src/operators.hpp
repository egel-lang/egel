#ifndef OPERATORS_HPP
#define OPERATORS_HPP

#include "utils.hpp"

#define OPERATOR_BOTTOM "="

int     operator_compare(const UnicodeString& o0, const UnicodeString& o1);

bool    operator_is_infix(const UnicodeString& o);
bool    operator_is_prefix(const UnicodeString& o);
bool    operator_is_postfix(const UnicodeString& o);

bool    operator_is_left_associative(const UnicodeString& o);
bool    operator_is_right_associative(const UnicodeString& o);
bool    operator_is_not_associative(const UnicodeString& o);

UnicodeString operator_to_ascii(const UnicodeString& o);

#endif
