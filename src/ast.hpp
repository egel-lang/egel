#pragma once

#include <memory>
#include <set>
#include <sstream>
#include <vector>

#include "constants.hpp"
#include "error.hpp"
#include "position.hpp"
#include "utils.hpp"

// XXX: rewrite all the code once to make use of the next templates
template <typename T> using Ptr = std::shared_ptr<T>;
template <typename T> using Ptrs = std::vector<std::shared_ptr<T>>;
template <typename T, typename... Args>
std::shared_ptr<T> make_ptr(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

// AST tags are enumerated as a manner to implement switches conveniently
enum ast_tag_t {
    AST_EMPTY,
    // literals
    AST_EXPR_INTEGER,
    AST_EXPR_HEXINTEGER,
    AST_EXPR_FLOAT,
    AST_EXPR_CHARACTER,
    AST_EXPR_TEXT,
    // variables and constants
    AST_EXPR_VARIABLE,
    AST_EXPR_WILDCARD,  // desugared
    AST_EXPR_COMBINATOR,
    AST_EXPR_OPERATOR,  // compiled out
    // special pattern
    AST_EXPR_TAG,
    // list, tuple and object
    AST_EXPR_LIST,   // desugared
    AST_EXPR_TUPLE,  // desugared
    // compound statements
    AST_EXPR_APPLICATION,
    AST_EXPR_BLOCK,
    AST_EXPR_MATCH,  // guards are desugared
    AST_EXPR_TRY,
    AST_EXPR_THROW,
    AST_EXPR_LAMBDA,     // desugared
    AST_EXPR_LET,        // desugared
    AST_EXPR_IF,         // desugared
    AST_EXPR_STATEMENT,  // desugared
    // directives
    AST_DIRECT_IMPORT,  // flattened out
    AST_DIRECT_USING,   // flattened out
    // declarations,
    AST_DECL_NAMESPACE,  // flattened out
    AST_DECL_DATA,
    AST_DECL_DEFINITION,
    AST_DECL_OPERATOR,
    AST_DECL_OBJECT,  // desugared
    // wrapper
    AST_WRAPPER,
    // set
    AST_DECL_VALUE,
};

class Ast;
using AstPtr = std::shared_ptr<Ast>;
using AstPtrs = std::vector<AstPtr>;

using text_index_t = int;

class Ast {
public:
    Ast(ast_tag_t t, const Position &p) : _tag(t), _position(p) {
    }

    virtual ~Ast() {  // keep the compiler happy
    }

    Position position() const {
        return _position;
    }

    ast_tag_t tag() const {
        return _tag;
    }

    friend std::ostream &operator<<(std::ostream &os, const AstPtr &a) {
        a->render(os, 0);
        return os;
    }

    static const text_index_t line_length = 80;

    virtual text_index_t approximate_length(text_index_t indent) const = 0;

    void skip(std::ostream &os, text_index_t indent) const {
        for (text_index_t i = 0; i < indent; ++i) {
            os << " ";
        }
    }
    void skip_line(std::ostream &os, text_index_t indent) const {
        os << std::endl;
        skip(os, indent);
    }

    virtual void render(std::ostream &os, text_index_t indent) const = 0;

    virtual icu::UnicodeString to_text() const {
        std::stringstream ss;
        render(ss, Ast::line_length);
        icu::UnicodeString u(ss.str().c_str());
        return u;
    }

    static int compare(const AstPtr &a0, const AstPtr &a1);

protected:  // a collection of helper functions
    static int compare_tag(ast_tag_t t, const AstPtr &a0, const AstPtr &a1);
    static int compare_ast2(const AstPtr &a0, const AstPtr &a1,
                            const AstPtr &a2, const AstPtr &a3);
    static int compare_ast3(const AstPtr &a0, const AstPtr &a1,
                            const AstPtr &a2, const AstPtr &a3,
                            const AstPtr &a4, const AstPtr &a5);
    static int compare_asts(const AstPtrs &aa0, const AstPtrs &aa1);

private:
    ast_tag_t _tag;
    Position _position;
};

struct EqualAstPtr {
    bool operator()(const AstPtr &a0, const AstPtr &a1) const {
        return (Ast::compare(a0, a1) == 0);
    }
};

struct LessAstPtr {
    bool operator()(const AstPtr &a0, const AstPtr &a1) const {
        return (Ast::compare(a0, a1) == -1);
    }
};

using AstPtrSet = std::set<AstPtr, LessAstPtr>;

// filler for optional cases

class AstEmpty : public Ast {
public:
    AstEmpty() : Ast(AST_EMPTY, Position(icu::UnicodeString(""), 0, 0)) {
    }

    AstEmpty(const AstEmpty &) : AstEmpty() {
    }

    static AstPtr create() {
        return std::make_shared<AstEmpty>();
    }

    static std::shared_ptr<AstEmpty> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstEmpty>(a);
    }

    text_index_t approximate_length(text_index_t indent) const override {
        return indent;
    }

    void render(std::ostream &os, text_index_t indent) const override {
        os << "(XX)";
    }
};

using AstEmptyPtr = std::shared_ptr<AstEmpty>;
#define AST_EMPTY_CAST(a) std::static_pointer_cast<AstEmptyPtr>(a)

// the atom class is a convenience base class for simple text representations

class AstAtom : public Ast {
public:
    AstAtom(ast_tag_t t, const Position &p, const icu::UnicodeString &n)
        : Ast(t, p), _text(n) {
    }

    icu::UnicodeString text() const {
        return _text;
    }

    text_index_t approximate_length(text_index_t indent) const {
        return (indent + text().length());
    }

    void render(std::ostream &os, text_index_t) const {
        os << text();
    }

private:
    icu::UnicodeString _text;
};

// expression literals

class AstExprInteger : public AstAtom {
public:
    AstExprInteger(const Position &p, const icu::UnicodeString &text)
        : AstAtom(AST_EXPR_INTEGER, p, text){};

    AstExprInteger(const AstExprInteger &l)
        : AstExprInteger(l.position(), l.text()) {
    }

    static AstPtr create(const Position &p, const icu::UnicodeString &text) {
        return std::make_shared<AstExprInteger>(p, text);
    }

    static std::shared_ptr<AstExprInteger> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprInteger>(a);
    }

    static std::tuple<Position, icu::UnicodeString> split(const AstPtr &a) {
        auto a0 = AstExprInteger::cast(a);
        auto p = a0->position();
        auto s = a0->text();
        return {p, s};
    }
};

using AstExprIntegerPtr = std::shared_ptr<AstExprInteger>;
#define AST_EXPR_INTEGER_CAST(a) std::static_pointer_cast<AstExprInteger>(a)
#define AST_EXPR_INTEGER_SPLIT(a, p, t)   \
    auto _##a = AST_EXPR_INTEGER_CAST(a); \
    auto p = _##a->position();            \
    auto t = _##a->text();

class AstExprHexInteger : public AstAtom {
public:
    AstExprHexInteger(const Position &p, const icu::UnicodeString &text)
        : AstAtom(AST_EXPR_HEXINTEGER, p, text){};

    AstExprHexInteger(const AstExprHexInteger &l)
        : AstExprHexInteger(l.position(), l.text()) {
    }

    static AstPtr create(const Position &p, const icu::UnicodeString &hex) {
        return std::make_shared<AstExprHexInteger>(p, hex);
    }

    static std::shared_ptr<AstExprHexInteger> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprHexInteger>(a);
    }

    static std::tuple<Position, icu::UnicodeString> split(const AstPtr &a) {
        auto a0 = AstExprHexInteger::cast(a);
        auto p = a0->position();
        auto s = a0->text();
        return {p, s};
    }
};

using AstExprHexIntegerPtr = std::shared_ptr<AstExprHexInteger>;
#define AST_EXPR_HEXINTEGER_CAST(a) \
    std::static_pointer_cast<AstExprHexInteger>(a)
#define AST_EXPR_HEXINTEGER_SPLIT(a, p, t)   \
    auto _##a = AST_EXPR_HEXINTEGER_CAST(a); \
    auto p = _##a->position();               \
    auto t = _##a->text();

class AstExprFloat : public AstAtom {
public:
    AstExprFloat(const Position &p, const icu::UnicodeString &text)
        : AstAtom(AST_EXPR_FLOAT, p, text){};

    AstExprFloat(const AstExprFloat &l) : AstExprFloat(l.position(), l.text()) {
    }

    static AstPtr create(const Position &p, const icu::UnicodeString &text) {
        return std::make_shared<AstExprFloat>(p, text);
    }

    static std::shared_ptr<AstExprFloat> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprFloat>(a);
    }

    static std::tuple<Position, icu::UnicodeString> split(const AstPtr &a) {
        auto a0 = AstExprFloat::cast(a);
        auto p = a0->position();
        auto s = a0->text();
        return {p, s};
    }
};

using AstExprFloatPtr = std::shared_ptr<AstExprFloat>;
#define AST_EXPR_FLOAT_CAST(a) std::static_pointer_cast<AstExprFloat>(a)
#define AST_EXPR_FLOAT_SPLIT(a, p, t)   \
    auto _##a = AST_EXPR_FLOAT_CAST(a); \
    auto p = _##a->position();          \
    auto t = _##a->text();

class AstExprCharacter : public AstAtom {
public:
    AstExprCharacter(const Position &p, const icu::UnicodeString &text)
        : AstAtom(AST_EXPR_CHARACTER, p, text){};

    AstExprCharacter(const AstExprCharacter &l)
        : AstExprCharacter(l.position(), l.text()) {
    }

    static AstPtr create(const Position &p, const icu::UnicodeString &text) {
        return std::make_shared<AstExprCharacter>(p, text);
    }

    static std::shared_ptr<AstExprCharacter> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprCharacter>(a);
    }

    static std::tuple<Position, icu::UnicodeString> split(const AstPtr &a) {
        auto a0 = AstExprCharacter::cast(a);
        auto p = a0->position();
        auto s = a0->text();
        return {p, s};
    }
};

using AstExprCharacterPtr = std::shared_ptr<AstExprCharacter>;
#define AST_EXPR_CHARACTER_CAST(a) std::static_pointer_cast<AstExprCharacter>(a)
#define AST_EXPR_CHARACTER_SPLIT(a, p, t)   \
    auto _##a = AST_EXPR_CHARACTER_CAST(a); \
    auto p = _##a->position();              \
    auto t = _##a->text();

class AstExprText : public AstAtom {
public:
    AstExprText(const Position &p, const icu::UnicodeString &text)
        : AstAtom(AST_EXPR_TEXT, p, text){};

    AstExprText(const AstExprFloat &l) : AstExprText(l.position(), l.text()) {
    }

    static AstPtr create(const Position &p, const icu::UnicodeString &text) {
        return std::make_shared<AstExprText>(p, text);
    }

    static std::shared_ptr<AstExprText> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprText>(a);
    }

    static std::tuple<Position, icu::UnicodeString> split(const AstPtr &a) {
        auto a0 = AstExprText::cast(a);
        auto p = a0->position();
        auto s = a0->text();
        return {p, s};
    }
};

using AstExprTextPtr = std::shared_ptr<AstExprText>;
#define AST_EXPR_TEXT_CAST(a) std::static_pointer_cast<AstExprText>(a)
#define AST_EXPR_TEXT_SPLIT(a, p, t)   \
    auto _##a = AST_EXPR_TEXT_CAST(a); \
    auto p = _##a->position();         \
    auto t = _##a->text();

// expression variables and constants

class AstExprVariable : public AstAtom {
public:
    AstExprVariable(const Position &p, const icu::UnicodeString &text)
        : AstAtom(AST_EXPR_VARIABLE, p, text){};

    AstExprVariable(const AstExprVariable &l)
        : AstExprVariable(l.position(), l.text()) {
    }

    static AstPtr create(const Position &p, const icu::UnicodeString &text) {
        return std::make_shared<AstExprVariable>(p, text);
    }

    static std::shared_ptr<AstExprVariable> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprVariable>(a);
    }

    static std::tuple<Position, icu::UnicodeString> split(const AstPtr &a) {
        auto a0 = AstExprVariable::cast(a);
        auto p = a0->position();
        auto s = a0->text();
        return {p, s};
    }
};

using AstExprVariablePtr = std::shared_ptr<AstExprVariable>;
#define AST_EXPR_VARIABLE_CAST(a) std::static_pointer_cast<AstExprVariable>(a)
#define AST_EXPR_VARIABLE_SPLIT(a, p, t)   \
    auto _##a = AST_EXPR_VARIABLE_CAST(a); \
    auto p = _##a->position();             \
    auto t = _##a->text();

class AstExprWildcard : public AstAtom {
public:
    AstExprWildcard(const Position &p, const icu::UnicodeString &text)
        : AstAtom(AST_EXPR_WILDCARD, p, text){};

    AstExprWildcard(const AstExprWildcard &l)
        : AstExprWildcard(l.position(), l.text()) {
    }

    static AstPtr create(const Position &p, const icu::UnicodeString &text) {
        return std::make_shared<AstExprWildcard>(p, text);
    }

    static std::shared_ptr<AstExprWildcard> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprWildcard>(a);
    }

    static std::tuple<Position, icu::UnicodeString> split(const AstPtr &a) {
        auto a0 = AstExprWildcard::cast(a);
        auto p = a0->position();
        auto s = a0->text();
        return {p, s};
    }
};

using AstExprWildcardPtr = std::shared_ptr<AstExprWildcard>;
#define AST_EXPR_WILDCARD_CAST(a) std::static_pointer_cast<AstExprWildcard>(a)
#define AST_EXPR_WILDCARD_SPLIT(a, p, t)   \
    auto _##a = AST_EXPR_WILDCARD_CAST(a); \
    auto p = _##a->position();             \
    auto t = _##a->text();

class AstExprTag : public Ast {
public:
    AstExprTag(const Position &p, const AstPtr &e, const AstPtr &t)
        : Ast(AST_EXPR_TAG, p), _expression(e), _tag(t) {
    }

    AstExprTag(const AstExprTag &a)
        : AstExprTag(a.position(), a.expression(), a.tag()) {
    }

    static AstPtr create(const Position &p, const AstPtr &e, const AstPtr &t) {
        return std::make_shared<AstExprTag>(p, e, t);
    }

    static std::shared_ptr<AstExprTag> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprTag>(a);
    }

    static std::tuple<Position, AstPtr, AstPtr> split(const AstPtr &a) {
        auto a0 = AstExprTag::cast(a);
        auto p = a0->position();
        auto e = a0->expression();
        auto t = a0->tag();
        return {p, e, t};
    }

    AstPtr expression() const {
        return _expression;
    }

    AstPtr tag() const {
        return _tag;
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent;
        l = expression()->approximate_length(l);
        l += 2;
        if (l >= line_length) return l;
        l = tag()->approximate_length(l);
        return l;
    }

    void render(std::ostream &os, text_index_t indent) const {
        if (approximate_length(indent) <= line_length) {
            os << "(" << expression() << STRING_COLON << tag() << ")";
        } else {
            os << "(";
            expression()->render(os, indent + 1);
            skip_line(os, indent + 1);
            os << STRING_COLON;
            tag()->render(os, indent + 1);
            os << ")";
            skip_line(os, indent);
        }
    }

private:
    AstPtr _expression;
    AstPtr _tag;
};

using AstExprTagPtr = std::shared_ptr<AstExprTag>;
#define AST_EXPR_TAG_CAST(a) std::static_pointer_cast<AstExprTag>(a)
#define AST_EXPR_TAG_SPLIT(a, p, e, t) \
    auto _##a = AST_EXPR_TAG_CAST(a);  \
    auto p = _##a->position();         \
    auto e = _##a->expression();       \
    auto t = _##a->tag();

class AstExprCombinator : public Ast {
public:
    AstExprCombinator(const Position &p, const icu::UnicodeString &c)
        : Ast(AST_EXPR_COMBINATOR, p), _combinator(c) {
        UnicodeStrings nn;
        _path = nn;
    };

    AstExprCombinator(const Position &p, const UnicodeStrings &pp,
                      const icu::UnicodeString &c)
        : Ast(AST_EXPR_COMBINATOR, p), _path(pp), _combinator(c){};

    AstExprCombinator(const Position &p, const icu::UnicodeString &n,
                      const icu::UnicodeString &c)
        : Ast(AST_EXPR_COMBINATOR, p), _combinator(c) {
        UnicodeStrings nn;
        nn.push_back(n);
        _path = nn;
    };

    AstExprCombinator(const AstExprCombinator &c)
        : AstExprCombinator(c.position(), c.path(), c.combinator()) {
    }

    static AstPtr create(const Position &p, const icu::UnicodeString &c) {
        return std::make_shared<AstExprCombinator>(p, c);
    }

    static AstPtr create(const Position &p, const UnicodeStrings &pp,
                         const icu::UnicodeString &c) {
        return std::make_shared<AstExprCombinator>(p, pp, c);
    }

    static AstPtr create(const Position &p, const icu::UnicodeString &n,
                         const icu::UnicodeString &c) {
        return std::make_shared<AstExprCombinator>(p, n, c);
    }

    static std::shared_ptr<AstExprCombinator> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprCombinator>(a);
    }

    static std::tuple<Position, std::vector<icu::UnicodeString>, icu::UnicodeString> split(const AstPtr &a) {
        auto a0 = AstExprCombinator::cast(a);
        auto p = a0->position();
        auto cc = a0->path();
        auto c = a0->combinator();
        return {p, cc, c};
    }

    UnicodeStrings path() const {
        return _path;
    }

    icu::UnicodeString combinator() const {
        return _combinator;
    }

    icu::UnicodeString to_text() const override {
        icu::UnicodeString str = "";  // XXX: change to some other datatype
        for (auto &p : _path) {
            str += p;
            str += STRING_DCOLON;
        }
        str += _combinator;
        return str;
    }

    text_index_t approximate_length(text_index_t indent) const override {
        text_index_t l = indent;
        for (auto &p : _path) {
            l += p.length() + 1;
        }
        l += _combinator.length();
        return l;
    }

    void render(std::ostream &os, text_index_t) const override {
        os << to_text();
    }

    bool is_local() const {
        return ((_path.size() > 0) && (_path.back() == STRING_LOCAL));
    }

private:
    UnicodeStrings _path;
    icu::UnicodeString _combinator;
};

using AstExprCombinatorPtr = std::shared_ptr<AstExprCombinator>;
#define AST_EXPR_COMBINATOR_CAST(a) \
    std::static_pointer_cast<AstExprCombinator>(a)
#define AST_EXPR_COMBINATOR_SPLIT(a, p, pp, c) \
    auto _##a = AST_EXPR_COMBINATOR_CAST(a);   \
    auto p = _##a->position();                 \
    auto pp = _##a->path();                    \
    auto c = _##a->combinator();

class AstExprOperator : public Ast {
public:
    AstExprOperator(const Position &p, const UnicodeStrings &pp,
                    const icu::UnicodeString &c)
        : Ast(AST_EXPR_OPERATOR, p), _path(pp), _combinator(c){};

    AstExprOperator(const Position &p, const icu::UnicodeString &n,
                    const icu::UnicodeString &c)
        : Ast(AST_EXPR_OPERATOR, p), _combinator(c) {
        UnicodeStrings nn;
        nn.push_back(n);
        _path = nn;
    };

    AstExprOperator(const AstExprOperator &c)
        : AstExprOperator(c.position(), c.path(), c.combinator()) {
    }

    static AstPtr create(const Position &p, const UnicodeStrings &pp,
                         const icu::UnicodeString &c) {
        return std::make_shared<AstExprOperator>(p, pp, c);
    }

    static AstPtr create(const Position &p, const icu::UnicodeString &n,
                         const icu::UnicodeString &c) {
        return std::make_shared<AstExprOperator>(p, n, c);
    }

    static std::shared_ptr<AstExprOperator> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprOperator>(a);
    }

    static std::tuple<Position, std::vector<icu::UnicodeString>, icu::UnicodeString> split(const AstPtr &a) {
        auto a0 = AstExprOperator::cast(a);
        auto p = a0->position();
        auto cc = a0->path();
        auto c = a0->combinator();
        return {p, cc, c};
    }

    UnicodeStrings path() const {
        return _path;
    }

    icu::UnicodeString combinator() const {
        return _combinator;
    }

    icu::UnicodeString text() const {
        icu::UnicodeString str = "";
        for (auto &p : _path) {
            str += p;
            str += STRING_DCOLON;
        }
        str += _combinator;
        return str;
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent;
        for (auto &p : _path) {
            l += p.length();
            l += 1;
            if (l >= line_length) return l;
        }
        l += _combinator.length();
        return l;
    }

    void render(std::ostream &os, text_index_t) const {
        os << text();
    }

private:
    UnicodeStrings _path;
    icu::UnicodeString _combinator;
};

using AstExprOperatorPtr = std::shared_ptr<AstExprOperator>;
#define AST_EXPR_OPERATOR_CAST(a) std::static_pointer_cast<AstExprOperator>(a)
#define AST_EXPR_OPERATOR_SPLIT(a, p, pp, c) \
    auto _##a = AST_EXPR_OPERATOR_CAST(a);   \
    auto p = _##a->position();               \
    auto pp = _##a->path();                  \
    auto c = _##a->combinator();

// list and tuple

class AstExprList : public Ast {
public:
    AstExprList(const Position &p, const AstPtrs &c)
        : Ast(AST_EXPR_LIST, p), _content(c), _tail(nullptr) {
    }

    AstExprList(const Position &p, const AstPtrs &c, const AstPtr &tl)
        : Ast(AST_EXPR_LIST, p), _content(c), _tail(tl) {
    }

    AstExprList(const AstExprList &c)
        : AstExprList(c.position(), c.content(), c.tail()) {
    }

    static AstPtr create(const Position &p, const AstPtrs &c) {
        return std::make_shared<AstExprList>(p, c);
    }

    static AstPtr create(const Position &p, const AstPtrs &c,
                         const AstPtr &tl) {
        return std::make_shared<AstExprList>(p, c, tl);
    }

    static std::shared_ptr<AstExprList> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprList>(a);
    }

    static std::tuple<Position, std::vector<std::shared_ptr<Ast>>, std::shared_ptr<Ast>> split(const AstPtr &a) {
        auto a0 = AstExprList::cast(a);
        auto p = a0->position();
        auto cc = a0->content();
        auto c = a0->tail();
        return {p, cc, c};
    }

    AstPtrs content() const {
        return _content;
    }

    AstPtr tail() const {
        return _tail;
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent;
        l += 1;
        for (auto e : _content) {
            l = e->approximate_length(l);
            l += 2;
            if (l >= line_length) return l;
        }
        if (tail() != nullptr) {
            l += 2;
            l = tail()->approximate_length(l);
        }
        l += 1;
        return l;
    }

    void render(std::ostream &os, text_index_t indent) const {
        if (approximate_length(indent) < line_length) {
            os << "{";
            bool first = true;
            for (auto &c : content()) {
                if (first) {
                    first = false;
                } else {
                    os << ",";
                }
                c->render(os, indent + 4);
            }
            if (tail() != nullptr) {
                os << "|";
                tail()->render(os, indent + 2);
            }
            os << "}";
        } else {
            os << "{";
            bool first = true;
            for (auto &c : content()) {
                if (first) {
                    first = false;
                } else {
                    os << ",";
                }
                c->render(os, indent + 4);
                skip_line(os, indent + 4);
            }
            if (tail() != nullptr) {
                os << "|";
                tail()->render(os, indent + 2);
            }
            os << "}";
        }
    }

private:
    AstPtrs _content;
    AstPtr _tail;
};

using AstExprListPtr = std::shared_ptr<AstExprList>;
#define AST_EXPR_LIST_CAST(a) std::static_pointer_cast<AstExprList>(a)
#define AST_EXPR_LIST_SPLIT(a, p, dd, tl) \
    auto _##a = AST_EXPR_LIST_CAST(a);    \
    auto p = _##a->position();            \
    auto dd = _##a->content();            \
    auto tl = _##a->tail();

class AstExprTuple : public Ast {
public:
    AstExprTuple(const Position &p, const AstPtrs &c)
        : Ast(AST_EXPR_TUPLE, p), _content(c) {
    }

    AstExprTuple(const AstExprTuple &c)
        : AstExprTuple(c.position(), c.content()) {
    }

    static AstPtr create(const Position &p, const AstPtrs &c) {
        return std::make_shared<AstExprTuple>(p, c);
    }

    static std::shared_ptr<AstExprTuple> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprTuple>(a);
    }

    static std::tuple<Position, std::vector<std::shared_ptr<Ast>>> split(const AstPtr &a) {
        auto a0 = AstExprTuple::cast(a);
        auto p = a0->position();
        auto cc = a0->content();
        return {p, cc};
    }

    AstPtrs content() const {
        return _content;
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent;
        l += 1;
        for (auto e : _content) {
            l = e->approximate_length(l);
            l += 2;
            if (l >= line_length) return l;
        }
        l += 1;
        return l;
    }

    void render(std::ostream &os, text_index_t indent) const {
        if (approximate_length(indent) < line_length) {
            os << "(";
            bool first = true;
            for (auto &c : content()) {
                if (first) {
                    first = false;
                } else {
                    os << ",";
                }
                c->render(os, indent + 4);
            }
            os << ")";
        } else {
            os << "(";
            bool first = true;
            for (auto &c : content()) {
                if (first) {
                    first = false;
                } else {
                    os << ",";
                }
                c->render(os, indent + 4);
                skip_line(os, indent + 4);
            }
            os << ")";
        }
    }

private:
    AstPtrs _content;
};

using AstExprTuplePtr = std::shared_ptr<AstExprTuple>;
#define AST_EXPR_TUPLE_CAST(a) std::static_pointer_cast<AstExprTuple>(a)
#define AST_EXPR_TUPLE_SPLIT(a, p, dd)  \
    auto _##a = AST_EXPR_TUPLE_CAST(a); \
    auto p = _##a->position();          \
    auto dd = _##a->content();

// expression compound statements

class AstExprApplication : public Ast {
public:
    AstExprApplication(const Position &p, const AstPtrs &aa)
        : Ast(AST_EXPR_APPLICATION, p), _arguments(aa) {
    }

    AstExprApplication(const Position &p, const AstPtr &l, const AstPtr &r)
        : Ast(AST_EXPR_APPLICATION, p) {
        _arguments = AstPtrs();
        _arguments.push_back(l);
        _arguments.push_back(r);
    }

    AstExprApplication(const Position &p, const AstPtr &op, const AstPtr &e0,
                       const AstPtr &e1)
        : Ast(AST_EXPR_APPLICATION, p) {
        _arguments = AstPtrs();
        _arguments.push_back(op);
        _arguments.push_back(e0);
        _arguments.push_back(e1);
    }

    AstExprApplication(const Position &p, const AstPtr &c, const AstPtrs &aa)
        : Ast(AST_EXPR_APPLICATION, p) {
        _arguments = AstPtrs();
        _arguments.push_back(c);
        for (auto &a : aa) {
            _arguments.push_back(a);
        }
    }

    AstExprApplication(const AstExprApplication &a)
        : AstExprApplication(a.position(), a.arguments()) {
    }

    static AstPtr create(const Position &p, const AstPtrs &aa) {
        return std::make_shared<AstExprApplication>(p, aa);
    }

    static AstPtr create(const Position &p, const AstPtr &l, const AstPtr &r) {
        return std::make_shared<AstExprApplication>(p, l, r);
    }

    static AstPtr create(const Position &p, const AstPtr &op, const AstPtr &e0,
                         const AstPtr &e1) {
        return std::make_shared<AstExprApplication>(p, op, e0, e1);
    }

    static std::shared_ptr<AstExprApplication> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprApplication>(a);
    }

    static std::tuple<Position, std::vector<std::shared_ptr<Ast>>> split(const AstPtr &a) {
        auto a0 = AstExprApplication::cast(a);
        auto p  = a0->position();
        auto aa = a0->arguments();
        return {p, aa};
    }

    AstPtrs arguments() const {
        return _arguments;
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent;
        l += 1;
        for (auto a : arguments()) {
            l = a->approximate_length(l);
            l += 1;
            if (l >= line_length) return l;
        }
        l += 1;
        return l;
    }

    void render(std::ostream &os, text_index_t indent) const {
        if (approximate_length(indent) < line_length) {
            os << "(";
            bool first = true;
            for (auto &a : arguments()) {
                if (first) {
                    first = false;
                } else {
                    os << " ";
                }
                a->render(os, indent + 4);
            }
            os << ")";
        } else {
            os << "(";
            bool first = true;
            for (auto &a : arguments()) {
                if (first) {
                    first = false;
                } else {
                    os << " ";
                }
                a->render(os, indent + 4);
                skip_line(os, indent + 4);
            }
            os << ")";
        }
    }

private:
    AstPtrs _arguments;
};

using AstExprApplicationPtr = std::shared_ptr<AstExprApplication>;
#define AST_EXPR_APPLICATION_CAST(a) \
    std::static_pointer_cast<AstExprApplication>(a)
#define AST_EXPR_APPLICATION_SPLIT(a, p, aa)  \
    auto _##a = AST_EXPR_APPLICATION_CAST(a); \
    auto p = _##a->position();                \
    auto aa = _##a->arguments();

class AstExprMatch : public Ast {
public:
    AstExprMatch(const Position &p, const AstPtrs &pp, const AstPtr &g,
                 const AstPtr &r)
        : Ast(AST_EXPR_MATCH, p), _patterns(pp), _guard(g), _result(r) {
    }

    AstExprMatch(const AstExprMatch &c)
        : AstExprMatch(c.position(), c.patterns(), c.guard(), c.result()) {
    }

    static AstPtr create(const Position &p, const AstPtrs &pp, const AstPtr &g,
                         const AstPtr &r) {
        return std::make_shared<AstExprMatch>(p, pp, g, r);
    }

    static std::shared_ptr<AstExprMatch> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprMatch>(a);
    }

    static std::tuple<Position, std::vector<std::shared_ptr<Ast>>, std::shared_ptr<Ast>, std::shared_ptr<Ast>> 
        split(const AstPtr &a) {
        auto a0 = AstExprMatch::cast(a);
        auto p = a0->position();
        auto pp = a0->patterns();
        auto g = a0->guard();
        auto r = a0->result();
        return {p, pp, g, r};
    }


    AstPtrs patterns() const {
        return _patterns;
    }

    AstPtr guard() const {
        return _guard;
    }

    AstPtr result() const {
        return _result;
    }

    int arity() const {
        return _patterns.size();
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent;
        l += 1;
        for (auto p : patterns()) {
            l = p->approximate_length(l);
            l += 2;
            if (l >= line_length) return l;
        }
        l += 2;
        l = guard()->approximate_length(l);
        if (l >= line_length) return l;
        l += 2;
        l = result()->approximate_length(l);
        return l;
    }

    void render(std::ostream &os, text_index_t indent) const {
        if (approximate_length(indent) <= line_length) {
            bool first = true;
            for (auto &p : patterns()) {
                if (first) {
                    first = false;
                } else {
                    os << " ";
                }
                p->render(os, indent);
            }
            if (guard()->tag() != AST_EMPTY) {
                os << " ? " << guard();
            }
            os << " -> ";
            result()->render(os, indent + 4);
        } else {
            bool first = true;
            for (auto &p : patterns()) {
                if (first) {
                    first = false;
                } else {
                    os << " ";
                }
                p->render(os, indent);
            }
            skip_line(os, indent);
            if (guard()->tag() != AST_EMPTY) {
                os << "? " << guard();
                skip_line(os, indent);
            }
            os << "-> ";
            skip_line(os, indent + 4);
            result()->render(os, indent + 4);
        }
    }

private:
    AstPtrs _patterns;
    AstPtr _guard;
    AstPtr _result;
};

using AstExprMatchPtr = std::shared_ptr<AstExprMatch>;
#define AST_EXPR_MATCH_CAST(a) std::static_pointer_cast<AstExprMatch>(a)
#define AST_EXPR_MATCH_SPLIT(a, p, pp, g, r) \
    auto _##a = AST_EXPR_MATCH_CAST(a);      \
    auto p = _##a->position();               \
    auto pp = _##a->patterns();              \
    auto g = _##a->guard();                  \
    auto r = _##a->result();

class AstExprBlock : public Ast {
public:
    AstExprBlock(const Position &p, const AstPtrs &mm)
        : Ast(AST_EXPR_BLOCK, p), _matches(mm) {
    }

    AstExprBlock(const Position &p, const AstPtr &m) : Ast(AST_EXPR_BLOCK, p) {
        AstPtrs mm;
        mm.push_back(m);
        _matches = mm;
    }

    AstExprBlock(const AstExprBlock &c)
        : AstExprBlock(c.position(), c.matches()) {
    }

    static AstPtr create(const Position &p, const AstPtr &m) {
        return std::make_shared<AstExprBlock>(p, m);
    }

    static AstPtr create(const Position &p, const AstPtrs &mm) {
        return std::make_shared<AstExprBlock>(p, mm);
    }

    static std::shared_ptr<AstExprBlock> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprBlock>(a);
    }

    static std::tuple<Position, std::vector<std::shared_ptr<Ast>>> split(const AstPtr &a) {
        auto a0 = AstExprBlock::cast(a);
        auto p = a0->position();
        auto mm = a0->matches();
        return {p, mm};
    }

    AstPtrs matches() const {
        return _matches;
    }

    text_index_t arity() const {
        if ((_matches.size() > 0) && (_matches[0]->tag() == AST_EXPR_MATCH)) {
            auto m0 = AST_EXPR_MATCH_CAST(_matches[0]);
            return m0->arity();
        } else {
            return 0;
        }
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent;
        l += 1;
        if (l >= line_length) return l;
        for (auto m : matches()) {
            l = m->approximate_length(l);
            l += 2;
            if (l >= line_length) return l;
        }
        return l;
    }

    void render(std::ostream &os, text_index_t indent) const {
        if (approximate_length(indent) <= line_length) {
            os << "[ ";
            bool first = true;
            for (auto &m : matches()) {
                if (first) {
                    first = false;
                } else {
                    os << " | ";
                }
                m->render(os, indent);
            }
            os << " ]";
        } else {
            os << "[ ";
            bool first = true;
            for (auto &m : matches()) {
                if (first) {
                    first = false;
                } else {
                    skip_line(os, indent);
                    os << "| ";
                }
                m->render(os, indent + 4);
            }
            os << " ]";
        }
    }

private:
    AstPtrs _matches;
};

using AstExprBlockPtr = std::shared_ptr<AstExprBlock>;
#define AST_EXPR_BLOCK_CAST(a) std::static_pointer_cast<AstExprBlock>(a)
#define AST_EXPR_BLOCK_SPLIT(a, p, mm)  \
    auto _##a = AST_EXPR_BLOCK_CAST(a); \
    auto p = _##a->position();          \
    auto mm = _##a->matches();

class AstExprLambda : public Ast {
public:
    AstExprLambda(const Position &p, const AstPtr &m)
        : Ast(AST_EXPR_LAMBDA, p), _match(m) {
    }

    AstExprLambda(const AstExprLambda &c)
        : AstExprLambda(c.position(), c.match()) {
    }

    static AstPtr create(const Position &p, const AstPtr &m) {
        return std::make_shared<AstExprLambda>(p, m);
    }

    static std::shared_ptr<AstExprLambda> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprLambda>(a);
    }

    static std::tuple<Position, std::shared_ptr<Ast>> split(const AstPtr &a) {
        auto a0 = AstExprLambda::cast(a);
        auto p = a0->position();
        auto m = a0->match();
        return {p, m};
    }

    AstPtr match() const {
        return _match;
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent;
        l += 1;
        l = match()->approximate_length(l);
        return l;
    }

    void render(std::ostream &os, text_index_t indent) const {
        // XXX
        os << "\\";
        match()->render(os, indent + 1);
        os << " ";
    }

private:
    AstPtr _match;
};

using AstExprLambdaPtr = std::shared_ptr<AstExprLambda>;
#define AST_EXPR_LAMBDA_CAST(a) std::static_pointer_cast<AstExprLambda>(a)
#define AST_EXPR_LAMBDA_SPLIT(a, p, m)   \
    auto _##a = AST_EXPR_LAMBDA_CAST(a); \
    auto p = _##a->position();           \
    auto m = _##a->match();

class AstExprLet : public Ast {
public:
    AstExprLet(const Position &p, const AstPtrs &ee, const AstPtr &e1,
               const AstPtr e2)
        : Ast(AST_EXPR_LET, p), _lhs(ee), _rhs(e1), _expression(e2) {
    }

    AstExprLet(const AstExprLet &a)
        : AstExprLet(a.position(), a.left_hand_side(), a.right_hand_side(),
                     a.expression()) {
    }

    static AstPtr create(const Position &p, const AstPtrs &ee, const AstPtr &e1,
                         const AstPtr e2) {
        return std::make_shared<AstExprLet>(p, ee, e1, e2);
    }

    static std::shared_ptr<AstExprLet> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprLet>(a);
    }

    static std::tuple<Position, std::vector<std::shared_ptr<Ast>>, std::shared_ptr<Ast>, std::shared_ptr<Ast>> split(const AstPtr &a) {
        auto a0 = AstExprLet::cast(a);
        auto p = a0->position();
        auto l = a0->left_hand_side();
        auto r = a0->right_hand_side();
        auto e = a0->expression();
        return {p, l, r, e};
    }

    AstPtrs left_hand_side() const {
        return _lhs;
    }

    AstPtr right_hand_side() const {
        return _rhs;
    }

    AstPtr expression() const {
        return _expression;
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent;
        if (l >= line_length) return l;
        for (auto p : left_hand_side()) {
            l = p->approximate_length(l);
            l += 2;
            if (l >= line_length) return l;
        }
        l += 3;
        if (l >= line_length) return l;
        l = right_hand_side()->approximate_length(l);
        l += 2;
        if (l >= line_length) return l;
        l = expression()->approximate_length(l);
        return l;
    }

    void render(std::ostream &os, text_index_t indent) const {
        if (approximate_length(indent) <= line_length) {
            os << "(let ";
            for (auto p : left_hand_side()) {
                os << p << " ";
            };
            os << " = " << right_hand_side() << " in " << expression() << ")";
        } else {
            os << "(let ";
            for (auto p : left_hand_side()) {
                p->render(os, indent);
            };
            os << " =";
            skip_line(os, indent + 4);
            right_hand_side()->render(os, indent + 4);
            os << " in ";
            skip_line(os, indent);
            expression()->render(os, indent);
            os << ")";
        }
    }

private:
    AstPtrs _lhs;
    AstPtr _rhs;
    AstPtr _expression;
};

using AstExprLetPtr = std::shared_ptr<AstExprLet>;
#define AST_EXPR_LET_CAST(a) std::static_pointer_cast<AstExprLet>(a)
#define AST_EXPR_LET_SPLIT(a, p, l, r, e) \
    auto _##a = AST_EXPR_LET_CAST(a);     \
    auto p = _##a->position();            \
    auto l = _##a->left_hand_side();      \
    auto r = _##a->right_hand_side();     \
    auto e = _##a->expression();

class AstExprTry : public Ast {
public:
    AstExprTry(const Position &p, const AstPtr &e0, const AstPtr &e1)
        : Ast(AST_EXPR_TRY, p), _try(e0), _catch(e1) {
    }

    AstExprTry(const AstExprTry &a)
        : AstExprTry(a.position(), a.try0(), a.catch0()) {
    }

    static AstPtr create(const Position &p, const AstPtr &e0,
                         const AstPtr &e1) {
        return std::make_shared<AstExprTry>(p, e0, e1);
    }

    static std::shared_ptr<AstExprTry> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprTry>(a);
    }

    static std::tuple<Position, std::shared_ptr<Ast>, std::shared_ptr<Ast>> split(const AstPtr &a) {
        auto a0 = AstExprTry::cast(a);
        auto p = a0->position();
        auto t = a0->try0();
        auto c = a0->catch0();
        return {p, t, c};
    }

    AstPtr try0() const {
        return _try;
    }

    AstPtr catch0() const {
        return _catch;
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent;
        l += 4;
        if (l >= line_length) return l;
        l = try0()->approximate_length(l);
        l += 8;
        if (l >= line_length) return l;
        l = catch0()->approximate_length(l);
        l += 1;
        return l;
    }

    void render(std::ostream &os, text_index_t indent) const {
        if (approximate_length(indent) <= line_length) {
            os << "try (";
            try0()->render(os, indent + 4);
            os << ") catch (";
            catch0()->render(os, indent + 4);
            os << ")";
        } else {
            os << "try (";
            skip_line(os, indent + 4);
            try0()->render(os, indent + 4);
            skip_line(os, indent);
            os << ") catch (";
            skip_line(os, indent + 4);
            catch0()->render(os, indent + 4);
            skip_line(os, indent);
            os << ")";
        }
    }

private:
    AstPtr _try;
    AstPtr _catch;
};

using AstExprTryPtr = std::shared_ptr<AstExprTry>;
#define AST_EXPR_TRY_CAST(a) std::static_pointer_cast<AstExprTry>(a)
#define AST_EXPR_TRY_SPLIT(a, p, t, c) \
    auto _##a = AST_EXPR_TRY_CAST(a);  \
    auto p = _##a->position();         \
    auto t = _##a->try0();             \
    auto c = _##a->catch0();

class AstExprThrow : public Ast {
public:
    AstExprThrow(const Position &p, const AstPtr &e0)
        : Ast(AST_EXPR_THROW, p), _throw(e0) {
    }

    AstExprThrow(const AstExprThrow &a)
        : AstExprThrow(a.position(), a.throw0()) {
    }

    static AstPtr create(const Position &p, const AstPtr &e0) {
        return std::make_shared<AstExprThrow>(p, e0);
    }

    static std::shared_ptr<AstExprThrow> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprThrow>(a);
    }

    static std::tuple<Position, std::shared_ptr<Ast>> split(const AstPtr &a) {
        auto a0 = AstExprThrow::cast(a);
        auto p = a0->position();
        auto t = a0->throw0();
        return {p, t};
    }

    AstPtr throw0() const {
        return _throw;
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent;
        l += 6;
        l = throw0()->approximate_length(l);
        return l;
    }

    void render(std::ostream &os, text_index_t indent) const {
        if (approximate_length(indent) <= line_length) {
            os << "throw (" << throw0() << ")";
        } else {
            os << "throw (";
            skip_line(os, indent + 4);
            throw0()->render(os, indent + 4);
            skip_line(os, indent);
            os << ")";
            skip_line(os, indent);
        }
    }

private:
    AstPtr _throw;
};

using AstExprThrowPtr = std::shared_ptr<AstExprThrow>;
#define AST_EXPR_THROW_CAST(a) std::static_pointer_cast<AstExprThrow>(a)
#define AST_EXPR_THROW_SPLIT(a, p, e)   \
    auto _##a = AST_EXPR_THROW_CAST(a); \
    auto p = _##a->position();          \
    auto e = _##a->throw0();

class AstExprIf : public Ast {
public:
    AstExprIf(const Position &p, const AstPtr &e0, const AstPtr &e1,
              const AstPtr &e2)
        : Ast(AST_EXPR_IF, p), _if(e0), _then(e1), _else(e2) {
    }

    AstExprIf(const AstExprIf &a)
        : AstExprIf(a.position(), a.if0(), a.then0(), a.else0()) {
    }

    static AstPtr create(const Position &p, const AstPtr &e0, const AstPtr &e1,
                         const AstPtr &e2) {
        return std::make_shared<AstExprIf>(p, e0, e1, e2);
    }

    static std::shared_ptr<AstExprIf> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprIf>(a);
    }

    static std::tuple<Position, std::shared_ptr<Ast>, std::shared_ptr<Ast>, std::shared_ptr<Ast>> split(const AstPtr &a) {
        auto a0 = AstExprIf::cast(a);
        auto p = a0->position();
        auto i = a0->if0();
        auto t = a0->then0();
        auto e = a0->else0();
        return {p, i, t, e};
    }

    AstPtr if0() const {
        return _if;
    }

    AstPtr then0() const {
        return _then;
    }

    AstPtr else0() const {
        return _else;
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent;
        l += 3;
        l = if0()->approximate_length(l);
        l += 6;
        if (l >= line_length) return l;
        l = then0()->approximate_length(l);
        l += 6;
        if (l >= line_length) return l;
        l = else0()->approximate_length(l);
        l += 2;
        return l;
    }

    void render(std::ostream &os, text_index_t indent) const {
        if (approximate_length(indent) <= line_length) {
            os << "if ";
            if0()->render(os, indent);
            os << " then ";
            then0()->render(os, indent);
            os << " else ";
            then0()->render(os, indent);
        } else {
            os << "if";
            skip_line(os, indent + 4);
            if0()->render(os, indent + 4);
            skip_line(os, indent);
            os << "then";
            skip_line(os, indent + 4);
            then0()->render(os, indent + 4);
            skip_line(os, indent);
            os << "else";
            skip_line(os, indent + 4);
            then0()->render(os, indent + 4);
        }
    }

private:
    AstPtr _if;
    AstPtr _then;
    AstPtr _else;
};

using AstExprIfPtr = std::shared_ptr<AstExprIf>;
#define AST_EXPR_IF_CAST(a) std::static_pointer_cast<AstExprIf>(a)
#define AST_EXPR_IF_SPLIT(a, p, i, t, e) \
    auto _##a = AST_EXPR_IF_CAST(a);     \
    auto p = _##a->position();           \
    auto i = _##a->if0();                \
    auto t = _##a->then0();              \
    auto e = _##a->else0();

class AstExprStatement : public Ast {
public:
    AstExprStatement(const Position &p, const AstPtr &e0, const AstPtr &e1)
        : Ast(AST_EXPR_STATEMENT, p), _lhs(e0), _rhs(e1) {
    }

    AstExprStatement(const AstExprStatement &a)
        : AstExprStatement(a.position(), a.lhs(), a.rhs()) {
    }

    static AstPtr create(const Position &p, const AstPtr &e0,
                         const AstPtr &e1) {
        return std::make_shared<AstExprStatement>(p, e0, e1);
    }

    static std::shared_ptr<AstExprStatement> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstExprStatement>(a);
    }

    static std::tuple<Position, std::shared_ptr<Ast>, std::shared_ptr<Ast>> split(const AstPtr &a) {
        auto a0 = AstExprStatement::cast(a);
        auto p = a0->position();
        auto l = a0->lhs();
        auto r = a0->rhs();
        return {p, l, r};
    }

    AstPtr lhs() const {
        return _lhs;
    }

    AstPtr rhs() const {
        return _rhs;
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent;
        l += 3;
        l = lhs()->approximate_length(l);
        l += 2;
        if (l >= line_length) return l;
        l = rhs()->approximate_length(l);
        return l;
    }

    void render(std::ostream &os, text_index_t indent) const {
        if (approximate_length(indent) <= line_length) {
            lhs()->render(os, indent);
            os << "; ";
            rhs()->render(os, indent);
        } else {
            skip_line(os, indent + 4);
            lhs()->render(os, indent + 4);
            os << ";";
            skip_line(os, indent);
            rhs()->render(os, indent + 4);
        }
    }

private:
    AstPtr _lhs;
    AstPtr _rhs;
};

using AstExprStatementPtr = std::shared_ptr<AstExprStatement>;
#define AST_EXPR_STATEMENT_CAST(a) std::static_pointer_cast<AstExprStatement>(a)
#define AST_EXPR_STATEMENT_SPLIT(a, p, l, r) \
    auto _##a = AST_EXPR_STATEMENT_CAST(a);  \
    auto p = _##a->position();               \
    auto l = _##a->lhs();                    \
    auto r = _##a->rhs();

// declarations
class AstDeclNamespace : public Ast {
public:
    AstDeclNamespace(const Position &p, const UnicodeStrings &name,
                     const AstPtrs &c)
        : Ast(AST_DECL_NAMESPACE, p), _name(name), _content(c) {
    }

    AstDeclNamespace(const AstDeclNamespace &c)
        : AstDeclNamespace(c.position(), c.name(), c.content()) {
    }

    static AstPtr create(const Position &p, const UnicodeStrings &name,
                         const AstPtrs &c) {
        return std::make_shared<AstDeclNamespace>(p, name, c);
    }

    static std::shared_ptr<AstDeclNamespace> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstDeclNamespace>(a);
    }

    static std::tuple<Position, std::vector<icu::UnicodeString>, std::vector<std::shared_ptr<Ast>>> split(const AstPtr &a) {
        auto a0 = AstDeclNamespace::cast(a);
        auto p = a0->position();
        auto n = a0->name();
        auto c = a0->content();
        return {p, n, c};
    }

    UnicodeStrings name() const {
        return _name;
    }

    AstPtrs content() const {
        return _content;
    }

    text_index_t approximate_length(text_index_t indent) const {
        return indent;
    }

    void render(std::ostream &os, text_index_t indent) const {
        skip(os, indent);
        os << "namespace ";
        bool first = true;
        for (auto &n : name()) {
            if (!first) os << STRING_DCOLON;
            first = false;
            os << n;
        }
        os << " (" << std::endl;
        for (auto e : content()) {
            e->render(os, indent + 4);
        }
        skip(os, indent);
        os << ")" << std::endl;
    }

private:
    UnicodeStrings _name;
    AstPtrs _content;
};

using AstDeclNamespacePtr = std::shared_ptr<AstDeclNamespace>;
#define AST_DECL_NAMESPACE_CAST(a) std::static_pointer_cast<AstDeclNamespace>(a)
#define AST_DECL_NAMESPACE_SPLIT(a, p, n, dd) \
    auto _##a = AST_DECL_NAMESPACE_CAST(a);   \
    auto p = _##a->position();                \
    auto n = _##a->name();                    \
    auto dd = _##a->content();

class AstDeclData : public Ast {
public:
    AstDeclData(const Position &p, const AstPtrs &nn)
        : Ast(AST_DECL_DATA, p), _names(nn) {
    }

    AstDeclData(const AstDeclData &a) : AstDeclData(a.position(), a.names()) {
    }

    static AstPtr create(const Position &p, const AstPtrs &nn) {
        return std::make_shared<AstDeclData>(p, nn);
    }

    static std::shared_ptr<AstDeclData> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstDeclData>(a);
    }

    static std::tuple<Position, std::vector<std::shared_ptr<Ast>>> split(const AstPtr &a) {
        auto a0 = AstDeclData::cast(a);
        auto p = a0->position();
        auto nn = a0->names();
        return {p, nn};
    }

    AstPtrs names() const {
        return _names;
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent;
        l += 5;
        for (auto n : names()) {
            l = n->approximate_length(l);
            l += 2;
            if (l >= line_length) return l;
        }
        return l;
    }

    void render(std::ostream &os, text_index_t indent) const {
        if (approximate_length(indent) <= line_length) {
            skip(os, indent);
            os << STRING_DATA << " ";
            bool first = true;
            for (auto n : names()) {
                if (first) {
                    first = false;
                } else {
                    os << ", ";
                }
                n->render(os, indent);
            }
            os << std::endl;
        } else {
            skip(os, indent);
            os << STRING_DATA << " ";
            bool first = true;
            for (auto n : names()) {
                if (first) {
                    first = false;
                } else {
                    os << ",";
                    skip_line(os, indent + 4);
                }
                n->render(os, indent + 4);
            }
            os << std::endl;
        }
    }

private:
    AstPtrs _names;
};

using AstDeclDataPtr = std::shared_ptr<AstDeclData>;
#define AST_DECL_DATA_CAST(a) std::static_pointer_cast<AstDeclData>(a)
#define AST_DECL_DATA_SPLIT(a, p, nn)  \
    auto _##a = AST_DECL_DATA_CAST(a); \
    auto p = _##a->position();         \
    auto nn = _##a->names();

class AstDeclDefinition : public Ast {
public:
    AstDeclDefinition(const Position &p, const AstPtr &n, const AstPtr &e)
        : Ast(AST_DECL_DEFINITION, p), _name(n), _expression(e) {
    }

    AstDeclDefinition(const AstDeclDefinition &a)
        : AstDeclDefinition(a.position(), a.name(), a.expression()) {
    }

    static AstPtr create(const Position &p, const AstPtr &n, const AstPtr &e) {
        return std::make_shared<AstDeclDefinition>(p, n, e);
    }

    static std::shared_ptr<AstDeclDefinition> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstDeclDefinition>(a);
    }

    static std::tuple<Position, std::shared_ptr<Ast>, std::shared_ptr<Ast>> split(const AstPtr &a) {
        auto a0 = AstDeclDefinition::cast(a);
        auto p = a0->position();
        auto n = a0->name();
        auto e = a0->expression();
        return {p, n, e};
    }

    AstPtr name() const {
        return _name;
    }

    AstPtr expression() const {
        return _expression;
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent;
        l += 4;
        l = name()->approximate_length(l);
        if (l >= line_length) return l;
        l += 3;
        l = expression()->approximate_length(l);
        return l;
    }

    void render(std::ostream &os, text_index_t indent) const {
        if (approximate_length(indent) <= line_length) {
            skip(os, indent);
            os << STRING_DEF << " ";
            os << name() << " = ";
            os << expression();
            os << std::endl;
        } else {
            skip(os, indent);
            os << STRING_DEF << " ";
            os << name() << " = ";
            skip_line(os, indent + 4);
            expression()->render(os, indent + 4);
            os << std::endl;
        }
    }

private:
    AstPtr _name;
    AstPtr _expression;
};

using AstDeclDefinitionPtr = std::shared_ptr<AstDeclDefinition>;
#define AST_DECL_DEFINITION_CAST(a) \
    std::static_pointer_cast<AstDeclDefinition>(a)
#define AST_DECL_DEFINITION_SPLIT(a, p, n, e) \
    auto _##a = AST_DECL_DEFINITION_CAST(a);  \
    auto p = _##a->position();                \
    auto n = _##a->name();                    \
    auto e = _##a->expression();

class AstDeclValue : public Ast {
public:
    AstDeclValue(const Position &p, const AstPtr &e0, const AstPtr &e1)
        : Ast(AST_DECL_VALUE, p), _name(e0), _expression(e1) {
    }

    AstDeclValue(const AstDeclValue &a)
        : AstDeclValue(a.position(), a.name(), a.expression()) {
    }

    static AstPtr create(const Position &p, const AstPtr &e0,
                         const AstPtr &e1) {
        return std::make_shared<AstDeclValue>(p, e0, e1);
    }

    static std::shared_ptr<AstDeclValue> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstDeclValue>(a);
    }

    static std::tuple<Position, std::shared_ptr<Ast>, std::shared_ptr<Ast>> split(const AstPtr &a) {
        auto a0 = AstDeclValue::cast(a);
        auto p = a0->position();
        auto n = a0->name();
        auto e = a0->expression();
        return {p, n, e};
    }

    AstPtr name() const {
        return _name;
    }

    AstPtr expression() const {
        return _expression;
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent;
        l += 4;
        l = name()->approximate_length(l);
        if (l >= line_length) return l;
        l += 3;
        l = expression()->approximate_length(l);
        return l;
    }

    void render(std::ostream &os, text_index_t indent) const {
        if (approximate_length(indent) <= line_length) {
            skip(os, indent);
            os << STRING_VAL << " ";
            os << name() << " = ";
            os << expression();
            os << std::endl;
        } else {
            skip(os, indent);
            os << STRING_VAL << " ";
            os << name() << " = ";
            skip_line(os, indent + 4);
            expression()->render(os, indent + 4);
            os << std::endl;
        }
    }

private:
    AstPtr _name;
    AstPtr _expression;
};

using AstDeclValuePtr = std::shared_ptr<AstDeclValue>;
#define AST_DECL_VALUE_CAST(a) std::static_pointer_cast<AstDeclValue>(a)
#define AST_DECL_VALUE_SPLIT(a, p, l, r) \
    auto _##a = AST_DECL_VALUE_CAST(a);  \
    auto p = _##a->position();           \
    auto l = _##a->name();               \
    auto r = _##a->expression();

class AstDeclOperator : public Ast {
public:
    AstDeclOperator(const Position &p, const AstPtr &c, const AstPtr &e)
        : Ast(AST_DECL_OPERATOR, p), _combinator(c), _expression(e) {
    }

    AstDeclOperator(const AstDeclOperator &a)
        : AstDeclOperator(a.position(), a.combinator(), a.expression()) {
    }

    static AstPtr create(const Position &p, const AstPtr &c, const AstPtr &e) {
        return std::make_shared<AstDeclOperator>(p, c, e);
    }

    static std::shared_ptr<AstDeclOperator> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstDeclOperator>(a);
    }

    static std::tuple<Position, std::shared_ptr<Ast>, std::shared_ptr<Ast>> split(const AstPtr &a) {
        auto a0 = AstDeclOperator::cast(a);
        auto p = a0->position();
        auto n = a0->combinator();
        auto e = a0->expression();
        return {p, n, e};
    }

    AstPtr combinator() const {
        return _combinator;
    }

    AstPtr expression() const {
        return _expression;
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent;
        l += 4;
        l = combinator()->approximate_length(l);
        if (l >= line_length) return l;
        l += 3;
        l = expression()->approximate_length(l);
        return l;
    }

    void render(std::ostream &os, text_index_t indent) const {
        if (approximate_length(indent) <= line_length) {
            skip(os, indent);
            os << STRING_DEF << " ";
            os << combinator();
            os << " = ";
            os << expression();
            os << std::endl;
        } else {
            skip(os, indent);
            os << STRING_DEF << " ";
            os << combinator();
            os << " = ";
            skip_line(os, indent + 4);
            expression()->render(os, indent + 4);
            os << std::endl;
        }
    }

private:
    AstPtr _combinator;
    AstPtr _expression;
};

using AstDeclOperatorPtr = std::shared_ptr<AstDeclOperator>;
#define AST_DECL_OPERATOR_CAST(a) std::static_pointer_cast<AstDeclOperator>(a)
#define AST_DECL_OPERATOR_SPLIT(a, p, c, e) \
    auto _##a = AST_DECL_OPERATOR_CAST(a);  \
    auto p = _##a->position();              \
    auto c = _##a->combinator();            \
    auto e = _##a->expression();

class AstDeclObject : public Ast {
public:
    AstDeclObject(const Position &p, const AstPtr &n, const AstPtrs &vv,
                  const AstPtrs &ff, const AstPtrs &ee)
        : Ast(AST_DECL_OBJECT, p),
          _name(n),
          _variables(vv),
          _fields(ff),
          _extends(ee) {
    }

    AstDeclObject(const AstDeclObject &a)
        : AstDeclObject(a.position(), a.name(), a.variables(), a.fields(),
                        a.extends()) {
    }

    static AstPtr create(const Position &p, const AstPtr &n, const AstPtrs &vv,
                         const AstPtrs &ff, const AstPtrs &ee) {
        return std::make_shared<AstDeclObject>(p, n, vv, ff, ee);
    }

    static std::shared_ptr<AstDeclObject> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstDeclObject>(a);
    }

    static std::tuple<Position, 
            std::shared_ptr<Ast>,
            std::vector<std::shared_ptr<Ast>>,
            std::vector<std::shared_ptr<Ast>>,
            std::vector<std::shared_ptr<Ast>>
            > split(const AstPtr &a) {
        auto a0 = AstDeclObject::cast(a);
        auto p = a0->position();
        auto n = a0->name();
        auto vv = a0->variables();
        auto ff = a0->fields();
        auto ee = a0->extends();
        return {p, n, vv, ff, ee};
    }

    AstPtr name() const {
        return _name;
    }

    AstPtrs variables() const {
        return _variables;
    }

    AstPtrs fields() const {
        return _fields;
    }

    AstPtrs extends() const {
        return _extends;
    }

    text_index_t approximate_length(text_index_t indent) const {
        return indent;
    }

    void render(std::ostream &os, text_index_t indent) const {
        skip(os, indent);
        os << STRING_OBJECT << " " << name() << " ";
        for (auto v : variables()) {
            v->render(os, indent);
            os << " ";
        }
        if (extends().size() > 0) {
            bool first = true;
            os << " extends ";
            for (auto e : extends()) {
                if (first) {
                    first = false;
                } else {
                    os << ", ";
                }
                e->render(os, indent);
            }
            os << " with ";
        }
        os << "(" << std::endl;
        // skip_line(os, indent+4);
        for (auto f : fields()) {
            f->render(os, indent + 4);
        }
        skip(os, indent);
        os << ")";
        os << std::endl;
    }

private:
    AstPtr _name;
    AstPtrs _variables;
    AstPtrs _fields;
    AstPtrs _extends;
};

using AstDeclObjectPtr = std::shared_ptr<AstDeclObject>;
#define AST_DECL_OBJECT_CAST(a) std::static_pointer_cast<AstDeclObject>(a)
#define AST_DECL_OBJECT_SPLIT(a, p, n, vv, ff, ee) \
    auto _##a = AST_DECL_OBJECT_CAST(a);           \
    auto p = _##a->position();                     \
    auto n = _##a->name();                         \
    auto vv = _##a->variables();                   \
    auto ff = _##a->fields();                      \
    auto ee = _##a->extends();

class AstDirectImport : public Ast {
public:
    AstDirectImport(const Position &p, const icu::UnicodeString &v)
        : Ast(AST_DIRECT_IMPORT, p), _import(v) {
    }

    AstDirectImport(const AstDirectImport &c)
        : AstDirectImport(c.position(), c.import()) {
    }

    static AstPtr create(const Position &p, const icu::UnicodeString &v) {
        return std::make_shared<AstDirectImport>(p, v);
    }

    static std::shared_ptr<AstDirectImport> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstDirectImport>(a);
    }

    static std::tuple<Position, 
            icu::UnicodeString
            > split(const AstPtr &a) {
        auto a0 = AstDirectImport::cast(a);
        auto p = a0->position();
        auto s = a0->import();
        return {p, s};
    }

    icu::UnicodeString import() const {
        return _import;
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent + 6;
        l += _import.length();
        return l;
    }

    void render(std::ostream &os, text_index_t indent) const {
        skip(os, indent);
        os << "import " << import() << std::endl;
    }

private:
    icu::UnicodeString _import;
};

using AstDirectImportPtr = std::shared_ptr<AstDirectImport>;
#define AST_DIRECT_IMPORT_CAST(a) std::static_pointer_cast<AstDirectImport>(a)
#define AST_DIRECT_IMPORT_SPLIT(a, p, i)   \
    auto _##a = AST_DIRECT_IMPORT_CAST(a); \
    auto p = _##a->position();             \
    auto i = _##a->import();

class AstDirectUsing : public Ast {
public:
    AstDirectUsing(const Position &p, const UnicodeStrings &v)
        : Ast(AST_DIRECT_USING, p), _using(v) {
    }

    AstDirectUsing(const AstDirectUsing &c)
        : AstDirectUsing(c.position(), c.using0()) {
    }

    static AstPtr create(const Position &p, const UnicodeStrings &v) {
        return std::make_shared<AstDirectUsing>(p, v);
    }

    static std::shared_ptr<AstDirectUsing> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstDirectUsing>(a);
    }

    static std::tuple<Position, 
            std::vector<icu::UnicodeString>
            > split(const AstPtr &a) {
        auto a0 = AstDirectUsing::cast(a);
        auto p = a0->position();
        auto uu = a0->using0();
        return {p, uu};
    }

    UnicodeStrings using0() const {
        return _using;
    }

    text_index_t approximate_length(text_index_t indent) const {
        text_index_t l = indent;
        for (auto &u : using0()) {
            l += u.length() + 1;
        }
        return l;
    }

    void render(std::ostream &os, text_index_t indent) const {
        skip(os, indent);
        os << "using ";
        bool first = true;
        for (auto &u : using0()) {
            if (!first) os << STRING_DCOLON;
            first = false;
            os << u;
        }
        os << std::endl;
    }

private:
    UnicodeStrings _using;
};

using AstDirectUsingPtr = std::shared_ptr<AstDirectUsing>;
#define AST_DIRECT_USING_CAST(a) std::static_pointer_cast<AstDirectUsing>(a)
#define AST_DIRECT_USING_SPLIT(a, p, u)   \
    auto _##a = AST_DIRECT_USING_CAST(a); \
    auto p = _##a->position();            \
    auto u = _##a->using0();

class AstWrapper : public Ast {
public:
    AstWrapper(const Position &p, const AstPtrs &c)
        : Ast(AST_WRAPPER, p), _content(c) {
    }

    AstWrapper(const AstWrapper &c) : AstWrapper(c.position(), c.content()) {
    }

    static AstPtr create(const Position &p, const AstPtrs &c) {
        return std::make_shared<AstWrapper>(p, c);
    }

    static std::shared_ptr<AstWrapper> cast(const AstPtr &a) {
        return std::static_pointer_cast<AstWrapper>(a);
    }

    static std::tuple<Position, 
            std::vector<std::shared_ptr<Ast>>
            > split(const AstPtr &a) {
        auto a0 = AstWrapper::cast(a);
        auto p = a0->position();
        auto ee = a0->content();
        return {p, ee};
    }

    AstPtrs content() const {
        return _content;
    }

    text_index_t approximate_length(text_index_t indent) const {
        return indent;
    }

    void render(std::ostream &os, text_index_t indent) const {
        for (auto e : content()) {
            e->render(os, indent);
        }
    }

private:
    AstPtrs _content;
};

using AstWrapperPtr = std::shared_ptr<AstWrapper>;
#define AST_WRAPPER_CAST(a) std::static_pointer_cast<AstWrapper>(a)
#define AST_WRAPPER_SPLIT(a, p, dd)  \
    auto _##a = AST_WRAPPER_CAST(a); \
    auto p = _##a->position();       \
    auto dd = _##a->content();

int Ast::compare(const AstPtr &a0, const AstPtr &a1) {
    if ((a0 == nullptr) && (a1 == nullptr)) return true;
    if (a0 == nullptr) return 1;
    if (a1 == nullptr) return -1;
    ast_tag_t t0 = a0->tag();
    ast_tag_t t1 = a1->tag();
    if (t0 < t1) {
        return -1;
    } else if (t1 < t0) {
        return 1;
    } else {
        return Ast::compare_tag(t0, a0, a1);
    }
}

int Ast::compare_asts(const AstPtrs &aa0, const AstPtrs &aa1) {
    int sz0 = aa0.size();
    int sz1 = aa1.size();
    if (sz0 < sz1) {
        return -1;
    } else if (sz1 < sz0) {
        return 1;
    } else {
        int sz = sz0;
        for (int i = 0; i < sz; i++) {
            auto a0 = aa0[i];
            auto a1 = aa1[i];
            int c = Ast::compare(a0, a1);
            if (c != 0) return c;
        }
        return 0;
    }
}

static int compare_text(const icu::UnicodeString &t0,
                        const icu::UnicodeString &t1) {
    return t0.compare(t1);
}

static int compare_texts(const UnicodeStrings &aa0, const UnicodeStrings &aa1) {
    int sz0 = aa0.size();
    int sz1 = aa1.size();
    if (sz0 < sz1) {
        return -1;
    } else if (sz1 < sz0) {
        return 1;
    } else {
        int sz = sz0;
        for (int i = 0; i < sz; i++) {
            auto a0 = aa0[i];
            auto a1 = aa1[i];
            int c = compare_text(a0, a1);
            if (c != 0) return c;
        }
        return 0;
    }
}

int Ast::compare_ast2(const AstPtr &a0, const AstPtr &a1, const AstPtr &a2,
                      const AstPtr &a3) {
    int c = compare(a0, a1);
    if (c != 0) return c;
    return compare(a2, a3);
}

int Ast::compare_ast3(const AstPtr &a0, const AstPtr &a1, const AstPtr &a2,
                      const AstPtr &a3, const AstPtr &a4, const AstPtr &a5) {
    int c = compare(a0, a1);
    if (c != 0) return c;
    c = compare(a2, a3);
    if (c != 0) return c;
    return compare(a4, a5);
}

int Ast::compare_tag(ast_tag_t t, const AstPtr &a0, const AstPtr &a1) {
    int c;

    switch (t) {
        case AST_EMPTY: {
            return 0;
        }
        // literals
        case AST_EXPR_INTEGER: {
            auto [p0, t0] = AstExprInteger::split(a0);
            auto [p1, t1] = AstExprInteger::split(a1);
            return compare_text(t0, t1);
            break;
        }
        case AST_EXPR_HEXINTEGER: {
            auto [p0, t0] = AstExprHexInteger::split(a0);
            auto [p1, t1] = AstExprHexInteger::split(a1);
            return compare_text(t0, t1);
            break;
        }
        case AST_EXPR_FLOAT: {
            auto [p0, t0] = AstExprFloat::split(a0);
            auto [p1, t1] = AstExprFloat::split(a1);
            return compare_text(t0, t1);
            break;
        }
        case AST_EXPR_CHARACTER: {
            auto [p0, t0] = AstExprCharacter::split(a0);
            auto [p1, t1] = AstExprCharacter::split(a1);
            return compare_text(t0, t1);
            break;
        }
        case AST_EXPR_TEXT: {
            auto [p0, t0] = AstExprText::split(a0);
            auto [p1, t1] = AstExprText::split(a1);
            return compare_text(t0, t1);
            break;
        }
        // variables and constants
        case AST_EXPR_VARIABLE: {
            auto [p0, t0] = AstExprVariable::split(a0);
            auto [p1, t1] = AstExprVariable::split(a1);
            return compare_text(t0, t1);
            break;
        }
        case AST_EXPR_WILDCARD: {
            auto [p0, t0] = AstExprWildcard::split(a0);
            auto [p1, t1] = AstExprWildcard::split(a1);
            return compare_text(t0, t1);
            break;
        }
        case AST_EXPR_COMBINATOR: {
            auto [p0, q0, t0] = AstExprCombinator::split(a0);
            auto [p1, q1, t1] = AstExprCombinator::split(a1);
            c = compare_texts(q0, q1);
            if (c != 0) return c;
            return compare_text(t0, t1);
            break;
        }
        case AST_EXPR_OPERATOR: {
            auto [p0, q0, t0] = AstExprOperator::split(a0);
            auto [p1, q1, t1] = AstExprOperator::split(a1);
            c = compare_texts(q0, q1);
            if (c != 0) return c;
            return compare_text(t0, t1);
            break;
        }
        //  list and tuple
        case AST_EXPR_LIST: {
            auto [p0, tt0, tl0] = AstExprList::split(a0);
            auto [p1, tt1, tl1] = AstExprList::split(a1);
            c = compare_asts(tt0, tt1);
            if (c != 0) return c;
            return Ast::compare(tl0, tl1);
            break;
        }
        case AST_EXPR_TUPLE: {
            auto [p0, tt0] = AstExprTuple::split(a0);
            auto [p1, tt1] = AstExprTuple::split(a1);
            return compare_asts(tt0, tt1);
            break;
        }
        // compound statements
        case AST_EXPR_APPLICATION: {
            auto [p0, aa0] = AstExprApplication::split(a0);
            auto [p1, aa1] = AstExprApplication::split(a1);
            return compare_asts(aa0, aa1);
            break;
        }
        case AST_EXPR_LAMBDA: {
            auto [p0, m0] = AstExprLambda::split(a0);
            auto [p1, m1] = AstExprLambda::split(a1);
            return Ast::compare(m0, m1);
            break;
        }
        case AST_EXPR_BLOCK: {
            auto [p0, mm0] = AstExprBlock::split(a0);
            auto [p1, mm1] = AstExprBlock::split(a1);
            return compare_asts(mm0, mm1);
            break;
        }
        case AST_EXPR_MATCH: {
            auto [p0, mm0, g0, e0] = AstExprMatch::split(a0);
            auto [p1, mm1, g1, e1] = AstExprMatch::split(a1);
            c = compare_asts(mm0, mm1);
            if (c != 0) return c;
            return compare_ast2(g0, g1, e0, e1);
            break;
        }
        case AST_EXPR_LET: {
            auto [p0, l0, r0, e0] = AstExprLet::split(a0);
            auto [p1, l1, r1, e1] = AstExprLet::split(a1);
            c = compare_asts(l0, l1);
            if (c != 0) return c;
            return compare_ast2(r0, r1, e0, e1);
            break;
        }
        case AST_EXPR_TAG: {
            auto [p0, e0, t0] = AstExprTag::split(a0);
            auto [p1, e1, t1] = AstExprTag::split(a1);
            return compare_ast2(e0, e1, t0, t1);
            break;
        }
        case AST_EXPR_IF: {
            auto [p0, i0, t0, e0] = AstExprIf::split(a0);
            auto [p1, i1, t1, e1] = AstExprIf::split(a1);
            return compare_ast3(i0, i1, t0, t1, e0, e1);
            break;
        }
        case AST_EXPR_STATEMENT: {
            auto [p0, l0, r0] = AstExprStatement::split(a0);
            auto [p1, l1, r1] = AstExprStatement::split(a1);
            return compare_ast2(r0, r1, l0, l1);
            break;
        }
        case AST_EXPR_TRY: {
            auto [p0, t0, c0] = AstExprTry::split(a0);
            auto [p1, t1, c1] = AstExprTry::split(a1);
            // XXX
            break;
        }
        case AST_EXPR_THROW: {
            auto [p0, e0] = AstExprThrow::split(a0);
            auto [p1, e1] = AstExprThrow::split(a1);
            // XXX
            break;
        }
        // directives
        case AST_DIRECT_IMPORT: {
            auto [p0, n0] = AstDirectImport::split(a0);
            auto [p1, n1] = AstDirectImport::split(a1);
            return compare_text(n0, n1);
            break;
        }
        case AST_DIRECT_USING: {
            auto [p0, pp0] = AstDirectUsing::split(a0);
            auto [p1, pp1] = AstDirectUsing::split(a1);
            return compare_texts(pp0, pp1);
            break;
        }
        // declarations
        case AST_DECL_DATA: {
            auto [p0, nn0] = AstDeclData::split(a0);
            auto [p1, nn1] = AstDeclData::split(a1);
            return compare_asts(nn0, nn1);
            break;
        }
        case AST_DECL_DEFINITION: {
            auto [p0, n0, e0] = AstDeclDefinition::split(a0);
            auto [p1, n1, e1] = AstDeclDefinition::split(a1);
            return compare_ast2(n0, n1, e0, e1);
            break;
        }
        case AST_DECL_OPERATOR: {
            auto [p0, c0, e0] = AstDeclOperator::split(a0);
            auto [p1, c1, e1] = AstDeclOperator::split(a1);
            return compare_ast2(c0, c1, e0, e1);
            break;
        }
        case AST_DECL_OBJECT: {
            auto [p0, c0, vv0, ff0, ee0] = AstDeclObject::split(a0);
            auto [p1, c1, vv1, ff1, ee1] = AstDeclObject::split(a1);
            c = Ast::compare(c0, c1);
            if (c != 0) return c;
            c = compare_asts(vv0, vv1);
            if (c != 0) return c;
            c = compare_asts(ff0, ff1);
            if (c != 0) return c;
            return compare_asts(ee0, ee1);
            break;
        }
        case AST_DECL_NAMESPACE: {
            auto [p0, nn0, dd0] = AstDeclNamespace::split(a0);
            auto [p1, nn1, dd1] = AstDeclNamespace::split(a1);
            c = compare_texts(nn0, nn1);
            if (c != 0) return c;
            return compare_asts(dd0, dd1);
            break;
        }
        // wrapper
        case AST_WRAPPER: {
            auto [p0, dd0] = AstWrapper::split(a0);
            auto [p1, dd1] = AstWrapper::split(a1);
            return compare_asts(dd0, dd1);
            break;
        }
        case AST_DECL_VALUE: {
            auto [p0, l0, r0] = AstDeclValue::split(a0);
            auto [p1, l1, r1] = AstDeclValue::split(a1);
            return compare_ast2(l0, l1, r0, r1);
            break;
        }
        default:
            PANIC("compare ast failed");
    }
    return 0;
}
