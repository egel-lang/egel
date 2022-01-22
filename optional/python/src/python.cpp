// #define PY_SSIZE_T_CLEAN    // for obscure reasons this goes first
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

#define LIBRARY_VERSION_MAJOR "0"
#define LIBRARY_VERSION_MINOR "0"
#define LIBRARY_VERSION_PATCH "1"

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
class CPythonObject {
public:
    CPythonObject() : _object(nullptr) {
        inc_ref();
    }

    CPythonObject(PyObject* o) : _object(o) {
        inc_ref();
    }

    CPythonObject(const PyObject* o) : _object((PyObject*) o) { // XXX: figure out how to handle const later
        inc_ref();
    }

    CPythonObject(const CPythonObject& o): _object(o._object) {
        inc_ref();
    }

    ~CPythonObject() {
        dec_ref();
        _object= nullptr;
    }

    PyObject* get_object() const {
        return _object;
    }

    PyObject* inc_ref()
    {
        // std::cout << "inc_ref " << _object << std::endl;
        if(_object) Py_XINCREF(_object);
        return _object;
    }

    void dec_ref() {
        // std::cout << "dec_ref " << _object << std::endl;
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
        dec_ref();
        _object = o;
        inc_ref();
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
 * None       | none
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

#define PYTHON_HAS_TYPE(a,b) (a->ob_type == &b)

static PyObject* egel_to_python(VM* machine, const VMObjectPtr& o) {
    if (machine->is_none(o)) {
         return Py_None;
    } else if (machine->is_false(o)) {
         return Py_False;
    } else if (machine->is_true(o)) {
         return Py_True;
    } else if (machine->is_integer(o)) {
        auto n0 = machine->get_integer(o);
        PyObject* n1 = Py_BuildValue("l", n0); // XXX: l = long int; L = long long
        return n1;
    } else if (machine->is_float(o)) {
        auto f0 = machine->get_float(o);
        PyObject* f1 = Py_BuildValue("f", f0);
        return f1;
    } else if (machine->is_char(o)) {
        auto s0 = icu::UnicodeString() + machine->get_char(o);
        char* s1 = unicode_to_char(s0);
        PyObject* s2 = Py_BuildValue("s", s1);
        delete s1;
        return s2;
    } else if (machine->is_text(o)) {
        auto s0 = machine->get_text(o);
        char* s1 = unicode_to_char(s0);
        PyObject* s2 = Py_BuildValue("s", s1);
        delete s1;
        return s2;
    } else {
        return nullptr;
    }
};

static VMObjectPtr python_to_egel(VM* machine, const CPythonObject& object) {
    auto o = object.get_object();
    if (Py_Is(o, Py_None)) {
        return machine->create_none();
    } else if (Py_Is(o, Py_False)) {
        return machine->create_false();
    } else if (Py_Is(o, Py_True)) {
        return machine->create_true();
    } else if (PYTHON_HAS_TYPE(o, PyLong_Type)) {
        vm_int_t n0;
        PyArg_Parse(o, "l", &n0);
        auto n1 = machine->create_integer(n0);
        return n1;
    } else if (PYTHON_HAS_TYPE(o, PyFloat_Type)) {
        vm_float_t f0;
        PyArg_Parse(o, "d", &f0);
        auto f1 = machine->create_float(f0);
        return f1;
    } else if (PYTHON_HAS_TYPE(o, PyUnicode_Type)) {
        char *s0; // XXX: check for possible leak once
        PyArg_Parse(o, "s", &s0);
        auto s1 = icu::UnicodeString(s0);
        auto s2 = machine->create_text(s1);
        return s2;
    } else {
        throw machine->create_text("conversion failed"); 
    }
};

/**
 * A Python machine.
 **/

//## Python::machine - create a python machine
class PythonMachine: public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_PYTHON_OBJECT, PythonMachine, "Python", "machine");

    static VMObjectPtr create(VM* m) {
        return std::make_shared<PythonMachine>(m); // XXX: closes and creates?
    }

    ~PythonMachine() {
        if (_value) delete _value;
    }

    int compare(const VMObjectPtr& o) override {
        return false;
        /* XXX: for later
        auto v = (std::static_pointer_cast<PythonMachine>(o))->value();
        if (_value < v) return -1;
        else if (v < _value) return 1;
        else return 0;
        */
    }

    void run() {
        _value = new CPythonMachine();
    }

    CPythonMachine* value() const {
        return _value;
    }

protected:
    CPythonMachine* _value = nullptr; // pass raw references around cause funny stuff
};

// XXX: this test shouldn't work
#define PYTHON_MACHINE_TEST(o) \
    ((o->subtag() == VM_SUB_PYTHON_OBJECT) && \
     (o->to_text() == "Python:::machine"))
#define PYTHON_MACHINE_CAST(o) \
    std::static_pointer_cast<PythonMachine>(o) 

/**
 * A Python object.
 **/

class PythonObject;
typedef std::shared_ptr<PythonObject>   PythonObjectPtr;

//## Python::object - opaque Python object values
class PythonObject: public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_PYTHON_OBJECT, PythonObject, "Python", "object");

    PythonObject(const PythonObject& o): Opaque(o.subtag(), o.machine(), o.symbol()), _value(o.value()) {
    }

    PythonObject(VM* vm, const PyObject* o): Opaque(VM_SUB_PYTHON_OBJECT, vm, "Python", "object"), _value(o) {
    }

    VMObjectPtr create() const {
        return std::make_shared<PythonObject>(*this);
    }

    static VMObjectPtr create(VM* vm, PyObject* o) {
        return std::make_shared<PythonObject>(vm, o);
    }

    static VMObjectPtr create(VM* vm, const PyObject* o) {
        return create(vm , (PyObject*) o);
    }

    int compare(const VMObjectPtr& o) override {
        auto v = (std::static_pointer_cast<PythonObject>(o))->value();
        // XXX: use python compare
        return false;
    }

    void set_value(CPythonObject& v) {
        _value = v;
    }

    CPythonObject value() const {
        return _value;
    }

    static bool test(const VMObjectPtr& o) {
        return (o->subtag() == VM_SUB_PYTHON_OBJECT);
    }

    static PythonObjectPtr cast(const VMObjectPtr& o) {
        return std::static_pointer_cast<PythonObject>(o);
    }

    static CPythonObject value(const VMObjectPtr& o) {
        return cast(o)->_value;
    }

protected:
    CPythonObject _value;
};

#define PYTHON_OBJECT_TEST(o) \
    PythonObject::test(o)
#define PYTHON_OBJECT_VALUE(o) \
    PythonObject::value(o)

//## Python::run none - create a Python machine
class PythonRun: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonRun, "Python", "run");

    // XXX: TODO: add extra initialization options once
    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_none(arg0)) {
            auto m = PythonMachine::create(machine());
            PYTHON_MACHINE_CAST(m)->run();
            return m;
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::to_object - convert a primitive Egel object to a Python object
class PythonToObject: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonToObject, "Python", "to_object");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        try {
            auto p0 = egel_to_python(machine(), arg0);
            return PythonObject::create(machine(), p0);
        } catch (std::exception& e) {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::from_object - convert a primitive Python object to an Egel object
class PythonFromObject: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonFromObject, "Python", "from_object");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        try {
            if (PYTHON_OBJECT_TEST(arg0)) {
                auto o = PYTHON_OBJECT_VALUE(arg0);
                auto e = python_to_egel(machine(), o);
                return e;
            } else {
                throw machine()->bad_args(this, arg0);
            }
        } catch (std::exception& e) {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::is_none - check for none
class PythonIsNone: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonIsNone, "Python", "is_none");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto o = PYTHON_OBJECT_VALUE(arg0);
            return machine()->create_bool(Py_Is(o, Py_None));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::is_false - check for false
class PythonIsFalse: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonIsFalse, "Python", "is_false");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto o = PYTHON_OBJECT_VALUE(arg0);
            return machine()->create_bool(Py_Is(o, Py_False));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::is_true - check for true
class PythonIsTrue: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonIsTrue, "Python", "is_true");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto o = PYTHON_OBJECT_VALUE(arg0);
            return machine()->create_bool(Py_Is(o, Py_True));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::is_integer - check for integer
class PythonIsInteger: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonIsInteger, "Python", "is_integer");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto o = PYTHON_OBJECT_VALUE(arg0);
            return machine()->create_bool(PYTHON_HAS_TYPE(o, PyLong_Type));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::is_float - check for float
class PythonIsFloat: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonIsFloat, "Python", "is_float");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto o = PYTHON_OBJECT_VALUE(arg0);
            return machine()->create_bool(PYTHON_HAS_TYPE(o, PyFloat_Type));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::is_text - check for text
class PythonIsText: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonIsText, "Python", "is_text");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto o = PYTHON_OBJECT_VALUE(arg0);
            return machine()->create_bool(PYTHON_HAS_TYPE(o, PyUnicode_Type));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::from_integer - convert from integer
class PythonFromInteger: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonFromInteger, "Python", "from_integer");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto o = PYTHON_OBJECT_VALUE(arg0);
            vm_int_t n;
            PyArg_Parse(o, "l", &n);
            return machine()->create_integer(n);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::from_float - convert from float
class PythonFromFloat: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonFromFloat, "Python", "from_float");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto o = PYTHON_OBJECT_VALUE(arg0);
            vm_float_t f;
            PyArg_Parse(o, "d", &f);
            return machine()->create_float(f);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::from_text - convert from text
class PythonFromText: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonFromText, "Python", "from_text");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto o = PYTHON_OBJECT_VALUE(arg0);
            char *s0; // XXX: check for possible leak once
            PyArg_Parse(o, "s", &s0);
            auto s1 = icu::UnicodeString(s0);
            return machine()->create_text(s1);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::get_attribute o n - retrieve attribute from object by name
class PythonGetAttribute: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonGetAttribute, "Python", "get_attribute");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if (PYTHON_OBJECT_TEST(arg0) && VM_OBJECT_TEXT_TEST(arg1)) {
            auto mod  = PYTHON_OBJECT_VALUE(arg0);
            auto s    = machine()->get_text(arg1);
            char* cc  = unicode_to_char(s);

            auto attr = PyObject_GetAttrString(mod, cc);
            delete cc;
            if (attr) {
                return PythonObject::create(machine(), attr);
            } else {
                throw machine()->create_text("no attribute: " + s);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

//## Python::set_attribute mod n a - set attribute from object by name
class PythonSetAttribute: public Ternary {
public:
    TERNARY_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonSetAttribute, "Python", "set_attribute");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        if (PYTHON_OBJECT_TEST(arg0) && VM_OBJECT_TEXT_TEST(arg1) && PYTHON_OBJECT_TEST(arg2)) {
            auto mod = PYTHON_OBJECT_VALUE(arg0);
            auto s   = machine()->get_text(arg1);
            char* n  = unicode_to_char(s);
            auto a   = PYTHON_OBJECT_VALUE(arg2);

            PyObject_SetAttrString(mod, n, a);
            delete n;
            return machine()->create_none();
        } else {
            throw machine()->bad_args(this, arg0, arg1, arg2);
        }
    }
};

//## Python::get_item o n - retrieve item from object 
class PythonGetItem: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonGetItem, "Python", "get_item");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if (PYTHON_OBJECT_TEST(arg0) && PYTHON_OBJECT_TEST(arg1)) {
            auto o = PYTHON_OBJECT_VALUE(arg0);
            auto n = PYTHON_OBJECT_VALUE(arg0);

            auto obj0 = PyObject_GetItem(o, n);

            return PythonObject::create(machine(), obj0);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

//## Python::set_item o key a - set item from object by item
class PythonSetItem: public Ternary {
public:
    TERNARY_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonSetItem, "Python", "set_item");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        if (PYTHON_OBJECT_TEST(arg0) && PYTHON_OBJECT_TEST(arg1) && PYTHON_OBJECT_TEST(arg2)) {
            auto o   = PYTHON_OBJECT_VALUE(arg0);
            auto key = PYTHON_OBJECT_VALUE(arg1);
            auto a   = PYTHON_OBJECT_VALUE(arg2);
            PyObject_SetItem(o, key, a);
            return machine()->create_none();
        } else {
            throw machine()->bad_args(this, arg0, arg1, arg2);
        }
    }
};

/* Container conversions.
 *
 * Python     | Egel
 * ---------------------
 * Tuple      | tuple
 * List       | list
 * Set        | list
 * Dict       | list of pairs
 */


//## Python::is_tuple l - tuple check
class PythonIsTuple: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonIsTuple, "Python", "is_tuple");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto p = PYTHON_OBJECT_VALUE(arg0);
            return m->create_bool(Py_IS_TYPE(p, &PyTuple_Type));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::to_tuple l - from egel tuple to a python tuple
class PythonToTuple: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonToTuple, "Python", "to_tuple");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (m->is_array(arg0) && (m->array_size(arg0) > 0) && (m->is_tuple(m->array_get(arg0, 0)))) {
            auto aa = VM_OBJECT_ARRAY_CAST(arg0);
            auto sz = aa->size();

            // well-formedness check
            for (size_t n = 1; n < sz; n++) {
                if (!PYTHON_OBJECT_TEST(aa->get(n))) throw machine()->bad_args(this, arg0);
            }

            // translation
            PyObject* t;
            t = PyTuple_New(sz-1);
            for (size_t n = 1; n < sz; n++) {
                auto o = PYTHON_OBJECT_VALUE(aa->get(n));
                PyTuple_SetItem(t, n-1, o);
            }
            return PythonObject::create(m, t);
        } else if (m->is_tuple(arg0)) { // handle the case of the empty tuple
            PyObject* t;
            t = PyTuple_New(0);
            return PythonObject::create(m, t);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::from_tuple l - from python tuple to egel tuple
class PythonFromTuple: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonFromTuple, "Python", "from_tuple");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto p = PYTHON_OBJECT_VALUE(arg0);
            if (Py_IS_TYPE(p, &PyTuple_Type)) {
                auto sz = PyTuple_Size(p);
                if (sz == 0) {
                    return m->create_tuple();
                } else {
                    VMObjectPtrs oo;
                    for (int n = 0; n < sz; n++) {
                        auto i0 = PyTuple_GetItem(p, n);
                        auto i1 = PythonObject::create(m, i0);
                        oo.push_back(i1);
                    }
                    return m->to_tuple(oo);
                }
            } else {
                throw machine()->bad_args(this, arg0);
            }
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::is_list l - check list
class PythonIsList: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonIsList, "Python", "is_list");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto p = PYTHON_OBJECT_VALUE(arg0);
            return m->create_bool(Py_IS_TYPE(p, &PyList_Type));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::to_list l - from egel list to a python list
class PythonToList: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonToList, "Python", "to_list");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (m->is_list(arg0)) {
            auto aa = m->from_list(arg0);
            auto sz = aa.size();

            // well-formedness check
            for (unsigned long n = 0; n < sz; n++) {
                if (!PYTHON_OBJECT_TEST(aa[n])) throw machine()->bad_args(this, arg0);
            }

            // translation
            PyObject* t;
            t = PyList_New(sz);
            for (unsigned long n = 0; n < sz; n++) {
                auto o = PYTHON_OBJECT_VALUE(aa[n]);
                PyList_SetItem(t, n, o);
            }
            return PythonObject::create(m, t);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::from_list l - from python list to egel list
class PythonFromList: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonFromList, "Python", "from_list");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto p = PYTHON_OBJECT_VALUE(arg0);
            if (Py_IS_TYPE(p, &PyList_Type)) {
                auto sz = PyList_Size(p);

                VMObjectPtrs oo;
                for (int n = 0; n < sz; n++) {
                    auto i0 = PyList_GetItem(p, n);
                    auto i1 = PythonObject::create(m, i0);
                    oo.push_back(i1);
                }

                return m->to_list(oo);
            } else {
                throw machine()->bad_args(this, arg0);
            }
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::is_set l - python set check
class PythonIsSet: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonIsSet, "Python", "is_set");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto p = PYTHON_OBJECT_VALUE(arg0);
            return m->create_bool(Py_IS_TYPE(p, &PySet_Type));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::to_set l - from egel list to python set
class PythonToSet: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonToSet, "Python", "to_set");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (m->is_list(arg0)) {
            auto aa = m->from_list(arg0);
            auto sz = aa.size();

            // well-formedness check
            for (unsigned long n = 0; n < sz; n++) {
                if (!PYTHON_OBJECT_TEST(aa[n])) throw machine()->bad_args(this, arg0);
            }

            // translation
            PyObject* s;
            s = PySet_New(nullptr);
            for (unsigned long n = 0; n < sz; n++) {
                auto o = PYTHON_OBJECT_VALUE(aa[n]);
                PySet_Add(s, o);
            }
            return PythonObject::create(m, s);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::from_set l - from python set to egel list
class PythonFromSet: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonFromSet, "Python", "from_set");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto p = PYTHON_OBJECT_VALUE(arg0);
            if (Py_IS_TYPE(p, &PySet_Type)) {
                VMObjectPtrs oo;
                auto it = PyObject_GetIter(p);
                while (auto e0 = PyIter_Next(it)) {
                    auto e1 = PythonObject::create(m, e0);
                    oo.push_back(e1);
                }
                return m->to_list(oo);
            } else {
                throw machine()->bad_args(this, arg0);
            }
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::is_dictionary l - from egel list of tuples to a python dictionary
class PythonIsDictionary: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonIsDictionary, "Python", "is_dictionary");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto p = PYTHON_OBJECT_VALUE(arg0);
            return m->create_bool(Py_IS_TYPE(p, &PyDict_Type));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::to_dictionary l - from egel list of tuples to a python dictionary
class PythonToDictionary: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonToDictionary, "Python", "to_dictionary");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        auto aa = m->from_list(arg0);
        auto sz = aa.size();

        // well-formedness check
        for (unsigned long n = 0; n < sz; n++) {
            auto o = aa[n];
            if (m->is_array(o) && (m->array_size(arg0) == 3) 
                  && (m->is_tuple(m->array_get(o, 0)))
                  && (PYTHON_OBJECT_TEST(m->array_get(o,1))) 
                  && (PYTHON_OBJECT_TEST(m->array_get(o,2)))) {
            } else {
                throw machine()->bad_args(this, arg0);
            }
        }

        // translation
        PyObject* s;
        s = PyDict_New();
        for (unsigned long n = 0; n < sz; n++) {
            auto o = aa[n];
            auto key = PYTHON_OBJECT_VALUE(m->array_get(o,1));
            auto val = PYTHON_OBJECT_VALUE(m->array_get(o,2));
            PyDict_SetItem(s, key, val);
        }
        return PythonObject::create(m, s);
    }
};

//## Python::from_dictionary l - from python dictionary to egel list of tuples
class PythonFromDictionary: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonFromDictionary, "Python", "from_dictionary");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto p = PYTHON_OBJECT_VALUE(arg0);
            if (Py_IS_TYPE(p, &PyDict_Type)) {
                VMObjectPtrs oo;

                PyObject *key, *val;
                Py_ssize_t n = 0;

                while (PyDict_Next(p, &n, &key, &val)) {
                    auto k = PythonObject::create(m, key);
                    auto v = PythonObject::create(m, val);
                    VMObjectPtrs tt;
                    tt.push_back(k);
                    tt.push_back(v);
                    oo.push_back(m->to_tuple(tt));
                }

                return m->to_list(oo);
            } else {
                throw machine()->bad_args(this, arg0);
            }
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::directory o - get the object list
class PythonDirectory: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonDirectory, "Python", "directory");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto o = PYTHON_OBJECT_VALUE(arg0);
            auto d = PyObject_Dir(o);
            if (d == nullptr) {
                throw machine()->create_text("no object directory");
            } else {
                auto p = PythonObject::create(m, d);
                return p;
            }
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::is_callable f - callable test
class PythonIsCallable: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonIsCallable, "Python", "is_callable");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();
        if (PYTHON_OBJECT_TEST(arg0)) {
            auto f = PYTHON_OBJECT_VALUE(arg0);
            return m->create_bool(PyCallable_Check(f) != 0);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::apply f (x0, ..) - call a python function with a tuple of arguments
class PythonApply: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonApply, "Python", "apply");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        auto m = machine();
        if ((PYTHON_OBJECT_TEST(arg0)) && (PYTHON_OBJECT_TEST(arg1))) {
            auto f = PYTHON_OBJECT_VALUE(arg0);
            auto x = PYTHON_OBJECT_VALUE(arg1);

            if (Py_IS_TYPE(x, &PyTuple_Type)) {
                auto r = PyObject_Call(f, x, nullptr);
                if (r == nullptr) {
                    auto e = PyErr_Occurred();
                    if (e) {
                        throw PythonObject::create(m, e);
                    } else {
                        throw m->create_text("Python exception");
                    }
                } else {
                    return PythonObject::create(m, r);
                }
            } else {
                throw machine()->bad_args(this, arg0, arg1);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

//## Python::call f (x0, ..) d - call a python function with a tuple and a dictionary
class PythonCall: public Triadic {
public:
    TRIADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonCall, "Python", "call");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        auto m = machine();
        if ((PYTHON_OBJECT_TEST(arg0)) && (PYTHON_OBJECT_TEST(arg1)) && (PYTHON_OBJECT_TEST(arg2))) {
            auto f = PYTHON_OBJECT_VALUE(arg0);
            auto x = PYTHON_OBJECT_VALUE(arg1);
            auto d = PYTHON_OBJECT_VALUE(arg2);

            if ((Py_IS_TYPE(x, &PyTuple_Type)) && (Py_IS_TYPE(d, &PyDict_Type))) {
                auto r = PyObject_Call(f, x, d);
                if (r == nullptr) {
                    auto e = PyErr_Occurred();
                    if (e) {
                        throw PythonObject::create(m, e);
                    } else {
                        throw m->create_text("Python exception");
                    }
                } else {
                    return PythonObject::create(m, r);
                }
            } else {
                throw machine()->bad_args(this, arg0, arg1, arg2);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1, arg2);
        }
    }
};

//## Python::eval s - evaluate a Python code string
class PythonEval: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonEval, "Python", "eval");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s     = machine()->get_text(arg0);
            char* code = unicode_to_char(s);
            PyRun_SimpleString(code);
            delete code;
            auto e = PyErr_Occurred();
            if (e) {
                throw PythonObject::create(machine(), e);
            } else {
                return machine()->create_none();
            }
            return machine()->create_none();
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::eval_file s - evaluate a Python file
class PythonEvalFile: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonEvalFile, "Python", "eval_file");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {

            auto fn0 = machine()->get_text(arg0);
            char* fn1 = unicode_to_char(fn0);
            FILE* fp;
            fp = fopen(fn1, "r"); // XXX: fp is never closed
            if (fp == nullptr) {
                throw machine()->create_text("cannot open file: " + fn0);
            }
            PyRun_SimpleFile(fp, fn1);
            delete fn1;
            fclose(fp);
            auto e = PyErr_Occurred();
            if (e) {
                throw PythonObject::create(machine(), e);
            } else {
                return machine()->create_none();
            }
            return machine()->create_none();
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::module_add s - fetch a loaded module
class PythonModuleAdd: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonModuleAdd, "Python", "module_add");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            auto  m0  = machine()->get_text(arg0);
            char* m1 = unicode_to_char(m0);
            PyObject* mod = PyImport_AddModule(m1);
            delete m1;
            if (mod == nullptr) {
                auto e = PyErr_Occurred();
                if (e) {
                    throw PythonObject::create(machine(), e);
                } else {
                    throw machine()->create_text("cannot add module: " + m0);
                }
            }
            auto m = PythonObject::create(machine(), mod);
            Py_XDECREF(mod);
            return m;
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//## Python::module_import fn - load a module
class PythonModuleImport: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonModuleImport, "Python", "module_import");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            auto fn0   = machine()->get_text(arg0);
            char* fn1  = unicode_to_char(fn0);
            PyObject* fn2 = PyUnicode_DecodeFSDefault(fn1);
            PyObject* mod = PyImport_Import(fn2);
            delete fn1;
            Py_XDECREF(fn2);
            if (mod == nullptr) {
                auto e = PyErr_Occurred();
                if (e) {
                    throw PythonObject::create(machine(), e);
                } else {
                    throw machine()->create_text("cannot open module: " + fn0);
                }
            }
            auto m = PythonObject::create(machine(), mod);
            Py_XDECREF(mod);
            return m;
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};
// implement: PyModule_GetDict

//## Python::get_function mod f - retrieve function from module
class PythonFunction: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_PYTHON_COMBINATOR, PythonFunction, "Python", "get_function");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if (PYTHON_OBJECT_TEST(arg0) && VM_OBJECT_TEXT_TEST(arg1)) {
            auto mod = PYTHON_OBJECT_VALUE(arg0);
            auto s   = machine()->get_text(arg1);
            char* cc = unicode_to_char(s);

            auto func0 = PyObject_GetAttrString(mod, cc);
            delete cc;
            if (func0 && PyCallable_Check(func0)) {
                return PythonObject::create(machine(), func0);
            } else {
                throw machine()->create_text("no function: " + s);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

extern "C" std::vector<icu::UnicodeString> egel_imports() {
    return std::vector<icu::UnicodeString>();
}

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(VMObjectStub::create(vm, "Python::machine"));
    oo.push_back(PythonRun::create(vm));

    oo.push_back(VMObjectStub::create(vm, "Python::object"));
    oo.push_back(PythonToObject::create(vm));
    oo.push_back(PythonFromObject::create(vm));

    oo.push_back(PythonIsNone::create(vm));
    oo.push_back(PythonIsFalse::create(vm));
    oo.push_back(PythonIsTrue::create(vm));
    oo.push_back(PythonIsInteger::create(vm));
    oo.push_back(PythonIsFloat::create(vm));
    oo.push_back(PythonIsText::create(vm));
    oo.push_back(PythonFromInteger::create(vm));
    oo.push_back(PythonFromFloat::create(vm));
    oo.push_back(PythonFromText::create(vm));

    oo.push_back(PythonIsTuple::create(vm));
    oo.push_back(PythonToTuple::create(vm));
    oo.push_back(PythonFromTuple::create(vm));
    oo.push_back(PythonIsList::create(vm));
    oo.push_back(PythonToList::create(vm));
    oo.push_back(PythonFromList::create(vm));
    oo.push_back(PythonIsSet::create(vm));
    oo.push_back(PythonToSet::create(vm));
    oo.push_back(PythonFromSet::create(vm));
    oo.push_back(PythonIsDictionary::create(vm));
    oo.push_back(PythonToDictionary::create(vm));
    oo.push_back(PythonFromDictionary::create(vm));

    oo.push_back(PythonDirectory::create(vm));
    oo.push_back(PythonGetAttribute::create(vm));
    oo.push_back(PythonSetAttribute::create(vm));
    oo.push_back(PythonGetItem::create(vm));
    oo.push_back(PythonSetItem::create(vm));

    oo.push_back(PythonEval::create(vm));
    oo.push_back(PythonEvalFile::create(vm));

    oo.push_back(PythonIsCallable::create(vm));
    oo.push_back(PythonApply::create(vm));
    oo.push_back(PythonCall::create(vm));
    oo.push_back(PythonFunction::create(vm));
    oo.push_back(PythonModuleAdd::create(vm));
    oo.push_back(PythonModuleImport::create(vm));

    return oo;
}
