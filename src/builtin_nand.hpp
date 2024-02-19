#pragma once

#include <utility>
#include <vector>
#include <map>
#include <stack>

#include "runtime.hpp"

namespace egel {

class NandStore {
public:
    NandStore() {
        _store.push_back(std::pair<int,int>(0,1));
        _store.push_back(std::pair<int,int>(1,0));
    }

    ~NandStore() {
    }

    int create_zero() const {
        return 0;
    }

    int create_one() const {
        return 1;
    }

    int find(int n0, int n1) const {
        //std::map<std::pair<int,int>, int>::iterator it;
        auto it = _store_inv.find(std::pair<int,int>(n0,n1)); 
        if (it == _store_inv.end()) {
            return -1;
        } else {
            return it->second;
        }
    }

    int insert(int n0, int n1) {
        auto n = _store.size();
        _store.push_back(std::pair<int,int>(n0,n1));
        _store_inv[std::pair<int,int>(n0,n1)] = n;
        return n;
    }

    int create_var(int v) {
        auto n = find(-1, v);
        if (n > 0) {
            return n;
        } else {
            return insert(-1,v);
        }
    }

    int create_nand(int n0, int n1) {
        auto n = find(n0, n1);
        if (n > 0) {
            return n;
        } else if (n1 < n0) {
            return create_nand(n1, n0);
        } else {
            if (is_zero(n0) || is_zero(n1)) {
                return create_one();
            } else if (is_one(n0) && is_one(n1)) {
                return create_zero();
            } else if (is_one(n0)) {
                return create_nand(n1,n1); // could be a not not
            } else if (is_one(n1)) {
                return create_nand(n0,n0); // could be a not not
            } else if ((n0 == n1) && is_not(n0)) {
                return get_not(n0);
            } else {
                return insert(n0,n1);
            }
        }
    }

    bool is_zero(int n) const {
        return n == 0;
    }

    bool is_one(int n) const {
        return n == 1;
    }

    bool is_var(int n) const {
        return _store[n].first < 0;
    }

    bool is_nand(int n) const {
        return !is_zero(n) && !is_one(n) && !is_var(n);
    }

    bool is_not(int n) const {
        return _store[n].first == _store[n].second;
    }

    int get_var(int n) const {
        return _store[n].second;
    }

    std::pair<int,int> get_nand(int n) const {
        return _store[n];
    }

    int get_not(int n) const {
        return _store[n].first;
    }

    int size(int n) const {
        int sz = 0;
        std::set<int> visited;
        std::stack<int> work;

        work.push(n);
        while (!work.empty()) {
            auto n = work.top();
            work.pop();
            if (visited.count(n) == 0) {
                visited.insert(n);
                sz++;
                if (is_nand(n)) {
                    auto p = get_nand(n);
                    work.push(p.second);
                    work.push(p.first);
                }
            }
        }

        return sz;
    }

    int sub(int n0, int n1, int n2) {
        std::map<int,int>   subs;
        std::stack<int>     work;

        work.push(n0);
        subs[n1] = n2;
        while (!work.empty()) {
            auto n = work.top();
            if (subs.count(n) == 0) {
                if (is_zero(n)) {
                    subs[n] = n;
                } else if (is_one(n)) {
                    subs[n] = n;
                } else if (is_var(n)) {
                    subs[n] = n;
                } else { // is_nand
                    auto p = get_nand(n);
                    if ((subs.count(p.first)) > 0 && (subs.count(p.second) > 0)) {
                        auto n0 = create_nand(subs[p.first], subs[p.second]);
                        subs[n] = n0;
                        work.pop();
                    } else {
                        work.push(p.second);
                        work.push(p.first);
                    }
                }
            } else {
                work.pop();
            }
        }

        return subs[n0];
    }

    void incref(int n) {
        if (_count.count(n) == 0) {
            _count[n] = 1;
        } else {
            _count[n] = _count[n] + 1;
        }
    }

    void decref(int n) {
        if (_count[n] == 1) {
            _count.erase(n);
        } else {
            _count[n] = _count[n] - 1;
        }
    }

    int make_root(int n) {
        int r = 0;
        while (_roots.count(r) > 0) r++; // linear search : XXX
        _roots[r] = n;
        incref(n);
        return r;
    }

    void remove_root(int r) {
        auto n = _roots[r];
        decref(n);
        _roots.erase(r);
    }

    bool root_is_zero(int r) {
        auto n = _roots[r];
        return is_zero(n);
    }

    bool root_is_one(int r) {
        auto n = _roots[r];
        return is_one(n);
    }

    bool root_is_var(int r) {
        auto n = _roots[r];
        return is_var(n);
    }

    bool root_is_nand(int r) {
        auto n = _roots[r];
        return is_nand(n);
    }

    bool root_is_not(int r) {
        auto n = _roots[r];
        return is_not(n);
    }

    int root_create_zero() {
        auto n = create_zero();
        auto r = make_root(n);
        return r;
    }

    int root_create_one() {
        auto n = create_one();
        auto r = make_root(n);
        return r;
    }

    int root_create_var(int v) {
        auto n = create_var(v);
        auto r = make_root(n);
        return r;
    }

    int root_create_nand(int r0, int r1) {
        auto n0 = _roots[r0];
        auto n1 = _roots[r1];
        auto n = create_nand(n0,n1);
        auto r = make_root(n);
        return r;
    }

    int root_get_var(int r) {
        auto n = _roots[r];
        return get_var(n);
    }

    std::pair<int,int> root_get_nand(int r) {
        auto n = _roots[r];
        auto p = get_nand(n);
        return std::pair<int,int>(make_root(p.first), make_root(p.second));
        
    }

    int root_get_not(int r) {
        auto n = _roots[r];
        auto n0 = get_not(n);
        return make_root(n0);
        
    }

    int root_size(int r) {
        auto n = _roots[r];
        return size(n);
    }

    int root_sub(int r0, int r1, int r2) {
        auto n0 = _roots[r0];
        auto n1 = _roots[r1];
        auto n2 = _roots[r2];
        auto n = sub(n0, n1, n2);
        return make_root(n);
    }

    void gc() {
        NandStore new_store;
        std::map<int, int> to_new;

        for (const auto& pair : _roots) {
            //auto r = pair.first;
            auto n = pair.second;;

            std::stack<int> work;
            work.push(n);
            while (!work.empty()) {
                auto n = work.top();
                if (to_new.count(n) == 0) {
                    if (is_zero(n)) {
                        auto n0 = new_store.create_zero();
                        to_new[n] = n0;
                    } else if (is_one(n)) {
                        auto n0 = new_store.create_one();
                        to_new[n] = n0;
                    } else if (is_var(n)) {
                        auto v = get_var(n);
                        auto n0 = new_store.create_var(v);
                        to_new[n] = n0;
                    } else { // is_nand
                        auto p = get_nand(n);
                        if ((to_new.count(p.first)) > 0 && (to_new.count(p.second) > 0)) {
                            auto n0 = new_store.create_nand(to_new[p.first], to_new[p.second]);
                            to_new[n] = n0;
                            work.pop();
                        } else {
                            work.push(p.second);
                            work.push(p.first);
                        }
                    }
                } else {
                    work.pop();
                }
            }
        }

        _store = new_store._store;
        _store_inv = new_store._store_inv;
        _count = new_store._count;

        for (const auto& pair : _roots) {
            _roots[pair.first] = to_new[pair.second];
        }
    }

    void debug() const {
        std::cout << "nand terms" << std::endl;
        std::cout << "store" << std::endl;
        for (size_t n = 0; n < _store.size(); n++) {
            auto p = _store[n];
            std::cout << "[" << n << "] = (" << p.first << ", " << p.second << ")" << std::endl;
        }
        std::cout << "store_inv" << std::endl;
        for (auto &p : _store_inv) {
            std::cout << "[(" << p.first.first << ", " << p.first.second << ")] = " << p.second << std::endl;
        }
        std::cout << "roots" << std::endl;
        for (auto &p : _roots) {
            std::cout << "[" << p.first << "] = " << p.second << std::endl;
        }
        std::cout << "count" << std::endl;
        for (auto &p : _count) {
            std::cout << "[" << p.first << "] = " << p.second << std::endl;
        }
    }

private:
    std::vector<std::pair<int,int>> _store;
    std::map<std::pair<int,int>, int> _store_inv;
    std::map<int,int>   _roots;
    std::map<int,int>   _count;
};

inline NandStore global_store;

class NandTerm: public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_BUILTIN, NandTerm, STRING_NAND, "nand");

    NandTerm(const NandTerm& t)
        : Opaque(VM_SUB_BUILTIN, t.machine(), t.symbol()) {
        _root = t.root();
    }

    ~NandTerm() {
        global_store.remove_root(root());
    }

    static VMObjectPtr create(VM* m, int r) {
        auto o = std::make_shared<NandTerm>(m);
        o->set_root(r);
        return o;
    }

    static bool is_nand(const VMObjectPtr& o) {
        auto& r = *o.get();
        return typeid(r) == typeid(NandTerm);
    }

    static std::shared_ptr<NandTerm> cast(const VMObjectPtr& o) {
        return std::static_pointer_cast<NandTerm>(o);
    }

    void set_root(const int r) {
        _root = r;
    }

    int root() const {
        return _root;
    }

    int compare(const VMObjectPtr& o) override {
        auto r = cast(o)->root();
        if (root() < r) {
            return -1;
        } else if (r < root()) {
            return 1;
        } else {
            return 0;
        }
    }

private:
    int _root;
};

class NandZero : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, NandZero, STRING_NAND, "zero");

    VMObjectPtr apply() const override {
        return NandTerm::create(machine(), global_store.root_create_zero());
    }
};

class NandOne : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, NandOne, STRING_NAND, "one");

    VMObjectPtr apply() const override {
        return NandTerm::create(machine(), global_store.root_create_one());
    }
};

class NandVar : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, NandVar, STRING_NAND, "var");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto v = machine()->get_integer(arg0);
            return NandTerm::create(machine(), global_store.root_create_var(v));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class NandNand : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, NandNand, STRING_NAND, "nand");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if (NandTerm::is_nand(arg0) && NandTerm::is_nand(arg1)) {
            auto t0 = NandTerm::cast(arg0);
            auto t1 = NandTerm::cast(arg1);
            return NandTerm::create(machine(), global_store.root_create_nand(t0->root(), t1->root()));
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class NandIsZero : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, NandIsZero, STRING_NAND, "is_zero");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (NandTerm::is_nand(arg0)) {
            auto t0 = NandTerm::cast(arg0);
            return machine()->create_bool(global_store.root_is_zero(t0->root()));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class NandIsOne : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, NandIsOne, STRING_NAND, "is_one");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (NandTerm::is_nand(arg0)) {
            auto t0 = NandTerm::cast(arg0);
            return machine()->create_bool(global_store.root_is_one(t0->root()));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class NandIsVar : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, NandIsVar, STRING_NAND, "is_var");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (NandTerm::is_nand(arg0)) {
            auto t0 = NandTerm::cast(arg0);
            return machine()->create_bool(global_store.root_is_var(t0->root()));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class NandIsNand : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, NandIsNand, STRING_NAND, "is_nand");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (NandTerm::is_nand(arg0)) {
            auto t0 = NandTerm::cast(arg0);
            return machine()->create_bool(global_store.root_is_nand(t0->root()));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class NandIsNot : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, NandIsNot, STRING_NAND, "is_not");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (NandTerm::is_nand(arg0)) {
            auto t0 = NandTerm::cast(arg0);
            return machine()->create_bool(global_store.root_is_not(t0->root()));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class NandGetVar : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, NandGetVar, STRING_NAND, "get_var");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (NandTerm::is_nand(arg0)) {
            auto t0 = NandTerm::cast(arg0);
            return machine()->create_integer(global_store.root_get_var(t0->root()));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class NandGetNand : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, NandGetNand, STRING_NAND, "get_nand");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (NandTerm::is_nand(arg0)) {
            auto t0 = NandTerm::cast(arg0);
            auto p = global_store.root_get_nand(t0->root());
            auto t1 = NandTerm::create(machine(), p.first);
            auto t2 = NandTerm::create(machine(), p.second);
            return machine()->create_tuple(t1, t2);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class NandGetNot : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, NandGetNot, STRING_NAND, "get_not");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (NandTerm::is_nand(arg0)) {
            auto t0 = NandTerm::cast(arg0);
            auto r = global_store.root_get_not(t0->root());
            return NandTerm::create(machine(), r);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class NandSize : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, NandSize, STRING_NAND, "size");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (NandTerm::is_nand(arg0)) {
            auto t0 = NandTerm::cast(arg0);
            auto sz = global_store.root_size(t0->root());
            return machine()->create_integer(sz);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class NandSub : public Triadic {
public:
    TRIADIC_PREAMBLE(VM_SUB_BUILTIN, NandSub, STRING_NAND, "sub");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        if (NandTerm::is_nand(arg0) && NandTerm::is_nand(arg1) && NandTerm::is_nand(arg2)) {
            auto t0 = NandTerm::cast(arg0);
            auto t1 = NandTerm::cast(arg1);
            auto t2 = NandTerm::cast(arg2);
            auto r = global_store.sub(t0->root(), t1->root(), t2->root());
            return NandTerm::create(machine(), r);
        } else {
            throw machine()->bad_args(this, arg0, arg1, arg2);
        }
    }
};

class NandGC : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, NandGC, STRING_NAND, "gc");

    VMObjectPtr apply() const override {
        global_store.gc();
        return machine()->create_none();
    }
};

class NandDebug : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, NandDebug, STRING_NAND, "debug");

    VMObjectPtr apply() const override {
        global_store.debug();
        return machine()->create_none();
    }
};

inline std::vector<VMObjectPtr> builtin_nand(VM *vm) {
    std::vector<VMObjectPtr> oo;
    
    oo.push_back(NandTerm::create(vm));
    oo.push_back(NandZero::create(vm));
    oo.push_back(NandOne::create(vm));
    oo.push_back(NandVar::create(vm));
    oo.push_back(NandNand::create(vm));
    oo.push_back(NandIsZero::create(vm));
    oo.push_back(NandIsOne::create(vm));
    oo.push_back(NandIsVar::create(vm));
    oo.push_back(NandIsNand::create(vm));
    oo.push_back(NandIsNot::create(vm));
    oo.push_back(NandGetVar::create(vm));
    oo.push_back(NandGetNand::create(vm));
    oo.push_back(NandGetNot::create(vm));
    oo.push_back(NandSize::create(vm));
    oo.push_back(NandSub::create(vm));
    oo.push_back(NandGC::create(vm));
    oo.push_back(NandDebug::create(vm));

    return oo;
};   

}  // namespace egel
