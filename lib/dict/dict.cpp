#include "../../src/runtime.hpp"

#include <stdlib.h>
#include <iostream>
#include <fstream>

#include <string>
#include <map>

typedef std::map<VMObjectPtr, VMObjectPtr,LessVMObjectPtr> dict_t;

const icu::UnicodeString STRING_DICT = "Dict";

//## Dict::dictionary - a dictionary
class Dictionary: public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_EGO, Dictionary, STRING_DICT, "dictionary");

    Dictionary(VM* m, const dict_t& d): Dictionary(m) {
        _value = d;
    }

    Dictionary(const Dictionary& d): Dictionary(d.machine(), d.value()) {
    }

    static VMObjectPtr create(VM* m, const dict_t& d) {
        return VMObjectPtr(new Dictionary(m, d));
    }

    static std::shared_ptr<Dictionary> cast(const VMObjectPtr& o) {
        return std::static_pointer_cast<Dictionary>(o);
    }

    int compare(const VMObjectPtr& o) override {
        return -1; // XXX: for later
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
        for (auto&k : _value) {
            oo.push_back(k.first);
        }
        return oo;
    }

protected:
    dict_t _value;
};

//## Dict::dict - create a dict object
class Dict: public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_EGO, Dict, STRING_DICT, "dict");

    VMObjectPtr apply() const override {
        return Dictionary::create(machine(), dict_t());
    }
};

//## Dict::has d k - check for key
class DictHas: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, DictHas, STRING_DICT, "has");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        auto m = machine();
        if (m->is_opaque(arg0) && m->symbol(arg0) == "Dict::dictionary") {
            auto d = Dictionary::cast(arg0);
            return m->create_bool(d->has(arg1));
        } else {
            THROW_BADARGS;
        }
    }
};

//## Dict::get d k - get a value by key
class DictGet: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, DictGet, STRING_DICT, "get");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        auto m = machine();
        if (m->is_opaque(arg0) && m->symbol(arg0) == "Dict::dictionary") {
            auto d = Dictionary::cast(arg0);
            return d->get(arg1);
        } else {
            THROW_BADARGS;
        }
    }
};

//## Dict::set d k v - set a value by key
class DictSet: public Ternary {
public:
    TERNARY_PREAMBLE(VM_SUB_EGO, DictSet, STRING_DICT, "set");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        auto m = machine();
        if (m->is_opaque(arg0) && m->symbol(arg0) == "Dict::dictionary") {
            auto d = Dictionary::cast(arg0);
            d->set(arg1, arg2);
            return arg0;
        } else {
            THROW_BADARGS;
        }
    }
};

//## Dict::erase d k - erase a value by key
class DictErase: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, DictErase, STRING_DICT, "erase");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        auto m = machine();
        if (m->is_opaque(arg0) && m->symbol(arg0) == "Dict::dictionary") {
            auto d = Dictionary::cast(arg0);
            d->erase(arg1);
            return d;
        } else {
            THROW_BADARGS;
        }
    }
};

//## Dict::keys d - dictionary keys as list
class DictKeys: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, DictKeys, STRING_DICT, "keys");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (m->is_opaque(arg0) && m->symbol(arg0) == "Dict::dictionary") {
            auto d = Dictionary::cast(arg0);
            auto oo = d->keys();
            return m->to_list(oo);
        } else {
            THROW_BADARGS;
        }
    }
};


extern "C" std::vector<icu::UnicodeString> egel_imports() {
    return std::vector<icu::UnicodeString>();
}

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Dict::create(vm));
    oo.push_back(DictHas::create(vm));
    oo.push_back(DictGet::create(vm));
    oo.push_back(DictSet::create(vm));
    oo.push_back(DictErase::create(vm));
    oo.push_back(DictKeys::create(vm));

    return oo;
}
