module;

#include <exception>

#include "unicode/unistr.h"
#include "unicode/ustdio.h"
#include "unicode/ustream.h"

export module error;

import utils;
import position;
import constants;

export enum error_tag_t {
    ERROR_IO,
    ERROR_LEXICAL,
    ERROR_SYNTACTICAL,
    ERROR_IDENTIFICATION,
    ERROR_SEMANTICAL,
    ERROR_INTERNAL,
};

export class Error : public std::exception {
public:
    Error(error_tag_t t, const Position &p, const icu::UnicodeString &m)
        : _tag(t), _position(p), _message(m) {
        // useful for debugging, uncomment to get stack trace in debug mode
        // Error* e = (Error*) nullptr;
        // e->position();
    }

    ~Error() {
    }

    Position position() const {
        return _position;
    }

    icu::UnicodeString message() const {
        return _message;
    }

    icu::UnicodeString error() const {
        icu::UnicodeString s = "";
        if (position().resource() != "") {
            s += position().to_text() + ":";
        }
        switch (_tag) {
            case ERROR_IO:
                s += Constants::STRING_IO;
                break;
            case ERROR_LEXICAL:
                s += Constants::STRING_LEXICAL;
                break;
            case ERROR_SYNTACTICAL:
                s += Constants::STRING_SYNTACTICAL;
                break;
            case ERROR_IDENTIFICATION:
                s += Constants::STRING_IDENTIFICATION;
                break;
            case ERROR_SEMANTICAL:
                s += Constants::STRING_SEMANTICAL;
                break;
            case ERROR_INTERNAL:
                s += Constants::STRING_INTERNAL;
                break;
        }
        s += ":" + message();
        return s;
    }

    friend std::ostream &operator<<(std::ostream &o, const Error &e) {
        o << e.error();
        return o;
    }

private:
    error_tag_t _tag;
    Position _position;
    icu::UnicodeString _message;
};

export class ErrorIO : public Error {
public:
    ErrorIO(const Position &p, const icu::UnicodeString &m)
        : Error(ERROR_IO, p, m) {
    }

    ErrorIO(const icu::UnicodeString &m) : Error(ERROR_IO, Position(), m) {
    }
};

export class ErrorLexical : public Error {
public:
    ErrorLexical(const Position &p, const icu::UnicodeString &m)
        : Error(ERROR_LEXICAL, p, m) {
    }
};

export class ErrorSyntactical : public Error {
public:
    ErrorSyntactical(const Position &p, const icu::UnicodeString &m)
        : Error(ERROR_SYNTACTICAL, p, m) {
    }
};

export class ErrorIdentification : public Error {
public:
    ErrorIdentification(const Position &p, const icu::UnicodeString &m)
        : Error(ERROR_IDENTIFICATION, p, m) {
    }
};

export class ErrorSemantical : public Error {
public:
    ErrorSemantical(const Position &p, const icu::UnicodeString &m)
        : Error(ERROR_SEMANTICAL, p, m) {
    }

    ErrorSemantical(const icu::UnicodeString &m)
        : Error(ERROR_SEMANTICAL, Position(), m) {
    }
};

export class ErrorInternal : public Error {
public:
    ErrorInternal(const icu::UnicodeString &m)
        : Error(ERROR_INTERNAL, Position(), m) {
    }
};
