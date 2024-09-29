#pragma once

#include <map>
#include <memory>

#include "error.hpp"
#include "runtime.hpp"

#include "unicode/regex.h"

namespace egel {

class Scope;
using ScopePtr = std::shared_ptr<Scope>;

class Scope {
public:
    Scope() : _outer(nullptr), _namespace("") {
    }

    Scope(ScopePtr scope) : _outer(scope), _namespace(scope->_namespace) {
    }

    static ScopePtr create() {
        return std::make_shared<Scope>();
    }

    static ScopePtr create(ScopePtr scope) {
        return std::make_shared<Scope>(scope);
    }

    std::map<icu::UnicodeString, icu::UnicodeString> map() {
        return _map;
    }

    ScopePtr outer() const {
        return _outer;
    }

    icu::UnicodeString get_namespace() const {
        return _namespace;
    }

    void extend_namespace(const icu::UnicodeString &s) {
        if (_namespace == "") {
            _namespace = s;
        } else {
            _namespace += "::" + s;
        }
    }

    void declare(const icu::UnicodeString &k, const icu::UnicodeString &v) {
        if (_map.count(k) > 0) {
            throw ErrorSemantical("redeclaration of " + k);
        } else {
            _map[k] = v;
        }
    }

    void redeclare(const icu::UnicodeString &k,
                          const icu::UnicodeString &v) {
        _map[k] = v;
    }

    icu::UnicodeString get(const icu::UnicodeString &k) const {
        if (_map.count(k) > 0) {
            return _map.at(k);
        } else if (_outer != nullptr) {
            return _outer->get(k);
        } else {
            return "";
        }
    }

    void render(std::ostream &os, int indent) const {
        os << "scope (" << std::endl;
        os << "namespace: " << _namespace << std::endl;
        for (const auto &kv:_map) {
            os << kv.first << " -> " << kv.second << std::endl;
        }
        if (_outer != nullptr) {
            os << "embedded in " << std::endl;
            _outer->render(os, indent+4);
        }
        os << ")" << std::endl;
    }

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
    ScopePtr _outer;
    icu::UnicodeString _namespace;
};

// scope manipulation

inline ScopePtr scope_global() {
    return Scope::create();
}

inline ScopePtr enter_scope(ScopePtr& scope) {
    return Scope::create(scope);
}

inline ScopePtr leave_scope(ScopePtr& scope) {
    return scope->outer();
}

inline ScopePtr get_global_scope(ScopePtr scope) {
    if (scope->outer() == nullptr) {
        return scope;
    } else {
        return get_global_scope(scope->outer());
    }
}

inline icu::UnicodeString get_local(ScopePtr scope, const icu::UnicodeString &s) {
    return scope->get(s);
}

inline icu::UnicodeString get_global(ScopePtr scope, const icu::UnicodeString &s) {
    return get_local(get_global_scope(scope), s);
}

inline void declare_local(ScopePtr scope, const icu::UnicodeString &s) {
    scope->declare(s, s);
}

inline void declare_local(ScopePtr scope, const icu::UnicodeString &k, const icu::UnicodeString &v) {
    scope->declare(k, v);
}

inline void declare_global(ScopePtr scope, const icu::UnicodeString& s) {
    declare_local(get_global_scope(scope), s);
}

inline void redeclare_local(ScopePtr scope, const icu::UnicodeString& s) {
    scope->redeclare(s, s);
}

inline void redeclare_global(ScopePtr scope, const icu::UnicodeString& s) {
    redeclare_local(get_global_scope(scope), s);
}

inline void declare_rename(ScopePtr scope, const icu::UnicodeString& k, const icu::UnicodeString &v) {
    scope->declare(k, v);
}

inline std::vector<icu::UnicodeString> get_namespace(ScopePtr scope, const icu::UnicodeString& ns) {
    auto map = get_global_scope(scope)->map();
    std::vector<icu::UnicodeString> dd;
    for (const auto &kv:map) {
        if (kv.first.startsWith(ns)) {
            dd.push_back(kv.first);
        }
    }
    return dd;
}

inline ScopePtr enter_namespace(ScopePtr scope, const icu::UnicodeString& s) {
    auto sc = enter_scope(scope);
    sc->extend_namespace(s);
    auto ns = sc->get_namespace();
    auto dd = get_namespace(scope, ns);
    auto l = ns.countChar32() + 2;
    for (const auto &s:dd) {
        auto s0 = s;
        s0.removeBetween(0,l);
        declare_rename(sc, s0, s);
    }
    return sc;
}

inline ScopePtr leave_namespace(ScopePtr scope) {
    return leave_scope(scope);
}

inline void debug_scope(ScopePtr scope) {
    scope->render(std::cout, 0);
}

}  // namespace egel
