export module ast;

import<memory>;
import<sstream>;
import<vector>;
import<set>;

import utils;
import constants;
import position;
import error;

// AST tags are enumerated as a manner to implement switches conveniently

typedef enum {
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
} ast_tag_t;

class Ast;
typedef std::shared_ptr<Ast> AstPtr;

int compare_ast(const AstPtr &a0, const AstPtr &a1);

class Ast {
public:
    Ast(ast_tag_t t, const Position &p) : _tag(t), _position(p) {}

    virtual ~Ast() {  // keep the compiler happy
    }

    Position position() const { return _position; }

    ast_tag_t tag() const { return _tag; }

    friend std::ostream &operator<<(std::ostream &os, const AstPtr &a) {
        a->render(os, 0);
        return os;
    }

    static uint_t line_length;

    virtual uint_t approximate_length(uint_t indent) const = 0;

    void skip(std::ostream &os, uint_t indent) const {
        for (uint_t i = 0; i < indent; ++i) {
            os << " ";
        }
    }
    void skip_line(std::ostream &os, uint_t indent) const {
        os << std::endl;
        skip(os, indent);
    }

    virtual void render(std::ostream &os, uint_t indent) const = 0;

    virtual icu::UnicodeString to_text() const {
        std::stringstream ss;
        render(ss, Ast::line_length);
        icu::UnicodeString u(ss.str().c_str());
        return u;
    }

private:
    ast_tag_t _tag;
    Position _position;
};

struct EqualAstPtr {
    bool operator()(const AstPtr &a0, const AstPtr &a1) const {
        return (compare_ast(a0, a1) == 0);
    }
};

struct LessAstPtr {
    bool operator()(const AstPtr &a0, const AstPtr &a1) const {
        return (compare_ast(a0, a1) == -1);
    }
};

typedef std::vector<AstPtr> AstPtrs;
typedef std::set<AstPtr, LessAstPtr> AstPtrSet;

// filler for optional cases

class AstEmpty : public Ast {
public:
    AstEmpty() : Ast(AST_EMPTY, Position(icu::UnicodeString(""), 0, 0)) {}

    AstEmpty(const AstEmpty &) : AstEmpty() {}

    static AstPtr create() { return AstPtr(new AstEmpty()); }

    uint_t approximate_length(uint_t indent) const { return indent; }

    void render(std::ostream &os, uint_t) const { os << "(XX)"; }
};

typedef std::shared_ptr<AstEmpty> AstEmptyPtr;
#define AST_EMPTY_CAST(a) std::static_pointer_cast<AstEmptyPtr>(a)

// the atom class is a convenience base class for simple text representations

class AstAtom : public Ast {
public:
    AstAtom(ast_tag_t t, const Position &p, const icu::UnicodeString &n)
        : Ast(t, p), _text(n) {}

    icu::UnicodeString text() const { return _text; }

    uint_t approximate_length(uint_t indent) const {
        return (indent + text().length());
    }

    void render(std::ostream &os, uint_t) const { os << text(); }

private:
    icu::UnicodeString _text;
};

// expression literals

class AstExprInteger : public AstAtom {
public:
    AstExprInteger(const Position &p, const icu::UnicodeString &text)
        : AstAtom(AST_EXPR_INTEGER, p, text){};

    AstExprInteger(const AstExprInteger &l)
        : AstExprInteger(l.position(), l.text()) {}

    static AstPtr create(const Position &p, const icu::UnicodeString &text) {
        return AstPtr(new AstExprInteger(p, text));
    }
};

typedef std::shared_ptr<AstExprInteger> AstExprIntegerPtr;
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
        : AstExprHexInteger(l.position(), l.text()) {}

    static AstPtr create(const Position &p, const icu::UnicodeString &hex) {
        return AstPtr(new AstExprHexInteger(p, hex));
    }
};

typedef std::shared_ptr<AstExprHexInteger> AstExprHexIntegerPtr;
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

    AstExprFloat(const AstExprFloat &l)
        : AstExprFloat(l.position(), l.text()) {}

    static AstPtr create(const Position &p, const icu::UnicodeString &text) {
        return AstPtr(new AstExprFloat(p, text));
    }
};

typedef std::shared_ptr<AstExprFloat> AstExprFloatPtr;
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
        : AstExprCharacter(l.position(), l.text()) {}

    static AstPtr create(const Position &p, const icu::UnicodeString &text) {
        return AstPtr(new AstExprCharacter(p, text));
    }
};

typedef std::shared_ptr<AstExprCharacter> AstExprCharacterPtr;
#define AST_EXPR_CHARACTER_CAST(a) std::static_pointer_cast<AstExprCharacter>(a)
#define AST_EXPR_CHARACTER_SPLIT(a, p, t)   \
    auto _##a = AST_EXPR_CHARACTER_CAST(a); \
    auto p = _##a->position();              \
    auto t = _##a->text();

class AstExprText : public AstAtom {
public:
    AstExprText(const Position &p, const icu::UnicodeString &text)
        : AstAtom(AST_EXPR_TEXT, p, text){};

    AstExprText(const AstExprFloat &l) : AstExprText(l.position(), l.text()) {}

    static AstPtr create(const Position &p, const icu::UnicodeString &text) {
        return AstPtr(new AstExprText(p, text));
    }
};

typedef std::shared_ptr<AstExprText> AstExprTextPtr;
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
        : AstExprVariable(l.position(), l.text()) {}

    static AstPtr create(const Position &p, const icu::UnicodeString &text) {
        return AstPtr(new AstExprVariable(p, text));
    }
};

typedef std::shared_ptr<AstExprVariable> AstExprVariablePtr;
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
        : AstExprWildcard(l.position(), l.text()) {}

    static AstPtr create(const Position &p, const icu::UnicodeString &text) {
        return AstPtr(new AstExprWildcard(p, text));
    }
};

typedef std::shared_ptr<AstExprWildcard> AstExprWildcardPtr;
#define AST_EXPR_WILDCARD_CAST(a) std::static_pointer_cast<AstExprWildcard>(a)
#define AST_EXPR_WILDCARD_SPLIT(a, p, t)   \
    auto _##a = AST_EXPR_WILDCARD_CAST(a); \
    auto p = _##a->position();             \
    auto t = _##a->text();

class AstExprTag : public Ast {
public:
    AstExprTag(const Position &p, const AstPtr &e, const AstPtr &t)
        : Ast(AST_EXPR_TAG, p), _expression(e), _tag(t) {}

    AstExprTag(const AstExprTag &a)
        : AstExprTag(a.position(), a.expression(), a.tag()) {}

    static AstPtr create(const Position &p, const AstPtr &e, const AstPtr &t) {
        return AstPtr(new AstExprTag(p, e, t));
    }

    AstPtr expression() const { return _expression; }

    AstPtr tag() const { return _tag; }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent;
        l = expression()->approximate_length(l);
        l += 2;
        if (l >= line_length) return l;
        l = tag()->approximate_length(l);
        return l;
    }

    void render(std::ostream &os, uint_t indent) const {
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

typedef std::shared_ptr<AstExprTag> AstExprTagPtr;
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
        : AstExprCombinator(c.position(), c.path(), c.combinator()) {}

    static AstPtr create(const Position &p, const icu::UnicodeString &c) {
        return AstPtr(new AstExprCombinator(p, c));
    }

    static AstPtr create(const Position &p, const UnicodeStrings &pp,
                         const icu::UnicodeString &c) {
        return AstPtr(new AstExprCombinator(p, pp, c));
    }

    static AstPtr create(const Position &p, const icu::UnicodeString &n,
                         const icu::UnicodeString &c) {
        return AstPtr(new AstExprCombinator(p, n, c));
    }

    UnicodeStrings path() const { return _path; }

    icu::UnicodeString combinator() const { return _combinator; }

    icu::UnicodeString to_text() const override {
        icu::UnicodeString str = "";  // XXX: change to some other datatype
        for (auto &p : _path) {
            str += p;
            str += STRING_DCOLON;
        }
        str += _combinator;
        return str;
    }

    uint_t approximate_length(uint_t indent) const override {
        uint_t l = indent;
        for (auto &p : _path) {
            l += p.length() + 1;
        }
        l += _combinator.length();
        return l;
    }

    void render(std::ostream &os, uint_t) const override { os << to_text(); }

    bool is_local() const {
        return ((_path.size() > 0) && (_path.back() == STRING_LOCAL));
    }

private:
    UnicodeStrings _path;
    icu::UnicodeString _combinator;
};

typedef std::shared_ptr<AstExprCombinator> AstExprCombinatorPtr;
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
        : AstExprOperator(c.position(), c.path(), c.combinator()) {}

    static AstPtr create(const Position &p, const UnicodeStrings &pp,
                         const icu::UnicodeString &c) {
        return AstPtr(new AstExprOperator(p, pp, c));
    }

    static AstPtr create(const Position &p, const icu::UnicodeString &n,
                         const icu::UnicodeString &c) {
        return AstPtr(new AstExprOperator(p, n, c));
    }

    UnicodeStrings path() const { return _path; }

    icu::UnicodeString combinator() const { return _combinator; }

    icu::UnicodeString text() const {
        icu::UnicodeString str = "";
        for (auto &p : _path) {
            str += p;
            str += STRING_DCOLON;
        }
        str += _combinator;
        return str;
    }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent;
        for (auto &p : _path) {
            l += p.length();
            l += 1;
            if (l >= line_length) return l;
        }
        l += _combinator.length();
        return l;
    }

    void render(std::ostream &os, uint_t) const { os << text(); }

private:
    UnicodeStrings _path;
    icu::UnicodeString _combinator;
};

typedef std::shared_ptr<AstExprOperator> AstExprOperatorPtr;
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
        : Ast(AST_EXPR_LIST, p), _content(c), _tail(nullptr) {}

    AstExprList(const Position &p, const AstPtrs &c, const AstPtr &tl)
        : Ast(AST_EXPR_LIST, p), _content(c), _tail(tl) {}

    AstExprList(const AstExprList &c)
        : AstExprList(c.position(), c.content(), c.tail()) {}

    static AstPtr create(const Position &p, const AstPtrs &c) {
        return AstPtr(new AstExprList(p, c));
    }

    static AstPtr create(const Position &p, const AstPtrs &c,
                         const AstPtr &tl) {
        return AstPtr(new AstExprList(p, c, tl));
    }

    AstPtrs content() const { return _content; }

    AstPtr tail() const { return _tail; }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent;
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

    void render(std::ostream &os, uint_t indent) const {
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

typedef std::shared_ptr<AstExprList> AstExprListPtr;
#define AST_EXPR_LIST_CAST(a) std::static_pointer_cast<AstExprList>(a)
#define AST_EXPR_LIST_SPLIT(a, p, dd, tl) \
    auto _##a = AST_EXPR_LIST_CAST(a);    \
    auto p = _##a->position();            \
    auto dd = _##a->content();            \
    auto tl = _##a->tail();

class AstExprTuple : public Ast {
public:
    AstExprTuple(const Position &p, const AstPtrs &c)
        : Ast(AST_EXPR_TUPLE, p), _content(c) {}

    AstExprTuple(const AstExprTuple &c)
        : AstExprTuple(c.position(), c.content()) {}

    static AstPtr create(const Position &p, const AstPtrs &c) {
        return AstPtr(new AstExprTuple(p, c));
    }

    AstPtrs content() const { return _content; }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent;
        l += 1;
        for (auto e : _content) {
            l = e->approximate_length(l);
            l += 2;
            if (l >= line_length) return l;
        }
        l += 1;
        return l;
    }

    void render(std::ostream &os, uint_t indent) const {
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

typedef std::shared_ptr<AstExprTuple> AstExprTuplePtr;
#define AST_EXPR_TUPLE_CAST(a) std::static_pointer_cast<AstExprTuple>(a)
#define AST_EXPR_TUPLE_SPLIT(a, p, dd)  \
    auto _##a = AST_EXPR_TUPLE_CAST(a); \
    auto p = _##a->position();          \
    auto dd = _##a->content();

// expression compound statements

class AstExprApplication : public Ast {
public:
    AstExprApplication(const Position &p, const AstPtrs &aa)
        : Ast(AST_EXPR_APPLICATION, p), _arguments(aa) {}

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
        : AstExprApplication(a.position(), a.arguments()) {}

    static AstPtr create(const Position &p, const AstPtrs &aa) {
        return AstPtr(new AstExprApplication(p, aa));
    }

    static AstPtr create(const Position &p, const AstPtr &l, const AstPtr &r) {
        return AstPtr(new AstExprApplication(p, l, r));
    }

    static AstPtr create(const Position &p, const AstPtr &op, const AstPtr &e0,
                         const AstPtr &e1) {
        return AstPtr(new AstExprApplication(p, op, e0, e1));
    }

    AstPtrs arguments() const { return _arguments; }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent;
        l += 1;
        for (auto a : arguments()) {
            l = a->approximate_length(l);
            l += 1;
            if (l >= line_length) return l;
        }
        l += 1;
        return l;
    }

    void render(std::ostream &os, uint_t indent) const {
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

typedef std::shared_ptr<AstExprApplication> AstExprApplicationPtr;
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
        : Ast(AST_EXPR_MATCH, p), _patterns(pp), _guard(g), _result(r) {}

    AstExprMatch(const AstExprMatch &c)
        : AstExprMatch(c.position(), c.patterns(), c.guard(), c.result()) {}

    static AstPtr create(const Position &p, const AstPtrs &pp, const AstPtr &g,
                         const AstPtr &r) {
        return AstPtr(new AstExprMatch(p, pp, g, r));
    }

    AstPtrs patterns() const { return _patterns; }

    AstPtr guard() const { return _guard; }

    AstPtr result() const { return _result; }

    uint_t arity() const { return _patterns.size(); }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent;
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

    void render(std::ostream &os, uint_t indent) const {
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

typedef std::shared_ptr<AstExprMatch> AstExprMatchPtr;
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
        : Ast(AST_EXPR_BLOCK, p), _matches(mm) {}

    AstExprBlock(const Position &p, const AstPtr &m) : Ast(AST_EXPR_BLOCK, p) {
        AstPtrs mm;
        mm.push_back(m);
        _matches = mm;
    }

    AstExprBlock(const AstExprBlock &c)
        : AstExprBlock(c.position(), c.matches()) {}

    static AstPtr create(const Position &p, const AstPtr &m) {
        return AstPtr(new AstExprBlock(p, m));
    }

    static AstPtr create(const Position &p, const AstPtrs &mm) {
        return AstPtr(new AstExprBlock(p, mm));
    }

    AstPtrs matches() const { return _matches; }

    uint_t arity() const {
        if ((_matches.size() > 0) && (_matches[0]->tag() == AST_EXPR_MATCH)) {
            auto m0 = AST_EXPR_MATCH_CAST(_matches[0]);
            return m0->arity();
        } else {
            return 0;
        }
    }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent;
        l += 1;
        if (l >= line_length) return l;
        for (auto m : matches()) {
            l = m->approximate_length(l);
            l += 2;
            if (l >= line_length) return l;
        }
        return l;
    }

    void render(std::ostream &os, uint_t indent) const {
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

typedef std::shared_ptr<AstExprBlock> AstExprBlockPtr;
#define AST_EXPR_BLOCK_CAST(a) std::static_pointer_cast<AstExprBlock>(a)
#define AST_EXPR_BLOCK_SPLIT(a, p, mm)  \
    auto _##a = AST_EXPR_BLOCK_CAST(a); \
    auto p = _##a->position();          \
    auto mm = _##a->matches();

class AstExprLambda : public Ast {
public:
    AstExprLambda(const Position &p, const AstPtr &m)
        : Ast(AST_EXPR_LAMBDA, p), _match(m) {}

    AstExprLambda(const AstExprLambda &c)
        : AstExprLambda(c.position(), c.match()) {}

    static AstPtr create(const Position &p, const AstPtr &m) {
        return AstPtr(new AstExprLambda(p, m));
    }

    AstPtr match() const { return _match; }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent;
        l += 1;
        l = match()->approximate_length(l);
        return l;
    }

    void render(std::ostream &os, uint_t indent) const {
        // XXX
        os << "\\";
        match()->render(os, indent + 1);
        os << " ";
    }

private:
    AstPtr _match;
};

typedef std::shared_ptr<AstExprLambda> AstExprLambdaPtr;
#define AST_EXPR_LAMBDA_CAST(a) std::static_pointer_cast<AstExprLambda>(a)
#define AST_EXPR_LAMBDA_SPLIT(a, p, m)   \
    auto _##a = AST_EXPR_LAMBDA_CAST(a); \
    auto p = _##a->position();           \
    auto m = _##a->match();

class AstExprLet : public Ast {
public:
    AstExprLet(const Position &p, const AstPtrs &ee, const AstPtr &e1,
               const AstPtr e2)
        : Ast(AST_EXPR_LET, p), _lhs(ee), _rhs(e1), _expression(e2) {}

    AstExprLet(const AstExprLet &a)
        : AstExprLet(a.position(), a.left_hand_side(), a.right_hand_side(),
                     a.expression()) {}

    static AstPtr create(const Position &p, const AstPtrs &ee, const AstPtr &e1,
                         const AstPtr e2) {
        return AstPtr(new AstExprLet(p, ee, e1, e2));
    }

    AstPtrs left_hand_side() const { return _lhs; }

    AstPtr right_hand_side() const { return _rhs; }

    AstPtr expression() const { return _expression; }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent;
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

    void render(std::ostream &os, uint_t indent) const {
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

typedef std::shared_ptr<AstExprLet> AstExprLetPtr;
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
        : Ast(AST_EXPR_TRY, p), _try(e0), _catch(e1) {}

    AstExprTry(const AstExprTry &a)
        : AstExprTry(a.position(), a.try0(), a.catch0()) {}

    static AstPtr create(const Position &p, const AstPtr &e0,
                         const AstPtr &e1) {
        return AstPtr(new AstExprTry(p, e0, e1));
    }

    AstPtr try0() const { return _try; }

    AstPtr catch0() const { return _catch; }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent;
        l += 4;
        if (l >= line_length) return l;
        l = try0()->approximate_length(l);
        l += 8;
        if (l >= line_length) return l;
        l = catch0()->approximate_length(l);
        l += 1;
        return l;
    }

    void render(std::ostream &os, uint_t indent) const {
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

typedef std::shared_ptr<AstExprTry> AstExprTryPtr;
#define AST_EXPR_TRY_CAST(a) std::static_pointer_cast<AstExprTry>(a)
#define AST_EXPR_TRY_SPLIT(a, p, t, c) \
    auto _##a = AST_EXPR_TRY_CAST(a);  \
    auto p = _##a->position();         \
    auto t = _##a->try0();             \
    auto c = _##a->catch0();

class AstExprThrow : public Ast {
public:
    AstExprThrow(const Position &p, const AstPtr &e0)
        : Ast(AST_EXPR_THROW, p), _throw(e0) {}

    AstExprThrow(const AstExprThrow &a)
        : AstExprThrow(a.position(), a.throw0()) {}

    static AstPtr create(const Position &p, const AstPtr &e0) {
        return AstPtr(new AstExprThrow(p, e0));
    }

    AstPtr throw0() const { return _throw; }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent;
        l += 6;
        l = throw0()->approximate_length(l);
        return l;
    }

    void render(std::ostream &os, uint_t indent) const {
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

typedef std::shared_ptr<AstExprThrow> AstExprThrowPtr;
#define AST_EXPR_THROW_CAST(a) std::static_pointer_cast<AstExprThrow>(a)
#define AST_EXPR_THROW_SPLIT(a, p, e)   \
    auto _##a = AST_EXPR_THROW_CAST(a); \
    auto p = _##a->position();          \
    auto e = _##a->throw0();

class AstExprIf : public Ast {
public:
    AstExprIf(const Position &p, const AstPtr &e0, const AstPtr &e1,
              const AstPtr &e2)
        : Ast(AST_EXPR_IF, p), _if(e0), _then(e1), _else(e2) {}

    AstExprIf(const AstExprIf &a)
        : AstExprIf(a.position(), a.if0(), a.then0(), a.else0()) {}

    static AstPtr create(const Position &p, const AstPtr &e0, const AstPtr &e1,
                         const AstPtr &e2) {
        return AstPtr(new AstExprIf(p, e0, e1, e2));
    }

    AstPtr if0() const { return _if; }

    AstPtr then0() const { return _then; }

    AstPtr else0() const { return _else; }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent;
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

    void render(std::ostream &os, uint_t indent) const {
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

typedef std::shared_ptr<AstExprIf> AstExprIfPtr;
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
        : Ast(AST_EXPR_STATEMENT, p), _lhs(e0), _rhs(e1) {}

    AstExprStatement(const AstExprStatement &a)
        : AstExprStatement(a.position(), a.lhs(), a.rhs()) {}

    static AstPtr create(const Position &p, const AstPtr &e0,
                         const AstPtr &e1) {
        return AstPtr(new AstExprStatement(p, e0, e1));
    }

    AstPtr lhs() const { return _lhs; }

    AstPtr rhs() const { return _rhs; }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent;
        l += 3;
        l = lhs()->approximate_length(l);
        l += 2;
        if (l >= line_length) return l;
        l = rhs()->approximate_length(l);
        return l;
    }

    void render(std::ostream &os, uint_t indent) const {
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

typedef std::shared_ptr<AstExprStatement> AstExprStatementPtr;
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
        : Ast(AST_DECL_NAMESPACE, p), _name(name), _content(c) {}

    AstDeclNamespace(const AstDeclNamespace &c)
        : AstDeclNamespace(c.position(), c.name(), c.content()) {}

    static AstPtr create(const Position &p, const UnicodeStrings &name,
                         const AstPtrs &c) {
        return AstPtr(new AstDeclNamespace(p, name, c));
    }

    UnicodeStrings name() const { return _name; }

    AstPtrs content() const { return _content; }

    uint_t approximate_length(uint_t indent) const { return indent; }

    void render(std::ostream &os, uint_t indent) const {
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

typedef std::shared_ptr<AstDeclNamespace> AstDeclNamespacePtr;
#define AST_DECL_NAMESPACE_CAST(a) std::static_pointer_cast<AstDeclNamespace>(a)
#define AST_DECL_NAMESPACE_SPLIT(a, p, n, dd) \
    auto _##a = AST_DECL_NAMESPACE_CAST(a);   \
    auto p = _##a->position();                \
    auto n = _##a->name();                    \
    auto dd = _##a->content();

class AstDeclData : public Ast {
public:
    AstDeclData(const Position &p, const AstPtrs &nn)
        : Ast(AST_DECL_DATA, p), _names(nn) {}

    AstDeclData(const AstDeclData &a) : AstDeclData(a.position(), a.names()) {}

    static AstPtr create(const Position &p, const AstPtrs &nn) {
        return AstPtr(new AstDeclData(p, nn));
    }

    AstPtrs names() const { return _names; }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent;
        l += 5;
        for (auto n : names()) {
            l = n->approximate_length(l);
            l += 2;
            if (l >= line_length) return l;
        }
        return l;
    }

    void render(std::ostream &os, uint_t indent) const {
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

typedef std::shared_ptr<AstDeclData> AstDeclDataPtr;
#define AST_DECL_DATA_CAST(a) std::static_pointer_cast<AstDeclData>(a)
#define AST_DECL_DATA_SPLIT(a, p, nn)  \
    auto _##a = AST_DECL_DATA_CAST(a); \
    auto p = _##a->position();         \
    auto nn = _##a->names();

class AstDeclDefinition : public Ast {
public:
    AstDeclDefinition(const Position &p, const AstPtr &n, const AstPtr &e)
        : Ast(AST_DECL_DEFINITION, p), _name(n), _expression(e) {}

    AstDeclDefinition(const AstDeclDefinition &a)
        : AstDeclDefinition(a.position(), a.name(), a.expression()) {}

    static AstPtr create(const Position &p, const AstPtr &n, const AstPtr &e) {
        return AstPtr(new AstDeclDefinition(p, n, e));
    }

    AstPtr name() const { return _name; }

    AstPtr expression() const { return _expression; }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent;
        l += 4;
        l = name()->approximate_length(l);
        if (l >= line_length) return l;
        l += 3;
        l = expression()->approximate_length(l);
        return l;
    }

    void render(std::ostream &os, uint_t indent) const {
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

typedef std::shared_ptr<AstDeclDefinition> AstDeclDefinitionPtr;
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
        : Ast(AST_DECL_VALUE, p), _name(e0), _expression(e1) {}

    AstDeclValue(const AstDeclValue &a)
        : AstDeclValue(a.position(), a.name(), a.expression()) {}

    static AstPtr create(const Position &p, const AstPtr &e0,
                         const AstPtr &e1) {
        return AstPtr(new AstDeclValue(p, e0, e1));
    }

    AstPtr name() const { return _name; }

    AstPtr expression() const { return _expression; }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent;
        l += 4;
        l = name()->approximate_length(l);
        if (l >= line_length) return l;
        l += 3;
        l = expression()->approximate_length(l);
        return l;
    }

    void render(std::ostream &os, uint_t indent) const {
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

typedef std::shared_ptr<AstDeclValue> AstDeclValuePtr;
#define AST_DECL_VALUE_CAST(a) std::static_pointer_cast<AstDeclValue>(a)
#define AST_DECL_VALUE_SPLIT(a, p, l, r) \
    auto _##a = AST_DECL_VALUE_CAST(a);  \
    auto p = _##a->position();           \
    auto l = _##a->name();               \
    auto r = _##a->expression();

class AstDeclOperator : public Ast {
public:
    AstDeclOperator(const Position &p, const AstPtr &c, const AstPtr &e)
        : Ast(AST_DECL_OPERATOR, p), _combinator(c), _expression(e) {}

    AstDeclOperator(const AstDeclOperator &a)
        : AstDeclOperator(a.position(), a.combinator(), a.expression()) {}

    static AstPtr create(const Position &p, const AstPtr &c, const AstPtr &e) {
        return AstPtr(new AstDeclOperator(p, c, e));
    }

    AstPtr combinator() const { return _combinator; }

    AstPtr expression() const { return _expression; }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent;
        l += 4;
        l = combinator()->approximate_length(l);
        if (l >= line_length) return l;
        l += 3;
        l = expression()->approximate_length(l);
        return l;
    }

    void render(std::ostream &os, uint_t indent) const {
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

typedef std::shared_ptr<AstDeclOperator> AstDeclOperatorPtr;
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
          _extends(ee) {}

    AstDeclObject(const AstDeclObject &a)
        : AstDeclObject(a.position(), a.name(), a.variables(), a.fields(),
                        a.extends()) {}

    static AstPtr create(const Position &p, const AstPtr &n, const AstPtrs &vv,
                         const AstPtrs &ff, const AstPtrs &ee) {
        return AstPtr(new AstDeclObject(p, n, vv, ff, ee));
    }

    AstPtr name() const { return _name; }

    AstPtrs variables() const { return _variables; }

    AstPtrs fields() const { return _fields; }

    AstPtrs extends() const { return _extends; }

    uint_t approximate_length(uint_t indent) const { return indent; }

    void render(std::ostream &os, uint_t indent) const {
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

typedef std::shared_ptr<AstDeclObject> AstDeclObjectPtr;
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
        : Ast(AST_DIRECT_IMPORT, p), _import(v) {}

    AstDirectImport(const AstDirectImport &c)
        : AstDirectImport(c.position(), c.import()) {}

    static AstPtr create(const Position &p, const icu::UnicodeString &v) {
        return AstPtr(new AstDirectImport(p, v));
    }

    icu::UnicodeString import() const { return _import; }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent + 6;
        l += _import.length();
        return l;
    }

    void render(std::ostream &os, uint_t indent) const {
        skip(os, indent);
        os << "import " << import() << std::endl;
    }

private:
    icu::UnicodeString _import;
};

typedef std::shared_ptr<AstDirectImport> AstDirectImportPtr;
#define AST_DIRECT_IMPORT_CAST(a) std::static_pointer_cast<AstDirectImport>(a)
#define AST_DIRECT_IMPORT_SPLIT(a, p, i)   \
    auto _##a = AST_DIRECT_IMPORT_CAST(a); \
    auto p = _##a->position();             \
    auto i = _##a->import();

class AstDirectUsing : public Ast {
public:
    AstDirectUsing(const Position &p, const UnicodeStrings &v)
        : Ast(AST_DIRECT_USING, p), _using(v) {}

    AstDirectUsing(const AstDirectUsing &c)
        : AstDirectUsing(c.position(), c.using0()) {}

    static AstPtr create(const Position &p, const UnicodeStrings &v) {
        return AstPtr(new AstDirectUsing(p, v));
    }

    UnicodeStrings using0() const { return _using; }

    uint_t approximate_length(uint_t indent) const {
        uint_t l = indent;
        for (auto &u : using0()) {
            l += u.length() + 1;
        }
        return l;
    }

    void render(std::ostream &os, uint_t indent) const {
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

typedef std::shared_ptr<AstDirectUsing> AstDirectUsingPtr;
#define AST_DIRECT_USING_CAST(a) std::static_pointer_cast<AstDirectUsing>(a)
#define AST_DIRECT_USING_SPLIT(a, p, u)   \
    auto _##a = AST_DIRECT_USING_CAST(a); \
    auto p = _##a->position();            \
    auto u = _##a->using0();

class AstWrapper : public Ast {
public:
    AstWrapper(const Position &p, const AstPtrs &c)
        : Ast(AST_WRAPPER, p), _content(c) {}

    AstWrapper(const AstWrapper &c) : AstWrapper(c.position(), c.content()) {}

    static AstPtr create(const Position &p, const AstPtrs &c) {
        return AstPtr(new AstWrapper(p, c));
    }

    AstPtrs content() const { return _content; }

    uint_t approximate_length(uint_t indent) const { return indent; }

    void render(std::ostream &os, uint_t indent) const {
        for (auto e : content()) {
            e->render(os, indent);
        }
    }

private:
    AstPtrs _content;
};

typedef std::shared_ptr<AstWrapper> AstWrapperPtr;
#define AST_WRAPPER_CAST(a) std::static_pointer_cast<AstWrapper>(a)
#define AST_WRAPPER_SPLIT(a, p, dd)  \
    auto _##a = AST_WRAPPER_CAST(a); \
    auto p = _##a->position();       \
    auto dd = _##a->content();

uint_t Ast::line_length = 80;

int compare_ast_tag(ast_tag_t t, const AstPtr &a0, const AstPtr &a1);

int compare_ast(const AstPtr &a0, const AstPtr &a1) {
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
        return compare_ast_tag(t0, a0, a1);
    }
}

int equal_ast(const AstPtr &a0, const AstPtr &a1) {
    return compare_ast(a0, a1) == 0;
}

int compare_asts(const AstPtrs &aa0, const AstPtrs &aa1) {
    uint_t sz0 = aa0.size();
    uint_t sz1 = aa1.size();
    if (sz0 < sz1) {
        return -1;
    } else if (sz1 < sz0) {
        return 1;
    } else {
        uint_t sz = sz0;
        for (uint_t i = 0; i < sz; i++) {
            auto a0 = aa0[i];
            auto a1 = aa1[i];
            int c = compare_ast(a0, a1);
            if (c != 0) return c;
        }
        return 0;
    }
}

int compare_text(const icu::UnicodeString &t0, const icu::UnicodeString &t1) {
    return t0.compare(t1);
}

int compare_texts(const UnicodeStrings &aa0, const UnicodeStrings &aa1) {
    uint_t sz0 = aa0.size();
    uint_t sz1 = aa1.size();
    if (sz0 < sz1) {
        return -1;
    } else if (sz1 < sz0) {
        return 1;
    } else {
        uint_t sz = sz0;
        for (uint_t i = 0; i < sz; i++) {
            auto a0 = aa0[i];
            auto a1 = aa1[i];
            int c = compare_text(a0, a1);
            if (c != 0) return c;
        }
        return 0;
    }
}

int compare_ast2(const AstPtr &a0, const AstPtr &a1, const AstPtr &a2,
                 const AstPtr &a3) {
    uint_t c = compare_ast(a0, a1);
    if (c != 0) return c;
    return compare_ast(a2, a3);
}

int compare_ast3(const AstPtr &a0, const AstPtr &a1, const AstPtr &a2,
                 const AstPtr &a3, const AstPtr &a4, const AstPtr &a5) {
    uint_t c = compare_ast(a0, a1);
    if (c != 0) return c;
    c = compare_ast(a2, a3);
    if (c != 0) return c;
    return compare_ast(a4, a5);
}

int compare_ast_tag(ast_tag_t t, const AstPtr &a0, const AstPtr &a1) {
    int c;

    switch (t) {
        case AST_EMPTY: {
            return 0;
        }
        // literals
        case AST_EXPR_INTEGER: {
            AST_EXPR_INTEGER_SPLIT(a0, p0, t0);
            AST_EXPR_INTEGER_SPLIT(a1, p1, t1);
            return compare_text(t0, t1);
            break;
        }
        case AST_EXPR_HEXINTEGER: {
            AST_EXPR_HEXINTEGER_SPLIT(a0, p0, t0);
            AST_EXPR_HEXINTEGER_SPLIT(a1, p1, t1);
            return compare_text(t0, t1);
            break;
        }
        case AST_EXPR_FLOAT: {
            AST_EXPR_FLOAT_SPLIT(a0, p0, t0);
            AST_EXPR_FLOAT_SPLIT(a1, p1, t1);
            return compare_text(t0, t1);
            break;
        }
        case AST_EXPR_CHARACTER: {
            AST_EXPR_CHARACTER_SPLIT(a0, p0, t0);
            AST_EXPR_CHARACTER_SPLIT(a1, p1, t1);
            return compare_text(t0, t1);
            break;
        }
        case AST_EXPR_TEXT: {
            AST_EXPR_TEXT_SPLIT(a0, p0, t0);
            AST_EXPR_TEXT_SPLIT(a1, p1, t1);
            return compare_text(t0, t1);
            break;
        }
        // variables and constants
        case AST_EXPR_VARIABLE: {
            AST_EXPR_VARIABLE_SPLIT(a0, p0, t0);
            AST_EXPR_VARIABLE_SPLIT(a1, p1, t1);
            return compare_text(t0, t1);
            break;
        }
        case AST_EXPR_WILDCARD: {
            AST_EXPR_WILDCARD_SPLIT(a0, p0, t0);
            AST_EXPR_WILDCARD_SPLIT(a1, p1, t1);
            return compare_text(t0, t1);
            break;
        }
        case AST_EXPR_COMBINATOR: {
            AST_EXPR_COMBINATOR_SPLIT(a0, p0, q0, t0);
            AST_EXPR_COMBINATOR_SPLIT(a1, p1, q1, t1);
            c = compare_texts(q0, q1);
            if (c != 0) return c;
            return compare_text(t0, t1);
            break;
        }
        case AST_EXPR_OPERATOR: {
            AST_EXPR_OPERATOR_SPLIT(a0, p0, q0, t0);
            AST_EXPR_OPERATOR_SPLIT(a1, p1, q1, t1);
            c = compare_texts(q0, q1);
            if (c != 0) return c;
            return compare_text(t0, t1);
            break;
        }
        //  list and tuple
        case AST_EXPR_LIST: {
            AST_EXPR_LIST_SPLIT(a0, p0, tt0, tl0);
            AST_EXPR_LIST_SPLIT(a1, p1, tt1, tl1);
            c = compare_asts(tt0, tt1);
            if (c != 0) return c;
            return compare_ast(tl0, tl1);
            break;
        }
        case AST_EXPR_TUPLE: {
            AST_EXPR_TUPLE_SPLIT(a0, p0, tt0);
            AST_EXPR_TUPLE_SPLIT(a1, p1, tt1);
            return compare_asts(tt0, tt1);
            break;
        }
        // compound statements
        case AST_EXPR_APPLICATION: {
            AST_EXPR_APPLICATION_SPLIT(a0, p0, aa0);
            AST_EXPR_APPLICATION_SPLIT(a1, p1, aa1);
            return compare_asts(aa0, aa1);
            break;
        }
        case AST_EXPR_LAMBDA: {
            AST_EXPR_LAMBDA_SPLIT(a0, p0, m0);
            AST_EXPR_LAMBDA_SPLIT(a1, p1, m1);
            return compare_ast(m0, m1);
            break;
        }
        case AST_EXPR_BLOCK: {
            AST_EXPR_BLOCK_SPLIT(a0, p0, mm0);
            AST_EXPR_BLOCK_SPLIT(a1, p1, mm1);
            return compare_asts(mm0, mm1);
            break;
        }
        case AST_EXPR_MATCH: {
            AST_EXPR_MATCH_SPLIT(a0, p0, mm0, g0, e0);
            AST_EXPR_MATCH_SPLIT(a1, p1, mm1, g1, e1);
            c = compare_asts(mm0, mm1);
            if (c != 0) return c;
            return compare_ast2(g0, g1, e0, e1);
            break;
        }
        case AST_EXPR_LET: {
            AST_EXPR_LET_SPLIT(a0, p0, l0, r0, e0);
            AST_EXPR_LET_SPLIT(a1, p1, l1, r1, e1);
            c = compare_asts(l0, l1);
            if (c != 0) return c;
            return compare_ast2(r0, r1, e0, e1);
            break;
        }
        case AST_EXPR_TAG: {
            AST_EXPR_TAG_SPLIT(a0, p0, e0, t0);
            AST_EXPR_TAG_SPLIT(a1, p1, e1, t1);
            return compare_ast2(e0, e1, t0, t1);
            break;
        }
        case AST_EXPR_IF: {
            AST_EXPR_IF_SPLIT(a0, p0, i0, t0, e0);
            AST_EXPR_IF_SPLIT(a1, p1, i1, t1, e1);
            return compare_ast3(i0, i1, t0, t1, e0, e1);
            break;
        }
        case AST_EXPR_STATEMENT: {
            AST_EXPR_STATEMENT_SPLIT(a0, p0, l0, r0);
            AST_EXPR_STATEMENT_SPLIT(a1, p1, l1, r1);
            return compare_ast2(r0, r1, l0, l1);
            break;
        }
        case AST_EXPR_TRY: {
            AST_EXPR_TRY_SPLIT(a0, p0, t0, c0);
            AST_EXPR_TRY_SPLIT(a1, p1, t1, c1);
            break;
        }
        case AST_EXPR_THROW: {
            AST_EXPR_THROW_SPLIT(a0, p0, exc0);
            AST_EXPR_THROW_SPLIT(a1, p1, exc1);
            break;
        }
        // directives
        case AST_DIRECT_IMPORT: {
            AST_DIRECT_IMPORT_SPLIT(a0, p0, n0);
            AST_DIRECT_IMPORT_SPLIT(a1, p1, n1);
            return compare_text(n0, n1);
            break;
        }
        case AST_DIRECT_USING: {
            AST_DIRECT_USING_SPLIT(a0, p0, pp0);
            AST_DIRECT_USING_SPLIT(a1, p1, pp1);
            return compare_texts(pp0, pp1);
            break;
        }
        // declarations
        case AST_DECL_DATA: {
            AST_DECL_DATA_SPLIT(a0, p0, nn0);
            AST_DECL_DATA_SPLIT(a1, p1, nn1);
            return compare_asts(nn0, nn1);
            break;
        }
        case AST_DECL_DEFINITION: {
            AST_DECL_DEFINITION_SPLIT(a0, p0, n0, e0);
            AST_DECL_DEFINITION_SPLIT(a1, p1, n1, e1);
            return compare_ast2(n0, n1, e0, e1);
            break;
        }
        case AST_DECL_OPERATOR: {
            AST_DECL_OPERATOR_SPLIT(a0, p0, c0, e0);
            AST_DECL_OPERATOR_SPLIT(a1, p1, c1, e1);
            return compare_ast2(c0, c1, e0, e1);
            break;
        }
        case AST_DECL_OBJECT: {
            AST_DECL_OBJECT_SPLIT(a0, p0, c0, vv0, ff0, ee0);
            AST_DECL_OBJECT_SPLIT(a1, p1, c1, vv1, ff1, ee1);
            c = compare_ast(c0, c1);
            if (c != 0) return c;
            c = compare_asts(vv0, vv1);
            if (c != 0) return c;
            c = compare_asts(ff0, ff1);
            if (c != 0) return c;
            return compare_asts(ee0, ee1);
            break;
        }
        case AST_DECL_NAMESPACE: {
            AST_DECL_NAMESPACE_SPLIT(a0, p0, nn0, dd0);
            AST_DECL_NAMESPACE_SPLIT(a1, p1, nn1, dd1);
            c = compare_texts(nn0, nn1);
            if (c != 0) return c;
            return compare_asts(dd0, dd1);
            break;
        }
        // wrapper
        case AST_WRAPPER: {
            AST_WRAPPER_SPLIT(a0, p0, dd0);
            AST_WRAPPER_SPLIT(a1, p1, dd1);
            return compare_asts(dd0, dd1);
            break;
        }
        case AST_DECL_VALUE: {
            AST_DECL_VALUE_SPLIT(a0, p0, l0, r0);
            AST_DECL_VALUE_SPLIT(a1, p1, l1, r1);
            return compare_ast2(l0, l1, r0, r1);
            break;
        }
        default:
            PANIC("compare ast failed");
    }
    return 0;
}
