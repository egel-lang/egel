#include "../../src/runtime.hpp"

#include <stdlib.h>
#include <unicode/regex.h>

/**
 * Start of a simplistic Regex library lifting most of libicu.
 */

#define REGEX_STRING    "Regex"

typedef std::vector<icu::UnicodeString> UnicodeStrings;

// convenience function
VMObjectPtr strings_to_list(VM* vm, UnicodeStrings ss) {
    auto _nil = vm->get_data_string("System", "nil");

    auto _cons = vm->get_data_string("System", "cons");

    VMObjectPtr result = _nil;

    for (int n = ss.size() - 1; n >= 0; n--) {
        VMObjectPtrs vv;
        vv.push_back(_cons);
        vv.push_back(VMObjectText(ss[n]).clone());
        vv.push_back(result);

        result = VMObjectArray(vv).clone();
    }

    return result;
}


//## Regex:pattern - an opaque object holding a pattern
class Regex;
typedef std::shared_ptr<Regex>  RegexPtr;

class Regex: public Opaque {
public:
    Regex(VM* vm, icu::RegexPattern* p)
        : Opaque(vm, REGEX_STRING, "pattern"), _pattern(p) {
    }

    Regex(const Regex& r): Regex(r.machine(), r.pattern()) {
    }

    ~Regex() {
        // XXX: leak for now
        // delete _pattern; 
    }

    VMObjectPtr clone() const override {
        return VMObjectPtr(new Regex(*this));
    }

    int compare(const VMObjectPtr& o) override {
        if ((o->tag() == VM_OBJECT_OPAQUE) &&
                (o->symbol() == this->symbol())) {
            RegexPtr r = std::static_pointer_cast<Regex>(o);
            if (string() < r->string()) {
                return -1;
            } else if (r->string() < string()) {
                return 1;
            } else {
                return 0;
            }
        } else {
            if (tag() < o->tag()) {
                return -1;
            } else {
                return 1;
            }
        }
    }

    icu::RegexPattern* pattern() const {
        return _pattern;
    }

    icu::UnicodeString string() const {
        return _pattern->pattern();
    }
    
    icu::RegexMatcher* matcher(const icu::UnicodeString& s) {
        UErrorCode  error_code = U_ZERO_ERROR;
        auto m = _pattern->matcher(s, error_code);
        if (U_FAILURE(error_code)) {
            INVALID; // XXX: do I leak here?
        } else {
            return m;
        }
    }

    vm_int_t flags() const {
        return (vm_int_t) _pattern->flags();
    }

    static bool is_regex_pattern(const VMObjectPtr& o) {
        if (o->tag() == VM_OBJECT_OPAQUE) {
            auto p = VM_OBJECT_OPAQUE_CAST(o);
            symbol_t sym = p->machine()->enter_symbol(REGEX_STRING, "pattern");
            return (p->symbol() == sym);
        } else {
            return false;
        }
    }

    static RegexPtr regex_pattern_cast(const VMObjectPtr& o) {
        return std::static_pointer_cast<Regex>(o);
    }

private:
    icu::RegexPattern* _pattern;
};

//## Regex:compile s0 - compile text to a pattern
class Compile: public Monadic {
public:
    MONADIC_PREAMBLE(Compile, REGEX_STRING, "compile");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            UParseError parse_error;
            UErrorCode  error_code = U_ZERO_ERROR;
            icu::UnicodeString pat = s0;
            icu::RegexPattern* p = icu::RegexPattern::compile(pat, parse_error, error_code);
            if (U_FAILURE(error_code)) {
                INVALID;
            } else {
                return Regex(this->machine(), p).clone();
            }
        } else {
            BADARGS;
        }
    }
};

//## Regex:match pat s0 - true if the pattern matches the entire string
class Match: public Dyadic {
public:
    DYADIC_PREAMBLE(Match, REGEX_STRING, "match");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((Regex::is_regex_pattern(arg0)) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto pat = Regex::regex_pattern_cast(arg0);
            auto s0 = VM_OBJECT_TEXT_VALUE(arg1);

            auto r = pat->matcher(s0);
            if (r == nullptr) INVALID;

            UErrorCode  error_code = U_ZERO_ERROR;
            auto b = r->matches(error_code);
            delete r;

            return create_bool(b);
        } else {
            BADARGS;
        }
    }
};

//## Regex:split pat s0 - split a text according to a pattern
class Split: public Dyadic {
public:
    DYADIC_PREAMBLE(Split, REGEX_STRING, "split");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((Regex::is_regex_pattern(arg0)) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto pat = Regex::regex_pattern_cast(arg0);
            auto s0 = VM_OBJECT_TEXT_VALUE(arg1);

            auto r = pat->matcher(s0);
            if (r == nullptr) INVALID;

            UnicodeStrings ss;
            int32_t pos = 0;
            int32_t start = 0;
            int32_t end = 0;
            while (r->find()) {
                UErrorCode  error_code = U_ZERO_ERROR;
                start = r->start(error_code);
                end   = r->end(error_code);
                
                icu::UnicodeString s;
                s0.extract(pos, start-pos, s);
                ss.push_back(s);

                pos = end;
            }
            icu::UnicodeString s;
            s0.extract(pos, s0.length()-pos, s);
            ss.push_back(s);
            delete r;

            return strings_to_list(machine(), ss);
        } else {
            BADARGS;
        }
    }
};

//## Regex:matches pat s0 - return a list of pattern matches in a string
class Matches: public Dyadic {
public:
    DYADIC_PREAMBLE(Matches, REGEX_STRING, "matches");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((Regex::is_regex_pattern(arg0)) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto pat = Regex::regex_pattern_cast(arg0);
            auto s0 = VM_OBJECT_TEXT_VALUE(arg1);

            auto r = pat->matcher(s0);
            if (r == nullptr) INVALID;

            UnicodeStrings ss;
            while (r->find()) {
                UErrorCode  error_code = U_ZERO_ERROR;
                auto start = r->start(error_code);
                auto end   = r->end(error_code);
                
                icu::UnicodeString s;
                s0.extract(start, end-start, s);
                ss.push_back(s);
            }
            delete r;

            return strings_to_list(machine(), ss);
        } else {
            BADARGS;
        }
    }
};

//## Regex:replace pat s0 s1 - replace the first occurence of pattern in a string with a string
class Replace: public Triadic {
public:
    TRIADIC_PREAMBLE(Replace, REGEX_STRING, "replace");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        if ((Regex::is_regex_pattern(arg0)) && (arg1->tag() == VM_OBJECT_TEXT) && (arg2->tag() == VM_OBJECT_TEXT)) {
            auto pat = Regex::regex_pattern_cast(arg0);
            auto s0 = VM_OBJECT_TEXT_VALUE(arg1);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg2);

            auto r = pat->matcher(s0);
            if (r == nullptr) INVALID;

            UErrorCode  error_code = U_ZERO_ERROR;
            auto s2 = r->replaceFirst(s1, error_code);
            delete r;

            if (U_FAILURE(error_code)) {
                INVALID;
            } else {
                return VMObjectText(s2).clone();
            }
        } else {
            BADARGS;
        }
    }
};

//## Regex:replace_all pat s0 s1 - replace the all occurences of pattern in a string with a string
class ReplaceAll: public Triadic {
public:
    TRIADIC_PREAMBLE(ReplaceAll, REGEX_STRING, "replace_all");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        if ((Regex::is_regex_pattern(arg0)) && (arg1->tag() == VM_OBJECT_TEXT) && (arg2->tag() == VM_OBJECT_TEXT)) {
            auto pat = Regex::regex_pattern_cast(arg0);
            auto s0 = VM_OBJECT_TEXT_VALUE(arg1);
            auto s1 = VM_OBJECT_TEXT_VALUE(arg2);

            auto r = pat->matcher(s0);
            if (r == nullptr) INVALID;

            UErrorCode  error_code = U_ZERO_ERROR;
            auto s2 = r->replaceAll(s1, error_code);
            delete r;

            if (U_FAILURE(error_code)) {
                INVALID;
            } else {
                return VMObjectText(s2).clone();
            }
        } else {
            BADARGS;
        }
    }
};

//## Regex:group pat s0 - return the matched groups in a string
class Group: public Dyadic {
public:
    DYADIC_PREAMBLE(Group, REGEX_STRING, "group");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((Regex::is_regex_pattern(arg0)) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto pat = Regex::regex_pattern_cast(arg0);
            auto s0 = VM_OBJECT_TEXT_VALUE(arg1);

            auto r = pat->matcher(s0);
            if (r == nullptr) INVALID;

            UnicodeStrings ss;
            UErrorCode  error_code = U_ZERO_ERROR;

            if (r->matches(error_code)) {
                int32_t gc = r->groupCount();
                for (int n = 1; n <= gc; n++) {
                    error_code = U_ZERO_ERROR;
                    auto s = r->group(n, error_code);
                    ss.push_back(s);
                }
            }

            delete r;

            return strings_to_list(machine(), ss);
        } else {
            BADARGS;
        }
    }
};

extern "C" std::vector<icu::UnicodeString> egel_imports() {
    return std::vector<icu::UnicodeString>();
}

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Compile(vm).clone());
    oo.push_back(Match(vm).clone());
    oo.push_back(Split(vm).clone());
    oo.push_back(Matches(vm).clone());
    oo.push_back(Replace(vm).clone());
    oo.push_back(ReplaceAll(vm).clone());
    oo.push_back(Group(vm).clone());

    return oo;

}
