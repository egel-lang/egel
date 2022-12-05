#pragma once

#include "ast.hpp"
#include "error.hpp"
#include "lexical.hpp"
#include "operators.hpp"
#include "syntactical.hpp"
#include "transform.hpp"

class Parser {
public:
    Parser(TokenReaderPtr &r) {
        _tokenreader = r;
    }

    Token look(int n = 0) {
        return _tokenreader->look(n);
    }

    void skip() {
        // std::cout << "skipped: " << look() << std::endl;
        _tokenreader->skip();
    }

    Position position() {
        return _tokenreader->look().position();
    }

    token_t tag(int n = 0) {
        return _tokenreader->look(n).tag();
    }

    void check_token(token_t t) {
        if (tag() != t) {
            Position p = position();
            throw ErrorSyntactical(p, token_text(t) + " expected");
        };
    }

    void force_token(token_t t) {
        check_token(t);
        skip();
    }

    // literals
    AstPtr parse_integer() {
        check_token(TOKEN_INTEGER);
        AstPtr a = AstExprInteger::create(position(), look().text());
        skip();
        return a;
    }

    AstPtr parse_hexinteger() {
        check_token(TOKEN_HEXINTEGER);
        AstPtr a = AstExprHexInteger::create(position(), look().text());
        skip();
        return a;
    }

    AstPtr parse_float() {
        check_token(TOKEN_FLOAT);
        AstPtr a = AstExprFloat::create(position(), look().text());
        skip();
        return a;
    }

    AstPtr parse_character() {
        check_token(TOKEN_CHAR);
        AstPtr a = AstExprCharacter::create(position(), look().text());
        skip();
        return a;
    }

    AstPtr parse_text() {
        check_token(TOKEN_TEXT);
        AstPtr a = AstExprText::create(position(), look().text());
        skip();
        return a;
    }

    // parse (qualified) names or operators

    bool is_variable() {
        return look().tag() == TOKEN_UPPERCASE;
    }

    bool is_wildcard() {
        return look().text().compare("_") == 0;
    }

    bool is_combinator() {
        int i = 0;
        while ((look(i).tag() == TOKEN_UPPERCASE) &&
               (look(i + 1).tag() == TOKEN_DCOLON)) {
            i += 2;
        }
        return look(i).tag() == TOKEN_LOWERCASE;
    }

    bool is_operator() {
        int i = 0;
        while ((look(i).tag() == TOKEN_UPPERCASE) &&
               (look(i + 1).tag() == TOKEN_DCOLON)) {
            i += 2;
        }
        return look(i).tag() == TOKEN_OPERATOR;
    }

    bool is_namespace() {
        return look().tag() == TOKEN_UPPERCASE;
    }

    icu::UnicodeString peek_operator() {
        // (uppercase '.')* operator
        int i = 0;
        while ((look(i).tag() == TOKEN_UPPERCASE) &&
               (look(i + 1).tag() == TOKEN_DCOLON)) {
            i += 2;
        }
        if (look(i).tag() == TOKEN_OPERATOR) {
            return look(i).text();
        } else {
            return "";
        }
    }

    bool is_enclosed_operator() {
        // '(' (uppercase '::')* operator ')'
        int i = 0;
        if (tag(i) != TOKEN_LPAREN) return false;
        i++;
        while ((look(i).tag() == TOKEN_UPPERCASE) &&
               (look(i + 1).tag() == TOKEN_DCOLON)) {
            i += 2;
        }
        if (tag(i) != TOKEN_OPERATOR) return false;
        i++;
        if (tag(i) != TOKEN_RPAREN) return false;
        return true;
    }

    AstPtr parse_variable() {
        check_token(TOKEN_UPPERCASE);
        Position p = position();
        icu::UnicodeString s = look().text();
        skip();
        return AstExprVariable::create(p, s);
    }

    AstPtr parse_wildcard() {
        Position p = position();
        icu::UnicodeString s = look().text();
        if (s != "_") throw ErrorSyntactical(p, "wildcard expected");
        skip();
        return AstExprWildcard::create(p, s);
    }

    AstPtr parse_combinator() {
        Position p = position();
        UnicodeStrings nn;
        while ((tag(0) == TOKEN_UPPERCASE) && (tag(1) == TOKEN_DCOLON)) {
            icu::UnicodeString n = look().text();
            skip();
            skip();
            nn.push_back(n);
        };
        check_token(TOKEN_LOWERCASE);
        icu::UnicodeString n = look().text();
        skip();
        return AstExprCombinator::create(p, nn, n);
    }

    AstPtr parse_operator() {
        Position p = position();
        UnicodeStrings nn;
        while ((tag(0) == TOKEN_UPPERCASE) && (tag(1) == TOKEN_DCOLON)) {
            icu::UnicodeString n = look().text();
            skip();
            skip();
            nn.push_back(n);
        };
        check_token(TOKEN_OPERATOR);
        icu::UnicodeString n = look().text();
        skip();
        return AstExprCombinator::create(p, nn, n);
    }

    AstPtr parse_prefix_operator() {
        Position p = position();
        UnicodeStrings nn;
        while ((tag(0) == TOKEN_UPPERCASE) && (tag(1) == TOKEN_DCOLON)) {
            icu::UnicodeString n = look().text();
            skip();
            skip();
            nn.push_back(n);
        };
        check_token(TOKEN_OPERATOR);
        icu::UnicodeString n = '!' + look().text();
        skip();
        return AstExprCombinator::create(p, nn, n);
    }

    AstPtr parse_enclosed_operator() {
        force_token(TOKEN_LPAREN);
        AstPtr o = parse_operator();
        force_token(TOKEN_RPAREN);
        return o;
    }

    UnicodeStrings parse_namespace() {
        UnicodeStrings ss;
        check_token(TOKEN_UPPERCASE);
        icu::UnicodeString s = look().text();
        ss.push_back(s);
        skip();
        while ((tag(0) == TOKEN_DCOLON) && (tag(1) == TOKEN_UPPERCASE)) {
            skip();
            icu::UnicodeString s = look().text();
            skip();
            ss.push_back(s);
        };
        return ss;
    }

    // patterns
    bool is_pattern_primary() {
        switch (tag()) {
            case TOKEN_INTEGER:
            case TOKEN_HEXINTEGER:
            case TOKEN_FLOAT:
            case TOKEN_CHAR:
            case TOKEN_TEXT:
            case TOKEN_LPAREN:
            case TOKEN_LCURLY:
                return true;
                break;
            default:
                return (is_wildcard() || is_variable() || is_combinator());
                break;
        }
    }

    AstPtr parse_pattern_primary() {
        switch (tag()) {
            case TOKEN_INTEGER:
                return parse_integer();
                break;
            case TOKEN_HEXINTEGER:
                return parse_hexinteger();
                break;
            case TOKEN_FLOAT:
                return parse_float();
                break;
            case TOKEN_CHAR:
                return parse_character();
                break;
            case TOKEN_TEXT:
                return parse_text();
                break;
            case TOKEN_LPAREN:
                return parse_pattern_enclosed();
                break;
            case TOKEN_LCURLY:
                return parse_pattern_list();
                break;
            default:
                AstPtr e;
                if (is_wildcard()) {
                    e = parse_wildcard();
                } else if (is_combinator()) {
                    e = parse_combinator();
                } else if (is_variable()) {
                    e = parse_variable();
                } else {
                    throw ErrorSyntactical(position(), "pattern expected");
                }
                if (tag() == TOKEN_COLON) {
                    Position p = position();
                    skip();
                    auto c = parse_combinator();
                    return AstExprTag::create(p, e, c);
                } else {
                    return e;
                }
                break;
        }
        throw ErrorSyntactical(position(), "pattern expected");
    }

    AstPtr parse_pattern_primaries() {
        Position p = position();
        if (is_pattern_primary()) {
            AstPtr q = parse_pattern_primary();
            if (is_pattern_primary()) {
                AstPtrs qq = AstPtrs();
                qq.push_back(q);
                while (is_pattern_primary()) {
                    AstPtr q = parse_pattern_primary();
                    qq.push_back(q);
                }
                return AstExprApplication::create(p, qq);
            } else {
                return q;
            }
        } else {
            throw ErrorSyntactical(position(), "pattern expected");
        }
    }

    AstPtr parse_pattern_enclosed() {
        if (is_enclosed_operator()) return parse_enclosed_operator();
        Position p = position();
        force_token(TOKEN_LPAREN);
        AstPtr q = parse_pattern_primaries();
        if (tag() == TOKEN_COMMA) {
            AstPtrs qq = AstPtrs();
            qq.push_back(q);
            while (tag() == TOKEN_COMMA) {
                skip();
                AstPtr q = parse_pattern_primaries();
                qq.push_back(q);
            }
            force_token(TOKEN_RPAREN);
            return AstExprTuple::create(p, qq);
        } else {
            force_token(TOKEN_RPAREN);
            return q;
        }
    }

    AstPtr parse_pattern_list() {
        Position p = position();
        force_token(TOKEN_LCURLY);
        AstPtrs qq = AstPtrs();
        if (tag() == TOKEN_RCURLY) {
            skip();
            return AstExprList::create(p, qq);
        } else {
            AstPtr q = parse_pattern();
            qq.push_back(q);
            while (tag() == TOKEN_COMMA) {
                skip();
                AstPtr q = parse_pattern();
                qq.push_back(q);
            }
            q = nullptr;
            if (tag() == TOKEN_BAR) {
                skip();
                q = parse_pattern();
            }
            force_token(TOKEN_RCURLY);
            return AstExprList::create(p, qq, q);
        }
    }

    AstPtr parse_pattern() {
        Position p = position();
        if (is_pattern_primary()) {
            return parse_pattern_primary();
        } else {
            throw ErrorSyntactical(p, "pattern expected");
        }
    }

    AstPtrs parse_patterns() {
        AstPtrs pp = AstPtrs();
        while (is_pattern_primary()) {
            AstPtr p = parse_pattern();
            pp.push_back(p);
        }
        return pp;
    }

    // expressions
    AstPtr parse_match() {
        Position p = position();
        AstPtrs mm = parse_patterns();
        /*
        if (tag() == TOKEN_QUESTION) {
            skip();
            AstPtr q = parse_expression();
            force_token(TOKEN_ARROW);
            AstPtr e = parse_expression();
            return AstExprMatch::create(p, mm, q, e);
        } else {
            AstPtr q = AstEmpty::create();
            force_token(TOKEN_ARROW);
            AstPtr e = parse_expression();
            return AstExprMatch::create(p, mm, q, e);
        }
        */
        AstPtr q = AstEmpty::create();
        force_token(TOKEN_ARROW);
        AstPtr e = parse_expression();
        return AstExprMatch::create(p, mm, q, e);
    }

    AstPtr parse_block() {
        Position p = position();
        AstPtrs mm = AstPtrs();
        check_token(TOKEN_LSQUARE);
        do {
            skip();
            AstPtr m = parse_match();
            mm.push_back(m);
        } while (tag() == TOKEN_BAR);
        force_token(TOKEN_RSQUARE);
        return AstExprBlock::create(p, mm);
    }

    AstPtr parse_lambda() {
        Position p = position();
        AstPtrs mm = AstPtrs();
        force_token(TOKEN_LAMBDA);
        AstPtr m = parse_match();
        return AstExprLambda::create(p, m);
    }

    AstPtr parse_enclosed() {
        if (is_enclosed_operator()) return parse_enclosed_operator();
        Position p = position();
        force_token(TOKEN_LPAREN);
        AstPtr q = parse_expression();
        if (tag() == TOKEN_COMMA) {
            AstPtrs qq = AstPtrs();
            qq.push_back(q);
            while (tag() == TOKEN_COMMA) {
                skip();
                AstPtr q = parse_expression();
                qq.push_back(q);
            }
            force_token(TOKEN_RPAREN);
            return AstExprTuple::create(p, qq);
        } else {
            force_token(TOKEN_RPAREN);
            return q;
        }
    }

    AstPtr parse_list() {
        Position p = position();
        force_token(TOKEN_LCURLY);
        AstPtrs qq = AstPtrs();
        if (tag() == TOKEN_RCURLY) {
            skip();
            return AstExprList::create(p, qq);
        } else {
            AstPtr q = parse_expression();
            qq.push_back(q);
            while (tag() == TOKEN_COMMA) {
                skip();
                AstPtr q = parse_expression();
                qq.push_back(q);
            }
            q = nullptr;
            if (tag() == TOKEN_BAR) {
                skip();
                q = parse_expression();
            }
            force_token(TOKEN_RCURLY);
            return AstExprList::create(p, qq, q);
        }
    }

    AstPtr parse_if() {
        Position p = position();
        force_token(TOKEN_IF);
        AstPtr e0 = parse_expression();
        force_token(TOKEN_THEN);
        AstPtr e1 = parse_expression();
        force_token(TOKEN_ELSE);
        AstPtr e2 = parse_expression();
        return AstExprIf::create(p, e0, e1, e2);
    }

    AstPtr parse_try() {
        Position p = position();
        force_token(TOKEN_TRY);
        AstPtr e0 = parse_expression();
        force_token(TOKEN_CATCH);
        AstPtr e1 = parse_expression();
        return AstExprTry::create(p, e0, e1);
    }

    AstPtr parse_throw() {
        Position p = position();
        force_token(TOKEN_THROW);
        AstPtr e0 = parse_expression();
        return AstExprThrow::create(p, e0);
    }

    AstPtr parse_let() {
        Position p = position();
        force_token(TOKEN_LET);
        AstPtrs ee = parse_patterns();
        force_token(TOKEN_EQ);
        AstPtr e1 = parse_expression();
        force_token(TOKEN_IN);
        AstPtr e2 = parse_expression();
        return AstExprLet::create(p, ee, e1, e2);
    }

    AstPtr parse_do() {
        Position p = position();
        force_token(TOKEN_DO);
        AstPtr e0 = parse_expression();
        return AstExprDo::create(p, e0);
    }

    bool is_primary() {
        switch (tag()) {
            case TOKEN_INTEGER:
            case TOKEN_HEXINTEGER:
            case TOKEN_FLOAT:
            case TOKEN_CHAR:
            case TOKEN_TEXT:
            case TOKEN_LPAREN:
            case TOKEN_IF:
            case TOKEN_TRY:
            case TOKEN_THROW:
            case TOKEN_LSQUARE:
            case TOKEN_LCURLY:
                return true;
                break;
            default:
                if (is_operator())
                    return false;  // qualified operator and variable overlap
                return (is_combinator() || is_variable());
                break;
        }
    }

    AstPtr parse_primary() {
        switch (tag()) {
            case TOKEN_INTEGER:
                return parse_integer();
                break;
            case TOKEN_HEXINTEGER:
                return parse_hexinteger();
                break;
            case TOKEN_FLOAT:
                return parse_float();
                break;
            case TOKEN_CHAR:
                return parse_character();
                break;
            case TOKEN_TEXT:
                return parse_text();
                break;
            case TOKEN_LPAREN:
                return parse_enclosed();
                break;
            case TOKEN_LCURLY:
                return parse_list();
                break;
            case TOKEN_LSQUARE:
                return parse_block();
                break;
            case TOKEN_LAMBDA:
                return parse_lambda();
                break;
            case TOKEN_IF:
                return parse_if();
                break;
            case TOKEN_TRY:
                return parse_try();
                break;
            case TOKEN_THROW:
                return parse_throw();
                break;
            case TOKEN_LET:
                return parse_let();
            case TOKEN_DO:
                return parse_do();
            default:
                if (is_combinator()) {
                    return parse_combinator();
                } else if (is_variable()) {
                    return parse_variable();
                } else {
                    Position p = position();
                    throw ErrorSyntactical(p, "primary expression expected");
                }
                break;
        }
    }

    AstPtr parse_primary_prefix() {
        icu::UnicodeString s = peek_operator();
        if ((s != "") && operator_is_prefix(s)) {
            AstPtr o = parse_prefix_operator();
            AstPtr e = parse_primary_prefix();
            return app(o, e);
        } else {
            return parse_primary();
        }
    }

    AstPtr parse_primaries() {
        Position p = position();
        AstPtr e = parse_primary_prefix();
        if (is_primary()) {
            AstPtrs ee;
            ee.push_back(e);
            while (is_primary()) {
                AstPtr e = parse_primary_prefix();
                ee.push_back(e);
            }
            return AstExprApplication::create(p, ee);
        } else {
            return e;
        }
    }

    /*
     *  Taken from Wikipedia
     * https://en.wikipedia.org/wiki/Operator-precedence_parser
     *
     *  parse_expression_1 (lhs, min_precedence)
     *      lookahead := peek next token
     *      while lookahead is a binary operator whose precedence is >=
     * min_precedence op := lookahead advance to next token rhs := parse_primary
     * () lookahead := peek next token while lookahead is a binary operator
     * whose precedence is greater than op's, or a right-associative operator
     * whose precedence is equal to op's rhs := parse_expression_1 (rhs,
     * lookahead's precedence) lookahead := peek next token lhs := the result of
     * applying op with operands lhs and rhs return lhs
     */

    AstPtr parse_arithmetic_expression_1(AstPtr e0, icu::UnicodeString mp) {
        AstPtr lhs = e0;
        icu::UnicodeString la = peek_operator();
        while (((la.compare("") != 0) && operator_is_infix(la)) &&
               (operator_compare(la, mp) >= 0)) {
            Position p = position();
            icu::UnicodeString opt = la;
            AstPtr op = parse_operator();
            AstPtr rhs = parse_primaries();
            la = peek_operator();
            while (((la.compare("") != 0) && operator_is_infix(la)) &&
                   ((operator_compare(la, opt) > 0) ||
                    ((opt.compare(la) == 0) &&
                     operator_is_right_associative(la)))) {
                rhs = parse_arithmetic_expression_1(rhs, la);
                la = peek_operator();
            }
            lhs = AstExprApplication::create(p, op, lhs, rhs);
        }
        return lhs;
    }

    AstPtr parse_arithmetic_expression() {
        AstPtr e = parse_primaries();
        return parse_arithmetic_expression_1(e, OPERATOR_BOTTOM);
    }

    AstPtr parse_expression() {
        Position p = position();
        AstPtr e0 = parse_arithmetic_expression();
        if (tag() == TOKEN_SEMICOLON) {
            skip();
            AstPtr e1 = parse_expression();
            return AstExprStatement::create(p, e0, e1);
        } else {
            return e0;
        }
    }

    // class fields
    bool is_field() {
        switch (tag()) {
            case TOKEN_DATA:
            case TOKEN_DEF:
                return true;
                break;
            default:
                return false;
                break;
        }
    }

    AstPtr parse_field_data() {
        Position p = position();
        force_token(TOKEN_DATA);
        AstPtrs ee = AstPtrs();
        auto n = parse_combinator();
        ee.push_back(n);
        force_token(TOKEN_COMMA);
        auto e = parse_expression();
        ee.push_back(e);
        return AstDeclData::create(p, ee);
    }

    AstPtr parse_field_definition() {
        Position p = position();
        force_token(TOKEN_DEF);
        if (is_combinator()) {
            AstPtr c = parse_combinator();
            force_token(TOKEN_EQ);
            AstPtr e = parse_expression();
            return AstDeclDefinition::create(p, c, e);
            /*
        } else if (is_operator()) {
            AstPtr c = parse_operator();
            force_token(TOKEN_EQ);
            AstPtr e = parse_expression();
            return AstDeclOperator::create(p, c, e);
            */
        } else {
            throw ErrorSyntactical(p, "combinator expected in field");
        }
    }

    AstPtr parse_field() {
        switch (tag()) {
            case TOKEN_DATA:
                return parse_field_data();
                break;
            case TOKEN_DEF:
                return parse_field_definition();
                break;
            default:
                return nullptr;  // XXX;
                break;
        }
    }

    AstPtrs parse_fields() {
        AstPtrs ff;
        while (is_field()) {
            auto f = parse_field();
            ff.push_back(f);
        }
        return ff;
    }

    // declarations
    AstPtr parse_decl_data() {
        Position p = position();
        force_token(TOKEN_DATA);
        AstPtrs ee = AstPtrs();
        AstPtr e = parse_combinator();
        ee.push_back(e);
        while ((tag() == TOKEN_COMMA)) {
            force_token(TOKEN_COMMA);
            e = parse_combinator();
            ee.push_back(e);
        };
        return AstDeclData::create(p, ee);
    }

    AstPtr parse_decl_definition() {
        Position p = position();
        force_token(TOKEN_DEF);
        if (is_combinator()) {
            AstPtr c = parse_combinator();
            force_token(TOKEN_EQ);
            AstPtr e = parse_expression();
            return AstDeclDefinition::create(p, c, e);
        } else if (is_operator()) {
            AstPtr c = parse_operator();
            force_token(TOKEN_EQ);
            AstPtr e = parse_expression();
            return AstDeclOperator::create(p, c, e);
        } else {
            throw ErrorSyntactical(p, "combinator or operator expected");
        }
    }

    AstPtr parse_decl_value() {
        Position p = position();
        force_token(TOKEN_VAL);
        if (is_combinator()) {
            AstPtr c = parse_combinator();
            force_token(TOKEN_EQ);
            AstPtr e = parse_expression();
            return AstDeclValue::create(p, c, e);
        } else {
            throw ErrorSyntactical(p, "combinator expected");
        }
    }

    AstPtr parse_decl_class() {
        Position p = position();
        force_token(TOKEN_CLASS);
        AstPtr c = parse_combinator();
        AstPtrs vv;
        while (is_variable()) {
            auto v = parse_variable();
            vv.push_back(v);
        }
        AstPtrs ee;
        if (tag() == TOKEN_EXTENDS) {
            force_token(TOKEN_EXTENDS);
            auto e = parse_expression();
            ee.push_back(e);
            while (tag() == TOKEN_COMMA) {
                force_token(TOKEN_COMMA);
                auto e = parse_expression();
                ee.push_back(e);
            }
            force_token(TOKEN_WITH);
        }
        force_token(TOKEN_LPAREN);
        auto ff = parse_fields();
        force_token(TOKEN_RPAREN);
        return AstDeclObject::create(p, c, vv, ff, ee);
    }

    AstPtr parse_decl_namespace() {
        Position p = position();
        force_token(TOKEN_NAMESPACE);
        UnicodeStrings n = parse_namespace();
        force_token(TOKEN_LPAREN);
        AstPtrs dd = AstPtrs();
        while (tag() != TOKEN_RPAREN) {
            AstPtr d = parse_decl_or_directive();
            dd.push_back(d);
        }
        force_token(TOKEN_RPAREN);
        return AstDeclNamespace::create(p, n, dd);
    }

    bool is_decl() {
        switch (tag()) {
            case TOKEN_DATA:
            case TOKEN_DEF:
            case TOKEN_VAL:
            case TOKEN_NAMESPACE:
            case TOKEN_CLASS:
                return true;
                break;
            default:
                return false;
                break;
        }
    }

    AstPtr parse_decl() {
        switch (tag()) {
            case TOKEN_DATA:
                return parse_decl_data();
                break;
            case TOKEN_DEF:
                return parse_decl_definition();
                break;
            case TOKEN_VAL:
                return parse_decl_value();
                break;
            case TOKEN_CLASS:
                return parse_decl_class();
                break;
            case TOKEN_NAMESPACE:
                return parse_decl_namespace();
                break;
            default:
                break;
        }
        Position p = position();
        throw ErrorSyntactical(p, "declaration expected");
    }

    // directives

    bool is_directive() {
        switch (tag()) {
            case TOKEN_IMPORT:
            case TOKEN_USING:
                return true;
                break;
            default:
                return false;
                break;
        }
    }

    AstPtr parse_using() {
        Position p = position();
        force_token(TOKEN_USING);
        UnicodeStrings n = parse_namespace();
        return AstDirectUsing::create(p, n);
    }

    AstPtr parse_import() {
        Position p = position();
        force_token(TOKEN_IMPORT);
        check_token(TOKEN_TEXT);
        icu::UnicodeString n = look().text();
        skip();
        return AstDirectImport::create(p, n);
    }

    AstPtr parse_directive() {
        switch (tag()) {
            case TOKEN_USING:
                return parse_using();
                break;
            case TOKEN_IMPORT:
                return parse_import();
                break;
            default:
                break;
        }
        Position p = position();
        throw ErrorSyntactical(p, "directive expected");
    }

    AstPtr parse_decl_or_directive() {
        if (is_decl()) {
            return parse_decl();
        } else if (is_directive()) {
            return parse_directive();
        } else {
            Position p = position();
            throw ErrorSyntactical(p, "declaration or directive expected");
        }
    }

    AstPtr parse() {
        Position p = position();
        AstPtrs dd = AstPtrs();
        while (tag() != TOKEN_EOF) {
            AstPtr d = parse_decl_or_directive();
            dd.push_back(d);
        }
        return AstWrapper::create(p, dd);
    }

protected:
    // convenience methods
    AstPtr app(AstPtr e0, AstPtr e1) {
        return AstExprApplication::create(e0->position(), e0, e1);
    }

private:
    TokenReaderPtr _tokenreader;
};

class LineParser : public Parser {
public:
    LineParser(TokenReaderPtr &r) : Parser(r) {
    }

    AstPtr parse_command() {
        if (is_directive()) {
            return parse_directive();
        } else if (tag() == TOKEN_DEF) {
            return parse_decl_definition();
        } else if (tag() == TOKEN_VAL) {
            return parse_decl_value();
        } else if (tag() == TOKEN_DATA) {
            return parse_decl_data();
        } else {
            return parse_expression();
        }
    }

    AstPtr parse_line() {
        Position p = position();
        AstPtrs aa;
        if (tag() == TOKEN_EOF) return AstWrapper::create(p, aa);
        auto a = parse_command();
        aa.push_back(a);
        while (tag() == TOKEN_DSEMICOLON) {
            force_token(TOKEN_DSEMICOLON);
            a = parse_command();
            aa.push_back(a);
        }
        if (tag() == TOKEN_EOF) {
            return AstWrapper::create(p, aa);
        } else {
            auto p = position();
            throw ErrorSyntactical(p, look(0).text() + " unexpected");
        }
    }
};

AstPtrs imports(const AstPtr &a);  // XXX: didn't know where to place this
AstPtrs values(const AstPtr &a);   // XXX: didn't know where to place this

AstPtr parse(TokenReaderPtr &r);

AstPtr parse_line(TokenReaderPtr &r);

class Imports : public Visit {
public:
    AstPtrs imports(const AstPtr &a) {
        visit(a);
        return _imports;
    }

    void visit_directive_import(const Position &p,
                                const icu::UnicodeString &i) override {
        _imports.push_back(AstDirectImport::create(p, i));
    }

    // cuts
    void visit_decl_data(const Position &p, const AstPtrs &nn) override {
    }

    void visit_decl_definition(const Position &p, const AstPtr &n,
                               const AstPtr &e) override {
    }

    void visit_decl_value(const Position &p, const AstPtr &n,
                          const AstPtr &e) override {
    }

    void visit_decl_operator(const Position &p, const AstPtr &c,
                             const AstPtr &e) override {
    }

private:
    AstPtrs _imports;
};

AstPtrs imports(const AstPtr &a) {
    Imports imports;
    return imports.imports(a);
}

class Values : public Visit {
public:
    AstPtrs values(const AstPtr &a) {
        visit(a);
        return _values;
    }

    void visit_decl_value(const Position &p, const AstPtr &n,
                          const AstPtr &e) override {
        _values.push_back(AstDeclValue::create(p, n, e));
    }

    // cuts
    void visit_decl_data(const Position &p, const AstPtrs &nn) override {
    }

    void visit_decl_definition(const Position &p, const AstPtr &n,
                               const AstPtr &e) override {
    }

    void visit_decl_operator(const Position &p, const AstPtr &c,
                             const AstPtr &e) override {
    }

private:
    AstPtrs _values;
};

AstPtrs values(const AstPtr &a) {
    Values values;
    return values.values(a);
}

AstPtr parse(TokenReaderPtr &r) {
    Parser p(r);
    AstPtr a = p.parse();
    return a;
}

AstPtr parse_line(TokenReaderPtr &r) {
    LineParser p(r);
    AstPtr a = p.parse_line();
    return a;
}
