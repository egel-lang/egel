#pragma once

#include "runtime.hpp"

/**
 * Egel's string combinators.
 *
 * Loosely follow a subset of libicu. Strings are immutable, combinators are
 *pure.
 **/

// DOCSTRING("namespace String - string support routines");

namespace egel {

class StringEq : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, StringEq, "String", "eq");
    DOCSTRING("String::eq s0 s1 - string equality operator");

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

class StringNeq : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, StringNeq, "String", "neq");
    DOCSTRING("String::neq s0 s1 - inequality operator");

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

class StringGt : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, StringGt, "String", "gt");
    DOCSTRING("String::gt s0 s1 - greater than operator");

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

class StringLs : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, StringLs, "String", "ls");
    DOCSTRING("String::ls s0 s1 - less than operator");

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

class StringGe : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, StringGe, "String", "ge");
    DOCSTRING("String::ge s0 s1 - greater than or equal operator");

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

class StringLe : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, StringLe, "String", "le");
    DOCSTRING("String::le s0 s1 - stringLess than or equal operator");

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

class Compare : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Compare, "String", "compare");
    DOCSTRING("String::compare s0 s1 - compare the characters bitwise");

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

class CompareCodePointOrder : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, CompareCodePointOrder, "String",
                    "compare_order");
    DOCSTRING("String::compare_order s0 s1 - compare in code point order");

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

class CaseCompare : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, CaseCompare, "String", "case_compare");
    DOCSTRING(
        "String::case_compare s0 s1 - compare two strings case-insensitivel");

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

class Extract : public Triadic {
public:
    TRIADIC_PREAMBLE(VM_SUB_BUILTIN, Extract, "String", "extract");
    DOCSTRING("String::extract n0 n1 s - extract range of chars from text");

    VMObjectPtr apply(const VMObjectPtr &arg0, const VMObjectPtr &arg1,
                      const VMObjectPtr &arg2) const override {
        if ((machine()->is_integer(arg0)) && (machine()->is_integer(arg1)) &&
            (machine()->is_text(arg2))) {
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

class StartsWith : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, StartsWith, "String", "starts_with");
    DOCSTRING("String::starts_with s0 s1 - starts with initial segment");

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

class EndsWith : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, EndsWith, "String", "ends_with");
    DOCSTRING("String::ends_with s0 s1 - ends with segment");

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

class IndexOf : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, IndexOf, "String", "index_of");
    DOCSTRING("String::index_of s0 s1 - the first occurrence of a text");

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

class LastIndexOf : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, LastIndexOf, "String", "last_index_of");
    DOCSTRING("String::last_index_of s0 s1 - the last occurrence of a text");

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

class CharAt : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, CharAt, "String", "char_at");
    DOCSTRING("String::char_at n s - the char at offset");

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

class MoveIndex : public Triadic {
public:
    TRIADIC_PREAMBLE(VM_SUB_BUILTIN, MoveIndex, "String", "move_index");
    DOCSTRING("String::move_index index delta s - move index by delta chars");

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

class CountChar : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, CountChar, "String", "count");
    DOCSTRING("String::count s - number of chars");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_integer(s.countChar32());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class IsEmpty : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsEmpty, "String", "is_empty");
    DOCSTRING("String::is_empty s - test whether the text is empty");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_bool(s.isEmpty());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class HashCode : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, HashCode, "String", "hash_code");
    DOCSTRING("String::hash_code s - generate a hash code for this text");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_integer(s.hashCode());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class IsBogus : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsBogus, "String", "is_bogus");
    DOCSTRING(
        "String::is_bogus s - determine if this object contains a valid "
        "string");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_bool(s.isBogus());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Append : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Append, "String", "append");
    DOCSTRING("String::append s0 s1 - append two texts");

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

class Insert : public Triadic {
public:
    TRIADIC_PREAMBLE(VM_SUB_BUILTIN, Insert, "String", "insert_at");
    DOCSTRING("String::insert_at s0 n s1 - insert at given position");

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

class FindAndReplace : public Triadic {
public:
    TRIADIC_PREAMBLE(VM_SUB_BUILTIN, FindAndReplace, "String", "replace");
    DOCSTRING("String::replace s0 s1 s2 - replace all occurrences");

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

class Remove : public Triadic {
public:
    TRIADIC_PREAMBLE(VM_SUB_BUILTIN, Remove, "String", "remove");
    DOCSTRING("String::remove n0 n1 s - remove characters in range");

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

class Retain : public Triadic {
public:
    TRIADIC_PREAMBLE(VM_SUB_BUILTIN, Retain, "String", "retain");
    DOCSTRING("String::retain n0 n1 s - retain the characters in the range");

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

class Trim : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Trim, "String", "trim");
    DOCSTRING("String::trim s - trims leading and trailing whitespac");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_text(s.trim());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Reverse : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Reverse, "String", "reverse");
    DOCSTRING("String::reverse s - reverse");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_text(s.reverse());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//  following the conventions of the default locale
class ToUpper : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, ToUpper, "String", "to_upper");
    DOCSTRING("String::to_upper s - convert to upper case");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_text(s.toUpper());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//  following the conventions of the default locale
class ToLower : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, ToLower, "String", "to_lower");
    DOCSTRING("String::to_lower s - convert to lower case");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_text(s.toLower());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class FoldCase : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, FoldCase, "String", "fold_case");
    DOCSTRING("String::fold_case s - case-folds the character");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_text(s.foldCase());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Unescape : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Unescape, "String", "unescape");
    DOCSTRING("String::unescape s - unescape characters");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            return machine()->create_text(s.unescape());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Ord : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Ord, "String", "ord");
    DOCSTRING("String::ord c - integer value of unicode point/character");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_char(arg0)) {
            auto c = machine()->get_char(arg0);
            return machine()->create_integer(c);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Chr : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Chr, "String", "chr");
    DOCSTRING("String::chr n - unicode point of integer value");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto n = machine()->get_integer(arg0);
            return machine()->create_char(n);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class ToChars : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, ToChars, "String", "to_chars");
    DOCSTRING("String::to_chars s - create a list of chars from a string");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_text(arg0)) {
            auto str = machine()->get_text(arg0);

            VMObjectPtrs ss;
            for (int i = 0; i < str.length(); i = str.moveIndex32(i, 1)) {
                auto c = machine()->create_char(str.char32At(i));
                ss.push_back(c);
            }
            return machine()->to_list(ss);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class FromChars : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, FromChars, "String", "from_chars");
    DOCSTRING("String::from_chars s - create a string from a list of chars");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        // rewrite to use machine()->is_nil
        static symbol_t _nil = 0;
        if (_nil == 0) _nil = machine()->enter_symbol("System", "nil");

        static symbol_t _cons = 0;
        if (_cons == 0) _cons = machine()->enter_symbol("System", "cons");

        icu::UnicodeString ss;
        auto a = arg0;

        while ((machine()->is_array(a))) {
            auto aa = machine()->get_array(a);
            if (aa.size() != 3) machine()->bad(this, "invalid");
            if (aa[0]->symbol() != _cons) machine()->bad(this, "invalid");
            if (aa[1]->tag() != VM_OBJECT_CHAR) machine()->bad(this, "invalid");

            UChar32 c = machine()->get_char(aa[1]);
            ss += c;
            a = aa[2];
        }

        return VMObjectText::create(ss);
    }
};

class StringModule : public CModule {
public:
    virtual ~StringModule() {
    }

    icu::UnicodeString name() const override {
        return "string";
    }

    icu::UnicodeString docstring() const override {
        return "The 'string' module defines string manipulation combinators.";
    }

    std::vector<VMObjectPtr> exports(VM *vm) override {
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
};

};  // namespace egel
