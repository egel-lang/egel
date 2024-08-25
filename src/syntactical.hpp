#pragma once

#include "ast.hpp"
#include "error.hpp"
#include "lexical.hpp"
#include "operators.hpp"
#include "runtime.hpp"
#include "syntactical.hpp"
#include "transform.hpp"

namespace egel {

class Parser {
public:
    Parser(Tokens &r) {
        _tokenreader = r;
    }

    Token look(int n = 0) {
        return _tokenreader.look(n);
    }

    void skip() {
        // std::cout << "skipped: " << look() << std::endl;
        _tokenreader.skip();
    }

    Position position() {
        return _tokenreader.look().position();
    }

    token_t tag(int n = 0) {
        return _tokenreader.look(n).tag();
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
    ptr<Ast> parse_integer() {
        check_token(TOKEN_INTEGER);
        auto a = AstExprInteger::create(position(), look().text());
        skip();
        return a;
    }

    ptr<Ast> parse_hexinteger() {
        check_token(TOKEN_HEXINTEGER);
        auto a = AstExprHexInteger::create(position(), look().text());
        skip();
        return a;
    }

    ptr<Ast> parse_float() {
        check_token(TOKEN_FLOAT);
        auto a = AstExprFloat::create(position(), look().text());
        skip();
        return a;
    }

    ptr<Ast> parse_character() {
        check_token(TOKEN_CHAR);
        auto a = AstExprCharacter::create(position(), look().text());
        skip();
        return a;
    }

    ptr<Ast> parse_text() {
        check_token(TOKEN_TEXT);
        auto a = AstExprText::create(position(), look().text());
        skip();
        return a;
    }

    ptr<Ast> parse_docstring() {
        check_token(TOKEN_TEXT);
        auto a = AstExprText::create(position(), look().text());
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

    ptr<Ast> parse_variable() {
        check_token(TOKEN_UPPERCASE);
        Position p = position();
        icu::UnicodeString s = look().text();
        skip();
        return AstExprVariable::create(p, s);
    }

    ptr<Ast> parse_wildcard() {
        Position p = position();
        icu::UnicodeString s = look().text();
        if (s != "_") throw ErrorSyntactical(p, "wildcard expected");
        skip();
        return AstExprWildcard::create(p, s);
    }

    ptr<Ast> parse_combinator() {
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

    ptr<Ast> parse_operator() {
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

    ptr<Ast> parse_prefix_operator() {
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

    ptr<Ast> parse_enclosed_operator() {
        force_token(TOKEN_LPAREN);
        auto o = parse_operator();
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

    ptr<Ast> parse_pattern_primary() {
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
                ptr<Ast> e;
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

    ptr<Ast> parse_pattern_primaries() {
        Position p = position();
        if (is_pattern_primary()) {
            auto q = parse_pattern_primary();
            if (is_pattern_primary()) {
                ptrs<Ast> qq = ptrs<Ast>();
                qq.push_back(q);
                while (is_pattern_primary()) {
                    ptr<Ast> q = parse_pattern_primary();
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

    ptr<Ast> parse_pattern_enclosed() {
        if (is_enclosed_operator()) return parse_enclosed_operator();
        Position p = position();
        force_token(TOKEN_LPAREN);
        auto q = parse_pattern_primaries();
        if (tag() == TOKEN_COMMA) {
            auto qq = ptrs<Ast>();
            qq.push_back(q);
            while (tag() == TOKEN_COMMA) {
                skip();
                ptr<Ast> q = parse_pattern_primaries();
                qq.push_back(q);
            }
            force_token(TOKEN_RPAREN);
            return AstExprTuple::create(p, qq);
        } else {
            force_token(TOKEN_RPAREN);
            return q;
        }
    }

    ptr<Ast> parse_pattern_list() {
        Position p = position();
        force_token(TOKEN_LCURLY);
        auto qq = ptrs<Ast>();
        if (tag() == TOKEN_RCURLY) {
            skip();
            return AstExprList::create(p, qq);
        } else {
            ptr<Ast> q = parse_pattern();
            qq.push_back(q);
            while (tag() == TOKEN_COMMA) {
                skip();
                ptr<Ast> q = parse_pattern();
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

    ptr<Ast> parse_pattern() {
        Position p = position();
        if (is_pattern_primary()) {
            return parse_pattern_primary();
        } else {
            throw ErrorSyntactical(p, "pattern expected");
        }
    }

    ptrs<Ast> parse_patterns() {
        auto pp = ptrs<Ast>();
        while (is_pattern_primary()) {
            ptr<Ast> p = parse_pattern();
            pp.push_back(p);
        }
        return pp;
    }

    // expressions
    ptr<Ast> parse_match() {
        Position p = position();
        auto mm = parse_patterns();
        /*
        if (tag() == TOKEN_QUESTION) {
            skip();
            ptr<Ast> q = parse_expression();
            force_token(TOKEN_ARROW);
            ptr<Ast> e = parse_expression();
            return AstExprMatch::create(p, mm, q, e);
        } else {
            ptr<Ast> q = AstEmpty::create();
            force_token(TOKEN_ARROW);
            ptr<Ast> e = parse_expression();
            return AstExprMatch::create(p, mm, q, e);
        }
        */
        auto q = AstEmpty::create();
        force_token(TOKEN_ARROW);
        auto e = parse_expression();
        return AstExprMatch::create(p, mm, q, e);
    }

    ptr<Ast> parse_block() {
        Position p = position();
        auto mm = ptrs<Ast>();
        check_token(TOKEN_LSQUARE);
        do {
            skip();
            ptr<Ast> m = parse_match();
            mm.push_back(m);
        } while (tag() == TOKEN_BAR);
        force_token(TOKEN_RSQUARE);
        return AstExprBlock::create(p, mm);
    }

    ptr<Ast> parse_lambda() {
        Position p = position();
        auto mm = ptrs<Ast>();
        force_token(TOKEN_LAMBDA);
        auto m = parse_match();
        return AstExprLambda::create(p, m);
    }

    ptr<Ast> parse_enclosed() {
        if (is_enclosed_operator()) return parse_enclosed_operator();
        Position p = position();
        force_token(TOKEN_LPAREN);
        auto q = parse_expression();
        if (tag() == TOKEN_COMMA) {
            ptrs<Ast> qq = ptrs<Ast>();
            qq.push_back(q);
            while (tag() == TOKEN_COMMA) {
                skip();
                ptr<Ast> q = parse_expression();
                qq.push_back(q);
            }
            force_token(TOKEN_RPAREN);
            return AstExprTuple::create(p, qq);
        } else {
            force_token(TOKEN_RPAREN);
            return q;
        }
    }

    ptr<Ast> parse_list() {
        Position p = position();
        force_token(TOKEN_LCURLY);
        auto qq = ptrs<Ast>();
        if (tag() == TOKEN_RCURLY) {
            skip();
            return AstExprList::create(p, qq);
        } else {
            ptr<Ast> q = parse_expression();
            qq.push_back(q);
            while (tag() == TOKEN_COMMA) {
                skip();
                ptr<Ast> q = parse_expression();
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

    ptr<Ast> parse_if() {
        Position p = position();
        force_token(TOKEN_IF);
        auto e0 = parse_expression();
        force_token(TOKEN_THEN);
        auto e1 = parse_expression();
        force_token(TOKEN_ELSE);
        auto e2 = parse_expression();
        return AstExprIf::create(p, e0, e1, e2);
    }

    ptr<Ast> parse_try() {
        Position p = position();
        force_token(TOKEN_TRY);
        auto e0 = parse_expression();
        force_token(TOKEN_CATCH);
        auto e1 = parse_expression();
        return AstExprTry::create(p, e0, e1);
    }

    ptr<Ast> parse_throw() {
        Position p = position();
        force_token(TOKEN_THROW);
        auto e0 = parse_expression();
        return AstExprThrow::create(p, e0);
    }

    ptr<Ast> parse_let() {
        Position p = position();
        force_token(TOKEN_LET);
        auto ee = parse_patterns();
        force_token(TOKEN_EQ);
        auto e1 = parse_expression();
        force_token(TOKEN_IN);
        auto e2 = parse_expression();
        return AstExprLet::create(p, ee, e1, e2);
    }

    ptr<Ast> parse_do() {
        Position p = position();
        force_token(TOKEN_DO);
        auto e0 = parse_expression();
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
            case TOKEN_LET:
            case TOKEN_DO:
            case TOKEN_LSQUARE:
            case TOKEN_LAMBDA:
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

    ptr<Ast> parse_primary() {
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

    ptr<Ast> parse_primary_prefix() {
        icu::UnicodeString s = peek_operator();
        if ((s != "") && operator_is_prefix(s)) {
            ptr<Ast> o = parse_prefix_operator();
            ptr<Ast> e = parse_primary_prefix();
            return app(o, e);
        } else {
            return parse_primary();
        }
    }

    ptr<Ast> parse_primaries() {
        Position p = position();
        auto e = parse_primary_prefix();
        if (is_primary()) {
            ptrs<Ast> ee;
            ee.push_back(e);
            while (is_primary()) {
                auto e = parse_primary_prefix();
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

    ptr<Ast> parse_arithmetic_expression_1(ptr<Ast> e0, icu::UnicodeString mp) {
        auto lhs = e0;
        icu::UnicodeString la = peek_operator();
        while (((la.compare("") != 0) && operator_is_infix(la)) &&
               (operator_compare(la, mp) >= 0)) {
            Position p = position();
            icu::UnicodeString opt = la;
            auto op = parse_operator();
            auto rhs = parse_primaries();
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

    ptr<Ast> parse_arithmetic_expression() {
        auto e = parse_primaries();
        return parse_arithmetic_expression_1(e, OPERATOR_BOTTOM);
    }

    ptr<Ast> parse_expression() {
        Position p = position();
        auto e0 = parse_arithmetic_expression();
        if (tag() == TOKEN_SEMICOLON) {
            skip();
            ptr<Ast> e1 = parse_expression();
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

    ptr<Ast> parse_field_data() {
        Position p = position();
        force_token(TOKEN_DATA);
        auto ee = ptrs<Ast>();
        auto n = parse_combinator();
        ee.push_back(n);
        force_token(TOKEN_COMMA);
        auto e = parse_expression();
        ee.push_back(e);
        return AstDeclData::create(p, ee);
    }

    ptr<Ast> parse_field_definition() {
        Position p = position();
        force_token(TOKEN_DEF);
        if (is_combinator()) {
            auto c = parse_combinator();
            force_token(TOKEN_EQ);
            auto e = parse_expression();
            return AstDeclDefinition::create(p, c, e);
            /*
        } else if (is_operator()) {
            ptr<Ast> c = parse_operator();
            force_token(TOKEN_EQ);
            ptr<Ast> e = parse_expression();
            return AstDeclOperator::create(p, c, e);
            */
        } else {
            throw ErrorSyntactical(p, "combinator expected in field");
        }
    }

    ptr<Ast> parse_field() {
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

    ptrs<Ast> parse_fields() {
        ptrs<Ast> ff;
        while (is_field()) {
            auto f = parse_field();
            ff.push_back(f);
        }
        return ff;
    }

    // declarations
    ptr<Ast> parse_decl_data() {
        Position p = position();
        force_token(TOKEN_DATA);
        auto ee = ptrs<Ast>();
        auto e = parse_combinator();
        ee.push_back(e);
        while ((tag() == TOKEN_COMMA)) {
            force_token(TOKEN_COMMA);
            e = parse_combinator();
            ee.push_back(e);
        };
        return AstDeclData::create(p, ee);
    }

    ptr<Ast> parse_decl_definition() {
        Position p = position();
        force_token(TOKEN_DEF);
        if (is_combinator()) {
            ptr<Ast> c = parse_combinator();
            force_token(TOKEN_EQ);
            ptr<Ast> e = parse_expression();
            return AstDeclDefinition::create(p, c, e);
        } else if (is_operator()) {
            ptr<Ast> c = parse_operator();
            force_token(TOKEN_EQ);
            ptr<Ast> e = parse_expression();
            return AstDeclOperator::create(p, c, e);
        } else {
            throw ErrorSyntactical(p, "combinator or operator expected");
        }
    }

    ptr<Ast> parse_decl_value() {
        Position p = position();
        force_token(TOKEN_VAL);
        if (is_combinator()) {
            auto c = parse_combinator();
            force_token(TOKEN_EQ);
            auto e = parse_expression();
            return AstDeclValue::create(p, c, e);
        } else {
            throw ErrorSyntactical(p, "combinator expected");
        }
    }

    ptr<Ast> parse_decl_namespace() {
        Position p = position();
        force_token(TOKEN_NAMESPACE);
        UnicodeStrings n = parse_namespace();
        force_token(TOKEN_LPAREN);
        auto dd = ptrs<Ast>();
        while (tag() != TOKEN_RPAREN) {
            auto d = parse_decl_or_directive();
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

    ptr<Ast> parse_decl() {
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

    ptr<Ast> parse_using() {
        Position p = position();
        force_token(TOKEN_USING);
        UnicodeStrings n = parse_namespace();
        return AstDirectUsing::create(p, n);
    }

    ptr<Ast> parse_import() {
        Position p = position();
        force_token(TOKEN_IMPORT);
        check_token(TOKEN_TEXT);
        icu::UnicodeString n = look().text();
        skip();
        return AstDirectImport::create(p, n);
    }

    ptr<Ast> parse_directive() {
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

    ptr<Ast> parse_decl_or_directive() {
        if (is_decl()) {
            return parse_decl();
        } else if (is_directive()) {
            return parse_directive();
        } else {
            Position p = position();
            throw ErrorSyntactical(p, "declaration or directive expected");
        }
    }

    ptr<Ast> parse() {
        Position p = position();
        auto dd = ptrs<Ast>();
        while (tag() != TOKEN_EOF) {
            auto d = parse_decl_or_directive();
            dd.push_back(d);
        }
        return AstWrapper::create(p, dd);
    }

protected:
    // convenience methods
    ptr<Ast> app(ptr<Ast> e0, ptr<Ast> e1) {
        return AstExprApplication::create(e0->position(), e0, e1);
    }

private:
    Tokens _tokenreader;
};

class LineParser : public Parser {
public:
    LineParser(Tokens &r) : Parser(r) {
    }

    ptr<Ast> parse_command() {
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

    ptr<Ast> parse_line() {
        Position p = position();
        ptrs<Ast> aa;
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

ptrs<Ast> imports(const ptr<Ast> &a);  // XXX: didn't know where to place this
ptrs<Ast> values(const ptr<Ast> &a);   // XXX: didn't know where to place this

ptr<Ast> parse(Tokens &r);

ptr<Ast> parse_line(Tokens &r);

class Imports : public Visit {
public:
    ptrs<Ast> imports(const ptr<Ast> &a) {
        visit(a);
        return _imports;
    }

    void visit_directive_import(const Position &p,
                                const icu::UnicodeString &i) override {
        _imports.push_back(AstDirectImport::create(p, i));
    }

    // cuts
    void visit_decl_data(const Position &p, const ptrs<Ast> &nn) override {
    }

    void visit_decl_definition(const Position &p, const ptr<Ast> &n,
                               const ptr<Ast> &e) override {
    }

    void visit_decl_value(const Position &p, const ptr<Ast> &n,
                          const ptr<Ast> &e) override {
    }

    void visit_decl_operator(const Position &p, const ptr<Ast> &c,
                             const ptr<Ast> &e) override {
    }

private:
    ptrs<Ast> _imports;
};

ptrs<Ast> imports(const ptr<Ast> &a) {
    Imports imports;
    return imports.imports(a);
}

class Values : public Visit {
public:
    ptrs<Ast> values(const ptr<Ast> &a) {
        visit(a);
        return _values;
    }

    void visit_decl_value(const Position &p, const ptr<Ast> &n,
                          const ptr<Ast> &e) override {
        _values.push_back(AstDeclValue::create(p, n, e));
    }

    // cuts
    void visit_decl_data(const Position &p, const ptrs<Ast> &nn) override {
    }

    void visit_decl_definition(const Position &p, const ptr<Ast> &n,
                               const ptr<Ast> &e) override {
    }

    void visit_decl_operator(const Position &p, const ptr<Ast> &c,
                             const ptr<Ast> &e) override {
    }

private:
    ptrs<Ast> _values;
};

ptrs<Ast> values(const ptr<Ast> &a) {
    Values values;
    return values.values(a);
}

ptr<Ast> parse(Tokens &r) {
    Parser p(r);
    auto a = p.parse();
    return a;
}

ptr<Ast> parse_line(Tokens &r) {
    LineParser p(r);
    auto a = p.parse_line();
    return a;
}

};  // namespace egel
