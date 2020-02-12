#ifndef OPERATORS_HPP
#define OPERATORS_HPP

#include "utils.hpp"

#define OPERATOR_BOTTOM "="

int     operator_compare(const icu::UnicodeString& o0, const icu::UnicodeString& o1);

bool    operator_is_infix(const icu::UnicodeString& o);
bool    operator_is_prefix(const icu::UnicodeString& o);
bool    operator_is_postfix(const icu::UnicodeString& o);

bool    operator_is_left_associative(const icu::UnicodeString& o);
bool    operator_is_right_associative(const icu::UnicodeString& o);
bool    operator_is_not_associative(const icu::UnicodeString& o);

icu::UnicodeString operator_to_ascii(const icu::UnicodeString& o);

#endif
