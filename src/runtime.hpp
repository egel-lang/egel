#ifndef RUNTIME_HPP
#define RUNTIME_HPP

// this is one stand-alone header file external libraries can link against.
// it must be self contained except for standard c++ and unicode (which should be phased out).

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <memory>
#include <sstream>
#include <vector>
#include <set>

#include "unicode/unistr.h"
#include "unicode/ustdio.h"
#include "unicode/uchar.h"
#include "unicode/unistr.h"
#include "unicode/ustream.h"

#ifndef PANIC
#define PANIC(s)    { std::cerr << s << std::endl; exit(1); }
#endif

// libicu doesn't provide escaping..

inline UnicodeString uescape(const UnicodeString& s) {
    UnicodeString s1;
    int i=0;
    int len = s.length();
    for (i = 0; i < len; i++) {
        UChar32 c = s.char32At(i);
        switch (c) {
        case '\a':
            s1 += "\\a";
            break;
        case '\b':
            s1 += "\\b";
            break;
        case '\t':
            s1 += "\\t";
            break;
        case '\n':
            s1 += "\\n";
            break;
        case '\v':
            s1 += "\\v";
            break;
        case '\f':
            s1 += "\\f";
            break;
        case '\r':
            s1 += "\\r";
            break;
        case '\"':
            s1 += "\\\"";
            break;
        case '\'':
            s1 += "\\'";
            break;
        case '\\':
            s1 += "\\\\";
            break;
        default:
            s1 += c;
            break;
        }
    }
    return s1;
}

/**
 * VM objects are
 * + the literals, integer, float, char, and text,
 * + opaque objects -which may hold file handles, pointers, etc.-,
 * + combinators, and arrays.
 **/
typedef enum {
    VM_OBJECT_INTEGER,
    VM_OBJECT_FLOAT,
    VM_OBJECT_CHAR,
    VM_OBJECT_TEXT,
    VM_OBJECT_OPAQUE,
    VM_OBJECT_COMBINATOR,
    VM_OBJECT_ARRAY,
} vm_object_tag_t;

typedef int64_t         vm_int_t;
typedef double          vm_float_t;
typedef UChar32         vm_char_t;
typedef UnicodeString   vm_text_t;

// predefined symbols
#define SYMBOL_INT      0
#define SYMBOL_FLOAT    1
#define SYMBOL_CHAR     2
#define SYMBOL_TEXT     3

typedef uint32_t    symbol_t;
typedef uint32_t    data_t;

typedef enum {
    VM_OBJECT_FLAG_LITERAL    = (1 << 0),
    VM_OBJECT_FLAG_DATA       = (1 << 1),
    VM_OBJECT_FLAG_COMBINATOR = (1 << 2),
    VM_OBJECT_FLAG_INTERNAL   = (1 << 3),
    VM_OBJECT_FLAG_DYNAMIC    = (1 << 4),
    VM_OBJECT_FLAG_OPERATOR   = (1 << 5),
    VM_OBJECT_FLAG_STUB       = (1 << 6),
} vm_object_flag_t;

class VMObject;
typedef std::shared_ptr<VMObject> VMObjectPtr;

class VMObject {
public:
    VMObject(vm_object_tag_t t, vm_object_flag_t f) : _tag(t), _flag(f) {
    }

    vm_object_tag_t tag() const {
        return _tag;
    }

    vm_object_flag_t flag() const {
        return _flag;
    }

    virtual VMObjectPtr clone() const = 0;

    friend std::ostream& operator<<(std::ostream& os, const VMObjectPtr& a) {
        a->render(os);
        return os;
    }

    virtual VMObjectPtr reduce(const VMObjectPtr& thunk) const = 0;

    virtual void render(std::ostream& os) const = 0;

    virtual void debug(std::ostream& os) const = 0;

    virtual symbol_t symbol() const = 0;

    UnicodeString to_text() {
        std::stringstream ss;
        render(ss);
        UnicodeString u(ss.str().c_str());
        return u;
    }

private:
    vm_object_tag_t       _tag;
    vm_object_flag_t      _flag;
};

typedef std::vector<VMObjectPtr> VMObjectPtrs;

// the virtual machine

class VM {
public:
    VM() {};

    // import table manipulation
    virtual bool has_import(const UnicodeString& i) = 0;
    virtual void add_import(const UnicodeString& i) = 0;

    // symbol table manipulation
    virtual symbol_t enter_symbol(const UnicodeString& n) = 0;
    virtual symbol_t enter_symbol(const UnicodeString& n0, const UnicodeString& n1) = 0;
    virtual symbol_t enter_symbol(const std::vector<UnicodeString>& nn, const UnicodeString& n) = 0;
    virtual UnicodeString get_symbol(symbol_t s) = 0;

    // data table manipulation
    virtual data_t enter_data(const VMObjectPtr& o) = 0;
    virtual data_t define_data(const VMObjectPtr& o) = 0;
    virtual VMObjectPtr get_data(const data_t d) = 0;

    // reduce an expression
    virtual void reduce(const VMObjectPtr& e, const VMObjectPtr& ret, const VMObjectPtr& exc) = 0;
    virtual void result_handler(const VMObjectPtr& e) = 0;
    virtual void exception_handler(const VMObjectPtr& e) = 0;
    virtual void reduce(const VMObjectPtr& e) = 0;

    // for threadsafe reductions we lock the vm and rely on C++ threadsafe behavior on containers
    virtual void add_lock() = 0;
    virtual void release_lock() = 0;

    virtual void render(std::ostream& os) = 0;

    // convenience routines
    VMObjectPtr get_data_symbol(const symbol_t t);
    VMObjectPtr get_data_string(const UnicodeString& n);
    VMObjectPtr get_data_string(const UnicodeString& n0, const UnicodeString& n1);
    VMObjectPtr get_data_string(const std::vector<UnicodeString>& nn, const UnicodeString& n);

};


// VM object definitions

class VMObjectLiteral : public VMObject {
public:
    VMObjectLiteral(const vm_object_tag_t &t)
        : VMObject(t, VM_OBJECT_FLAG_LITERAL) {
    }

    void debug(std::ostream& os) const override {
        render(os);
    }

    // note: reduce is defined later in this header file for portability
    VMObjectPtr reduce(const VMObjectPtr& thunk) const override;
};

class VMObjectInteger : public VMObjectLiteral {
public:
    VMObjectInteger(const vm_int_t &v)
        : VMObjectLiteral(VM_OBJECT_INTEGER), _value(v) {
    };

    VMObjectInteger(const VMObjectInteger& l)
        : VMObjectInteger(l.value()) {
    }

    VMObjectPtr clone() const {
        return VMObjectPtr(new VMObjectInteger(*this));
    }

    symbol_t symbol() const override {
        return SYMBOL_INT;
    }

    void render(std::ostream& os) const override {
        os << value();
    }

    vm_int_t value() const {
        return _value;
    }

private:
    vm_int_t    _value;
};

typedef std::shared_ptr<VMObjectInteger> VMObjectIntegerPtr;
#define VM_OBJECT_IS_INTEGER(a) \
    (a-tag() == VM_OBJECT_INTEGER)
#define VM_OBJECT_INTEGER_CAST(a) \
    std::static_pointer_cast<VMObjectInteger>(a)
#define VM_OBJECT_INTEGER_VALUE(a) \
    (VM_OBJECT_INTEGER_CAST(a)->value())

class VMObjectFloat : public VMObjectLiteral {
public:
    VMObjectFloat(const vm_float_t &v)
        : VMObjectLiteral(VM_OBJECT_FLOAT), _value(v) {
    };

    VMObjectFloat(const VMObjectFloat& l)
        : VMObjectFloat(l.value()) {
    }

    VMObjectPtr clone() const {
        return VMObjectPtr(new VMObjectFloat(*this));
    }

    symbol_t symbol() const override {
        return SYMBOL_FLOAT;
    }

    void render(std::ostream& os) const override {
        os << value();
    }

    vm_float_t value() const {
        return _value;
    }

private:
    vm_float_t    _value;
};

typedef std::shared_ptr<VMObjectFloat> VMObjectFloatPtr;
#define VM_OBJECT_FLOAT_CAST(a) \
    std::static_pointer_cast<VMObjectFloat>(a)
#define VM_OBJECT_FLOAT_SPLIT(a, v) \
    auto _##a = VM_OBJECT_FLOAT_CAST(a); \
    auto v    = _##a->value();
#define VM_OBJECT_FLOAT_VALUE(a) \
    (VM_OBJECT_FLOAT_CAST(a)->value())

class VMObjectChar : public VMObjectLiteral {
public:
    VMObjectChar(const vm_char_t &v)
        : VMObjectLiteral(VM_OBJECT_CHAR), _value(v) {
    };

    VMObjectChar(const VMObjectChar& l)
        : VMObjectChar(l.value()) {
    }

    VMObjectPtr clone() const {
        return VMObjectPtr(new VMObjectChar(*this));
    }

    symbol_t symbol() const override {
        return SYMBOL_CHAR;
    }

    void render(std::ostream& os) const override {
        UnicodeString s;
        s = uescape(s+value());
        os << "'" << s << "'";
    }

    vm_char_t value() const {
        return _value;
    }

private:
    vm_char_t    _value;
};

typedef std::shared_ptr<VMObjectChar> VMObjectCharPtr;
#define VM_OBJECT_CHAR_CAST(a) \
    std::static_pointer_cast<VMObjectChar>(a)
#define VM_OBJECT_CHAR_SPLIT(a, v) \
    auto _##a = VM_OBJECT_CHAR_CAST(a); \
    auto v    = _##a->value();
#define VM_OBJECT_CHAR_VALUE(a) \
    (VM_OBJECT_CHAR_CAST(a)->value())

class VMObjectText : public VMObjectLiteral {
public:
    VMObjectText(const UnicodeString &v)
        : VMObjectLiteral(VM_OBJECT_TEXT), _value(v) {
    };

    VMObjectText(const VMObjectText& l)
        : VMObjectText(l.value()) {
    }

    VMObjectPtr clone() const {
        return VMObjectPtr(new VMObjectText(*this));
    }

    symbol_t symbol() const override {
        return SYMBOL_TEXT;
    }

    void render(std::ostream& os) const override {
        UnicodeString s;
        s = uescape(value());
        os << '"' << s << '"';
    }

    UnicodeString value() const {
        return _value;
    }

private:
    UnicodeString    _value;
};

typedef std::shared_ptr<VMObjectText> VMObjectTextPtr;
#define VM_OBJECT_TEXT_CAST(a) \
    std::static_pointer_cast<VMObjectText>(a)
#define VM_OBJECT_TEXT_SPLIT(a, v) \
    auto _##a = VM_OBJECT_TEXT_CAST(a); \
    auto v    = _##a->value();
#define VM_OBJECT_TEXT_VALUE(a) \
    (VM_OBJECT_TEXT_CAST(a)->value())


class VMObjectArray : public VMObject {
public:
    VMObjectArray(const VMObjectPtrs &v)
        : VMObject(VM_OBJECT_ARRAY, VM_OBJECT_FLAG_INTERNAL), _value(v) {
    };

    VMObjectArray(const VMObjectArray& l)
        : VMObjectArray(l.value()) {
    }

    VMObjectPtr clone() const {
        return VMObjectPtr(new VMObjectArray(*this));
    }

    symbol_t symbol() const override {
        return _value[0]->symbol();
    }

    VMObjectPtr get(uint i) const {
        return _value[i];
    }

    void set(uint i, const VMObjectPtr& o) {
        _value[i] = o;
    }

    VMObjectPtr reduce(const VMObjectPtr& thunk) const override;

    void debug(std::ostream& os) const override {
        render(os);
    }

    void render(std::ostream& os) const override {
        os << '(';
        bool first = true;
        for (auto& v:value()) {
            if (first) {
                first = false;
            } else {
                os << ' ';
            }
            if (v == nullptr) {
                os << ".";
            } else {
                v->render(os);
            }
        }
        os << ')';
    }

    VMObjectPtrs value() const {
        return _value;
    }

private:
    VMObjectPtrs  _value;
};

typedef std::shared_ptr<VMObjectArray> VMObjectArrayPtr;
#define VM_OBJECT_ARRAY_CAST(a) \
    std::static_pointer_cast<VMObjectArray>(a)
#define VM_OBJECT_ARRAY_SPLIT(a, v) \
    auto _##a = VM_OBJECT_ARRAY_CAST(a); \
    auto v    = _##a->value();
#define VM_OBJECT_ARRAY_VALUE(a) \
    (VM_OBJECT_ARRAY_CAST(a)->value())

// here we can safely declare reduce
inline VMObjectPtr VMObjectLiteral::reduce(const VMObjectPtr& thunk) const {
    auto tt    = VM_OBJECT_ARRAY_VALUE(thunk);
    auto rt    = tt[0];
    auto rti   = tt[1];
    auto k     = tt[2];
    // auto exc   = tt[3];
    auto c     = tt[4];

    auto index = VM_OBJECT_INTEGER_VALUE(rti);
    auto rta   = VM_OBJECT_ARRAY_CAST(rt);
    rta->set(index, c);

    return k;
}

inline VMObjectPtr VMObjectArray::reduce(const VMObjectPtr& thunk) const {
    auto tt    = VM_OBJECT_ARRAY_VALUE(thunk);
    auto rt    = tt[0];
    auto rti   = tt[1];
    auto k     = tt[2];
    auto exc   = tt[3];
    auto c     = tt[4];

    VMObjectPtrs vv;
    vv.push_back(rt);
    vv.push_back(rti);
    vv.push_back(k);
    vv.push_back(exc);
    auto aa = VM_OBJECT_ARRAY_VALUE(c);
    for (auto& a:aa) {
        vv.push_back(a);
    }
    for (uint n = 5; n < tt.size(); n++) {
        vv.push_back(tt[n]);
    }

    auto t = VMObjectArray(vv).clone();

    return t;
}

class VMObjectOpaque : public VMObject {
public:
    VMObjectOpaque(VM* m, const symbol_t s)
        : VMObject(VM_OBJECT_OPAQUE, VM_OBJECT_FLAG_INTERNAL), _machine(m), _symbol(s) {
    };

    VMObjectOpaque(VM* m, const UnicodeString& n)
        : VMObject(VM_OBJECT_OPAQUE, VM_OBJECT_FLAG_INTERNAL), _machine(m), _symbol(m->enter_symbol(n)) {
    };

    VMObjectOpaque(VM* m, const UnicodeString& n0, const UnicodeString& n1)
        : VMObject(VM_OBJECT_OPAQUE, VM_OBJECT_FLAG_INTERNAL), _machine(m), _symbol(m->enter_symbol(n0, n1)) {
    };

    VMObjectOpaque(VM* m, const std::vector<UnicodeString>& nn, const UnicodeString& n)
        : VMObject(VM_OBJECT_OPAQUE, VM_OBJECT_FLAG_INTERNAL), _machine(m), _symbol(m->enter_symbol(nn, n)) {
    };

    VM* machine() const {
        return _machine;
    }

    symbol_t symbol() const {
        return _symbol;
    }

    UnicodeString text() const {
        return _machine->get_symbol(_symbol);
    }

    VMObjectPtr reduce(const VMObjectPtr& thunk) const override {
        auto tt  = VM_OBJECT_ARRAY_VALUE(thunk);
        auto rt  = tt[0];
        auto rti = tt[1];
        auto k   = tt[2];
        // auto exc   = tt[3];
        // auto c   = tt[4];

        VMObjectPtr ret;
        if (tt.size() > 5) {
            VMObjectPtrs rr;
            for (uint i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            ret = VMObjectArray(rr).clone();
        } else {
            ret = tt[4];
        }

        auto index = VM_OBJECT_INTEGER_VALUE(rti);
        auto rta   = VM_OBJECT_ARRAY_CAST(rt);
        rta->set(index, ret);

        
        return k;
    }

    void debug(std::ostream& os) const override {
        render(os);
    }

    void render(std::ostream& os) const override {
        os << '<' << text() << '>';
    }

    // compare should implement a total order, even if the state changes..
    virtual int compare(const VMObjectPtr& o) = 0;

private:
    VM*         _machine;
    symbol_t    _symbol;
};

typedef std::shared_ptr<VMObjectOpaque> VMObjectOpaquePtr;
#define VM_OBJECT_OPAQUE_CAST(a) \
    std::static_pointer_cast<VMObjectOpaque>(a)
#define VM_OBJECT_OPAQUE_COMPARE(o0, o1) \
    (VM_OBJECT_OPAQUE_CAST(o0))->compare(o1);
#define VM_OBJECT_OPAQUE_SYMBOL(a) \
    (VM_OBJECT_OPAQUE_CAST(a)->symbol())
class VMObjectCombinator : public VMObject {
public:
    VMObjectCombinator(const vm_object_flag_t f, VM* m, const symbol_t s)
        : VMObject(VM_OBJECT_COMBINATOR, f), _machine(m), _symbol(s) {
    };

    VMObjectCombinator(const vm_object_flag_t f, VM* m, const UnicodeString& n)
        : VMObject(VM_OBJECT_COMBINATOR, f), _machine(m), _symbol(m->enter_symbol(n)) {
    };

    VMObjectCombinator(const vm_object_flag_t f, VM* m, const UnicodeString& n0, const UnicodeString& n1)
        : VMObject(VM_OBJECT_COMBINATOR, f), _machine(m), _symbol(m->enter_symbol(n0, n1)) {
    };

    VMObjectCombinator(const vm_object_flag_t f, VM* m, const std::vector<UnicodeString>& nn, const UnicodeString& n)
        : VMObject(VM_OBJECT_COMBINATOR, f), _machine(m), _symbol(m->enter_symbol(nn, n)) {
    };

    VM* machine() const {
        return _machine;
    }

    symbol_t symbol() const {
        return _symbol;
    }

    UnicodeString text() const {
        return _machine->get_symbol(_symbol);
    }

    void debug(std::ostream& os) const override {
        render(os);
    }

    void render(std::ostream& os) const override {
        os << text();
#ifdef DEBUG
        os << " (" << flag() << ") " ;
#endif
    }

private:
    VM*         _machine;
    symbol_t    _symbol;
};

class VMObjectData : public VMObjectCombinator {
public:
    VMObjectData(VM* m, const symbol_t s)
        : VMObjectCombinator(VM_OBJECT_FLAG_DATA, m, s) {
    };

    VMObjectData(VM* m, const UnicodeString& n)
        : VMObjectCombinator(VM_OBJECT_FLAG_DATA, m, n) {
    };

    VMObjectData(VM* m, const UnicodeString& n0, const UnicodeString& n1)
        : VMObjectCombinator(VM_OBJECT_FLAG_DATA, m, n0, n1) {
    };

    VMObjectData(VM* m, const std::vector<UnicodeString>& nn, const UnicodeString& n)
        : VMObjectCombinator(VM_OBJECT_FLAG_DATA, m, nn, n) {
    };

    VMObjectData(const VMObjectData& d)
        : VMObjectData(d.machine(), d.symbol()) {
    }

    VMObjectPtr clone() const override {
        return VMObjectPtr(new VMObjectData(*this));
    }

    VMObjectPtr reduce(const VMObjectPtr& thunk) const override {
        auto tt  = VM_OBJECT_ARRAY_VALUE(thunk);
        auto rt  = tt[0];
        auto rti = tt[1];
        auto k   = tt[2];
        // auto exc   = tt[3];
        // auto c   = tt[4];

        VMObjectPtr ret;
        if (tt.size() > 5) {
            VMObjectPtrs rr;
            for (uint i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            ret = VMObjectArray(rr).clone();
        } else {
            ret = tt[4];
        }

        auto index = VM_OBJECT_INTEGER_VALUE(rti);
        auto rta   = VM_OBJECT_ARRAY_CAST(rt);
        rta->set(index, ret);

        
        return k;
    }
};

typedef std::shared_ptr<VMObjectCombinator> VMObjectCombinatorPtr;
#define VM_OBJECT_COMBINATOR_CAST(a) \
    std::static_pointer_cast<VMObjectCombinator>(a)
#define VM_OBJECT_COMBINATOR_SYMBOL(a) \
    (VM_OBJECT_COMBINATOR_CAST(a)->symbol())

struct CompareVMObjectPtr : public std::binary_function<VMObjectPtr, VMObjectPtr, int>
{
    int operator() (const VMObjectPtr& a0, const VMObjectPtr& a1) const{
        auto t0 = a0->tag();
        auto t1 = a1->tag();
        if (t0 < t1) {
            return -1;
        } else if (t1 < t0) {
            return 1;
        } else {
            switch (t0) {
            case VM_OBJECT_INTEGER: {
                    auto v0 = VM_OBJECT_INTEGER_VALUE(a0);
                    auto v1 = VM_OBJECT_INTEGER_VALUE(a1);
                    if (v0 < v1) return -1;
                    else if (v1 < v0) return 1;
                    else return 0;
                }
                break;
            case VM_OBJECT_FLOAT: {
                    auto v0 = VM_OBJECT_FLOAT_VALUE(a0);
                    auto v1 = VM_OBJECT_FLOAT_VALUE(a1);
                    if (v0 < v1) return -1;
                    else if (v1 < v0) return 1;
                    else return 0;
                }
                break;
            case VM_OBJECT_CHAR: {
                    auto v0 = VM_OBJECT_CHAR_VALUE(a0);
                    auto v1 = VM_OBJECT_CHAR_VALUE(a1);
                    if (v0 < v1) return -1;
                    else if (v1 < v0) return 1;
                    else return 0;
                }
                break;
            case VM_OBJECT_TEXT: {
                    auto v0 = VM_OBJECT_TEXT_VALUE(a0);
                    auto v1 = VM_OBJECT_TEXT_VALUE(a1);
                    if (v0 < v1) return -1;
                    else if (v1 < v0) return 1;
                    else return 0;
                }
                break;
            case VM_OBJECT_OPAQUE: {
                    auto s0 = VM_OBJECT_OPAQUE_SYMBOL(a0);
                    auto s1 = VM_OBJECT_OPAQUE_SYMBOL(a1);
                    if (s0 < s1) return -1;
                    else if (s1 < s0) return 1;
                    else return VM_OBJECT_OPAQUE_COMPARE(a0, a1);
                }
                break;
            case VM_OBJECT_COMBINATOR: {
                    auto v0 = VM_OBJECT_COMBINATOR_SYMBOL(a0);
                    auto v1 = VM_OBJECT_COMBINATOR_SYMBOL(a1);
                    if (v0 < v1) return -1;
                    else if (v1 < v0) return 1;
                    else return 0;
                }
                break;
            case VM_OBJECT_ARRAY: {
                    auto v0 = VM_OBJECT_ARRAY_VALUE(a0);
                    auto v1 = VM_OBJECT_ARRAY_VALUE(a1);
                    auto s0 = v0.size();
                    auto s1 = v1.size();

                    if (s0 < s1) return -1;
                    else if (s1 < s0) return 1;
                    else {
                        for (unsigned int i = 0; i < s0; i++) {
                            auto c = operator()(v0[i], v1[i]);
                            if (c < 0) return -1;
                            if (c > 0) return 1;
                        }
                        return 0;
                    }
                }
                break;
            }
        }
        PANIC("switch failed");
        return 0;
    }
};
struct EqualVMObjectPtr : public std::binary_function<VMObjectPtr, VMObjectPtr, bool>
{
    bool operator() (const VMObjectPtr& a0, const VMObjectPtr& a1) const{
        CompareVMObjectPtr compare;
        return (compare(a0, a1) == 0);
    }
};
struct LessVMObjectPtr : public std::binary_function<VMObjectPtr, VMObjectPtr, bool>
{
    bool operator()(const VMObjectPtr& a0, const VMObjectPtr& a1) const
    {
        CompareVMObjectPtr compare;
        return (compare(a0, a1) == -1);
    }
};
typedef std::set<VMObjectPtr, LessVMObjectPtr> VMObjectPtrSet;

// a stub is used for finding objects by their string or symbol
class VMObjectStub: public VMObjectCombinator {
public:
    VMObjectStub(VM* vm, const symbol_t s)
        : VMObjectCombinator(VM_OBJECT_FLAG_STUB, vm, s) {
     }

     VMObjectStub(const VMObjectStub& d)
        : VMObjectStub(d.machine(), d.symbol()) {
     }

     VMObjectPtr clone() const {
         return VMObjectPtr(new VMObjectStub(*this));
     }

     VMObjectPtr reduce(const VMObjectPtr& thunk) const override {
         std::cerr << "symbol = " << symbol() << std::endl;
         PANIC("reduce on stub");
         return nullptr;
     }
};

inline VMObjectPtr VM::get_data_symbol(const symbol_t s) {
    auto o = VMObjectStub(this, s).clone();
    auto d = enter_data(o);
    return get_data(d);
}

inline VMObjectPtr VM::get_data_string(const UnicodeString& n) {
    auto i = enter_symbol(n);
    return get_data_symbol(i);
}

inline VMObjectPtr VM::get_data_string(const UnicodeString& n0, const UnicodeString& n1) {
    auto i = enter_symbol(n0, n1);
    return get_data_symbol(i);
}

inline VMObjectPtr VM::get_data_string(const std::vector<UnicodeString>& nn, const UnicodeString& n) {
    auto i = enter_symbol(nn, n);
    return get_data_symbol(i);
}

// convenience classes for defining built-in combinators

class Opaque: public VMObjectOpaque {
public:
    Opaque(VM* m, const UnicodeString& n0, const UnicodeString& n1): 
         VMObjectOpaque(m, n0, n1) {
    }

    Opaque(VM* m, const symbol_t s): 
         VMObjectOpaque(m, s) {
    }
};

#define OPAQUE_PREAMBLE(c, n0, n1) \
    c(VM* m): Opaque(m, n0, n1) { \
    } \
    c(VM* m, const symbol_t s): Opaque(m, s) { \
    }

class Medadic: public VMObjectCombinator {
public:
    Medadic(VM* m, const UnicodeString& n0, const UnicodeString& n1): 
         VMObjectCombinator(VM_OBJECT_FLAG_INTERNAL, m, n0, n1) {
    }

    Medadic(VM* m, const symbol_t s): 
         VMObjectCombinator(VM_OBJECT_FLAG_INTERNAL, m, s) {
    }

    virtual VMObjectPtr apply() const = 0;
        
    VMObjectPtr reduce(const VMObjectPtr& thunk) const override {
        auto tt  = VM_OBJECT_ARRAY_VALUE(thunk);
        auto rt  = tt[0];
        auto rti = tt[1];
        auto k   = tt[2];

        VMObjectPtr r;
        if (tt.size() > 4) {
            try {
                r = apply();
                if (r == nullptr) {
                    VMObjectPtrs rr;
                    for (uint i = 4; i<tt.size(); i++) {
                        rr.push_back(tt[i]);
                    }
                    r = VMObjectArray(rr).clone();
                }
            } catch (VMObjectPtr e) {
                auto exc   = tt[3];
                auto ee    = VM_OBJECT_ARRAY_VALUE(exc);

                VMObjectPtrs rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray(rr).clone();
            }
        } else {
            VMObjectPtrs rr;
            for (uint i = 4; i<tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray(rr).clone();
        }

        auto index = VM_OBJECT_INTEGER_VALUE(rti);
        auto rta   = VM_OBJECT_ARRAY_CAST(rt);
        rta->set(index, r);

        return k;
    }
};

#define MEDADIC_PREAMBLE(c, n0, n1) \
    c(VM* m): Medadic(m, n0, n1) { \
    } \
    c(VM* m, const symbol_t s): Medadic(m, s) { \
    } \
    c(const c& o) : c(o.machine(), o.symbol()) { \
    } \
    VMObjectPtr clone() const { \
        return VMObjectPtr(new c(*this)); \
    }

class Monadic: public VMObjectCombinator {
public:
    Monadic(VM* m, const UnicodeString& n0, const UnicodeString& n1): 
         VMObjectCombinator(VM_OBJECT_FLAG_INTERNAL, m, n0, n1) {
    }

    Monadic(VM* m, const symbol_t s): 
         VMObjectCombinator(VM_OBJECT_FLAG_INTERNAL, m, s) {
    }

    virtual VMObjectPtr apply(const VMObjectPtr& arg0) const = 0;
        
    VMObjectPtr reduce(const VMObjectPtr& thunk) const override {
        auto tt  = VM_OBJECT_ARRAY_VALUE(thunk);
        auto rt  = tt[0];
        auto rti = tt[1];
        auto k   = tt[2];

        VMObjectPtr r;
        if (tt.size() > 5) {
            auto arg0 = tt[5];

            try {
                r = apply(arg0);
                if (r == nullptr) {
                    VMObjectPtrs rr;
                    for (uint i = 4; i<tt.size(); i++) {
                        rr.push_back(tt[i]);
                    }
                    r = VMObjectArray(rr).clone();
                }
            } catch (VMObjectPtr e) {
                auto exc   = tt[3];
                auto ee    = VM_OBJECT_ARRAY_VALUE(exc);

                VMObjectPtrs rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray(rr).clone();
            }
        } else {
            VMObjectPtrs rr;
            for (uint i = 4; i<tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray(rr).clone();
        }

        auto index = VM_OBJECT_INTEGER_VALUE(rti);
        auto rta   = VM_OBJECT_ARRAY_CAST(rt);
        rta->set(index, r);

        return k;
    }
};

#define MONADIC_PREAMBLE(c, n0, n1) \
    c(VM* m): Monadic(m, n0, n1) { \
    } \
    c(VM* m, const symbol_t s): Monadic(m, s) { \
    } \
    c(const c& o) : c(o.machine(), o.symbol()) { \
    } \
    VMObjectPtr clone() const { \
        return VMObjectPtr(new c(*this)); \
    }

class Dyadic: public VMObjectCombinator {
public:
    Dyadic(VM* m, const UnicodeString& n0, const UnicodeString& n1): 
         VMObjectCombinator(VM_OBJECT_FLAG_INTERNAL, m, n0, n1) {
    }

    Dyadic(VM* m, const symbol_t s): 
         VMObjectCombinator(VM_OBJECT_FLAG_INTERNAL, m, s) {
    }

    virtual VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const = 0;
        
    VMObjectPtr reduce(const VMObjectPtr& thunk) const override {
        auto tt  = VM_OBJECT_ARRAY_VALUE(thunk);
        auto rt  = tt[0];
        auto rti = tt[1];
        auto k   = tt[2];

        VMObjectPtr r;
        if (tt.size() > 6) {
            auto arg0 = tt[5];
            auto arg1 = tt[6];

            try {
                r = apply(arg0, arg1);
                if (r == nullptr) {
                    VMObjectPtrs rr;
                    for (uint i = 4; i<tt.size(); i++) {
                        rr.push_back(tt[i]);
                    }
                    r = VMObjectArray(rr).clone();
                }
            } catch (VMObjectPtr e) {
                auto exc   = tt[3];
                auto ee    = VM_OBJECT_ARRAY_VALUE(exc);

                VMObjectPtrs rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray(rr).clone();
            }
        } else {
            VMObjectPtrs rr;
            for (uint i = 4; i<tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray(rr).clone();
        }

        auto index = VM_OBJECT_INTEGER_VALUE(rti);
        auto rta   = VM_OBJECT_ARRAY_CAST(rt);
        rta->set(index, r);

        return k;
    }
};

#define DYADIC_PREAMBLE(c, n0, n1) \
    c(VM* m): Dyadic(m, n0, n1) { \
    } \
    c(VM* m, const symbol_t s): Dyadic(m, s) { \
    } \
    c(const c& o) : c(o.machine(), o.symbol()) { \
    } \
    VMObjectPtr clone() const { \
        return VMObjectPtr(new c(*this)); \
    }

class Triadic: public VMObjectCombinator {
public:
    Triadic(VM* m, const UnicodeString& n0, const UnicodeString& n1): 
         VMObjectCombinator(VM_OBJECT_FLAG_INTERNAL, m, n0, n1) {
    }

    Triadic(VM* m, const symbol_t s): 
         VMObjectCombinator(VM_OBJECT_FLAG_INTERNAL, m, s) {
    }

    virtual VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const = 0;
        
    VMObjectPtr reduce(const VMObjectPtr& thunk) const override {
        auto tt  = VM_OBJECT_ARRAY_VALUE(thunk);
        auto rt  = tt[0];
        auto rti = tt[1];
        auto k   = tt[2];

        VMObjectPtr r;
        if (tt.size() > 7) {
            auto arg0 = tt[5];
            auto arg1 = tt[6];
            auto arg2 = tt[7];

            try {
                r = apply(arg0, arg1, arg2);
                if (r == nullptr) {
                    VMObjectPtrs rr;
                    for (uint i = 4; i<tt.size(); i++) {
                        rr.push_back(tt[i]);
                    }
                    r = VMObjectArray(rr).clone();
                }
            } catch (VMObjectPtr e) {
                auto exc   = tt[3];
                auto ee    = VM_OBJECT_ARRAY_VALUE(exc);

                VMObjectPtrs rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray(rr).clone();
            }
        } else {
            VMObjectPtrs rr;
            for (uint i = 4; i<tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray(rr).clone();
        }

        auto index = VM_OBJECT_INTEGER_VALUE(rti);
        auto rta   = VM_OBJECT_ARRAY_CAST(rt);
        rta->set(index, r);

        return k;
    }
};

#define TRIADIC_PREAMBLE(c, n0, n1) \
    c(VM* m): Triadic(m, n0, n1) { \
    } \
    c(VM* m, const symbol_t s): Triadic(m, s) { \
    } \
    c(const c& o) : c(o.machine(), o.symbol()) { \
    } \
    VMObjectPtr clone() const { \
        return VMObjectPtr(new c(*this)); \
    }

#endif
