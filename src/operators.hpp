#pragma once

#include "utils.hpp"

constexpr auto OPERATOR_BOTTOM = "=";

    inline int
    operator_compare(const icu::UnicodeString &o0,
                     const icu::UnicodeString &o1);

inline bool operator_is_infix(const icu::UnicodeString &o);
inline bool operator_is_prefix(const icu::UnicodeString &o);
inline bool operator_is_postfix(const icu::UnicodeString &o);

inline bool operator_is_left_associative(const icu::UnicodeString &o);
inline bool operator_is_right_associative(const icu::UnicodeString &o);
inline bool operator_is_not_associative(const icu::UnicodeString &o);

inline icu::UnicodeString operator_to_ascii(const icu::UnicodeString &o);

constexpr auto LEFT_ASSOC = (1 << 0);
constexpr auto RIGHT_ASSOC = (1 << 1);
constexpr auto NONE_ASSOC = (1 << 2);
constexpr auto PREFIX = (1 << 3);
constexpr auto POSTFIX = (1 << 4);
constexpr auto INFIX = (1 << 5);

using attr_t = uint8_t;

struct operator_t {
    UChar32 name;
    const char *translation;
    attr_t attribute;
};

constexpr auto OPERATORS_SIZE = (sizeof(operators) / sizeof(operator_t));

static operator_t operators[] = {
    {'=', "eq", NONE_ASSOC | INFIX},
    {'!', "ex", NONE_ASSOC | PREFIX},
    {'<', "lt", RIGHT_ASSOC | INFIX},
    {'>', "gt", RIGHT_ASSOC | INFIX},
    {'+', "pl", LEFT_ASSOC | PREFIX | INFIX},
    {'-', "sb", LEFT_ASSOC | PREFIX | INFIX},
    {'/', "dv", LEFT_ASSOC | INFIX},
    {'*', "ml", LEFT_ASSOC | INFIX},
    {'^', "pw", LEFT_ASSOC | INFIX},
    {'#', "hs", RIGHT_ASSOC | INFIX},
    {'%', "ct", RIGHT_ASSOC | INFIX},
    {'~', "tl", NONE_ASSOC | PREFIX | INFIX},
    {'@', "at", LEFT_ASSOC | INFIX},
    {'$', "do", LEFT_ASSOC | INFIX},
    {'&', "am", RIGHT_ASSOC | INFIX},
    {'|', "br", LEFT_ASSOC | INFIX},
    {'.', "dt", LEFT_ASSOC | INFIX},
};

int operator_char_entry(const UChar32 &c) {
    for (unsigned int i = 0; i < OPERATORS_SIZE; i++) {
        if (operators[i].name == c) {
            return i;
        }
    }
    return -1;
}

icu::UnicodeString operator_char_translation(int i) {
    return icu::UnicodeString(operators[i].translation);
}

attr_t operator_char_attributes(int i) {
    return operators[i].attribute;
}

int operator_char_compare(const UChar32 c0, const UChar32 c1) {
    int i0 = operator_char_entry(c0);
    int i1 = operator_char_entry(c1);

    if ((i0 < 0) && (i1 < 0)) {  // handle math and all other ops
        if (c0 < c1) {
            return -1;
        } else if (c1 < c0) {
            return 1;
        } else {
            return 0;
        }
    }

    if (i0 < i1) {
        return -1;
    } else if (i1 < i0) {
        return 1;
    } else {
        return 0;
    }
}

int operator_compare(const icu::UnicodeString &o0,
                     const icu::UnicodeString &o1) {
    int l0 = o0.length();
    int l1 = o1.length();

    ASSERT(l0 > 0);
    ASSERT(l1 > 0);

    int32_t i0 = 0;
    int32_t i1 = 0;

    for (int i = 0; (i < l0) && (i < l1); i++) {
        int b = operator_char_compare(o0.char32At(i0), o1.char32At(i1));
        if (b < 0) return -1;
        if (b > 0) return 1;
        i0 = o0.moveIndex32(i0, 1);
        i1 = o1.moveIndex32(i1, 1);
    };

    if (l0 < l1) return -1;
    if (l0 > l1) return 1;

    return 0;
}

UChar32 operator_head_char(const icu::UnicodeString &o) {
    ASSERT(o.length() > 0);
    UChar32 c = o.char32At(0);
    return c;
}

bool operator_is_infix(const icu::UnicodeString &o) {
    UChar32 c = operator_head_char(o);
    int i = operator_char_entry(c);
    if (i < 0) {
        return false;
    } else {
        attr_t f = operator_char_attributes(i);
        return (f & INFIX) == INFIX;
    }
}

bool operator_is_prefix(const icu::UnicodeString &o) {
    UChar32 c = operator_head_char(o);
    int i = operator_char_entry(c);
    if (i < 0) {
        return false;
    } else {
        attr_t f = operator_char_attributes(i);
        return (f & PREFIX) == PREFIX;
    }
}

bool operator_is_postfix(const icu::UnicodeString &o) {
    UChar32 c = operator_head_char(o);
    int i = operator_char_entry(c);
    if (i < 0) {
        return false;
    } else {
        attr_t f = operator_char_attributes(i);
        return (f & POSTFIX) == POSTFIX;
    }
}

bool operator_is_left_associative(const icu::UnicodeString &o) {
    UChar32 c = operator_head_char(o);
    int i = operator_char_entry(c);
    if (i < 0) {
        return false;
    } else {
        attr_t f = operator_char_attributes(i);
        return (f & LEFT_ASSOC) == LEFT_ASSOC;
    }
}

bool operator_is_right_associative(const icu::UnicodeString &o) {
    UChar32 c = operator_head_char(o);
    int i = operator_char_entry(c);
    if (i < 0) {
        return false;
    } else {
        attr_t f = operator_char_attributes(i);
        return (f & RIGHT_ASSOC) == RIGHT_ASSOC;
    }
}

bool operator_is_not_associative(const icu::UnicodeString &o) {
    UChar32 c = operator_head_char(o);
    int i = operator_char_entry(c);
    if (i < 0) {
        return false;
    } else {
        attr_t f = operator_char_attributes(i);
        return (f & NONE_ASSOC) == NONE_ASSOC;
    }
}

icu::UnicodeString operator_to_ascii(const icu::UnicodeString &o) {
    // XXX
    return "";
}
