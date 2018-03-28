#include "../../src/runtime.hpp"

#include <stdlib.h>
#include <unicode/regex.h>

/**
 * Start of a simplistic Regex library lifting most of libicu.
 */

#define REGEX_STRING    "Regex"

class Regex;
typedef std::shared_ptr<Regex>  RegexPtr;

class Regex: public Opaque {
public:
    Regex(VM* vm, RegexPattern* p)
        : Opaque(vm, REGEX_STRING, "pattern"), _pattern(p) {
    }

    Regex(const Regex& r): Regex(r.machine(), r.pattern()) {
    }

    ~Regex() {
        delete _pattern;
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

    RegexPattern* pattern() const {
        return _pattern;
    }

    UnicodeString string() const {
        return _pattern->pattern();
    }
    
    RegexMatcher* matcher(const UnicodeString& s) {
        UErrorCode  error_code;
        auto m = _pattern->matcher(s, error_code);
        if (U_FAILURE(error_code)) {
            return nullptr; // XXX: do I leak here?
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
    RegexPattern* _pattern;
};

// Regex.compile s0
class Compile: public Monadic {
public:
    MONADIC_PREAMBLE(Compile, REGEX_STRING, "compile");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            UParseError parse_error;
            UErrorCode  error_code;
            RegexPattern* p = icu::RegexPattern::compile(s0, parse_error, error_code);
            if (U_FAILURE(error_code)) {
                return nullptr;
            } else {
                return Regex(this->machine(), p).clone();
            }
        } else {
            return nullptr;
        }
    }
};

// Regex.match pat s0
// True if pat matches the entire string
class Match: public Dyadic {
public:
    DYADIC_PREAMBLE(Match, REGEX_STRING, "match");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        static VMObjectPtr _true = nullptr;
        if (_true == nullptr) _true = machine()->get_data_string("System", "true");

        static VMObjectPtr _false = nullptr;
        if (_false == nullptr) _false = machine()->get_data_string("System", "false");

        if ((Regex::is_regex_pattern(arg0)) && (arg1->tag() == VM_OBJECT_TEXT)) {
            auto pat = Regex::regex_pattern_cast(arg0);
            auto s0 = VM_OBJECT_TEXT_VALUE(arg0);
            UErrorCode  error_code;
            auto r = pat->matcher(s0);
            if (r == nullptr) return nullptr;
            auto b = r->matches(error_code);
            delete r;
            if (b) {
               return _true;
            } else {
               return _false;
            } 
        } else {
            return nullptr;
        }
    }
};

// Regex.split pat s0
// Regex.match pat s0
// Regex.replace pat s0 s1
// Regex.replaceAll pat s0 s1

extern "C" std::vector<UnicodeString> egel_imports() {
    return std::vector<UnicodeString>();
}

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Compile(vm).clone());

    return oo;

}
