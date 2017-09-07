#include "../../src/runtime.hpp"


/**
 * Egel's string combinators.
 *
 * Loosely follow a subset of libicu. Strings are immutable, combinators are pure.
 **/

// String.eq s0 s1
// Equality operator. 
class Eq: public Dyadic {
public:
    DYADIC_PREAMBLE(Eq, "String", "eq");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_bool(s0 == s1);
        } else {
            return nullptr;
        }
    }
};

// String.neq s0 s1
// Inequality operator. 
class Neq: public Dyadic {
public:
    DYADIC_PREAMBLE(Neq, "String", "neq");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_bool(s0 != s1);
        } else {
            return nullptr;
        }
    }
};

// String.gt s0 s1
// Greater than operator. 
class Gt: public Dyadic {
public:
    DYADIC_PREAMBLE(Gt, "String", "gt");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_bool(s0 > s1);
        } else {
            return nullptr;
        }
    }
};

// String.ls s0 s1
// Less than operator. 
class Ls: public Dyadic {
public:
    DYADIC_PREAMBLE(Ls, "String", "ls");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_bool(s0 < s1);
        } else {
            return nullptr;
        }
    }
};

// String.ge s0 s1
// Greater than or equal operator. 
class Ge: public Dyadic {
public:
    DYADIC_PREAMBLE(Ge, "String", "ge");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_bool(s0 >= s1);
        } else {
            return nullptr;
        }
    }
};

// String.le s0 s1
// Less than or equal operator. 
class Le: public Dyadic {
public:
    DYADIC_PREAMBLE(Le, "String", "le");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_bool(s0 <= s1);
        } else {
            return nullptr;
        }
    }
};

// String.compare s0 s1
// Compare the characters bitwise in this UnicodeString to the characters in text. 
class Compare: public Dyadic {
public:
    DYADIC_PREAMBLE(Compare, "String", "compare");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_integer(s0.compare(s1));
        } else {
            return nullptr;
        }
    }
};

// String.compareCodePointOrder s0 s1
// Compare two Unicode strings in code point order. 
class CompareCodePointOrder: public Dyadic {
public:
    DYADIC_PREAMBLE(CompareCodePointOrder, "String", "compareCodePointOrder");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_integer(s0.compareCodePointOrder(s1));
        } else {
            return nullptr;
        }
    }
};

// String.caseCompare s0 s1
// Compare two strings case-insensitively using full case folding. 
class CaseCompare: public Dyadic {
public:
    DYADIC_PREAMBLE(CaseCompare, "String", "caseCompare");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_integer(s0.caseCompare(s1, U_FOLD_CASE_DEFAULT));
        } else {
            return nullptr;
        }
    }
};

// String.startsWith s0 s1
// Determine if this starts with the characters in text 
class StartsWith: public Dyadic {
public:
    DYADIC_PREAMBLE(StartsWith, "String", "startsWith");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_bool(s1.startsWith(s0));
        } else {
            return nullptr;
        }
    }
};

// String.endsWith s0 s1
// Determine if this ends with the characters in text 
class EndsWith: public Dyadic {
public:
    DYADIC_PREAMBLE(EndsWith, "String", "endsWith");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_bool(s1.endsWith(s0));
        } else {
            return nullptr;
        }
    }
};

// String.indexOf s0 s1
// Locate in this the first occurrence of the characters in text, using bitwise comparison. 
class IndexOf: public Dyadic {
public:
    DYADIC_PREAMBLE(IndexOf, "String", "indexOf");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_integer(s1.indexOf(s0));
        } else {
            return nullptr;
        }
    }
};

// String.lastIndexOf s0 s1
// Locate in this the last occurrence of the characters in text, using bitwise comparison. 
class LastIndexOf: public Dyadic {
public:
    DYADIC_PREAMBLE(LastIndexOf, "String", "lastIndexOf");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg1);
            return create_integer(s1.lastIndexOf(s0));
        } else {
            return nullptr;
        }
    }
};

// String.charAt n s
// Return the code point that contains the code unit at offset offset. 
class CharAt: public Dyadic {
public:
    DYADIC_PREAMBLE(CharAt, "String", "charAt");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_INTEGER)) {
            auto n = VM_OBJECT_INTEGER_VALUE(arg0);
            auto s = VM_OBJECT_TEXT_VALUE(arg1);
            return create_char(s.char32At(n));
        } else {
            return nullptr;
        }
    }
};


// String.moveIndex s index delta
// Move the code unit index along the string by delta code points. 
class MoveIndex: public Triadic {
public:
    TRIADIC_PREAMBLE(MoveIndex, "String", "moveIndex");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_INTEGER) && (arg2->tag() == VM_OBJECT_INTEGER)) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            auto n = VM_OBJECT_INTEGER_VALUE(arg1);
            auto d = VM_OBJECT_INTEGER_VALUE(arg2);
            return create_integer(s.moveIndex32(n, d));
        } else {
            return nullptr;
        }
    }
};


// String.length s 
// Count Unicode code points in the string. 
class Length: public Monadic {
public:
    MONADIC_PREAMBLE(Length, "String", "length");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_integer(s.countChar32());
        } else {
            return nullptr;
        }
    }
};


// String.isEmpty s 
// Count Unicode code points in the string. 
class IsEmpty: public Monadic {
public:
    MONADIC_PREAMBLE(IsEmpty, "String", "isEmpty");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_bool(s.isEmpty());
        } else {
            return nullptr;
        }
    }
};

// String.hashCode s 
// Generate a hash code for this object. 
class HashCode: public Monadic {
public:
    MONADIC_PREAMBLE(HashCode, "String", "hashCode");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_integer(s.hashCode());
        } else {
            return nullptr;
        }
    }
};

// String.isBogus s 
// Determine if this object contains a valid string. 
class IsBogus: public Monadic {
public:
    MONADIC_PREAMBLE(IsBogus, "String", "isBogus");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_bool(s.isBogus());
        } else {
            return nullptr;
        }
    }
};

// String.append s0 s1
// Append the characters in srcText to the UnicodeString object. 
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
            return nullptr;
        }
    }
};


// String.insert s0 n s1
// Insert the characters in srcText into the UnicodeString object at offset start. 
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
            return nullptr;
        }
    }
};

// String.replace s0 s1 s2
// Replace all occurrences of characters in oldText with the characters in newText. 
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
            return nullptr;
        }
    }
};

// String.remove s0 n0 n1
// Remove the characters in the range [start, limit) from the UnicodeString object. 
class Remove: public Triadic {
public:
    TRIADIC_PREAMBLE(Remove, "String", "remove");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_INTEGER) && (arg2->tag() == VM_OBJECT_INTEGER)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto n0 = VM_OBJECT_INTEGER_VALUE(arg1);
            auto n1 = VM_OBJECT_INTEGER_VALUE(arg2);
            return create_text(s0.removeBetween(n0, n1));
        } else {
            return nullptr;
        }
    }
};

// String.retain s0 n0 n1
// Retain the characters in the range [start, limit) from the UnicodeString object. 
class Retain: public Triadic {
public:
    TRIADIC_PREAMBLE(Retain, "String", "retain");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_INTEGER) && (arg2->tag() == VM_OBJECT_INTEGER)) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            auto n0 = VM_OBJECT_INTEGER_VALUE(arg1);
            auto n1 = VM_OBJECT_INTEGER_VALUE(arg2);
            return create_text(s0.retainBetween(n0, n1));
        } else {
            return nullptr;
        }
    }
};

// String.trim s 
// Trims leading and trailing whitespace from this UnicodeString. 
class Trim: public Monadic {
public:
    MONADIC_PREAMBLE(Trim, "String", "trim");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_text(s.trim());
        } else {
            return nullptr;
        }
    }
};

// String.reverse s 
// Reverse this UnicodeString in place. 
class Reverse: public Monadic {
public:
    MONADIC_PREAMBLE(Reverse, "String", "reverse");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_text(s.reverse());
        } else {
            return nullptr;
        }
    }
};

// String.toUpper s 
// Convert the characters in this to upper case following the conventions of the default locale. 
class ToUpper: public Monadic {
public:
    MONADIC_PREAMBLE(ToUpper, "String", "toUpper");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_text(s.toUpper());
        } else {
            return nullptr;
        }
    }
};

// String.toLower s 
// Convert the characters in this to lower case following the conventions of the default locale. 
class ToLower: public Monadic {
public:
    MONADIC_PREAMBLE(ToLower, "String", "toLower");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_text(s.toLower());
        } else {
            return nullptr;
        }
    }
};

// String.foldCase s 
// Case-folds the characters in this string. 
class FoldCase: public Monadic {
public:
    MONADIC_PREAMBLE(FoldCase, "String", "foldCase");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_text(s.foldCase());
        } else {
            return nullptr;
        }
    }
};

// String.unescape s 
// Unescape a string of characters and return a string containing the result. 
class Unescape: public Monadic {
public:
    MONADIC_PREAMBLE(Unescape, "String", "unescape");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s = VM_OBJECT_TEXT_VALUE(arg0);
            return create_text(s.unescape());
        } else {
            return nullptr;
        }
    }
};

extern "C" std::vector<UnicodeString> egel_imports() {
    return std::vector<UnicodeString>();
}

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Eq(vm).clone());
    oo.push_back(Neq(vm).clone());
    oo.push_back(Gt(vm).clone());
    oo.push_back(Ls(vm).clone());
    oo.push_back(Ge(vm).clone());
    oo.push_back(Le(vm).clone());
    oo.push_back(Compare(vm).clone());
    oo.push_back(CompareCodePointOrder(vm).clone());
    oo.push_back(CaseCompare(vm).clone());
    oo.push_back(StartsWith(vm).clone());
    oo.push_back(EndsWith(vm).clone());
    oo.push_back(IndexOf(vm).clone());
    oo.push_back(LastIndexOf(vm).clone());
    oo.push_back(CharAt(vm).clone());
    oo.push_back(MoveIndex(vm).clone());
    oo.push_back(Length(vm).clone());
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
    oo.push_back(Unescape(vm).clone());

    return oo;
}
