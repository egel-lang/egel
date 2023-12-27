#pragma once

#include "runtime.hpp"
#include "constants.hpp"
#include "error.hpp"
#include "position.hpp"
#include "reader.hpp"

#include <iostream>
#include <memory>
#include <vector>

#include "unicode/unistr.h"
#include "unicode/ustdio.h"
#include "unicode/ustream.h"

namespace egel {

enum token_t {
    TOKEN_ERROR,
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
    TOKEN_DCOLON,
    TOKEN_SEMICOLON,
    TOKEN_DSEMICOLON,
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
    TOKEN_CLASS,
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

    //  do sugar,
    TOKEN_DO,

    //  declarations
    TOKEN_DATA,
    TOKEN_DEF,
    TOKEN_NAMESPACE,
    TOKEN_VAL,

    // directives
    TOKEN_USING,
    TOKEN_IMPORT,

};

icu::UnicodeString token_text(token_t t);

class Token {
public:
    Token(token_t tag, const Position &position, const icu::UnicodeString &text)
        : _tag(tag), _position(position), _text(text) {
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

    icu::UnicodeString text() const {
        return _text;
    }

    friend std::ostream &operator<<(std::ostream &os, const Token &t) {
        os << "[" << t.position() << ", :" << token_text(t.tag()) << ": "
           << t.text() << "]";
        return os;
    }

private:
    token_t _tag;
    Position _position;
    icu::UnicodeString _text;
};

class TokenReader;
using TokenReaderPtr = std::shared_ptr<TokenReader>;

class TokenReader {
public:
    virtual Token look(unsigned int n = 0) = 0;
    virtual void skip() = 0;

    virtual unsigned int get_cursor() = 0;
    virtual void set_cursor(unsigned int c) = 0;

    virtual TokenReaderPtr clone_reader() const = 0;
};

class TokenWriter;
using TokenWriterPtr = std::shared_ptr<TokenWriter>;

class TokenWriter {
public:
    virtual void push(const Token &token) = 0;
    virtual TokenReaderPtr clone_reader() const = 0;
};

class TokenVector : public TokenReader, public TokenWriter {
public:
    TokenVector() {
        _index = 0;
    }

    TokenVector(const TokenVector &v) {
        _index = 0;
        _tokens = v._tokens;
    }

    virtual ~TokenVector() {  // keep the compiler happy
    }

    Token look(unsigned int n = 0) {
        if (_index + n < _tokens.size()) {
            return _tokens[_index + n];
        } else {
            return _tokens[_tokens.size() - 1];
        }
    }

    void skip() {
        _index++;
    }

    unsigned int get_cursor() {
        return _index;
    }

    void set_cursor(unsigned int c) {
        _index = c;
    }

    void push(const Token &t) {
        _tokens.push_back(t);
    }

    TokenReaderPtr clone_reader() const {
        return TokenReaderPtr(new TokenVector(*this));
    }

    TokenWriterPtr clone_writer() const {
        return TokenWriterPtr(new TokenVector(*this));
    }

private:
    std::vector<Token> _tokens;
    unsigned int _index;
};

TokenReaderPtr tokenize_from_reader(CharReader &reader);

// XXX: everything below this should be module private

icu::UnicodeString token_text(token_t t);

// character classes

bool is_whitespace(UChar32 c) {
    return (bool)u_isUWhiteSpace(c);
}

bool is_eol(UChar32 c) {
    return (c == (UChar32)'\n');
}

bool is_hash(UChar32 c) {
    return (c == (UChar32)'#');
}

bool is_dot(UChar32 c) {
    return (c == (UChar32)'.');
}

bool is_comma(UChar32 c) {
    return (c == (UChar32)',');
}

bool is_eq(UChar32 c) {
    return (c == (UChar32)'=');
}

bool is_lparen(UChar32 c) {
    return (c == (UChar32)'(');
}

bool is_rparen(UChar32 c) {
    return (c == (UChar32)')');
}

bool is_lsquare(UChar32 c) {
    return (c == (UChar32)'[');
}

bool is_rsquare(UChar32 c) {
    return (c == (UChar32)']');
}

bool is_lcurly(UChar32 c) {
    return (c == (UChar32)'{');
}

bool is_rcurly(UChar32 c) {
    return (c == (UChar32)'}');
}

bool is_colon(UChar32 c) {
    return (c == (UChar32)':');
}

bool is_semicolon(UChar32 c) {
    return (c == (UChar32)';');
}

bool is_bar(UChar32 c) {
    return (c == (UChar32)'|');
}

bool is_underscore(UChar32 c) {
    return (c == (UChar32)'_');
}

bool is_digit(UChar32 c) {
    return (bool)u_isdigit(c);
}

bool is_hexdigit(UChar32 c) {
    return (bool)((u_isdigit(c)) || (c == (UChar32)'a') ||
                  (c == (UChar32)'b') || (c == (UChar32)'c') ||
                  (c == (UChar32)'d') || (c == (UChar32)'e') ||
                  (c == (UChar32)'f'));
}

bool is_lowercase(UChar32 c) {
    return (bool)u_isULowercase(c);
}

bool is_uppercase(UChar32 c) {
    return (bool)u_isUUppercase(c);
}

bool is_letter(UChar32 c) {
    return (bool)(is_uppercase(c) || is_lowercase(c) || is_digit(c) ||
                  (c == (UChar32)'_'));
}

bool is_quote(UChar32 c) {
    return (c == (UChar32)'\'');
}

bool is_dquote(UChar32 c) {
    return (c == (UChar32)'"');
}

bool is_backslash(UChar32 c) {
    return (c == (UChar32)'\\');
}

bool is_minus(UChar32 c) {
    return (c == (UChar32)'-');
}

bool is_exponent(UChar32 c) {
    return ((c == (UChar32)'e') || (c == (UChar32)'E'));
}

bool is_escaped(UChar32 c) {
    return ((c == (UChar32)'\\') || (c == (UChar32)'t') ||
            (c == (UChar32)'\'') || (c == (UChar32)'"') || (c == (UChar32)'n'));
}

bool is_math(UChar32 c) {
    return (
        ((c >= 0x2200) && (c <= 0x22FF)) ||  // Mathematical Operators
        ((c >= 0x27C0) &&
         (c <= 0x27EF)) ||  // Miscellaneous Mathematical Symbols-A
        ((c >= 0x2980) &&
         (c <= 0x29FF)) ||  // Miscellaneous Mathematical Symbols-B
        ((c >= 0x2A00) &&
         (c <= 0x2AFF)) ||  // Supplemental Mathematical Operators
        ((c >= 0x2100) && (c <= 0x214F)) ||  // Letterlike Symbols
        ((c >= 0x2308) && (c <= 0x230B)) ||  // Miscellaneous Technical
        ((c >= 0x25A0) && (c <= 0x25FF)) ||  // Geometric Shapes
        ((c >= 0x2B30) && (c <= 0x2B4C)) ||  // Miscellaneous Symbols and Arrows
        ((c >= 0x2190) && (c <= 0x21FF)) ||  // Arrows
        ((c >= 0x1D400) &&
         (c <= 0x1D7FF)))  // Mathematical Alphanumeric Symbols
        ;
}

bool is_operator(UChar32 c) {
    return (bool)((c == (UChar32)'~') || (c == (UChar32)'!') ||
                  (c == (UChar32)'@') || (c == (UChar32)'$') ||
                  (c == (UChar32)'%') || (c == (UChar32)'^') ||
                  (c == (UChar32)'&') || (c == (UChar32)'*') ||
                  (c == (UChar32)'-') || (c == (UChar32)'+') ||
                  (c == (UChar32)'=') || (c == (UChar32)'|') ||
                  (c == (UChar32)'\\') || (c == (UChar32)'/') ||
                  (c == (UChar32)'?') || (c == (UChar32)'<') ||
                  (c == (UChar32)'>') || (c == (UChar32)'.'));
}

struct token_text_t {
    token_t tag;
    const char *text;
};

static constexpr token_text_t token_text_table[]{
    {
        TOKEN_ERROR,
        STRING_ERROR,
    },
    {
        TOKEN_EOF,
        STRING_EOF,
    },

    //  delimiters
    {
        TOKEN_DOT,
        STRING_DOT,
    },
    {
        TOKEN_COMMA,
        STRING_COMMA,
    },
    {
        TOKEN_EQ,
        STRING_EQ,
    },
    {
        TOKEN_ASSIGN,
        STRING_ASSIGN,
    },
    {
        TOKEN_LPAREN,
        STRING_LPAREN,
    },
    {
        TOKEN_RPAREN,
        STRING_RPAREN,
    },
    {
        TOKEN_LSQUARE,
        STRING_LSQUARE,
    },
    {
        TOKEN_RSQUARE,
        STRING_RSQUARE,
    },
    {
        TOKEN_LCURLY,
        STRING_LCURLY,
    },
    {
        TOKEN_RCURLY,
        STRING_RCURLY,
    },
    {
        TOKEN_COLON,
        STRING_COLON,
    },
    {
        TOKEN_DCOLON,
        STRING_DCOLON,
    },
    {
        TOKEN_SEMICOLON,
        STRING_SEMICOLON,
    },
    {
        TOKEN_DSEMICOLON,
        STRING_DSEMICOLON,
    },
    {
        TOKEN_HASH,
        STRING_HASH,
    },
    {
        TOKEN_BAR,
        STRING_BAR,
    },
    {
        TOKEN_QUESTION,
        STRING_QUESTION,
    },

    //  operators, and lowercase and uppercase identifiers
    {
        TOKEN_OPERATOR,
        STRING_OPERATOR,
    },
    {
        TOKEN_LOWERCASE,
        STRING_LOWERCASE,
    },
    {
        TOKEN_UPPERCASE,
        STRING_UPPERCASE,
    },

    //  literals
    {
        TOKEN_INTEGER,
        STRING_INTEGER,
    },
    {
        TOKEN_HEXINTEGER,
        STRING_HEXINTEGER,
    },
    {
        TOKEN_FLOAT,
        STRING_FLOAT,
    },
    {
        TOKEN_CHAR,
        STRING_CHAR,
    },
    {
        TOKEN_TEXT,
        STRING_TEXT,
    },

    //  special symbols
    {
        TOKEN_LAMBDA,
        STRING_LAMBDA,
    },
    {
        TOKEN_CLASS,
        STRING_CLASS,
    },
    {
        TOKEN_EXTENDS,
        STRING_EXTENDS,
    },
    {
        TOKEN_WITH,
        STRING_WITH,
    },

    //  compound expressions
    {
        TOKEN_IF,
        STRING_IF,
    },
    {
        TOKEN_THEN,
        STRING_THEN,
    },
    {
        TOKEN_ELSE,
        STRING_ELSE,
    },

    {
        TOKEN_LET,
        STRING_LET,
    },
    {
        TOKEN_IN,
        STRING_IN,
    },

    {
        TOKEN_TRY,
        STRING_TRY,
    },
    {
        TOKEN_CATCH,
        STRING_CATCH,
    },
    {
        TOKEN_THROW,
        STRING_THROW,
    },
    {
        TOKEN_ARROW,
        STRING_ARROW,
    },
    {
        TOKEN_VAL,
        STRING_VAL,
    },
    {
        TOKEN_DO,
        STRING_DO,
    },

    // directives
    {
        TOKEN_USING,
        STRING_USING,
    },
    {
        TOKEN_IMPORT,
        STRING_IMPORT,
    },

    //  declarations
    {
        TOKEN_DATA,
        STRING_DATA,
    },
    {
        TOKEN_DEF,
        STRING_DEF,
    },
    {
        TOKEN_NAMESPACE,
        STRING_NAMESPACE,
    },
};

icu::UnicodeString token_text(token_t t) {
    for (auto &tt : token_text_table) {
        if (t == tt.tag) {
            return icu::UnicodeString(tt.text);
        }
    }
    // panic_fail("unknown token", __FILE__, __LINE__);
    // PANIC("unknown token");
    //  surpress warnings
    return icu::UnicodeString();
}

struct reserved_t {
    token_t tag;
    const char *text;
};

static reserved_t reserved_table[]{
    {
        TOKEN_BAR,
        STRING_BAR,
    },
    {
        TOKEN_EQ,
        STRING_EQ,
    },
    {
        TOKEN_ASSIGN,
        STRING_ASSIGN,
    },
    {
        TOKEN_QUESTION,
        STRING_QUESTION,
    },
    {
        TOKEN_LAMBDA,
        STRING_LAMBDA,
    },
    {
        TOKEN_CLASS,
        STRING_CLASS,
    },
    {
        TOKEN_EXTENDS,
        STRING_EXTENDS,
    },
    {
        TOKEN_WITH,
        STRING_WITH,
    },
    {
        TOKEN_IF,
        STRING_IF,
    },
    {
        TOKEN_THEN,
        STRING_THEN,
    },
    {
        TOKEN_ELSE,
        STRING_ELSE,
    },
    {
        TOKEN_LET,
        STRING_LET,
    },
    {
        TOKEN_IN,
        STRING_IN,
    },
    {
        TOKEN_TRY,
        STRING_TRY,
    },
    {
        TOKEN_CATCH,
        STRING_CATCH,
    },
    {
        TOKEN_THROW,
        STRING_THROW,
    },
    {
        TOKEN_DO,
        STRING_DO,
    },
    {
        TOKEN_ARROW,
        STRING_ARROW,
    },
    {
        TOKEN_IMPORT,
        STRING_IMPORT,
    },
    {
        TOKEN_USING,
        STRING_USING,
    },
    {
        TOKEN_DATA,
        STRING_DATA,
    },
    {
        TOKEN_DEF,
        STRING_DEF,
    },
    {
        TOKEN_NAMESPACE,
        STRING_NAMESPACE,
    },
    {
        TOKEN_VAL,
        STRING_VAL,
    },
};

Token adjust_reserved(Token &&t) {
    for (auto &tt : reserved_table) {
        if (t.text() == tt.text) {
            t.set_tag(tt.tag);
            return t;
        }
    }
    return t;
}

TokenReaderPtr tokenize_from_reader(CharReader &reader) {
    TokenVector token_writer = TokenVector();

    while (!reader.end() && is_whitespace(reader.look())) reader.skip();

    while (!reader.end()) {
        Position p = reader.position();
        UChar32 c = reader.look();

        if (is_hash(c)) {
            while (!reader.end() && !is_eol(reader.look())) reader.skip();
        } else if (is_comma(c)) {
            token_writer.push(Token(TOKEN_COMMA, p, c));
            reader.skip();
        } else if (is_lparen(c)) {
            token_writer.push(Token(TOKEN_LPAREN, p, c));
            reader.skip();
        } else if (is_rparen(c)) {
            token_writer.push(Token(TOKEN_RPAREN, p, c));
            reader.skip();
        } else if (is_lsquare(c)) {
            token_writer.push(Token(TOKEN_LSQUARE, p, c));
            reader.skip();
        } else if (is_rsquare(c)) {
            token_writer.push(Token(TOKEN_RSQUARE, p, c));
            reader.skip();
        } else if (is_lcurly(c)) {
            token_writer.push(Token(TOKEN_LCURLY, p, c));
            reader.skip();
        } else if (is_rcurly(c)) {
            token_writer.push(Token(TOKEN_RCURLY, p, c));
            reader.skip();
        } else if (is_colon(c)) {
            reader.skip();
            c = reader.look();
            if (is_colon(c)) {
                reader.skip();
                token_writer.push(Token(TOKEN_DCOLON, p, STRING_DCOLON));
            } else {
                token_writer.push(Token(TOKEN_COLON, p, c));
            }
        } else if (is_semicolon(c)) {
            reader.skip();
            c = reader.look();
            if (is_semicolon(c)) {
                reader.skip();
                token_writer.push(
                    Token(TOKEN_DSEMICOLON, p, STRING_DSEMICOLON));
            } else {
                token_writer.push(Token(TOKEN_SEMICOLON, p, c));
            }
        } else if (is_math(c)) {
            token_writer.push(Token(TOKEN_LOWERCASE, p, c));
            reader.skip();
            // compound tokens
        } else if (is_quote(c)) {
            // FIXME: doesn't handle backslashes correct
            icu::UnicodeString str = icu::UnicodeString("");
            str += c;
            reader.skip();
            if (reader.end() || is_eol(reader.look())) goto handle_char_error;
            c = reader.look();
            str += c;
            if (is_backslash(c)) {
                reader.skip();
                if (reader.end() || is_eol(reader.look()))
                    goto handle_char_error;
                c = reader.look();
                if (!is_escaped(c)) goto handle_char_error;
                str += c;
            };
            reader.skip();
            if (reader.end() || is_eol(reader.look())) goto handle_char_error;
            c = reader.look();
            if (!is_quote(c)) goto handle_char_error;
            str += c;
            reader.skip();
            token_writer.push(Token(TOKEN_CHAR, p, str));
        } else if (is_dquote(c)) {
            if (is_dquote(reader.look(1)) && is_dquote(reader.look(2)) &&
                is_eol(reader.look(3))) {
                icu::UnicodeString str = icu::UnicodeString("");
                str += c;
                reader.skip();
                reader.skip();
                reader.skip();
                reader.skip();
                while (!(is_dquote(reader.look(0)) &&
                         is_dquote(reader.look(1)) &&
                         is_dquote(reader.look(2)))) {
                    if (reader.end()) goto handle_string_error;
                    c = reader.look();
                    str += c;
                    reader.skip();
                }
                c = reader.look();
                str += c;
                reader.skip();
                reader.skip();
                reader.skip();
                reader.skip();
                token_writer.push(Token(TOKEN_TEXT, p, str));
            } else {
                icu::UnicodeString str = icu::UnicodeString("");
                str += c;
                reader.skip();
                if (reader.end() || is_eol(reader.look()))
                    goto handle_string_error;
                c = reader.look();
                while (!is_dquote(c)) {
                    if (is_backslash(c)) {
                        str += c;
                        reader.skip();
                        if (reader.end() || is_eol(reader.look()))
                            goto handle_string_error;
                        c = reader.look();
                        if (!is_escaped(c)) goto handle_string_error;
                    };
                    str += c;
                    reader.skip();
                    if (reader.end() || is_eol(reader.look()))
                        goto handle_string_error;
                    c = reader.look();
                };
                str += c;
                reader.skip();
                token_writer.push(Token(TOKEN_TEXT, p, str));
            }
            //        } else if (is_digit(c) || (is_minus(c) &&
            //        is_digit(reader.look(1)))) { // no longer lex a leading
            //        minus
        } else if (is_digit(c) || (is_minus(c) && is_digit(reader.look(1)))) {
            // XXX: LL(2), to be solved by swapping skip/look
            /* This code handles numbers which are integers and floats. Integer
             * and float regular expressions are simplistic and overlap on their
             * prefixes.
             *
             * An integer is in the regexp "[-]?[0-9]+". A float is either
             * "[-]?[0-9]+[.][0-9]+" or expanded with an exponent
             * "[-]?[0-9]+[.][0-9]+[e][-]?[0-9]+". A hexint is "0x[0-9a-f]+".
             */
            icu::UnicodeString str = icu::UnicodeString("");
            // handle leading '-'
            if (is_minus(c)) {
                str += c;
                reader.skip();
                c = reader.look();
            }
            // handle digits
            while (is_digit(c)) {
                str += c;
                reader.skip();
                c = reader.look();
            };
            // '0x' starts hexint
            if ((str == "0") && (c == 'x')) {
                str += c;
                reader.skip();
                if (reader.end()) goto handle_hexint_error;
                c = reader.look();
                if (!is_hexdigit(c)) goto handle_hexint_error;
                while (is_hexdigit(c)) {
                    str += c;
                    reader.skip();
                    c = reader.look();
                };
                token_writer.push(Token(TOKEN_INTEGER, p, str));
                // any '.' occurence signals the start of a forced floating
                // point
            } else if (!is_dot(c)) {
                token_writer.push(Token(TOKEN_INTEGER, p, str));
            } else {
                // handle '.'
                str += c;
                reader.skip();
                if (reader.end()) goto handle_float_error;
                c = reader.look();
                // handle digits
                if (!is_digit(c)) goto handle_float_error;
                while (is_digit(c)) {
                    str += c;
                    reader.skip();
                    c = reader.look();
                };
                // any 'e' occurence signals a forced floating point with an
                // exponent
                if (!is_exponent(c)) {
                    token_writer.push(Token(TOKEN_FLOAT, p, str));
                } else {
                    // handle 'e'
                    str += c;
                    reader.skip();
                    if (reader.end()) goto handle_float_error;
                    c = reader.look();
                    // handle leading '-'
                    if (is_minus(c)) {
                        str += c;
                        reader.skip();
                        if (reader.end()) goto handle_float_error;
                        c = reader.look();
                    }
                    // handle digits
                    if (!is_digit(c)) goto handle_float_error;
                    while (is_digit(c)) {
                        str += c;
                        reader.skip();
                        c = reader.look();
                    };
                    token_writer.push(Token(TOKEN_FLOAT, p, str));
                }
            }
        } else if (is_operator(c)) {
            icu::UnicodeString str = icu::UnicodeString("");
            while (is_operator(c)) {
                str += c;
                reader.skip();
                c = reader.look();
            };
            token_writer.push(adjust_reserved(Token(TOKEN_OPERATOR, p, str)));
        } else if (is_uppercase(c)) {
            icu::UnicodeString str = icu::UnicodeString("");
            while (is_letter(c)) {
                str += c;
                reader.skip();
                c = reader.look();
            };
            token_writer.push(adjust_reserved(Token(TOKEN_UPPERCASE, p, str)));
        } else if (is_lowercase(c)) {
            icu::UnicodeString str = icu::UnicodeString("");
            while (is_letter(c)) {
                str += c;
                reader.skip();
                c = reader.look();
            };
            token_writer.push(adjust_reserved(Token(TOKEN_LOWERCASE, p, str)));
        } else if (is_underscore(
                       c)) {  // XXX: push a lowercase for an underscore?
            icu::UnicodeString str = icu::UnicodeString("");
            str += c;
            reader.skip();
            token_writer.push(Token(TOKEN_LOWERCASE, p, str));
        } else {
            goto handle_error;
        }

        while (!reader.end() && is_whitespace(reader.look())) reader.skip();
    }

    {
        Position p = reader.position();
        token_writer.push(Token(TOKEN_EOF, p, icu::UnicodeString("EOF")));
    }

    return token_writer.clone_reader();

handle_error : {
    Position p = reader.position();
    throw ErrorLexical(
        p, icu::UnicodeString("unrecognized lexeme ") + reader.look());
}

handle_char_error : {
    Position p = reader.position();
    throw ErrorLexical(p, "error in char");
}

handle_string_error : {
    Position p = reader.position();
    throw ErrorLexical(p, "error in string");
}

handle_hexint_error : {
    Position p = reader.position();
    throw ErrorLexical(p, "error in hexadecimal int");
}

handle_float_error : {
    Position p = reader.position();
    throw ErrorLexical(p, "error in float");
}
}

TokenReaderPtr tokenize_from_egg_reader(CharReader &reader) {
    TokenVector token_writer = TokenVector();

    bool text = true;
    while (!reader.end()) {
        if (text) {
            if (reader.look(0) == '`' && reader.look(1) == '`' && reader.look(2) =='`') {
                while (!reader.end() && !is_eol(reader.look())) reader.skip();
                text = !text;
            } else {
                while (!reader.end() && !reader.eol()) reader.skip();
            }
        } else {
            while (!reader.end() && is_whitespace(reader.look())) reader.skip();

            while (!reader.end() && !(reader.look(0) == '`' && reader.look(1) == '`' && reader.look(2) == '`')) {
                Position p = reader.position();
                UChar32 c = reader.look();

                if (is_hash(c)) {
                    while (!reader.end() && !is_eol(reader.look())) reader.skip();
                } else if (is_comma(c)) {
                    token_writer.push(Token(TOKEN_COMMA, p, c));
                    reader.skip();
                } else if (is_lparen(c)) {
                    token_writer.push(Token(TOKEN_LPAREN, p, c));
                    reader.skip();
                } else if (is_rparen(c)) {
                    token_writer.push(Token(TOKEN_RPAREN, p, c));
                    reader.skip();
                } else if (is_lsquare(c)) {
                    token_writer.push(Token(TOKEN_LSQUARE, p, c));
                    reader.skip();
                } else if (is_rsquare(c)) {
                    token_writer.push(Token(TOKEN_RSQUARE, p, c));
                    reader.skip();
                } else if (is_lcurly(c)) {
                    token_writer.push(Token(TOKEN_LCURLY, p, c));
                    reader.skip();
                } else if (is_rcurly(c)) {
                    token_writer.push(Token(TOKEN_RCURLY, p, c));
                    reader.skip();
                } else if (is_colon(c)) {
                    reader.skip();
                    c = reader.look();
                    if (is_colon(c)) {
                        reader.skip();
                        token_writer.push(Token(TOKEN_DCOLON, p, STRING_DCOLON));
                    } else {
                        token_writer.push(Token(TOKEN_COLON, p, c));
                    }
                } else if (is_semicolon(c)) {
                    reader.skip();
                    c = reader.look();
                    if (is_semicolon(c)) {
                        reader.skip();
                        token_writer.push(
                            Token(TOKEN_DSEMICOLON, p, STRING_DSEMICOLON));
                    } else {
                        token_writer.push(Token(TOKEN_SEMICOLON, p, c));
                    }
                } else if (is_math(c)) {
                    token_writer.push(Token(TOKEN_LOWERCASE, p, c));
                    reader.skip();
                    // compound tokens
                } else if (is_quote(c)) {
                    // FIXME: doesn't handle backslashes correct
                    icu::UnicodeString str = icu::UnicodeString("");
                    str += c;
                    reader.skip();
                    if (reader.end() || is_eol(reader.look())) goto handle_char_error;
                    c = reader.look();
                    str += c;
                    if (is_backslash(c)) {
                        reader.skip();
                        if (reader.end() || is_eol(reader.look()))
                            goto handle_char_error;
                        c = reader.look();
                        if (!is_escaped(c)) goto handle_char_error;
                        str += c;
                    };
                    reader.skip();
                    if (reader.end() || is_eol(reader.look())) goto handle_char_error;
                    c = reader.look();
                    if (!is_quote(c)) goto handle_char_error;
                    str += c;
                    reader.skip();
                    token_writer.push(Token(TOKEN_CHAR, p, str));
                } else if (is_dquote(c)) {
                    if (is_dquote(reader.look(1)) && is_dquote(reader.look(2)) &&
                        is_eol(reader.look(3))) {
                        icu::UnicodeString str = icu::UnicodeString("");
                        str += c;
                        reader.skip();
                        reader.skip();
                        reader.skip();
                        reader.skip();
                        while (!(is_dquote(reader.look(0)) &&
                                 is_dquote(reader.look(1)) &&
                                 is_dquote(reader.look(2)))) {
                            if (reader.end()) goto handle_string_error;
                            c = reader.look();
                            str += c;
                            reader.skip();
                        }
                        c = reader.look();
                        str += c;
                        reader.skip();
                        reader.skip();
                        reader.skip();
                        reader.skip();
                        token_writer.push(Token(TOKEN_TEXT, p, str));
                    } else {
                        icu::UnicodeString str = icu::UnicodeString("");
                        str += c;
                        reader.skip();
                        if (reader.end() || is_eol(reader.look()))
                            goto handle_string_error;
                        c = reader.look();
                        while (!is_dquote(c)) {
                            if (is_backslash(c)) {
                                str += c;
                                reader.skip();
                                if (reader.end() || is_eol(reader.look()))
                                    goto handle_string_error;
                                c = reader.look();
                                if (!is_escaped(c)) goto handle_string_error;
                            };
                            str += c;
                            reader.skip();
                            if (reader.end() || is_eol(reader.look()))
                                goto handle_string_error;
                            c = reader.look();
                        };
                        str += c;
                        reader.skip();
                        token_writer.push(Token(TOKEN_TEXT, p, str));
                    }
                    //        } else if (is_digit(c) || (is_minus(c) &&
                    //        is_digit(reader.look(1)))) { // no longer lex a leading
                    //        minus
                } else if (is_digit(c) || (is_minus(c) && is_digit(reader.look(1)))) {
                    // XXX: LL(2), to be solved by swapping skip/look
                    /* This code handles numbers which are integers and floats. Integer
                     * and float regular expressions are simplistic and overlap on their
                     * prefixes.
                     *
                     * An integer is in the regexp "[-]?[0-9]+". A float is either
                     * "[-]?[0-9]+[.][0-9]+" or expanded with an exponent
                     * "[-]?[0-9]+[.][0-9]+[e][-]?[0-9]+". A hexint is "0x[0-9a-f]+".
                     */
                    icu::UnicodeString str = icu::UnicodeString("");
                    // handle leading '-'
                    if (is_minus(c)) {
                        str += c;
                        reader.skip();
                        c = reader.look();
                    }
                    // handle digits
                    while (is_digit(c)) {
                        str += c;
                        reader.skip();
                        c = reader.look();
                    };
                    // '0x' starts hexint
                    if ((str == "0") && (c == 'x')) {
                        str += c;
                        reader.skip();
                        if (reader.end()) goto handle_hexint_error;
                        c = reader.look();
                        if (!is_hexdigit(c)) goto handle_hexint_error;
                        while (is_hexdigit(c)) {
                            str += c;
                            reader.skip();
                            c = reader.look();
                        };
                        token_writer.push(Token(TOKEN_INTEGER, p, str));
                        // any '.' occurence signals the start of a forced floating
                        // point
                    } else if (!is_dot(c)) {
                        token_writer.push(Token(TOKEN_INTEGER, p, str));
                    } else {
                        // handle '.'
                        str += c;
                        reader.skip();
                        if (reader.end()) goto handle_float_error;
                        c = reader.look();
                        // handle digits
                        if (!is_digit(c)) goto handle_float_error;
                        while (is_digit(c)) {
                            str += c;
                            reader.skip();
                            c = reader.look();
                        };
                        // any 'e' occurence signals a forced floating point with an
                        // exponent
                        if (!is_exponent(c)) {
                            token_writer.push(Token(TOKEN_FLOAT, p, str));
                        } else {
                            // handle 'e'
                            str += c;
                            reader.skip();
                            if (reader.end()) goto handle_float_error;
                            c = reader.look();
                            // handle leading '-'
                            if (is_minus(c)) {
                                str += c;
                                reader.skip();
                                if (reader.end()) goto handle_float_error;
                                c = reader.look();
                            }
                            // handle digits
                            if (!is_digit(c)) goto handle_float_error;
                            while (is_digit(c)) {
                                str += c;
                                reader.skip();
                                c = reader.look();
                            };
                            token_writer.push(Token(TOKEN_FLOAT, p, str));
                        }
                    }
                } else if (is_operator(c)) {
                    icu::UnicodeString str = icu::UnicodeString("");
                    while (is_operator(c)) {
                        str += c;
                        reader.skip();
                        c = reader.look();
                    };
                    token_writer.push(adjust_reserved(Token(TOKEN_OPERATOR, p, str)));
                } else if (is_uppercase(c)) {
                    icu::UnicodeString str = icu::UnicodeString("");
                    while (is_letter(c)) {
                        str += c;
                        reader.skip();
                        c = reader.look();
                    };
                    token_writer.push(adjust_reserved(Token(TOKEN_UPPERCASE, p, str)));
                } else if (is_lowercase(c)) {
                    icu::UnicodeString str = icu::UnicodeString("");
                    while (is_letter(c)) {
                        str += c;
                        reader.skip();
                        c = reader.look();
                    };
                    token_writer.push(adjust_reserved(Token(TOKEN_LOWERCASE, p, str)));
                } else if (is_underscore(
                               c)) {  // XXX: push a lowercase for an underscore?
                    icu::UnicodeString str = icu::UnicodeString("");
                    str += c;
                    reader.skip();
                    token_writer.push(Token(TOKEN_LOWERCASE, p, str));
                } else {
                    goto handle_error;
                }

                while (!reader.end() && is_whitespace(reader.look())) reader.skip();

            }

            if (reader.look(0) == '`' && reader.look(1) == '`' && reader.look(2) =='`') {
                while (!reader.end() && !is_eol(reader.look())) reader.skip();
                text = !text;
            }
        }
    }

    {
        Position p = reader.position();
        token_writer.push(Token(TOKEN_EOF, p, icu::UnicodeString("EOF")));
    }

    return token_writer.clone_reader();

handle_error : {
    Position p = reader.position();
    throw ErrorLexical(
        p, icu::UnicodeString("unrecognized lexeme ") + reader.look());
}

handle_char_error : {
    Position p = reader.position();
    throw ErrorLexical(p, "error in char");
}

handle_string_error : {
    Position p = reader.position();
    throw ErrorLexical(p, "error in string");
}

handle_hexint_error : {
    Position p = reader.position();
    throw ErrorLexical(p, "error in hexadecimal int");
}

handle_float_error : {
    Position p = reader.position();
    throw ErrorLexical(p, "error in float");
}
}
}  // namespace egel
