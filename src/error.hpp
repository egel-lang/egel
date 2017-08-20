#ifndef ERROR_HPP
#define ERROR_HPP

#include "utils.hpp"
#include "position.hpp"
#include "constants.hpp"
#include <exception>

typedef enum {
    ERROR_IO,
    ERROR_LEXICAL,
    ERROR_SYNTACTICAL,
    ERROR_IDENTIFICATION,
    ERROR_SEMANTICAL,
} error_tag_t;

class Error : public std::exception {
public:
    Error(error_tag_t t, const Position &p, const UnicodeString &m) :
        _tag(t), _position(p), _message(m)  {
            // useful for debugging, uncomment to get stack trace in debug mode
            //Error* e = (Error*) nullptr;
            //e->position();
    }

    ~Error() {
    }

    Position position() const {
        return _position;
    }
    
    UnicodeString message() const {
        return _message;
    }

    UnicodeString error() const {
        UnicodeString s = "";
        if (position().resource() != "") {
            s += position().to_text() + ":";
        }
        switch(_tag) {
        case ERROR_IO:
            s += STRING_IO;
            break;
        case ERROR_LEXICAL:
            s += STRING_LEXICAL;
            break;
        case ERROR_SYNTACTICAL:
            s += STRING_SYNTACTICAL;
            break;
        case ERROR_IDENTIFICATION:
            s += STRING_IDENTIFICATION;
            break;
        case ERROR_SEMANTICAL:
            s += STRING_SEMANTICAL;
            break;
        }
        s += ":" + message();
        return s;
    }

    friend std::ostream & operator<<(std::ostream &o, const Error &e) { 
        o << e.error();
        return o;
    }

private:
    error_tag_t     _tag;
    Position        _position;
    UnicodeString   _message;
};

class ErrorIO: public Error {
public:
    ErrorIO(const Position& p, const UnicodeString& m)
        : Error(ERROR_IO, p, m) { }

    ErrorIO(const UnicodeString& m)
        : Error(ERROR_IO, Position(), m) { }
};

class ErrorLexical: public Error {
public:
    ErrorLexical(const Position& p, const UnicodeString& m)
        : Error(ERROR_LEXICAL, p, m) {}
};

class ErrorSyntactical: public Error {
public:
    ErrorSyntactical(const Position& p, const UnicodeString& m)
        : Error(ERROR_SYNTACTICAL, p, m) {}
};

class ErrorIdentification: public Error {
public:
    ErrorIdentification(const Position& p, const UnicodeString& m)
        : Error(ERROR_IDENTIFICATION, p, m) {}
};

class ErrorSemantical: public Error {
public:
    ErrorSemantical(const Position& p, const UnicodeString& m)
        : Error(ERROR_SEMANTICAL, p, m) {}

    ErrorSemantical(const UnicodeString& m)
        : Error(ERROR_SEMANTICAL, Position(), m) { }
};

#endif
