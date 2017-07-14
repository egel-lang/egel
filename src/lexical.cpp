#include "lexical.hpp"
#include "constants.hpp"

// character classes

bool is_whitespace(UChar32 c) {
    return (bool) u_isUWhiteSpace(c);
}

bool is_eol(UChar32 c) {
    return (c == (UChar32) '\n');
}

bool is_hash(UChar32 c) {
    return (c == (UChar32) '#');
}

bool is_dot(UChar32 c) {
    return (c == (UChar32) '.');
}

bool is_comma(UChar32 c) {
    return (c == (UChar32) ',');
}

bool is_eq(UChar32 c) {
    return (c == (UChar32) '=');
}

bool is_lparen(UChar32 c) {
    return (c == (UChar32) '(');
}

bool is_rparen(UChar32 c) {
    return (c == (UChar32) ')');
}

bool is_lsquare(UChar32 c) {
    return (c == (UChar32) '[');
}

bool is_rsquare(UChar32 c) {
    return (c == (UChar32) ']');
}

bool is_lcurly(UChar32 c) {
    return (c == (UChar32) '{');
}

bool is_rcurly(UChar32 c) {
    return (c == (UChar32) '}');
}

bool is_colon(UChar32 c) {
    return (c == (UChar32) ':');
}

bool is_semicolon(UChar32 c) {
    return (c == (UChar32) ';');
}

bool is_bar(UChar32 c) {
    return (c == (UChar32) '|');
}

bool is_underscore(UChar32 c) {
    return (c == (UChar32) '_');
}

bool is_digit(UChar32 c) {
    return (bool) u_isdigit(c);
}

bool is_lowercase(UChar32 c) {
    return (bool) u_isULowercase(c);
}

bool is_uppercase(UChar32 c) {
    return (bool) u_isUUppercase(c);
}

bool is_letter(UChar32 c) {
    return (bool) (is_uppercase(c) || is_lowercase(c) || is_digit(c) || (c ==  (UChar32) '_'));
}

bool is_quote(UChar32 c) {
    return (c == (UChar32) '\'');
}

bool is_dquote(UChar32 c) {
    return (c == (UChar32) '"');
}

bool is_backslash(UChar32 c) {
    return (c == (UChar32) '\\');
}

bool is_minus(UChar32 c) {
    return (c == (UChar32) '-');
}

bool is_exponent(UChar32 c) {
    return ((c == (UChar32) 'e') ||
            (c == (UChar32) 'E'));
}

bool is_escaped(UChar32 c) {
    return ((c == (UChar32) '\\') ||
            (c == (UChar32) 't') ||
            (c == (UChar32) '\'') ||
            (c == (UChar32) '"') ||
            (c == (UChar32) 'n'));
}

bool is_math(UChar32 c) {
    return (
        ( (c >= 0x2200) && (c <= 0x22FF) ) || //Mathematical Operators 
        ( (c >= 0x27C0) && (c <= 0x27EF) ) || //Miscellaneous Mathematical Symbols-A 
        ( (c >= 0x2980) && (c <= 0x29FF) ) || //Miscellaneous Mathematical Symbols-B 
        ( (c >= 0x2A00) && (c <= 0x2AFF) ) || //Supplemental Mathematical Operators 
        ( (c >= 0x2100) && (c <= 0x214F) ) || //Letterlike Symbols 
        ( (c >= 0x2308) && (c <= 0x230B) ) || //Miscellaneous Technical 
        ( (c >= 0x25A0) && (c <= 0x25FF) ) || //Geometric Shapes 
        ( (c >= 0x2B30) && (c <= 0x2B4C) ) || //Miscellaneous Symbols and Arrows 
        ( (c >= 0x2190) && (c <= 0x21FF) ) || //Arrows 
        ( (c >= 0x1D400) && (c <= 0x1D7FF) )) //Mathematical Alphanumeric Symbols 
        ;
}

bool is_operator(UChar32 c) {
    return (bool)
      ( (c == (UChar32) '~')    ||
        (c == (UChar32) '!')    ||
        (c == (UChar32) '@')    ||
        (c == (UChar32) '$')    ||
        (c == (UChar32) '%')    ||
        (c == (UChar32) '^')    ||
        (c == (UChar32) '&')    ||
        (c == (UChar32) '*')    ||
        (c == (UChar32) '-')    ||
        (c == (UChar32) '+')    ||
        (c == (UChar32) '=')    ||
        (c == (UChar32) '|')    ||
        (c == (UChar32) '\\')   ||
        (c == (UChar32) '/')    ||
        (c == (UChar32) '?')    ||
        (c == (UChar32) '<')    ||
        (c == (UChar32) '>')  );
}

typedef struct {
    token_t     tag;
    const char* text;
} token_text_t;

static token_text_t token_text_table[] {
    TOKEN_ERROR, STRING_ERROR,
    TOKEN_EOF, STRING_EOF,

//  delimiters
    TOKEN_DOT, STRING_DOT,
    TOKEN_COMMA, STRING_COMMA,
    TOKEN_EQ, STRING_EQ,
    TOKEN_ASSIGN, STRING_ASSIGN,
    TOKEN_LPAREN, STRING_LPAREN,
    TOKEN_RPAREN, STRING_RPAREN,
    TOKEN_LSQUARE, STRING_LSQUARE,
    TOKEN_RSQUARE, STRING_RSQUARE,
    TOKEN_LCURLY, STRING_LCURLY,
    TOKEN_RCURLY, STRING_RCURLY,
    TOKEN_COLON, STRING_COLON,
    TOKEN_SEMICOLON, STRING_SEMICOLON,
    TOKEN_HASH, STRING_HASH,
    TOKEN_BAR, STRING_BAR,
    TOKEN_QUESTION, STRING_QUESTION,

//  operators, and lowercase and uppercase identifiers
    TOKEN_OPERATOR, STRING_OPERATOR,
    TOKEN_LOWERCASE, STRING_LOWERCASE,
    TOKEN_UPPERCASE, STRING_UPPERCASE,

//  literals
    TOKEN_INTEGER, STRING_INTEGER,
    TOKEN_HEXINTEGER, STRING_HEXINTEGER,
    TOKEN_FLOAT, STRING_FLOAT,
    TOKEN_CHAR, STRING_CHAR,
    TOKEN_TEXT, STRING_TEXT,

//  special symbols
    TOKEN_LAMBDA, STRING_LAMBDA,

//  compound expressions
    TOKEN_IF, STRING_IF,
    TOKEN_THEN, STRING_THEN,
    TOKEN_ELSE, STRING_ELSE,

    TOKEN_TRY, STRING_TRY,
    TOKEN_CATCH, STRING_CATCH,
    TOKEN_THROW, STRING_THROW,

    TOKEN_ARROW, STRING_ARROW,

// directives
    TOKEN_USING, STRING_USING,
    TOKEN_IMPORT, STRING_IMPORT,

//  declarations
    TOKEN_DATA, STRING_DATA,
    TOKEN_DEF, STRING_DEF,
    TOKEN_NAMESPACE, STRING_NAMESPACE,
};

#define TOKEN_TEXT_TABLE_SIZE sizeof(token_text_table)/sizeof(token_text_t)

UnicodeString token_text(token_t t) {
    for (uint_t i = 0; i < TOKEN_TEXT_TABLE_SIZE; i++) {
        if (t == token_text_table[i].tag) {
            return UnicodeString(token_text_table[i].text);
        }
    }

    PANIC("Uknkown token.");
    // surpress warnings
    return UnicodeString();
}

typedef struct {
    token_t     tag;
    const char* text;
} reserved_t;

static reserved_t reserved_table[] {
    TOKEN_BAR, STRING_BAR,
    TOKEN_EQ, STRING_EQ,
    TOKEN_ASSIGN, STRING_ASSIGN,
    TOKEN_QUESTION, STRING_QUESTION,
    TOKEN_LAMBDA, STRING_LAMBDA,
    TOKEN_IF, STRING_IF,
    TOKEN_THEN, STRING_THEN,
    TOKEN_ELSE, STRING_ELSE,
    TOKEN_TRY, STRING_TRY,
    TOKEN_CATCH, STRING_CATCH,
    TOKEN_THROW, STRING_THROW,
    TOKEN_ARROW, STRING_ARROW,
    TOKEN_IMPORT, STRING_IMPORT,
    TOKEN_USING, STRING_USING,
    TOKEN_DATA, STRING_DATA,
    TOKEN_DEF,  STRING_DEF,
    TOKEN_NAMESPACE, STRING_NAMESPACE,
};

#define RESERVED_TABLE_SIZE sizeof(reserved_table)/sizeof(reserved_t)

Token adjust_reserved(Token&& t) {
    for (uint_t i = 0; i < RESERVED_TABLE_SIZE; i++) {
        if (t.text() == reserved_table[i].text) {
            t.set_tag(reserved_table[i].tag);
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
            token_writer.push(Token(TOKEN_HASH, p, c));
            reader.skip();
        } else if (is_dot(c)) {
            token_writer.push(Token(TOKEN_DOT, p, c));
            reader.skip();
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
            token_writer.push(Token(TOKEN_COLON, p, c));
            reader.skip();
        } else if (is_semicolon(c)) {
            token_writer.push(Token(TOKEN_SEMICOLON, p, c));
            reader.skip();
        } else if (is_math(c)) {
            token_writer.push(Token(TOKEN_LOWERCASE, p, c));
            reader.skip();
        // compound tokens
        } else if (is_quote(c)) {
            // FIXME: doesn't handle backslashes correct
            UnicodeString str = UnicodeString("");
            str += c;
            reader.skip();
            if (reader.end() || is_eol(reader.look())) goto handle_char_error;
            c = reader.look();
            str += c;
            if (is_backslash(c)) {
                reader.skip();
                if (reader.end() || is_eol(reader.look())) goto handle_char_error;
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
            UnicodeString str = UnicodeString("");
            str += c;
            reader.skip();
            if (reader.end() || is_eol(reader.look())) goto handle_string_error;
            c = reader.look();
            while (!is_dquote(c)) {
                if (is_backslash(c)) {
                    str += c;
                    reader.skip();
                    if (reader.end() || is_eol(reader.look())) goto handle_string_error;
                    c = reader.look();
                    if (!is_escaped(c)) goto handle_char_error;
                };
                str += c;
                reader.skip();
                if (reader.end() || is_eol(reader.look())) goto handle_string_error;
                c = reader.look();
            };
            str += c;
            reader.skip();
            token_writer.push(Token(TOKEN_TEXT, p, str));
        } else if (is_digit(c) || (is_minus(c) && is_digit(reader.look()))) {
            /* This code handles numbers which are integers and floats. Integer and float
             * regular expressions are simplistic and overlap on their prefixes.
             *
             * An integer is in the regexp "[-]?[0-9]+". A float is either "[-]?[0-9]+[.][0-9]+"
             * or expanded with an exponent "[-]?[0-9]+[.][0-9]+[e][-]?[0-9]+".
             */
            UnicodeString str = UnicodeString("");
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
            // any '.' occurence signals the start of a forced floating point
            if (!is_dot(c)) {
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
                // any 'e' occurence signals a forced floating point with an exponent
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
            UnicodeString str = UnicodeString("");
            while (is_operator(c)) {
                str += c;
                if (str == "//") {
                    while (!reader.end() && !is_eol(reader.look())) reader.skip();
                    goto ignore;
                }
                if (str == "/*") {
                    reader.skip();
                    do {
                        c = reader.look();
                        reader.skip();
                    } while (!reader.end() && !((c == '*') && (reader.look() == '/')));
                    if ((c == '*') && (reader.look() == '/')) reader.skip();
                    goto ignore;
                }
                reader.skip();
                c = reader.look();
            };
            token_writer.push(adjust_reserved(Token(TOKEN_OPERATOR, p, str)));
        } else if (is_uppercase(c)) {
            UnicodeString str = UnicodeString("");
            while (is_letter(c)) {
                str += c;
                reader.skip();
                c = reader.look();
            };
            token_writer.push(adjust_reserved(Token(TOKEN_UPPERCASE, p, str)));
        } else if (is_lowercase(c)) {
            UnicodeString str = UnicodeString("");
            while (is_letter(c)) {
                str += c;
                reader.skip();
                c = reader.look();
            };
            token_writer.push(adjust_reserved(Token(TOKEN_LOWERCASE, p, str)));
        } else if (is_underscore(c)) { // XXX: push a lowercase for an underscore?
            UnicodeString str = UnicodeString("");
            str += c;
            reader.skip();
            token_writer.push(Token(TOKEN_LOWERCASE, p, str));
        } else {
            goto handle_error;
        }

    ignore:

        while (!reader.end() && is_whitespace(reader.look())) reader.skip();
    }

    {
        Position p = reader.position();
        token_writer.push(Token(TOKEN_EOF, p, UnicodeString("EOF")));
    }

    return token_writer.clone_reader();

    handle_error: {
        Position p = reader.position();
        throw ErrorLexical(p, "unrecognized lexeme " + reader.look());
    }

    handle_char_error: {
        Position p = reader.position();
        throw ErrorLexical(p, "error in char");
    }

    handle_string_error: {
        Position p = reader.position();
        throw ErrorLexical(p, "error in string");
    }

    handle_float_error: {
        Position p = reader.position();
        throw ErrorLexical(p, "error in float");
    }
}
