#include "../../src/runtime.hpp"

#include <stdlib.h>
#include <iostream>
#include <fstream>

#include <string>
#include <map>

typedef std::map<VMObjectPtr, VMObjectPtr,LessVMObjectPtr> dict_t;

//## System::dictionary - a dictionary
class Dictionary: public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_EGO, Dictionary, "System", "dictionary");

    Dictionary(VM* m, const dict_t& d): Dictionary(m) {
        _value = d;
    }

    Dictionary(const Dictionary& d): Dictionary(d.machine(), d.value()) {
    }

    VMObjectPtr clone() const override {
        return VMObjectPtr(new Dictionary(*this));
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

    VMObjectPtr has(const VMObjectPtr key) {
        return _value[key];
    }

    VMObjectPtr get(const VMObjectPtr& key) {
        return _value[key];
    }

    void set(const VMObjectPtr& key, const VMObjectPtr& value) {
        _value[key] = value;
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

//## System::dict - create a dict object
class Dict: public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_EGO, Dict, "System", "dict");

    VMObjectPtr apply() const override {
        return Dictionary::create(machine(), dict_t());
    }
};

//## System::dict_has d k - check for key
class DictHas: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, DictHas, "System", "dict_has");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        auto m = machine();
        if (m->is_opaque(arg0) && m->value_symbol(arg0) == "System::dictionary") {
            auto d = Dictionary::cast(arg0);
            return d->has(arg1);
        } else {
            THROW_BADARGS;
        }
    }
};

//## System::dict_get d k - get a value by key
class DictGet: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, DictGet, "System", "dict_get");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        auto m = machine();
        if (m->is_opaque(arg0) && m->value_symbol(arg0) == "System::dictionary") {
            auto d = Dictionary::cast(arg0);
            return d->get(arg1);
        } else {
            THROW_BADARGS;
        }
    }
};

//## System::dict_set d k v - set a value by key
class DictSet: public Ternary {
public:
    TERNARY_PREAMBLE(VM_SUB_EGO, DictSet, "System", "dict_set");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        auto m = machine();
        if (m->is_opaque(arg0) && m->value_symbol(arg0) == "System::dictionary") {
            auto d = Dictionary::cast(arg0);
            d->set(arg1, arg2);
            return m->create_none();
        } else {
            THROW_BADARGS;
        }
    }
};

//## System::dict_keys d - dictionary keys as list
class DictKeys: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, DictKeys, "System", "dict_keys");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (m->is_opaque(arg0) && m->value_symbol(arg0) == "System::dictionary") {
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

    oo.push_back(Dictionary(vm).clone());
    oo.push_back(Dict(vm).clone());
    oo.push_back(DictHas(vm).clone());
    oo.push_back(DictGet(vm).clone());
    oo.push_back(DictSet(vm).clone());
    oo.push_back(DictKeys(vm).clone());

    return oo;
}
