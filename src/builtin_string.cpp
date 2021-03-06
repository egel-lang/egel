#include "runtime.hpp"
#include "builtin_string.hpp"


/**
 * Egel's string combinators.
 *
 * Loosely follow a subset of libicu. Strings are immutable, combinators are pure.
 **/

//## namespace String - string support routines

//## String:eq s0 s1 - string equality operator
class StringEq: public Dyadic {
public:
    DYADIC_PREAMBLE(StringEq, "String", "eq");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_bool(s0 == s1);
        } else {
            BADARGS;
        }
    }
};

//## String:neq s0 s1 - inequality operator
class StringNeq: public Dyadic {
public:
    DYADIC_PREAMBLE(StringNeq, "String", "neq");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_bool(s0 != s1);
        } else {
            BADARGS;
        }
    }
};

//## String:gt s0 s1 - greater than operator
class StringGt: public Dyadic {
public:
    DYADIC_PREAMBLE(StringGt, "String", "gt");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_bool(s0 > s1);
        } else {
            BADARGS;
        }
    }
};

//## String:ls s0 s1 - string less than operator
class StringLs: public Dyadic {
public:
    DYADIC_PREAMBLE(StringLs, "String", "ls");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_bool(s0 < s1);
        } else {
            BADARGS;
        }
    }
};

//## String:ge s0 s1 - greater than or equal operator
class StringGe: public Dyadic {
public:
    DYADIC_PREAMBLE(StringGe, "String", "ge");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_bool(s0 >= s1);
        } else {
            BADARGS;
        }
    }
};

//## String:le s0 s1 - stringLess than or equal operator
class StringLe: public Dyadic {
public:
    DYADIC_PREAMBLE(StringLe, "String", "le");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_bool(s0 <= s1);
        } else {
            BADARGS;
        }
    }
};

//## String:compare s0 s1 - compare the characters bitwise in this icu::UnicodeString to the characters in text
class Compare: public Dyadic {
public:
    DYADIC_PREAMBLE(Compare, "String", "compare");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_integer(s0.compare(s1));
        } else {
            BADARGS;
        }
    }
};

//## String:compare_order s0 s1 - compare two Unicode strings in code point order
class CompareCodePointOrder: public Dyadic {
public:
    DYADIC_PREAMBLE(CompareCodePointOrder, "String", "compare_order");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_integer(s0.compareCodePointOrder(s1));
        } else {
            BADARGS;
        }
    }
};

//## String:case_compare s0 s1 - compare two strings case-insensitively using full case folding
class CaseCompare: public Dyadic {
public:
    DYADIC_PREAMBLE(CaseCompare, "String", "case_compare");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_integer(s0.caseCompare(s1, U_FOLD_CASE_DEFAULT));
        } else {
            BADARGS;
        }
    }
};

//## String:starts_with s0 s1 - determine if this starts with the characters in text 
class StartsWith: public Dyadic {
public:
    DYADIC_PREAMBLE(StartsWith, "String", "starts_with");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_bool(s1.startsWith(s0));
        } else {
            BADARGS;
        }
    }
};

//## String:ends_with s0 s1 - determine if this ends with the characters in text 
class EndsWith: public Dyadic {
public:
    DYADIC_PREAMBLE(EndsWith, "String", "ends_with");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_bool(s1.endsWith(s0));
        } else {
            BADARGS;
        }
    }
};

//## String:index_of s0 s1 - locate in this the first occurrence of the characters in text, using bitwise comparison
class IndexOf: public Dyadic {
public:
    DYADIC_PREAMBLE(IndexOf, "String", "index_of");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_integer(s1.indexOf(s0));
        } else {
            BADARGS;
        }
    }
};

//## String:last_index_of s0 s1 - locate in this the last occurrence of the characters in text, using bitwise comparison
class LastIndexOf: public Dyadic {
public:
    DYADIC_PREAMBLE(LastIndexOf, "String", "last_index_of");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_integer(s1.lastIndexOf(s0));
        } else {
            BADARGS;
        }
    }
};

//## String:char_at n s - return the code point that contains the code unit at offset offset
class CharAt: public Dyadic {
public:
    DYADIC_PREAMBLE(CharAt, "String", "char_at");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_INTEGER) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto n = VM_OBJECT_INTEGER_VALUE(arg0);
            auto s = VM_OBJECT_TEXT_VALUE(arg1);
            return create_char(s.char32At(n));
        } else {
            BADARGS;
        }
    }
};


//## String:move_index index delta s - move the code unit index along the string by delta code points
class MoveIndex: public Triadic {
public:
    TRIADIC_PREAMBLE(MoveIndex, "String", "move_index");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        if ((arg0->tag() == VM_OBJECT_INTEGER) && (arg1->tag() == VM_OBJECT_INTEGER) && (arg2->tag() == VM_OBJECT_TEXT)) {
            auto n = VM_OBJECT_INTEGER_VALUE(arg0);
            auto d = VM_OBJECT_INTEGER_VALUE(arg1);
            auto s = VM_OBJECT_TEXT_VALUE(arg2);
            return create_integer(s.moveIndex32(n, d));
        } else {
            BADARGS;
        }
    }
};


//## String:strlen s - count Unicode code points in the string
class Strlen: public Monadic {
public:
    MONADIC_PREAMBLE(Strlen, "String", "strlen");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_integer(s.countChar32());
        } else {
            BADARGS;
        }
    }
};


//## String:is_empty s - test whether the string is empty
class IsEmpty: public Monadic {
public:
    MONADIC_PREAMBLE(IsEmpty, "String", "is_empty");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_bool(s.isEmpty());
        } else {
            BADARGS;
        }
    }
};

//## String:hash_code s - generate a hash code for this object
class HashCode: public Monadic {
public:
    MONADIC_PREAMBLE(HashCode, "String", "hash_code");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_integer(s.hashCode());
        } else {
            BADARGS;
        }
    }
};

//## String:is_bogus s - determine if this object contains a valid string
class IsBogus: public Monadic {
public:
    MONADIC_PREAMBLE(IsBogus, "String", "is_bogus");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_bool(s.isBogus());
        } else {
            BADARGS;
        }
    }
};

//## String:append s0 s1 - append the two strings
class Append: public Dyadic {
public:
    DYADIC_PREAMBLE(Append, "String", "append");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_text(s0.append(s1));
        } else if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_CHAR)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto c  = VM_OBJECT_CHAR_VALUE(arg1);
            return create_text(s0.append(c));
        } else {
            BADARGS;
        }
    }
};


//## String:insert s0 n s1 - insert the characters in s0 into the s1 at offset n
class Insert: public Triadic {
public:
    TRIADIC_PREAMBLE(Insert, "String", "insert");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_INTEGER) && (arg2->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto n  = VM_OBJECT_INTEGER_VALUE(arg1);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg2);
            return create_text(s1.insert(n, s0));
        } else {
            BADARGS;
        }
    }
};

//## String:replace s0 s1 s2 - replace all occurrences of characters s0 with the characters s2 in s0
class FindAndReplace: public Triadic {
public:
    TRIADIC_PREAMBLE(FindAndReplace, "String", "replace");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT) && (arg2->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            auto s2 = VM_OBJECT_TEXT_VALUE(arg2);
            return create_text(s2.findAndReplace(s0, s1));
        } else {
            BADARGS;
        }
    }
};

//## String:remove n0 n1 s - remove the characters in the range [n0, n1) from s
class Remove: public Triadic {
public:
    TRIADIC_PREAMBLE(Remove, "String", "remove");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        if ((arg0->tag() == VM_OBJECT_INTEGER) && (arg1->tag() == VM_OBJECT_INTEGER) && (arg2->tag() == VM_OBJECT_TEXT)) {
            auto n0 = VM_OBJECT_INTEGER_VALUE(arg0);
            auto n1 = VM_OBJECT_INTEGER_VALUE(arg1);
            auto s0 = VM_OBJECT_TEXT_VALUE(arg2);
            return create_text(s0.removeBetween(n0, n1));
        } else {
            BADARGS;
        }
    }
};

//## String:retain n0 n1 s - retain the characters in the range [n0, n1) from s
class Retain: public Triadic {
public:
    TRIADIC_PREAMBLE(Retain, "String", "retain");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        if ((arg0->tag() == VM_OBJECT_INTEGER) && (arg1->tag() == VM_OBJECT_INTEGER) && (arg2->tag() == VM_OBJECT_TEXT)) {
            auto n0 = VM_OBJECT_INTEGER_VALUE(arg0);
            auto n1 = VM_OBJECT_INTEGER_VALUE(arg1);
            auto s0 = VM_OBJECT_TEXT_VALUE(arg2);
            return create_text(s0.retainBetween(n0, n1));
        } else {
            BADARGS;
        }
    }
};

//## String:trim s - trims leading and trailing whitespace from this s 
class Trim: public Monadic {
public:
    MONADIC_PREAMBLE(Trim, "String", "trim");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_text(s.trim());
        } else {
            BADARGS;
        }
    }
};

//## String:reverse s - reverse s
class Reverse: public Monadic {
public:
    MONADIC_PREAMBLE(Reverse, "String", "reverse");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_text(s.reverse());
        } else {
            BADARGS;
        }
    }
};

//## String:to_upper s - convert the characters in this to upper case following the conventions of the default locale
class ToUpper: public Monadic {
public:
    MONADIC_PREAMBLE(ToUpper, "String", "to_upper");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_text(s.toUpper());
        } else {
            BADARGS;
        }
    }
};

//## String:to_lower s - convert the characters in this to lower case following the conventions of the default locale
class ToLower: public Monadic {
public:
    MONADIC_PREAMBLE(ToLower, "String", "to_lower");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_text(s.toLower());
        } else {
            BADARGS;
        }
    }
};

//## String:fold_case s - case-folds the characters in this string
class FoldCase: public Monadic {
public:
    MONADIC_PREAMBLE(FoldCase, "String", "fold_case");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_text(s.foldCase());
        } else {
            BADARGS;
        }
    }
};

//## String:unescape s - unescape a string of characters and return a string containing the result
class Unescape: public Monadic {
public:
    MONADIC_PREAMBLE(Unescape, "String", "unescape");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_text(s.unescape());
        } else {
            BADARGS;
        }
    }
};

//## String:ord c - integer value of unicode point/character
class Ord: public Monadic {
public:
    MONADIC_PREAMBLE(Ord, "String", "ord");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_CHAR) {
            auto c = VM_OBJECT_CHAR_VALUE(arg0);
            return create_integer(c);
        } else {
            BADARGS;
        }
    }
};

//## String:chr n - unicode point of integer value
class Chr: public Monadic {
public:
    MONADIC_PREAMBLE(Chr, "String", "chr");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_INTEGER) {
            auto n = VM_OBJECT_INTEGER_VALUE(arg0);
            return create_char(n);
        } else {
            BADARGS;
        }
    }
};


std::vector<VMObjectPtr> builtin_string(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(StringEq(vm).clone());
    oo.push_back(StringNeq(vm).clone());
    oo.push_back(StringGt(vm).clone());
    oo.push_back(StringLs(vm).clone());
    oo.push_back(StringGe(vm).clone());
    oo.push_back(StringLe(vm).clone());
    oo.push_back(Compare(vm).clone());
    oo.push_back(CompareCodePointOrder(vm).clone());
    oo.push_back(CaseCompare(vm).clone());
    oo.push_back(StartsWith(vm).clone());
    oo.push_back(EndsWith(vm).clone());
    oo.push_back(IndexOf(vm).clone());
    oo.push_back(LastIndexOf(vm).clone());
    oo.push_back(CharAt(vm).clone());
    oo.push_back(MoveIndex(vm).clone());
    oo.push_back(Strlen(vm).clone());
    oo.push_back(IsEmpty(vm).clone());
    oo.push_back(HashCode(vm).clone());
    oo.push_back(IsBogus(vm).clone());
    oo.push_back(Append(vm).clone());
    oo.push_back(Insert(vm).clone());
    oo.push_back(FindAndReplace(vm).clone());
    oo.push_back(Remove(vm).clone());
    oo.push_back(Retain(vm).clone());
    oo.push_back(Trim(vm).clone());
    oo.push_back(Reverse(vm).clone());
    oo.push_back(ToUpper(vm).clone());
    oo.push_back(ToLower(vm).clone());
    oo.push_back(FoldCase(vm).clone());
    oo.push_back(Ord(vm).clone());
    oo.push_back(Chr(vm).clone());
    oo.push_back(Unescape(vm).clone());

    return oo;
}
