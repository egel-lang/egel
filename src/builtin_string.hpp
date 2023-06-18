#pragma once

#include "runtime.hpp"

/**
 * Egel's string combinators.
 *
 * Loosely follow a subset of libicu. Strings are immutable, combinators are
 *pure.
 **/

// ## namespace String - string support routines

// ## String::eq s0 s1 - string equality operator
class StringEq : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, StringEq, "String", "eq");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            auto s0 = machine()->get_text(arg0);
            auto s1 = machine()->get_text(arg1);
            return machine()->create_bool(s0 == s1);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

// ## String::neq s0 s1 - inequality operator
class StringNeq : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, StringNeq, "String", "neq");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            auto s0 = machine()->get_text(arg0);
            auto s1 = machine()->get_text(arg1);
            return machine()->create_bool(s0 != s1);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

// ## String::gt s0 s1 - greater than operator
class StringGt : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, StringGt, "String", "gt");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            auto s0 = machine()->get_text(arg0);
            auto s1 = machine()->get_text(arg1);
            return machine()->create_bool(s0 > s1);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

// ## String::ls s0 s1 - string less than operator
class StringLs : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, StringLs, "String", "ls");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            auto s0 = machine()->get_text(arg0);
            auto s1 = machine()->get_text(arg1);
            return machine()->create_bool(s0 < s1);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

// ## String::ge s0 s1 - greater than or equal operator
class StringGe : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, StringGe, "String", "ge");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            auto s0 = machine()->get_text(arg0);
            auto s1 = machine()->get_text(arg1);
            return machine()->create_bool(s0 >= s1);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

// ## String::le s0 s1 - stringLess than or equal operator
class StringLe : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, StringLe, "String", "le");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            auto s0 = machine()->get_text(arg0);
            auto s1 = machine()->get_text(arg1);
            return machine()->create_bool(s0 <= s1);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

// ## String::compare s0 s1 - compare the characters bitwise in this
//  icu::UnicodeString to the characters in text
class Compare : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Compare, "String", "compare");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            auto s0 = machine()->get_text(arg0);
            auto s1 = machine()->get_text(arg1);
            return machine()->create_integer(s0.compare(s1));
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

// ## String::compare_order s0 s1 - compare two Unicode strings in code point
//  order
class CompareCodePointOrder : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, CompareCodePointOrder, "String",
                    "compare_order");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            auto s0 = machine()->get_text(arg0);
            auto s1 = machine()->get_text(arg1);
            return machine()->create_integer(s0.compareCodePointOrder(s1));
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

// ## String::case_compare s0 s1 - compare two strings case-insensitively using
//  full case folding
class CaseCompare : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, CaseCompare, "String", "case_compare");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            auto s0 = machine()->get_text(arg0);
            auto s1 = machine()->get_text(arg1);
            return machine()->create_integer(
                s0.caseCompare(s1, U_FOLD_CASE_DEFAULT));
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

// ## String::starts_with s0 s1 - determine if this starts with the characters
//  in text
class StartsWith : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, StartsWith, "String", "starts_with");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            auto s0 = machine()->get_text(arg0);
            auto s1 = machine()->get_text(arg1);
            return machine()->create_bool(s1.startsWith(s0));
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

// ## String::ends_with s0 s1 - determine if this ends with the characters in
//  text
class EndsWith : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, EndsWith, "String", "ends_with");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            auto s0 = machine()->get_text(arg0);
            auto s1 = machine()->get_text(arg1);
            return machine()->create_bool(s1.endsWith(s0));
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

// ## String::index_of s0 s1 - locate in this the first occurrence of the
//  characters in text, using bitwise comparison
class IndexOf : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, IndexOf, "String", "index_of");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            auto s0 = machine()->get_text(arg0);
            auto s1 = machine()->get_text(arg1);
            return machine()->create_integer(s1.indexOf(s0));
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

// ## String::last_index_of s0 s1 - locate in this the last occurrence of the
//  characters in text, using bitwise comparison
class LastIndexOf : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, LastIndexOf, "String", "last_index_of");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            auto s0 = machine()->get_text(arg0);
            auto s1 = machine()->get_text(arg1);
            return machine()->create_integer(s1.lastIndexOf(s0));
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

// ## String::char_at n s - return the code point that contains the code unit at
//  offset offset
class CharAt : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, CharAt, "String", "char_at");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_integer(arg0)) && (machine()->is_text(arg1))) {
            auto n = machine()->get_integer(arg0);
            auto s = machine()->get_text(arg1);
            return machine()->create_char(s.char32At(n));
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

// ## String::move_index index delta s - move the code unit index along the
//  string by delta code points
class MoveIndex : public Triadic {
public:
    TRIADIC_PREAMBLE(VM_SUB_BUILTIN, MoveIndex, "String", "move_index");

    VMObjectPtr apply(const VMObjectPtr &arg0, const VMObjectPtr &arg1,
                      const VMObjectPtr &arg2) const override {
        if ((machine()->is_integer(arg0)) && (machine()->is_integer(arg1)) &&
            (machine()->is_text(arg2))) {
            auto n = machine()->get_integer(arg0);
            auto d = machine()->get_integer(arg1);
            auto s = machine()->get_text(arg2);
            return machine()->create_integer(s.moveIndex32(n, d));
        } else {
            throw machine()->bad_args(this, arg0, arg1, arg2);
        }
    }
};

// ## String::count_char s - count Unicode code points in the string
class CountChar : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, CountChar, "String", "count_char");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_integer(s.countChar32());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## String::is_empty s - test whether the string is empty
class IsEmpty : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsEmpty, "String", "is_empty");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_bool(s.isEmpty());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## String::hash_code s - generate a hash code for this object
class HashCode : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, HashCode, "String", "hash_code");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_integer(s.hashCode());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## String::is_bogus s - determine if this object contains a valid string
class IsBogus : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsBogus, "String", "is_bogus");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_bool(s.isBogus());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## String::append s0 s1 - append the two strings
class Append : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Append, "String", "append");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            auto s0 = machine()->get_text(arg0);
            auto s1 = machine()->get_text(arg1);
            return machine()->create_text(s0.append(s1));
        } else if ((machine()->is_text(arg0)) && (machine()->is_char(arg1))) {
            auto s0 = machine()->get_text(arg0);
            auto c = machine()->get_char(arg1);
            return machine()->create_text(s0.append(c));
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

// ## String::insert s0 n s1 - insert the characters in s0 into the s1 at offset
//  n
class Insert : public Triadic {
public:
    TRIADIC_PREAMBLE(VM_SUB_BUILTIN, Insert, "String", "insert");

    VMObjectPtr apply(const VMObjectPtr &arg0, const VMObjectPtr &arg1,
                      const VMObjectPtr &arg2) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_integer(arg1)) &&
            (machine()->is_text(arg2))) {
            auto s0 = machine()->get_text(arg0);
            auto n = machine()->get_integer(arg1);
            auto s1 = machine()->get_text(arg2);
            return machine()->create_text(s1.insert(n, s0));
        } else {
            throw machine()->bad_args(this, arg0, arg1, arg2);
        }
    }
};

// ## String::replace s0 s1 s2 - replace all occurrences of characters s0 with
//  the characters s2 in s0
class FindAndReplace : public Triadic {
public:
    TRIADIC_PREAMBLE(VM_SUB_BUILTIN, FindAndReplace, "String", "replace");

    VMObjectPtr apply(const VMObjectPtr &arg0, const VMObjectPtr &arg1,
                      const VMObjectPtr &arg2) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1)) &&
            (machine()->is_text(arg2))) {
            auto s0 = machine()->get_text(arg0);
            auto s1 = machine()->get_text(arg1);
            auto s2 = machine()->get_text(arg2);
            return machine()->create_text(s2.findAndReplace(s0, s1));
        } else {
            throw machine()->bad_args(this, arg0, arg1, arg2);
        }
    }
};

// ## String::remove n0 n1 s - remove the characters in the range [n0, n1) from
//  s
class Remove : public Triadic {
public:
    TRIADIC_PREAMBLE(VM_SUB_BUILTIN, Remove, "String", "remove");

    VMObjectPtr apply(const VMObjectPtr &arg0, const VMObjectPtr &arg1,
                      const VMObjectPtr &arg2) const override {
        if ((machine()->is_integer(arg0)) && (machine()->is_integer(arg1)) &&
            (machine()->is_text(arg2))) {
            auto n0 = machine()->get_integer(arg0);
            auto n1 = machine()->get_integer(arg1);
            auto s0 = machine()->get_text(arg2);
            return machine()->create_text(s0.removeBetween(n0, n1));
        } else {
            throw machine()->bad_args(this, arg0, arg1, arg2);
        }
    }
};

// ## String::retain n0 n1 s - retain the characters in the range [n0, n1) from
//  s
class Retain : public Triadic {
public:
    TRIADIC_PREAMBLE(VM_SUB_BUILTIN, Retain, "String", "retain");

    VMObjectPtr apply(const VMObjectPtr &arg0, const VMObjectPtr &arg1,
                      const VMObjectPtr &arg2) const override {
        if ((machine()->is_integer(arg0)) && (machine()->is_integer(arg1)) &&
            (machine()->is_text(arg2))) {
            auto n0 = machine()->get_integer(arg0);
            auto n1 = machine()->get_integer(arg1);
            auto s0 = machine()->get_text(arg2);
            return machine()->create_text(s0.retainBetween(n0, n1));
        } else {
            throw machine()->bad_args(this, arg0, arg1, arg2);
        }
    }
};

// ## String::trim s - trims leading and trailing whitespace from this s
class Trim : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Trim, "String", "trim");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_text(s.trim());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## String::reverse s - reverse s
class Reverse : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Reverse, "String", "reverse");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_text(s.reverse());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## String::to_upper s - convert the characters in this to upper case
//  following the conventions of the default locale
class ToUpper : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, ToUpper, "String", "to_upper");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_text(s.toUpper());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## String::to_lower s - convert the characters in this to lower case
//  following the conventions of the default locale
class ToLower : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, ToLower, "String", "to_lower");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_text(s.toLower());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## String::fold_case s - case-folds the characters in this string
class FoldCase : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, FoldCase, "String", "fold_case");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_text(s.foldCase());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## String::unescape s - unescape a string of characters and return a string
//  containing the result
class Unescape : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Unescape, "String", "unescape");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_text(s.unescape());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## String::ord c - integer value of unicode point/character
class Ord : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Ord, "String", "ord");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_char(arg0)) {
            auto c = machine()->get_char(arg0);
            return machine()->create_integer(c);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## String::chr n - unicode point of integer value
class Chr : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Chr, "String", "chr");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto n = machine()->get_integer(arg0);
            return machine()->create_char(n);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

std::vector<VMObjectPtr> builtin_string(VM *vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(StringEq::create(vm));
    oo.push_back(StringNeq::create(vm));
    oo.push_back(StringGt::create(vm));
    oo.push_back(StringLs::create(vm));
    oo.push_back(StringGe::create(vm));
    oo.push_back(StringLe::create(vm));
    oo.push_back(Compare::create(vm));
    oo.push_back(CompareCodePointOrder::create(vm));
    oo.push_back(CaseCompare::create(vm));
    oo.push_back(StartsWith::create(vm));
    oo.push_back(EndsWith::create(vm));
    oo.push_back(IndexOf::create(vm));
    oo.push_back(LastIndexOf::create(vm));
    oo.push_back(CharAt::create(vm));
    oo.push_back(MoveIndex::create(vm));
    oo.push_back(CountChar::create(vm));
    oo.push_back(IsEmpty::create(vm));
    oo.push_back(HashCode::create(vm));
    oo.push_back(IsBogus::create(vm));
    oo.push_back(Append::create(vm));
    oo.push_back(Insert::create(vm));
    oo.push_back(FindAndReplace::create(vm));
    oo.push_back(Remove::create(vm));
    oo.push_back(Retain::create(vm));
    oo.push_back(Trim::create(vm));
    oo.push_back(Reverse::create(vm));
    oo.push_back(ToUpper::create(vm));
    oo.push_back(ToLower::create(vm));
    oo.push_back(FoldCase::create(vm));
    oo.push_back(Ord::create(vm));
    oo.push_back(Chr::create(vm));
    oo.push_back(Unescape::create(vm));

    return oo;
}
