#ifndef LEXER_HPP
#define LEXER_HPP

#include <vector>
#include <iostream>
#include <memory>
#include "position.hpp"
#include "error.hpp"
#include "reader.hpp"

typedef enum {
    TOKEN_ERROR=0,
    TOKEN_EOF,

//  delimiters
    TOKEN_DOT,
    TOKEN_COMMA,
    TOKEN_EQ,
    TOKEN_ASSIGN,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LSQUARE,
    TOKEN_RSQUARE,
    TOKEN_LCURLY,
    TOKEN_RCURLY,
    TOKEN_COLON,
    TOKEN_SEMICOLON,
    TOKEN_HASH,
    TOKEN_BAR,
    TOKEN_QUESTION,

//  operators and names
    TOKEN_OPERATOR,
    TOKEN_UPPERCASE,
    TOKEN_LOWERCASE,

//  literals
    TOKEN_INTEGER,
    TOKEN_HEXINTEGER,
    TOKEN_FLOAT,
    TOKEN_CHAR,
    TOKEN_TEXT,

//  special symbols
    TOKEN_LAMBDA,
    TOKEN_ARROW,

//  objects
    TOKEN_OBJECT,
    TOKEN_EXTENDS,
    TOKEN_WITH,

//  compound expressions
    TOKEN_IF,
    TOKEN_THEN,
    TOKEN_ELSE,
    TOKEN_TRY,
    TOKEN_CATCH,
    TOKEN_THROW,
    TOKEN_LET,
    TOKEN_IN,

    TOKEN_VAR,

//  declarations
    TOKEN_DATA,
    TOKEN_DEF,
    TOKEN_NAMESPACE,

// directives
    TOKEN_USING,
    TOKEN_IMPORT,

} token_t;

UnicodeString  token_text(token_t t);

class Token {
public:
    Token(token_t tag, const Position &position, const UnicodeString &text):
        _tag(tag), _position(position), _text(text) {
    }

    ~Token() {
    }

    bool is_tag(token_t tag) const {
        return _tag == tag;
    }

    token_t tag() const {
        return _tag;
    }

    void set_tag(token_t t) {
        _tag = t;
    }

    Position position() const {
        return _position;
    }

    UnicodeString text() const {
        return _text;
    }

    friend std::ostream& operator<<(std::ostream& os, const Token &t) {
        os << "[" << t.position() << ", :" << token_text(t.tag()) << ": " << t.text() << "]";
        return os;
    }

private:
    token_t         _tag;
    Position        _position;
    UnicodeString   _text;
};

class TokenReader;
typedef std::shared_ptr<TokenReader>    TokenReaderPtr;

class TokenReader {
public:

    virtual Token       look(uint_t n = 0)  = 0;
    virtual void        skip()  = 0;

    virtual uint_t      get_cursor()          = 0;
    virtual void        set_cursor(uint_t c)  = 0;

    virtual TokenReaderPtr  clone_reader() const = 0;
};

class TokenWriter;
typedef std::shared_ptr<TokenWriter>    TokenWriterPtr;

class TokenWriter {
public:
    virtual void            push(const Token &token) = 0;
    virtual TokenReaderPtr  clone_reader() const = 0;
};

class TokenVector: public TokenReader, public TokenWriter {
public:
    TokenVector() {
        _index = 0;
    }

    TokenVector(const TokenVector& v) {
        _index = 0;
        _tokens = v._tokens;
    }

    ~TokenVector() {
    }

    Token   look(uint_t n = 0) {
        if (_index + n < _tokens.size()) {
            return _tokens[_index + n];
        } else {
            return _tokens[_tokens.size() -1];
        }
    }

    void    skip() {
        _index++;
    }

    uint_t get_cursor() {
        return _index;
    }

    void    set_cursor(uint_t c) {
        _index = c;
    }

    void    push(const Token &t) {
        _tokens.push_back(t);
    }

    TokenReaderPtr  clone_reader() const {
        return TokenReaderPtr(new TokenVector(*this));
    }

    TokenWriterPtr  clone_writer() const {
        return TokenWriterPtr(new TokenVector(*this));
    }

private:
    std::vector<Token>  _tokens;
    uint_t              _index;
};

UnicodeString  token_text(token_t t);
TokenReaderPtr tokenize_from_reader(CharReader &reader);

#endif
