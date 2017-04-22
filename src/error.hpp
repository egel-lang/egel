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
    }

    ~Error() {
    }

    Position position() const {
        return _position;
    }
    
    UnicodeString message() const {
        return _message;
    }

    friend std::ostream & operator<<(std::ostream &o, const Error &e) { 
        if (e.position().resource() != "") {
            o << e.position().resource() << ":";
            o << e.position().row() << ":";
            o << e.position().column() << ": ";
        }
        switch(e._tag) {
        case ERROR_IO:
            o << STRING_IO;
            break;
        case ERROR_LEXICAL:
            o << STRING_LEXICAL;
            break;
        case ERROR_SYNTACTICAL:
            o << STRING_SYNTACTICAL;
            break;
        case ERROR_IDENTIFICATION:
            o << STRING_IDENTIFICATION;
            break;
        case ERROR_SEMANTICAL:
            o << STRING_SEMANTICAL;
            break;
        }
        o << " error: " << e.message();
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
