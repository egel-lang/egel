#pragma once

#include <map>
#include <memory>

#include "error.hpp"
#include "runtime.hpp"

namespace egel {


class Scope;
using ScopePtr = std::shared_ptr<Scope>;

class Scope {
public:
    std::map<icu::UnicodeString, icu::UnicodeString> map() {
        return _map;
    }

    void declare(const icu::UnicodeString &k, const icu::UnicodeString &v) {
        if (_map.count(k) > 0) {
            throw ErrorSemantical("redeclaration of " + k);
        } else {
            _map[k] = v;
        }
    }

    void declare_implicit(const icu::UnicodeString &k,
                          const icu::UnicodeString &v) {
        _map[k] = v;
    }

    icu::UnicodeString get(const icu::UnicodeString &k) const {
        if (_map.count(k) > 0) return _map.at(k);
        return "";
    }

    virtual void render(std::ostream &os, int indent) const = 0;

    void skip(std::ostream &os, int indent) const {
        for (int i = 0; i < indent; ++i) {
            os << " ";
        }
    }

    icu::UnicodeString to_text() {
        std::stringstream ss;
        render(ss, 0);
        icu::UnicodeString u(ss.str().c_str());
        return u;
    }

protected:
    std::map<icu::UnicodeString, icu::UnicodeString> _map;
};

class Namespace;
using NamespacePtr = std::shared_ptr<Namespace>;

class Namespace : public Scope {
public:
    Namespace() {
    }

    Namespace(const Namespace &nn) : _embeds(nn._embeds) {
    }

    virtual ~Namespace() {  // keep the compiler happy
    }

    static NamespacePtr create() {
        return std::make_shared<Namespace>();
    }

    NamespacePtr create_namespace(const icu::UnicodeString &n) {
        if (_embeds.count(n) > 0) {
            return _embeds.at(n);
        } else {
            auto nn = std::make_shared<Namespace>();
            _embeds[n] = nn;
            return nn;
        }
    }

    NamespacePtr find_namespace(const icu::UnicodeString &n) const {
        if (_embeds.count(n) > 0) {
            return _embeds.at(n);
        } else {
            return nullptr;
        }
    }

    void render(std::ostream &os, int indent) const override {
        for (auto const &sym : _map) {
            skip(os, indent);
            os << sym.first << " -> " << sym.second << std::endl;
        }

        for (auto const &space : _embeds) {
            skip(os, indent);
            os << space.first << " (" << std::endl;
            space.second->render(os, indent + 4);
            skip(os, indent);
            os << ")" << std::endl;
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const NamespacePtr &n) {
        n->render(os, 0);
        return os;
    }

private:
    std::map<icu::UnicodeString, NamespacePtr> _embeds;
};

class Range;
using RangePtr = std::shared_ptr<Range>;
using Uses = std::vector<NamespacePtr>;

class Range : public Scope {
public:
    Range(RangePtr r) : _embeds(r) {
    }

    virtual ~Range() {  // keep the compiler happy
    }

    void add_namespace(const NamespacePtr &n) {
        _uses.push_back(n);
    }

    Uses uses() const {
        return _uses;
    }

    RangePtr embeds() const {
        return _embeds;
    }

    void render(std::ostream &os, int indent) const override {
        for (auto const &sym : _map) {
            skip(os, indent);
            os << sym.first << " : " << sym.second << std::endl;
        }

        if (_embeds != nullptr) {
            skip(os, indent);
            os << "(" << std::endl;
            _embeds->render(os, indent + 4);
            skip(os, indent);
            os << ")" << std::endl;
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const RangePtr &n) {
        n->render(os, 0);
        return os;
    }

private:
    Uses _uses;
    RangePtr _embeds;
};

// the Namespace ADT

inline NamespacePtr namespace_nil() {
    return std::make_shared<Namespace>();
}

inline void declare(const NamespacePtr &p, const UnicodeStrings &nn,
                    const icu::UnicodeString &n, const icu::UnicodeString &v) {
    auto ptr = p;
    for (auto &n : nn) {
        ptr = ptr->create_namespace(n);
    }
    ptr->declare(n, v);
}

inline void declare_implicit(const NamespacePtr &p, const UnicodeStrings &nn,
                             const icu::UnicodeString &n,
                             const icu::UnicodeString &v) {
    auto ptr = p;
    for (auto &n : nn) {
        ptr = ptr->create_namespace(n);
    }
    ptr->declare_implicit(n, v);
}

inline icu::UnicodeString get(const NamespacePtr &p, const UnicodeStrings &nn,
                              const icu::UnicodeString &n) {
    auto ptr = p;
    for (auto &n : nn) {
        ptr = ptr->find_namespace(n);
        if (ptr == nullptr) return "";
    }
    return ptr->get(n);
}

inline NamespacePtr find_namespace(const NamespacePtr &p,
                                   const UnicodeStrings &nn) {
    auto ptr = p;
    for (auto &n : nn) {
        ptr = ptr->find_namespace(n);
        if (ptr == nullptr) return ptr;
    }
    return ptr;
}

// the Range ADT

inline RangePtr range_nil(const NamespacePtr &p) {
    auto q = std::make_shared<Range>(nullptr);
    q->add_namespace(p);
    return q;
}

inline RangePtr enter_range(const RangePtr &p) {
    return std::make_shared<Range>(p);
}

inline RangePtr leave_range(const RangePtr &p) {
    return p->embeds();
}

inline void declare(const RangePtr &p, const icu::UnicodeString &k,
                    const icu::UnicodeString &v) {
    p->declare(k, v);
}

inline icu::UnicodeString get(const RangePtr &r, const icu::UnicodeString &n) {
    static UnicodeStrings nn;
    if (r == nullptr) return "";
    icu::UnicodeString s = r->get(n);
    if (s != "") {
        return s;
    } else {
        for (auto &ptr : r->uses()) {
            icu::UnicodeString s = get(ptr, nn, n);
            if (s != "") return s;
        }
        return get(r->embeds(), n);
    }
}

inline icu::UnicodeString get(const RangePtr &r, const UnicodeStrings &nn,
                              const icu::UnicodeString &n) {
    if (r == nullptr) return "";
    for (auto &ptr : r->uses()) {
        icu::UnicodeString s = get(ptr, nn, n);
        if (s != "") return s;
    }
    return get(r->embeds(), nn, n);
}

inline void add_namespace(RangePtr &r, const UnicodeStrings &nn) {
    // find root
    auto p = r;
    if (p == nullptr) return;
    while (p->embeds() != nullptr) p = p->embeds();
    // take global namespace
    if (p->uses().size() == 0) return;
    auto globals = p->uses()[0];
    // find namespace
    auto n = find_namespace(globals, nn);
    if (n == nullptr) return;
    // insert namespace
    r->add_namespace(n);
}

inline std::vector<icu::UnicodeString> get_namespace(RangePtr &r, const UnicodeStrings &nn) {
    std::vector<icu::UnicodeString> dd;
    auto p = r;
    if (p == nullptr) return dd;
    while (p->embeds() != nullptr) p = p->embeds();
    if (p->uses().size() == 0) return dd;
    auto globals = p->uses()[0];
    auto n = find_namespace(globals, nn);
    if (n == nullptr) return dd;
    for (const auto& pair : n->map()) {
        dd.push_back(pair.first);
    }
    return dd;
}

}  // namespace egel
