#pragma once

#include "runtime.hpp"

/**
 * Egel's string combinators.
 *
 * Loosely follow a subset of libicu. Strings are immutable, combinators are
 *pure.
 **/

// ## namespace String - string support routines

namespace egel {

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

// ## String::ls s0 s1 - less than operator
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

// ## String::compare s0 s1 - compare the characters bitwise
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

// ## String::compare_order s0 s1 - compare in code point order
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

// ## String::case_compare s0 s1 - compare two strings case-insensitivel
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

// ## String::extract n0 n1 s - extract range of chars from text
class Extract : public Triadic {
public:
    TRIADIC_PREAMBLE(VM_SUB_BUILTIN, Extract, "String", "extract");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1,
                      const VMObjectPtr &arg2) const override {
        if ((machine()->is_integer(arg0)) && (machine()->is_integer(arg1)) && (machine()->is_text(arg2))) {
            auto n0 = machine()->get_integer(arg0);
            auto n1 = machine()->get_integer(arg1);
            auto s = machine()->get_text(arg2);
            UnicodeString r;

            s.extract(n0, n1, r);

            return machine()->create_text(r);
        } else {
            throw machine()->bad_args(this, arg0, arg1, arg2);
        }
    }
};

// ## String::starts_with s0 s1 - starts with initial segment
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

// ## String::ends_with s0 s1 - ends with segment
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

// ## String::index_of s0 s1 - the first occurrence of a text
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

// ## String::last_index_of s0 s1 - the last occurrence of a text
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

// ## String::char_at n s - the char at offset
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

// ## String::move_index index delta s - move index by delta chars
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

// ## String::length s - number of chars 
class CountChar : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, CountChar, "String", "length");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_integer(s.countChar32());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## String::is_empty s - test whether the text is empty
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

// ## String::hash_code s - generate a hash code for this text
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

// ## String::append s0 s1 - append two texts
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

// ## String::insert s0 n s1 - insert at given position
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

// ## String::replace s0 s1 s2 - replace all occurrences
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

// ## String::remove n0 n1 s - remove characters in range
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

// ## String::retain n0 n1 s - retain the characters in the range
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

// ## String::trim s - trims leading and trailing whitespac
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

// ## String::reverse s - reverse
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

// ## String::to_upper s - convert to upper case
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

// ## String::to_lower s - convert to lower case
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

// ## String::fold_case s - case-folds the character
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

// ## String::unescape s - unescape characters
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
    oo.push_back(Extract::create(vm));
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

};  // namespace egel
