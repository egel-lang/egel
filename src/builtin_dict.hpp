#pragma once

#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "runtime.hpp"

using namespace egel;

typedef std::map<VMObjectPtr, VMObjectPtr, LessVMObjectPtr> dict_t;

const icu::UnicodeString STRING_DICT = "Dict";

class Dictionary : public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_EGO, Dictionary, STRING_DICT, "dictionary");

DOCSTRING("Dict::dictionary - a dictionary");
    Dictionary(VM* m, const dict_t& d) : Dictionary(m) {
        _value = d;
    }

    Dictionary(const Dictionary& d) : Dictionary(d.machine(), d.value()) {
    }

    static VMObjectPtr create(VM* m, const dict_t& d) {
        return VMObjectPtr(new Dictionary(m, d));
    }

    int compare(const VMObjectPtr& o) override {
        return -1;  // XXX: for later
    }

    dict_t value() const {
        return _value;
    }

    size_t size() const {
        return _value.size();
    }

    bool has(const VMObjectPtr key) {
        return _value.count(key) > 0;
    }

    VMObjectPtr get(const VMObjectPtr& key) {
        return _value[key];
    }

    void set(const VMObjectPtr& key, const VMObjectPtr& value) {
        _value[key] = value;
    }

    void erase(const VMObjectPtr& key) {
        _value.erase(key);
    }

    VMObjectPtrs keys() const {
        VMObjectPtrs oo;
        for (auto& k : _value) {
            oo.push_back(k.first);
        }
        return oo;
    }

protected:
    dict_t _value;
};

class Dict : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_EGO, Dict, STRING_DICT, "dict");

DOCSTRING("Dict::dict - create a dict object");
    VMObjectPtr apply() const override {
        return Dictionary::create(machine(), dict_t());
    }
};

class DictHas : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, DictHas, STRING_DICT, "has");

DOCSTRING("Dict::has d k - check for key");
    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
        auto m = machine();
        if (Dictionary::is_type(arg0)) {
            auto d = Dictionary::cast(arg0);
            return m->create_bool(d->has(arg1));
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class DictGet : public Binary {
public:
    BINARY_PREAMBLE(VM_SUB_EGO, DictGet, STRING_DICT, "get");

DOCSTRING("Dict::get d k - get a value by key");
    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
        if (Dictionary::is_type(arg0)) {
            auto d = Dictionary::cast(arg0);
            return d->get(arg1);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class DictSet : public Ternary {
public:
    TERNARY_PREAMBLE(VM_SUB_EGO, DictSet, STRING_DICT, "set");

DOCSTRING("Dict::set d k v - set a value by key");
    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1,
                      const VMObjectPtr& arg2) const override {
        if (Dictionary::is_type(arg0)) {
            auto d = Dictionary::cast(arg0);
            d->set(arg1, arg2);
            return arg0;
        } else {
            throw machine()->bad_args(this, arg0, arg1, arg2);
        }
    }
};

class DictErase : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, DictErase, STRING_DICT, "erase");

DOCSTRING("Dict::erase d k - erase a value by key");
    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
        if (Dictionary::is_type(arg0)) {
            auto d = Dictionary::cast(arg0);
            d->erase(arg1);
            return d;
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class DictKeys : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, DictKeys, STRING_DICT, "keys");

DOCSTRING("Dict::keys d - dictionary keys as list");
    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (Dictionary::is_type(arg0)) {
            auto d = Dictionary::cast(arg0);
            auto oo = d->keys();
            return machine()->to_list(oo);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

inline std::vector<VMObjectPtr> builtin_dict(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Dict::create(vm));
    oo.push_back(DictHas::create(vm));
    oo.push_back(DictGet::create(vm));
    oo.push_back(DictSet::create(vm));
    oo.push_back(DictErase::create(vm));
    oo.push_back(DictKeys::create(vm));

    return oo;
}
