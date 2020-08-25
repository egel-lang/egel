#include "syntactical.hpp"
#include "transform.hpp"

class Imports: public Visit {
public:
    AstPtrs imports(const AstPtr& a) {
        visit(a);
        return _imports;
    }

    void visit_directive_import(const Position& p, const icu::UnicodeString& i) override {
        _imports.push_back(AstDirectImport(p, i).clone());
    }

    // cuts    
    void visit_decl_data(const Position& p, const AstPtrs& nn) override {
    }

    void visit_decl_definition(const Position& p, const AstPtr& n, const AstPtr& e) override {
    }

    void visit_decl_value(const Position& p, const AstPtr& n, const AstPtr& e) override {
    }

    void visit_decl_operator(const Position& p, const AstPtr& c, const AstPtr& e) override {
    }

private:
    AstPtrs _imports;
};

AstPtrs imports(const AstPtr& a) {
    Imports imports;
    return imports.imports(a);
}

class Values: public Visit {
public:
    AstPtrs values(const AstPtr& a) {
        visit(a);
        return _values;
    }

    void visit_decl_value(const Position& p, const AstPtr& n, const AstPtr& e) override {
        _values.push_back(AstDeclValue(p, n, e).clone());
    }

    // cuts    
    void visit_decl_data(const Position& p, const AstPtrs& nn) override {
    }

    void visit_decl_definition(const Position& p, const AstPtr& n, const AstPtr& e) override {
    }

    void visit_decl_operator(const Position& p, const AstPtr& c, const AstPtr& e) override {
    }

private:
    AstPtrs _values;
};

AstPtrs values(const AstPtr& a) {
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
