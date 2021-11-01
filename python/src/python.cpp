#define PY_SSIZE_T_CLEAN    // for obscure reasons this goes first
#include <Python.h>

#include <stdlib.h>
#include <iostream>
#include <fstream>

#include <string>
#include <memory>
#include <exception>
#include <thread>

// lets hope all this C stuff can once be gone
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <egel/runtime.hpp> // build against an installed egel
#include "utils.hpp"        // same as egel's but only the runtime should be included

#define LIBRARY_VERSION_MAJOR 0
#define LIBRARY_VERSION_MINOR 0
#define LIBRARY_VERSION_PATCH 1

class CPythonMachine {
public:
    CPythonMachine() {
        std::cerr << "Python initialized" << std::endl;
        Py_Initialize();
    }

    ~CPythonMachine() {
        std::cerr << "Python finalized" << std::endl;
        Py_Finalize();
    }
};

/* Placeholder to support RAII on PyObject*.
 *
 * You _can_ copy a CPyObject around.
 */
class CPyObject {
public:
    CPyObject() : _object(nullptr) {
    }

    CPyObject(PyObject* o) : _object(o) {
        inc_ref(o);
    }

    ~CPyObject() {
        dec_ref();
        _object= nullptr;
    }

    PyObject* get_object() {
        return _object;
    }

    PyObject* set_object(PyObject* o) {
        return (_object=o);
    }

    PyObject* inc_ref()
    {
        if(_object) Py_XINCREF(_object);
        return _object;
    }

    void dec_ref() {
        if(_object) Py_XDECREF(_object);
    }

    PyObject* operator ->() {
        return _object;
    }

    bool is() {
        return _object ? true : false;
    }

    operator PyObject*() {
        return _object;
    }

    PyObject* operator = (PyObject* o) {
        _object = o;
        return _object;
    }

    operator bool() {
        return (_object)? true : false;
    }
private:
    PyObject* _object;
};

/* Below the two work horses of this library, conversion between 
 * primitive types of Egel and Python.
 *
 * Python     | Egel
 * ---------------------
 * None       | nop
 * False      | false
 * True       | true
 * Long       | integer
 * Float      | float
 * String     | text
 *
 * Characters/codepoints are not handled.
 *
 * Anything more complex is handled from the Egel/Python side.
 */


static VMObjectPtr python_to_egel(VM* m, CPythonObject object) {
    auto o = CPythonObject();
    if (Py_IsNone(o)) {
         return m->create_nop();
    } else if (Py_IsFalse(o)) {
         return m->create_false();
    } else if (Py_IsTrue(o)) {
         return m->create_true();
    } else if (Py_IS_TYPE(o, PyLong_Type) {
         vm_int_t n0;
         PyArg_Parse(s1, "l", &n0);
         auto n1 = m->create_float(n0);
         return n1;
    } else if (Py_IS_TYPE(o, PyFloat_Type) {
         float f0;
         PyArg_Parse(s1, "f", &f0);
         auto f1 = m->create_float(f0);
         return f1;
    } else if (Py_IS_TYPE(o, PyUnicode_Type) {
         char *s0;
         PyArg_Parse(o, "s", &s0);
         auto s1 = char_to_unicode(s0);
         auto s2 = m->create_text(s1);
         return s2;
    } else {
         throw m->create_text("conversion failed"); 
    }
};

static CPythonObject egel_to_python(VM* machine, const VMObjectPtr& o) {
    if (VM_OBJECT_NOP_TEST(o) {
         return Py_None;
    } else if (VM_OBJECT_FALSE_TEST(o) {
         return Py_False;
    } else if (VM_OBJECT_TRUE_TEST(o) {
         return Py_True;
    } else if (VM_OBJECT_INTEGER_TEST(o) {
        auto n0 = VM_OBJECT_INTEGER_VALUE(o);
        PyObject* n1 = Py_BuildValue("l", n0); // XXX: l = long int; L = long long
        return CPythonObject(n1);
    } else if (VM_OBJECT_FLOAT_TEST(o)) {
        auto f0 = VM_OBJECT_FLOAT_VALUE(o);
        PyObject* f1 = Py_BuildValue("f", f0);
        return CPythonObject(f1);
    } else if (VM_OBJECT_TEXT_TEST(o)) {
        auto s0 = VM_OBJECT_TEXT_VALUE(o);
        auto s1 = unicode_to_char(s1);
        PyObject* s2 = Py_BuildValue("s", s1);
        delete[] s1;
        return CPythonObject(s2);
    } else {
        return CPythonObject(nullptr);
    }
};

/**
 * A Python machine.
 **/

//## Python:machine - opaque values which are input/output channels
class PythonMachine: public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_PYTHON_OBJECT, PythonMachine, "Python", "machine");

    /*
    PythonMachine(const PythonMachine& m): Opaque(VM_SUB_PYTHON_OBJECT, m.machine(), m.symbol()) {
        _value = m.value();
    }
    */

    VMObjectPtr clone() const override {
        return VMObjectPtr(new PythonMachine(*this)); // XXX: closes and creates?
    }

    int compare(const VMObjectPtr& o) override {
        auto v = (std::static_pointer_cast<PythonMachine>(o))->value();
        if (_value < v) return -1;
        else if (v < _value) return 1;
        else return 0;
    }

    void set_value(ChannelPtr cp) {
        _value = cp;
    }

    CPython value() const {
        return _value;
    }

protected:
    CPythonMachine _value;
};

/**
 * A Python object.
 **/

//## Python:object - opaque Python object values
class PythonObject: public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_PYTHON_OBJECT, PythonObject, "Python", "object");

    PythonObject(const PythonObject& o): Opaque(VM_SUB_PYTHON_OBJECT, chan.machine(), chan.symbol()) {
        _value = o.value();
    }

    VMObjectPtr clone() const override {
        return VMObjectPtr(new PythonObject(*this));
    }

    int compare(const VMObjectPtr& o) override {
        auto v = (std::static_pointer_cast<PyObject>(o))->value();
        if (_value < v) return -1;
        else if (v < _value) return 1;
        else return 0;
    }

    void set_value(CPyObject& v) {
        _value = v;
    }

    void inc_ref() {
        _value->inc_ref();
    }

    void dec_ref() {
    _value->dec_ref();
    }

    CPythonObject value() const {
        return _value;
    }

protected:
    CPythonObject _value;
};

#define PYTHON_OBJECT_TEST(o, sym) \
    ((o->subtag() == VM_SUB_PYTHON_OBJECT) 
#define PYTHON_OBJECT_VALUE(o) \
    ((std::static_pointer_cast<PyObject>(o))->value())

//## Python:from_object - convert a primitive Python object to an Egel object
class PythonFromObject: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonFromObject, "Python", "from_object");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->subtag() == VM_OBJECT_TEXT) {
            auto s = SERVER_OBJECT_CAST(arg0);
        char fn0[] = unicode_to_char(s);
        PyObject* fn1 = PyUnicode_DecodeFSDefault(fn0);
            PyObject* mod = PyImport_Import(fn1);;
        Py_XDECREF(fn1);
        if (mod == nullptr) {
                throw create_text("cannot open module: " + s);
        }
        auto m = PythonObject(mod).clone();
        Py_XDECREF(mod);
        return m;
        } else {
            THROW_INVALID;
        }
    }
}

//## Python:to_object - convert a primitive Egel object to a Python object
class PythonFromObject: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonFromObject, "Python", "from_object");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        try {
            auto p0 = egel_to_python(arg0);
            return PythonObject(
    }
}

//## Python:init nop - create a Python machine
class PythonInit: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonInit, "Python", "init");

    // XXX: TODO: add extra initialization options once
    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        return PythonMachine(machine()).clone();
    }
};


/*
// ## Python:module_run fn - open _and_ run a Python script
class PythonModuleRun: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonModuleRun, "Python", "module_run");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto fn0   = VM_OBJECT_TEXT_VALUE(arg0);
            char fn1[] = unicode_to_char(fn);
            FILE* fp;
            fp = _Py_fopen(fn1, "r"); // XXX: fp is never closed
            if (fp == nullptr) {
                throw create_text("cannot open file: " + fn);
            }
            PyRun_SimpleFile(fp, fn1);
            delete fn1;
            return create_nop();
        } else {
            THROW_INVALID;
        }
    }
}
*/

//## Python:module_import fn - load a module
class PythonModuleImport: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonModuleImport, "Python", "module_import");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto fn0   = VM_OBJECT_TEXT_VALUE(arg0);
            char fn1[] = unicode_to_char(fn0);
            PyObject* fn2 = PyUnicode_DecodeFSDefault(fn1);
            PyObject* mod = PyImport_Import(fn2);;
            Py_XDECREF(fn1);
            if (mod == nullptr) {
                throw create_text("cannot open module: " + s);
            }
            auto m = PythonObject(mod).clone();
            Py_XDECREF(mod);
            return m;
        } else {
            THROW_INVALID;
        }
    }
}
// implement: PyModule_GetDict

//## Python:get_function mod f - retrieve function from module
class PythonFunction: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonFunction, "Python", "get_function");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
    if (PYTHON_OBJECT_TEST(arg0) && VM_OBJECT_TEXT_TEST(arg1)) {
            auto mod = PYTHON_OBJECT_VALUE(arg0);
            auto s   = VM_OBJECT_TEXT_VALUE(arg1);
            auto cc  = unicode_to_char(arg1);

            auto func0 = PyObject_GetAttrString(mod, cc);
            delete cc;
            if (func0 && PyCallable_Check(func0)) {
                return PythonObject(func0).clone();
            } else {
                throw create_text("no function: " + s);
            }
        } else {
            THROW_INVALID;
        }
    }
};

//## Python:get_attribute o n - retrieve attribute from object by name
class PythonGetAttribute: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonGetAttribute, "Python", "get_attribute");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if (PYTHON_OBJECT_TEST(arg0) && VM_OBJECT_TEXT_TEST(arg1)) {
            auto mod = PYTHON_OBJECT_VALUE(arg0);
            auto s   = VM_OBJECT_TEXT_VALUE(arg1);
            auto cc  = unicode_to_char(arg1);

            auto obj0 = PyObject_GetAttrString(mod, cc);
            delete cc;
            if (obj0 {
                return PythonObject(obj0).clone();
            } else {
                throw create_text("no attribute: " + s);
            }
        } else {
            THROW_INVALID;
        }
    }
};

//## Python:set_attribute o n a - set attribute from object by name
class PythonSetAttribute: public Ternary {
public:
    TERNARY_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonSetAttribute, "Python", "set_attribute");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        if (PYTHON_OBJECT_TEST(arg0) && VM_OBJECT_TEXT_TEST(arg1) && PYTHON_OBJECT_TEST(arg2)) {
            auto mod = PYTHON_OBJECT_VALUE(arg0);
            auto s   = VM_OBJECT_TEXT_VALUE(arg1);
            auto a   = VM_OBJECT_TEXT_VALUE(arg2);
            auto cc  = unicode_to_char(arg1);

            auto obj0 = PyObject_SetAttrString(mod, cc, a);
            delete cc;
            if (obj0 {
                return PythonObject(obj0).clone();
            } else {
                throw create_text("no attribute: " + s);
            }
        } else {
            THROW_INVALID;
        }
    }
};

//## Python:get_item o n - retrieve item from object 
class PythonGetItem: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonGetItem, "Python", "get_item");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if (PYTHON_OBJECT_TEST(arg0) && VM_OBJECT_TEXT_TEST(arg1)) {
            auto mod = PYTHON_OBJECT_VALUE(arg0);
            auto s   = VM_OBJECT_TEXT_VALUE(arg1);
            auto cc  = unicode_to_char(arg1);

            auto obj0 = PyObject_GetItemString(mod, cc);
            delete cc;
            if (obj0 {
                return PythonObject(obj0).clone();
            } else {
                throw create_text("no attribute: " + s);
            }
        } else {
            THROW_INVALID;
        }
    }
};

//## Python:set_item o n a - set item from object by item
class PythonSetItem: public Ternary {
public:
    TERNARY_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonSetItem, "Python", "set_item");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        if (PYTHON_OBJECT_TEST(arg0) && VM_OBJECT_TEXT_TEST(arg1) && PYTHON_OBJECT_TEST(arg2)) {
            auto mod = PYTHON_OBJECT_VALUE(arg0);
            auto s   = VM_OBJECT_TEXT_VALUE(arg1);
            auto a   = VM_OBJECT_TEXT_VALUE(arg2);
            auto cc  = unicode_to_char(arg1);

            auto obj0 = PyObject_SetItemString(mod, cc, a);
            delete cc;
            if (obj0 {
                return PythonObject(obj0).clone();
            } else {
                throw create_text("no attribute: " + s);
            }
        } else {
            THROW_INVALID;
        }
    }
};

/* Container conversions.
 *
 * Python     | Egel
 * ---------------------
 * Tuple      | tuple
 * List       | list
 * Dict       | list of pairs
 * Set        | list
 */

#define THROW_STUB throw machine()->create_text("stub")

//## Python:to_tuple l - from egel tuple to a python tuple
class PythonToTuple: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonToTuple, "Python", "to_tuple");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        THROW_STUB;
    }
}

//## Python:from_tuple l - from python tuple to egel tuple
class PythonFromTuple: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonFromTuple, "Python", "from_tuple");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        THROW_STUB;
    }
}

//## Python:to_list l - from egel list to a python list
class PythonToList: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonToList, "Python", "to_list");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        THROW_STUB;
    }
}

//## Python:from_list l - from python list to egel list
class PythonFromList: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonFromList, "Python", "from_list");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        THROW_STUB;
    }
}

//## Python:to_dictionary l - from egel list of tuples to a python dictionary
class PythonToDictionary: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonToDictionary, "Python", "to_dictionary");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        THROW_STUB;
    }
}

//## Python:from_dictionary l - from python dictionary to egel list of tuples
class PythonFromDictionary: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonFromDictionary, "Python", "from_dictionary");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        THROW_STUB;
    }
}

//## Python:to_set l - from egel list to a python set
class PythonToSet: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonToSet, "Python", "to_set");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        THROW_STUB;
    }
}

//## Python:from_set l - from python set to egel list
class PythonFromSet: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonFromSet, "Python", "from_set");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        THROW_STUB;
    }
}

//## Python:call f (x0, ..) - call a python function
class PythonCall: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonCall, "Python", "call");

    VMObjectPtr apply(const VMObjectPtrs& args) const override {
        THROW_STUB;
    }
}

extern "C" std::vector<icu::UnicodeString> egel_imports() {
    return std::vector<icu::UnicodeString>();
}

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

//    oo.push_back(VMObjectData(vm, "Python", "channel").clone());


// hacked TCP protocol
    oo.push_back(Client(vm).clone());

    return oo;
}
