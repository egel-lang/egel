#pragma once

#include <atomic>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <set>
#include <sstream>
#include <vector>

#include "unicode/uchar.h"
#include "unicode/unistr.h"
#include "unicode/ustdio.h"
#include "unicode/ustream.h"
#include "unicode/ustring.h"

// this is one stand-alone interface file external libraries can link against.
// it must be self contained except for standard c++ and unicode (which should
// be phased out).

using namespace icu;  // use stable namespace

#ifndef PANIC
#define PANIC(s)                     \
    {                                \
        std::cerr << s << std::endl; \
        exit(1);                     \
    }
#endif

constexpr unsigned int EGEL_FLOAT_PRECISION =
    16;  // XXX: dbl::maxdigit doesn't seem to be defined on my system?

// libicu doesn't provide escaping..

inline icu::UnicodeString uescape(const icu::UnicodeString &s) {
    icu::UnicodeString s1;
    int i = 0;
    int len = s.countChar32();
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
enum vm_tag_t {
    VM_OBJECT_INTEGER,
    VM_OBJECT_FLOAT,
    VM_OBJECT_CHAR,
    VM_OBJECT_TEXT,
    VM_OBJECT_OPAQUE,
    VM_OBJECT_COMBINATOR,
    VM_OBJECT_ARRAY,
};

const auto STRING_DCOLON = ":";

icu::UnicodeString combine(const icu::UnicodeString &n0, const icu::UnicodeString &n1) {
        return n0 + STRING_DCOLON + n1;
};

icu::UnicodeString combine(const UnicodeStrings &nn, const icu::UnicodeString &n) {
        icu::UnicodeString s;
        for (auto &n0 : nn) {
            s += n0 + STRING_DCOLON;
        }
        s += n;
        return s;
};

/**
 * VM objects can have subtypes which are _unique_ 'magic' numbers.
 */
using vm_subtag_t = unsigned int;

const vm_subtag_t VM_SUB_DATA = 0x00;      // a combinator data object
const vm_subtag_t VM_SUB_BUILTIN = 0x01;   // a combinator internally defined
const vm_subtag_t VM_SUB_BYTECODE = 0x02;  // a bytecode combinator
const vm_subtag_t VM_SUB_COMPILED = 0x03;  // a compiled bytecode combinator
const vm_subtag_t VM_SUB_EGO = 0x04;       // a combinator from a .ego

const vm_subtag_t VM_SUB_MODULE = 0x0a;  // opaque module combinator

const vm_subtag_t VM_SUB_PYTHON_OBJECT = 0xfe;      // Python values
const vm_subtag_t VM_SUB_PYTHON_COMBINATOR = 0xff;  // Python combinators

using vm_bool_t = bool;
using vm_int_t = int64_t;
using vm_float_t = double;
using vm_char_t = UChar32;
using vm_text_t = icu::UnicodeString;

using symbol_t = uint32_t;
using data_t = uint32_t;

// tagbits (used internally but declared here for vm_object_t)

typedef unsigned int vm_tagbits_t;

// an egel value

union vm_object_t {
    std::atomic<vm_tagbits_t> tagbits;
    vm_object_t* next;
};

class VM;
inline void render(const vm_object_t *o, std::ostream &os);

class VMObject {
public:
    VMObject(const vm_tag_t t, const icu::UnicodeString& s) : _tag(t), _subtag(0), _text(s) {
    }

    VMObject(const vm_tag_t t, const vm_subtag_t st, const icu::UnicodeString &s) : _tag(t), _subtag(st), _text(s) {
    }

    virtual ~VMObject() {  
        // FIX: give a virtual destructor to keep the compiler(-s) happy
    }

    vm_tag_t tag() const {
        return _tag;
    }

    vm_subtag_t subtag() const {
        return _subtag;
    }

    icu::UnicodeString text() const {
        return _text;
    }

    void record(VM* m, symbol_t s) {
        _machine = m;
        _symbol  = s;
    }

    VM* machine() const {
        return _machine;
    }

    symbol_t symbol() const {
        return _symbol;
    }

    bool tag_test(vm_tag_t t) const {
        return _tag == t;
    }

    bool subtag_test(vm_subtag_t t) const {
        return _subtag == t;
    }

    friend std::ostream &operator<<(std::ostream &os, const VMObject &o) {
        os << o.text();
        return os;
    }

    virtual int compare(VMObject* o) = 0;

    virtual vm_object_t* reduce(const vm_object_t *thunk) const = 0;

    void render(std::ostream &os) const;

private:
    vm_tag_t _tag;
    vm_subtag_t _subtag;
    icu::UnicodeString _text;
    VM* _machine;
    symbol_t _symbol;
};

// egel allocation

const unsigned int VM_TAG_BITS = 3;
const vm_tagbits_t VM_TAG_MASK = (1 << VM_TAG_BITS) - 1;
const vm_tagbits_t VM_RC_ONE = 1 << VM_TAG_BITS;

inline vm_tag_t vm_tagbits_tag(const vm_tagbits_t bb) {
    return (vm_tag_t)(bb & VM_TAG_MASK);
};

inline vm_tagbits_t vm_tagbits_rc(const vm_tagbits_t bb) {
    return bb >> VM_TAG_BITS;
};

inline vm_tagbits_t vm_tagbits_inc(const vm_tagbits_t bb) {
    return bb + VM_RC_ONE;
};

inline vm_tagbits_t vm_tagbits_dec(const vm_tagbits_t bb) {
    return bb - VM_RC_ONE;
};

inline vm_tag_t vm_object_tag(const vm_object_t* p) {
    return vm_tagbits_tag(p->tagbits);
};

inline unsigned int vm_object_rc(const vm_object_t* p) {
    return vm_tagbits_rc(p->tagbits);
};

inline void vm_object_inc(vm_object_t* p) {
    bool updated = false;
    while (!updated) {
        vm_tagbits_t bb0 = p->tagbits;
        vm_tagbits_t bb1 = vm_tagbits_inc(bb0);
        updated = std::atomic_compare_exchange_weak(&(p->tagbits), &bb0, bb1);
    }
};

inline void vm_object_free(vm_object_t* p);

inline bool vm_object_dec_prim(vm_object_t* p) {
    bool updated = false;
    bool hit_zero = false;
    while (!updated) {  // an update may fail
        vm_tagbits_t bb0 = p->tagbits;
        vm_tagbits_t bb1 = vm_tagbits_dec(bb0);
        updated = std::atomic_compare_exchange_weak(&(p->tagbits), &bb0, bb1);
        if (updated) {  // only one thread should free a zero refcount
            hit_zero = (vm_tagbits_rc(bb1) == 0);
        }
    }
    return hit_zero;
};

inline void vm_object_dec(vm_object_t* p) {
    if (vm_object_dec_prim(p)) vm_object_free(p);
};

inline vm_object_t* vm_list_append(vm_object_t* p, vm_object_t* ll) {
    p->next = ll;
    return p;
};

inline vm_object_t* vm_list_head(
    vm_object_t* ll) {  // a bit weird for orthogonality
    return ll;
};

inline vm_object_t* vm_list_tail(const vm_object_t* ll) {
    return ll->next;
};

// egel ints

struct vm_object_int_t {
    vm_object_t base;
    int value;
};

inline vm_object_t* vm_int_create(int v) {
    auto p = (vm_object_int_t*)malloc(sizeof(vm_object_int_t));
    p->base.tagbits = VM_RC_ONE | VM_OBJECT_INTEGER;
    p->value = v;
    return (vm_object_t*)p;
};

inline bool vm_is_int(const vm_object_t* p) {
    return vm_object_tag(p) == VM_OBJECT_INTEGER;
};

inline int vm_int_value(const vm_object_t* p) {
    return ((vm_object_int_t*)p)->value;
};

// egel floats

struct vm_object_float_t {
    vm_object_t base;
    float value;
};

inline vm_object_t* vm_float_create(float v) {
    auto p = (vm_object_float_t*)malloc(sizeof(vm_object_float_t));
    p->base.tagbits = VM_RC_ONE | VM_OBJECT_FLOAT;
    p->value = v;
    return (vm_object_t*)p;
};

inline bool vm_is_float(const vm_object_t* p) {
    return vm_object_tag(p) == VM_OBJECT_FLOAT;
};

inline float vm_float_value(const vm_object_t* p) {
    return ((vm_object_float_t*)p)->value;
};

// egel UTF characters/code points

struct vm_object_char_t {
    vm_object_t base;
    UChar32 value;
};

inline vm_object_t* vm_char_create(UChar32 v) {
    auto p = (vm_object_char_t*)malloc(sizeof(vm_object_char_t));
    p->base.tagbits = VM_RC_ONE | VM_OBJECT_CHAR;
    p->value = v;
    return (vm_object_t*)p;
};

inline bool vm_is_char(const vm_object_t* p) {
    return vm_object_tag(p) == VM_OBJECT_CHAR;
};

inline UChar32 vm_char_value(const vm_object_t* p) {
    return ((vm_object_char_t*)p)->value;
};

// egel texts

struct vm_object_text_t {
    vm_object_t base;
    icu::UnicodeString value;
};

inline vm_object_t* vm_text_create(const icu::UnicodeString v) {
    auto p = (vm_object_text_t*)malloc(sizeof(vm_object_text_t));
    p->base.tagbits = VM_RC_ONE | VM_OBJECT_TEXT;
    p->value = v;
    return (vm_object_t*)p;
};

inline bool vm_is_text(const vm_object_t* p) {
    return vm_object_tag(p) == VM_OBJECT_TEXT;
};

inline icu::UnicodeString vm_text_value(const vm_object_t* p) {
    return ((vm_object_text_t*)p)->value;
};

// egel combinators

struct vm_object_combinator_t {
    vm_object_t base;
    VMObject* value;
};

inline vm_object_t* vm_combinator_create(VMObject* v) {
    auto p = (vm_object_combinator_t*)malloc(sizeof(vm_object_combinator_t));
    p->base.tagbits = VM_RC_ONE | VM_OBJECT_COMBINATOR;
    p->value = v;
    return (vm_object_t*)p;
};

inline bool vm_is_combinator(const vm_object_t* p) {
    return vm_object_tag(p) == VM_OBJECT_COMBINATOR;
};

inline VMObject* vm_combinator_value(const vm_object_t* p) {
    return ((vm_object_combinator_t*)p)->value;
};

// egel (mutable) opaque objects

struct vm_object_opaque_t {
    vm_object_t base;
    VMObject* value;
};

inline vm_object_t* vm_opaque_create(VMObject* v) {
    auto p = (vm_object_opaque_t*)malloc(sizeof(vm_object_opaque_t));
    p->base.tagbits = VM_RC_ONE | VM_OBJECT_OPAQUE;
    p->value = v;
    return (vm_object_t*)p;
};

inline bool vm_is_opaque(const vm_object_t* p) {
    return vm_object_tag(p) == VM_OBJECT_OPAQUE;
};

inline VMObject* vm_opaque_value(const vm_object_t* p) {
    return ((vm_object_opaque_t*)p)->value;
};

// egel arrays

struct vm_object_array_t {
    vm_object_t base;
    int size;
    vm_object_t* value[1];
};

inline vm_object_t* vm_array_create(int sz) {
    auto p = (vm_object_array_t*)malloc(sizeof(vm_object_array_t) +
                                        sz * sizeof(vm_object_t*));
    p->base.tagbits = VM_RC_ONE | VM_OBJECT_ARRAY;
    p->size = sz;
    for (int n = 0; n < sz; n++) {
        p->value[n] = nullptr;
    }
    return (vm_object_t*)p;
};

inline bool vm_is_array(const vm_object_t* p) {
    return vm_object_tag(p) == VM_OBJECT_ARRAY;
};

inline int vm_array_size(const vm_object_t* p) {
    return ((vm_object_array_t*)p)->size;
};

inline vm_object_t* vm_array_get(const vm_object_t* p, int n) {
    return ((vm_object_array_t*)p)->value[n];
};

inline void vm_array_set(vm_object_t* p, int n, vm_object_t* v) {
    ((vm_object_array_t*)p)->value[n] = v;
};

// freeing stuff

inline void vm_atom_free(vm_object_t* p) {
    if (vm_is_opaque(p)) {
        auto o = vm_opaque_value(p);
        delete o;
    }
    free(p);
};

inline void vm_array_free(vm_object_t* p) {
    p->next = nullptr;
    vm_object_t* do_list =
        p;  // do list is a list of array values with refcount 0
    vm_object_t* free_list = nullptr;

    while (do_list != nullptr) {
        auto p0 = vm_list_head(do_list);
        do_list = vm_list_tail(do_list);
        free_list = vm_list_append(p0, free_list);

        for (int n = 0; n < vm_array_size(p0); n++) {
            auto p1 = vm_array_get(p0, n);
            if (p1 != nullptr) {
                bool zero = vm_object_dec_prim(p1);
                if (zero) {
                    if (vm_is_array(p1)) {
                        do_list = vm_list_append(p1, do_list);
                    } else {
                        vm_atom_free(p1);
                    }
                }
            }
        }
    }

    while (free_list != nullptr) {
        auto p = vm_list_head(free_list);
        free_list = vm_list_tail(free_list);
        free(p);
    }
};

inline void vm_object_free(vm_object_t* p) {
    if (vm_is_array(p)) {
        vm_array_free(p);
    } else {
        vm_atom_free(p);
    }
};


// convenience
inline vm_subtag_t vm_object_subtag(vm_object_t* p) {
    if (vm_is_opaque(p)) {
        return vm_opaque_value(p)->subtag();
    } if (vm_is_combinator(p)) {
        return vm_combinator_value(p)->subtag();
    } else {
        return 0; // XXX
    }
};

inline symbol_t vm_object_symbol(const vm_object_t* p) {
    if (vm_is_opaque(p)) {
        return vm_opaque_value(p)->symbol();
    } if (vm_is_combinator(p)) {
        return vm_combinator_value(p)->symbol();
    } else {
        return 0;
    }
};

// predefined symbols (indices in symbol table)
const int SYMBOL_INT = 0;
const int SYMBOL_FLOAT = 1;
const int SYMBOL_CHAR = 2;
const int SYMBOL_TEXT = 3;

const int SYMBOL_NONE = 4;
const int SYMBOL_TRUE = 5;
const int SYMBOL_FALSE = 6;

const int SYMBOL_TUPLE = 7;
const int SYMBOL_NIL = 8;
const int SYMBOL_CONS = 9;

inline bool wf_is_tuple(const vm_object_t* o) {
    if ((o!=nullptr) && (vm_is_array(o)) && (vm_array_size(o) > 0)) {
        auto hd = vm_array_get(o, 0);
        return vm_is_combinator(hd) && (vm_object_symbol(hd) == SYMBOL_TUPLE);
    } else {
        return false;
    }
};

inline bool wf_is_nil(const vm_object_t* o) {
    return vm_is_combinator(o) && (vm_object_symbol(o) == SYMBOL_NIL);
};

inline bool wf_is_cons(const vm_object_t* o) {
    if ((o!=nullptr) && (vm_is_array(o)) && (vm_array_size(o) == 3)) {
        auto hd = vm_array_get(o, 0);
        return vm_is_combinator(hd) && (vm_object_symbol(hd) == SYMBOL_CONS);
    } else {
        return false;
    }
};

inline bool wf_is_list(const vm_object_t* o) {
    return wf_is_nil(o) || wf_is_cons(o);
}

inline void render_array(const vm_object_t *o, std::ostream &os) {
    if (vm_is_array(o)) {
        auto sz = vm_array_size(o);
        os << "(";
        for (int n = 0; n < sz - 1; n++) { // XXX: maybe once check for sz = 1 arrays
            render(vm_array_get(o, n), os);
            os << " ";
        }
        render(vm_array_get(o, sz - 1), os);
        os << ")";
    } else {
        PANIC("not an array");
    }
};

inline void render_tuple(const vm_object_t *o, std::ostream &os) {
    if (wf_is_tuple(o)) {
        auto sz = vm_array_size(o);
        if (sz <= 2) {
            render_array(o, os);
        } else {
            os << "(";
            for (int n = 0; n < sz -1; n++) {
                render(vm_array_get(o, n), os);
                os << ", ";
            }
            render(vm_array_get(o, sz - 1), os);
            os << ")";
        }
    } else {
        PANIC("not a tuple");
    }
};

inline void render_nil(const vm_object_t *o, std::ostream &os) {
    os << "{}";
};

inline void render_cons_elements(const vm_object_t *o, std::ostream &os) {
    if (wf_is_cons(o) && (wf_is_nil(vm_array_get(o,2)))) {
            render(vm_array_get(o,1), os);
    } else if (wf_is_cons(o) && (wf_is_cons(vm_array_get(o,2)))) {
        render(vm_array_get(o,1), os);
        os << ", ";
        render_cons_elements(vm_array_get(o,2), os);
    } else if (wf_is_cons(o)) {
        render(vm_array_get(o,1), os);
        os << "| ";
        render(vm_array_get(o,2), os);
    } else {
        PANIC("not a list");
    }
};

inline void render_cons(const vm_object_t *o, std::ostream &os) {
    os << "{";
    render_cons_elements(o, os);
    os << "}";
};

inline void render_list(const vm_object_t *o, std::ostream &os) {
    if (wf_is_nil(o)) {
        render_nil(o,os);
    } else if (wf_is_cons(o)) {
        render_cons(o, os);
    } else {
        PANIC("not a list");
    }
};

inline void render(const vm_object_t *o, std::ostream &os) {
    if (o == nullptr) {
        os << ".";
    } else if (vm_is_int(o)) {
        os << vm_int_value(o);
    } else if (vm_is_float(o)) {
        os << vm_float_value(o);
    } else if (vm_is_char(o)) {
        icu::UnicodeString s;
        s = uescape(s + vm_char_value(o));
        os << "'" << s << "'";
    } else if (vm_is_text(o)) {
        auto s = uescape(vm_text_value(o));
        os << "\"" << s << "\"";
    } else if (vm_is_opaque(o)) {
        os << vm_opaque_value(o)->text();
    } else if (vm_is_combinator(o)) {
        os << vm_combinator_value(o)->text();
    } else if (vm_is_array(o)) {
        if (wf_is_tuple(o)) {
            render_tuple(o, os);
        } else if (wf_is_list(o)) {
            render_list(o, os);
        } else {
            render_array(o, os);
        }
    }
};

int vm_object_compare(const vm_object_t *o0, const vm_object_t *o1) {
    if (o0 == o1) {
        return 0;
    } else if (o0 == nullptr) {
        return -1;
    } else if (o1 == nullptr) {
        return 1;
    }
    auto t0 = vm_object_tag(o0);
    auto t1 = vm_object_tag(o1);
    if (t0 < t1) {
        return -1;
    } else if (t1 < t0) {
        return 1;
    } else {
        switch (t0) {
            case VM_OBJECT_INTEGER: {
                auto v0 = vm_int_value(o0);
                auto v1 = vm_int_value(o1);
                if (v0 < v1)
                    return -1;
                else if (v1 < v0)
                    return 1;
                else
                    return 0;
            } break;
            case VM_OBJECT_FLOAT: {
                auto v0 = vm_float_value(o0);
                auto v1 = vm_float_value(o1);
                if (v0 < v1)
                    return -1;
                else if (v1 < v0)
                    return 1;
                else
                    return 0;
            } break;
            case VM_OBJECT_CHAR: {
                auto v0 = vm_char_value(o0);
                auto v1 = vm_char_value(o1);
                if (v0 < v1)
                    return -1;
                else if (v1 < v0)
                    return 1;
                else
                    return 0;
            } break;
            case VM_OBJECT_TEXT: {
                auto v0 = vm_text_value(o0);
                auto v1 = vm_text_value(o1);
                if (v0 < v1)
                    return -1;
                else if (v1 < v0)
                    return 1;
                else
                    return 0;
            } break;
            case VM_OBJECT_OPAQUE: {
                auto s0 = vm_opaque_value(o0)->symbol();
                auto s1 = vm_opaque_value(o1)->symbol();
                if (s0 < s1)
                    return -1;
                else if (s1 < s0)
                    return 1;
                else
                    return vm_opaque_value(o0)->compare(vm_opaque_value(o1));
            } break;
            case VM_OBJECT_COMBINATOR: {
                auto v0 = vm_combinator_value(o0)->symbol();
                auto v1 = vm_combinator_value(o1)->symbol();
                if (v0 < v1)
                    return -1;
                else if (v1 < v0)
                    return 1;
                else
                    return 0;
            } break;
            case VM_OBJECT_ARRAY: {
                auto s0 = vm_array_size(o0);
                auto s1 = vm_array_size(o1);

                if (s0 < s1)
                    return -1;
                else if (s1 < s0)
                    return 1;
                else {
                    for (int n = 0; n < s0; n++) {
                        auto c = vm_object_compare(vm_array_get(o0,n), vm_array_get(o1,n));
                        if (c < 0) return -1;
                        if (c > 0) return 1;
                    }
                    return 0;
                }
            } break;
        }
    }
    PANIC("switch failed");
    return 0;
};

bool vm_object_less(const vm_object_t* o0, const vm_object_t* o1) {
    return (vm_object_compare(o0, o1) == -1);
};

vm_object_t* reduce(const vm_object_t *thunk) {
    if (vm_is_array(thunk) && (vm_array_size(thunk) >= 5)) {
        auto sz = vm_array_size(thunk);
        
        // take apart the thunk
        auto rt = vm_array_get(thunk, 0);
        auto rti = vm_array_get(thunk, 1);
        auto k = vm_array_get(thunk, 2);
        auto exc = vm_array_get(thunk, 3);
        auto c = vm_array_get(thunk, 4);

        // rewrite according to head combinator
        vm_object_t* ret = nullptr;

        switch (vm_object_tag(c)) {
        case VM_OBJECT_INTEGER:
        case VM_OBJECT_FLOAT:
        case VM_OBJECT_CHAR:
        case VM_OBJECT_TEXT:
        case VM_OBJECT_OPAQUE: {
            if (sz > 5) {
                auto tt = vm_array_create(sz - 4);
                for (int i = 4; i < sz; i++) {
                    vm_array_set(tt, i - 4, vm_array_get(thunk, i));
                }
                ret = tt;
            } else {
                ret = c;
            }
            break;
        };
        case VM_OBJECT_ARRAY: {
            auto sz0 = vm_array_size(c);
            auto newthunk = vm_array_create(sz + sz0 - 1);
            for (int i = 0; i < 4; i++) {
                vm_array_set(newthunk, i, vm_array_get(thunk, i));
            }
            for (int i = 4; i < 4+sz0; i++) {
                vm_array_set(newthunk, i, vm_array_get(c, i-4));
            }
            for (int i = 4+sz0; i < sz + sz0 - 1; i++) {
                vm_array_set(newthunk, i, vm_array_get(thunk, i - sz0 + 1));
            }
            ret = newthunk;
            break;
        };
        case VM_OBJECT_COMBINATOR: {
            ret = vm_combinator_value(c)->reduce(thunk);
            break;
        }
        }

        return ret;
    } else {
        // not an array
        return nullptr; // FIXME: STUB FOR NOW
    }
};

using UnicodeStrings = std::vector<icu::UnicodeString>;

// the virtual machine

// stringlify this once
class Options {
public:
    Options()
        : _interactive_flag(false),
          _tokenize_flag(false),
          _unparse_flag(false),
          _semantical_flag(false),
          _desugar_flag(false),
          _lift_flag(false),
          _bytecode_flag(false) {
        _include_path = UnicodeStrings();
        _library_path = UnicodeStrings();
    }

    Options(bool i, bool t, bool u, bool s, bool d, bool l, bool b,
            const UnicodeStrings &ii, const UnicodeStrings &ll)
        : _interactive_flag(i),
          _tokenize_flag(t),
          _unparse_flag(u),
          _semantical_flag(s),
          _desugar_flag(d),
          _lift_flag(l),
          _bytecode_flag(b),
          _include_path(ii),
          _library_path(ll) {
    }

    Options(const Options &o)
        : _interactive_flag(o._interactive_flag),
          _tokenize_flag(o._tokenize_flag),
          _unparse_flag(o._unparse_flag),
          _semantical_flag(o._semantical_flag),
          _desugar_flag(o._desugar_flag),
          _lift_flag(o._lift_flag),
          _bytecode_flag(o._bytecode_flag),
          _include_path(o._include_path),
          _library_path(o._library_path) {
    }

    static std::shared_ptr<Options> create() {
        return std::make_shared<Options>();
    }

    void set_include_path(const UnicodeStrings &dd) {
        _include_path = dd;
    }

    UnicodeStrings get_include_path() const {
        return _include_path;
    }

    void set_library_path(const UnicodeStrings &dd) {
        _library_path = dd;
    }

    UnicodeStrings get_library_path() const {
        return _library_path;
    }

    void add_include_path(const icu::UnicodeString &p) {
        _include_path.push_back(p);
    }

    void add_library_path(const icu::UnicodeString &p) {
        _library_path.push_back(p);
    }

    void set_interactive(bool f) {
        _interactive_flag = f;
    }

    bool interactive() const {
        return _interactive_flag;
    }

    void set_tokenize(bool f) {
        _tokenize_flag = f;
    }

    bool only_tokenize() const {
        return _tokenize_flag;
    }

    void set_desugar(bool f) {
        _desugar_flag = f;
    }

    bool only_desugar() const {
        return _desugar_flag;
    }

    void set_unparse(bool f) {
        _unparse_flag = f;
    }

    bool only_unparse() const {
        return _unparse_flag;
    }

    void set_semantical(bool f) {
        _semantical_flag = f;
    }

    bool only_semantical() const {
        return _semantical_flag;
    }

    void set_lift(bool f) {
        _lift_flag = f;
    }

    bool only_lift() const {
        return _lift_flag;
    }

    void set_bytecode(bool f) {
        _bytecode_flag = f;
    }

    bool only_bytecode() const {
        return _bytecode_flag;
    }

    void render(std::ostream &os) const {
        os << "interactive:" << _interactive_flag << std::endl;
        os << "tokenize:   " << _tokenize_flag << std::endl;
        os << "unparse:    " << _unparse_flag << std::endl;
        os << "semantical: " << _semantical_flag << std::endl;
        os << "desugar:    " << _desugar_flag << std::endl;
        os << "lift:       " << _lift_flag << std::endl;
        os << "bytecode:   " << _bytecode_flag << std::endl;
        os << "include:    ";
        for (auto &i : _include_path) {
            os << i << ":";
        }
        os << std::endl;
        os << "library:    ";
        for (auto &i : _library_path) {
            os << i << ":";
        }
        os << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const std::shared_ptr<Options> &m) {
        m->render(os);
        return os;
    }

private:
    bool _interactive_flag;
    bool _tokenize_flag;
    bool _unparse_flag;
    bool _semantical_flag;
    bool _desugar_flag;
    bool _lift_flag;
    bool _bytecode_flag;
    UnicodeStrings _include_path;
    UnicodeStrings _library_path;
};

struct VMReduceResult {
    vm_object_t* result;
    bool exception;
};

enum reducer_state_t { RUNNING, SLEEPING, HALTED };

class VM;
using VMPtr = std::shared_ptr<VM>;

using callback_t = std::function<void(VM *vm, const vm_object_t *)>;

class VM {
public:
    VM(){};

    virtual ~VM(){
        // FIX: give a virtual destructor to keep the compiler(-s) happy
    };

    virtual void initialize(std::shared_ptr<Options> oo) = 0;

    // symbol table manipulation
    virtual bool has_symbol(const icu::UnicodeString &n) = 0;
    virtual vm_object_t* get_symbol(const symbol_t s) = 0;
    virtual icu::UnicodeString get_symbol_string(symbol_t s) = 0;
    virtual symbol_t enter_symbol(const icu::UnicodeString &n, const vm_object_t *o) = 0;
    virtual symbol_t enter_symbol(const icu::UnicodeString &n0,
                                  const icu::UnicodeString &n1, const vm_object_t *o) = 0;
    virtual symbol_t enter_symbol(const std::vector<icu::UnicodeString> &nn,
                                  const icu::UnicodeString &n, const vm_object_t *o) = 0;
    virtual int get_symbol_size() = 0;

    // reduce an expression
    virtual void reduce(const vm_object_t *e, const vm_object_t *ret,
                        const vm_object_t *exc, reducer_state_t *run) = 0;
    virtual void reduce(const vm_object_t *e, const vm_object_t *ret,
                        const vm_object_t *exc) = 0;
    virtual VMReduceResult reduce(const vm_object_t *e,
                                  reducer_state_t *run) = 0;
    virtual VMReduceResult reduce(const vm_object_t *e) = 0;

    // for threadsafe reductions we lock the vm and rely on C++ threadsafe
    // behavior on containers
    virtual void lock() = 0;
    virtual void unlock() = 0;

    virtual void render(std::ostream &os) = 0;

    // the VM sometimes needs to peek into it's context, which is the top level
    // evaluator
    virtual void set_context(void *c) = 0;
    virtual void *get_context() const = 0;

    // convenience routines
    virtual vm_object_t* get_combinator(const symbol_t t) = 0;
    virtual vm_object_t* get_combinator(const icu::UnicodeString &n) = 0;
    virtual vm_object_t* get_combinator(const icu::UnicodeString &n0,
                                       const icu::UnicodeString &n1) = 0;
    virtual vm_object_t* get_combinator(
        const std::vector<icu::UnicodeString> &nn,
        const icu::UnicodeString &n) = 0;

    // state
    virtual void define(
        const vm_object_t *o) = 0;  // define an undefined symbol
    virtual void overwrite(
        const vm_object_t *o) = 0;  // define or overwrite a symbol
    virtual vm_object_t* get(const vm_object_t *o) = 0;  // get a defined symbol

    // expose, necessary for switch convenience
    virtual vm_tag_t get_tag(const vm_object_t *o) = 0;
    virtual vm_subtag_t get_subtag(const vm_object_t *o) = 0;

    // creation
    virtual vm_object_t* create_integer(const vm_int_t b) = 0;
    virtual vm_object_t* create_float(const vm_float_t b) = 0;
    virtual vm_object_t* create_char(const vm_char_t b) = 0;
    virtual vm_object_t* create_text(const vm_text_t b) = 0;

    virtual bool is_integer(const vm_object_t *o) = 0;
    virtual bool is_float(const vm_object_t *o) = 0;
    virtual bool is_char(const vm_object_t *o) = 0;
    virtual bool is_text(const vm_object_t *o) = 0;

    virtual vm_int_t get_integer(const vm_object_t *o) = 0;
    virtual vm_float_t get_float(const vm_object_t *o) = 0;
    virtual vm_char_t get_char(const vm_object_t *o) = 0;
    virtual vm_text_t get_text(const vm_object_t *o) = 0;

    virtual vm_object_t* create_none() = 0;
    virtual vm_object_t* create_true() = 0;
    virtual vm_object_t* create_false() = 0;
    virtual vm_object_t* create_bool(const bool b) = 0;
    virtual vm_object_t* create_nil() = 0;
    virtual vm_object_t* create_cons() = 0;
    virtual vm_object_t* create_tuple() = 0;

    virtual bool is_none(const vm_object_t *o) = 0;
    virtual bool is_true(const vm_object_t *o) = 0;
    virtual bool is_false(const vm_object_t *o) = 0;
    virtual bool is_bool(const vm_object_t *o) = 0;
    virtual bool is_nil(const vm_object_t *o) = 0;
    virtual bool is_cons(const vm_object_t *o) = 0;
    virtual bool is_tuple(const vm_object_t *o) = 0;

    virtual vm_object_t* create_array(const unsigned int size) = 0;
    virtual bool is_array(const vm_object_t *o) = 0;
    virtual unsigned int array_size(const vm_object_t *o) = 0;
    virtual vm_object_t* array_get(const vm_object_t *o, int n) = 0;
    virtual void array_set(vm_object_t *o, int n, const vm_object_t *e) = 0;

    virtual vm_object_t* create_array(std::vector<vm_object_t*> &oo) = 0;
    virtual std::vector<vm_object_t*> get_array(const vm_object_t *o) = 0;

    virtual bool is_combinator(const vm_object_t *o) = 0;
    virtual bool is_opaque(const vm_object_t *o) = 0;
    virtual bool is_data(const vm_object_t *o) = 0;
    virtual bool is_bytecode(const vm_object_t *o) = 0;

    virtual vm_text_t symbol(const vm_object_t *o) = 0;

    virtual icu::UnicodeString get_bytecode(const vm_object_t *o) = 0;
    virtual std::vector<vm_object_t*> get_bytedata(const vm_object_t *o) = 0;
    virtual vm_object_t* create_bytecode(const icu::UnicodeString &s) = 0;

    virtual vm_object_t* create_data(const icu::UnicodeString &s) = 0;
    virtual vm_object_t* create_data(const icu::UnicodeString &s0,
                                    const icu::UnicodeString &s1) = 0;
    virtual vm_object_t* create_data(const std::vector<icu::UnicodeString> &ss,
                                    const icu::UnicodeString &s) = 0;

    virtual vm_object_t* create_medadic(const icu::UnicodeString &s,
                                       std::function<vm_object_t*()> f) = 0;
    virtual vm_object_t* create_medadic(const icu::UnicodeString &s0,
                                       const icu::UnicodeString &s1,
                                       std::function<vm_object_t*()> f) = 0;
    virtual vm_object_t* create_medadic(
        const std::vector<icu::UnicodeString> &ss, const icu::UnicodeString &s,
        std::function<vm_object_t*()> f) = 0;

    virtual vm_object_t* create_monadic(
        const icu::UnicodeString &s,
        std::function<vm_object_t*(const vm_object_t *a0)> f) = 0;
    virtual vm_object_t* create_monadic(
        const icu::UnicodeString &s0, const icu::UnicodeString &s1,
        std::function<vm_object_t*(const vm_object_t *a0)> f) = 0;
    virtual vm_object_t* create_monadic(
        const std::vector<icu::UnicodeString> &ss, const icu::UnicodeString &s,
        std::function<vm_object_t*(const vm_object_t *a0)> f) = 0;

    virtual vm_object_t* create_dyadic(
        const icu::UnicodeString &s,
        std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a1)>
            f) = 0;
    virtual vm_object_t* create_dyadic(
        const icu::UnicodeString &s0, const icu::UnicodeString &s1,
        std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a1)>
            f) = 0;
    virtual vm_object_t* create_dyadic(
        const std::vector<icu::UnicodeString> &ss, const icu::UnicodeString &s,
        std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a1)>
            f) = 0;
    /*
        virtual vm_object_t* create_triadic(const icu::UnicodeString& s,
                                           std::function<vm_object_t*(const
       vm_object_t*& a0, vm_object_t*& a1, vm_object_t*& a2)> f) = 0;
        virtual vm_object_t* create_triadic(const icu::UnicodeString& s0, const
       icu::UnicodeString& s1, std::function<vm_object_t*(vm_object_t*& a0,
       vm_object_t*& a1, vm_object_t*& a2)> f) = 0; virtual
       vm_object_t* create_triadic(const std::vector<icu::UnicodeString>& ss,
       const icu::UnicodeString& s, std::function<vm_object_t*(vm_object_t*&
       a0, vm_object_t*& a1, vm_object_t*& a2)> f) = 0;

        virtual vm_object_t* create_variadic(const icu::UnicodeString& s,
                                            std::function<vm_object_t*(const
       vm_object_t*s& aa)> f) = 0; virtual vm_object_t* create_variadic(const
       icu::UnicodeString& s0, const icu::UnicodeString& s1,
                                            std::function<vm_object_t*(const
       vm_object_t*s& aa)> f) = 0; virtual vm_object_t* create_variadic(const
       std::vector<icu::UnicodeString>& ss, const icu::UnicodeString& s,
                                            std::function<vm_object_t*(const
       vm_object_t*s& aa)> f) = 0;
    */

    // convenience (appends and strips tuple tag)
    virtual vm_object_t* to_tuple(std::vector<vm_object_t*> &oo) = 0;
    virtual std::vector<vm_object_t*> from_tuple(const vm_object_t *tt) = 0;

    // convenience (for internal usage)
    virtual bool is_list(const vm_object_t *o) = 0;
    virtual vm_object_t* to_list(std::vector<vm_object_t*> &oo) = 0;
    virtual std::vector<vm_object_t*> from_list(const vm_object_t *o) = 0;

    // evaluation
    virtual void eval_line(const icu::UnicodeString &in, const callback_t &main,
                           const callback_t &exc) = 0;
    virtual void eval_module(const icu::UnicodeString &fn) = 0;
    virtual void eval_command(const icu::UnicodeString &l) = 0;
    virtual void eval_main() = 0;
    virtual void eval_interactive() = 0;

    // module inspection
    virtual vm_object_t* query_modules() = 0;
    virtual bool is_module(const vm_object_t *m) = 0;
    virtual vm_object_t* query_module_name(const vm_object_t *m) = 0;
    virtual vm_object_t* query_module_path(const vm_object_t *m) = 0;
    virtual vm_object_t* query_module_imports(const vm_object_t *m) = 0;
    virtual vm_object_t* query_module_exports(const vm_object_t *m) = 0;
    virtual vm_object_t* query_module_values(const vm_object_t *m) = 0;

    // machine state
    virtual vm_object_t* query_symbols() = 0;
    virtual vm_object_t* query_data() = 0;

    virtual int compare(const vm_object_t *o0, const vm_object_t *o1) = 0;

    virtual vm_object_t* bad(const VMObject *o, const icu::UnicodeString &e) = 0;
    virtual vm_object_t* bad_args(const VMObject *o, const vm_object_t *a0) = 0;
    virtual vm_object_t* bad_args(const VMObject *o, const vm_object_t *a0,
                                 const vm_object_t *a1) = 0;
    virtual vm_object_t* bad_args(const VMObject *o, const vm_object_t *a0,
                                 const vm_object_t *a1,
                                 const vm_object_t *a2) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// stuff below is either for internal usage or for implementations which just
// need that bit of extra speed

// VM object definitions



// here we can safely declare reduce
inline vm_object_t* VMObjectLiteral::reduce(const vm_object_t *thunk) const {
    auto tt = VM_OBJECT_ARRAY_CAST(thunk);
    // optimize a bit for the case it's either a sole literal or an applied
    // literal
    if (tt->size() == 5) {
        auto rt = tt->get(0);
        auto rti = tt->get(1);
        auto k = tt->get(2);
        // auto exc   = tt[3];
        auto c = tt->get(4);

        auto index = VM_OBJECT_INTEGER_VALUE(rti);
        auto rta = VM_OBJECT_ARRAY_CAST(rt);
        rta->set(index, c);

        return k;
    } else {
        auto rt = tt->get(0);
        auto rti = tt->get(1);
        auto k = tt->get(2);

        vm_object_t*s vv;
        for (size_t n = 4; n < tt->size(); n++) {
            vv.push_back(tt->get(n));
        }

        auto r = VMObjectArray::create(vv);

        auto index = VM_OBJECT_INTEGER_VALUE(rti);
        auto rta = VM_OBJECT_ARRAY_CAST(rt);
        rta->set(index, r);

        return k;
    }
};

inline vm_object_t* VMObjectArray::reduce(const vm_object_t *thunk) const {
    auto tt = VM_OBJECT_ARRAY_VALUE(thunk);
    auto rt = tt[0];
    auto rti = tt[1];
    auto k = tt[2];
    auto exc = tt[3];
    auto c = tt[4];

    vm_object_t*s vv;
    vv.push_back(rt);
    vv.push_back(rti);
    vv.push_back(k);
    vv.push_back(exc);
    auto aa = VM_OBJECT_ARRAY_VALUE(c);
    for (auto &a : aa) {
        vv.push_back(a);
    }
    for (unsigned int n = 5; n < tt.size(); n++) {
        vv.push_back(tt[n]);
    }

    auto t = VMObjectArray::create(vv);

    return t;
}

class VMObjectOpaque : public VMObject {
public:
    VMObjectOpaque(const vm_subtag_t t, const icu::UnicodeString &n)
        : VMObject(VM_OBJECT_OPAQUE, t, n) {};

    VMObjectOpaque(const vm_subtag_t t, VM *m, const icu::UnicodeString &n0,
                   const icu::UnicodeString &n1)
        : VMObject(VM_OBJECT_OPAQUE, t, n0, n1) {};

    VMObjectOpaque(const vm_subtag_t t, VM *m,
                   const std::vector<icu::UnicodeString> &nn,
                   const icu::UnicodeString &n)
        : VMObject(VM_OBJECT_OPAQUE, t),
          _machine(m),
          _symbol(m->enter_symbol(nn, n)){};

    VM *machine() const {
        return _machine;
    }

    symbol_t symbol() const override {
        return _symbol;
    }

    icu::UnicodeString text() const {
        return _machine->get_combinator_string(_symbol);
    }

    vm_object_t* reduce(const vm_object_t *thunk) const override {
        auto tt = VM_OBJECT_ARRAY_VALUE(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];
        // auto exc   = tt[3];
        // auto c   = tt[4];

        vm_object_t* ret;
        if (tt.size() > 5) {
            vm_object_t*s rr;
            for (unsigned int i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            ret = VMObjectArray::create(rr);
        } else {
            ret = tt[4];
        }

        auto index = VM_OBJECT_INTEGER_VALUE(rti);
        auto rta = VM_OBJECT_ARRAY_CAST(rt);
        rta->set(index, ret);

        return k;
    }

    void debug(std::ostream &os) const override {
        render(os);
    }

    void render(std::ostream &os) const override {
        os << '<' << text() << '>';
    }

private:
    VM *_machine;
    symbol_t _symbol;
};

using VMObjectOpaquePtr = std::shared_ptr<VMObjectOpaque>;
#define VM_OBJECT_OPAQUE_TEST(a) (a->tag() == VM_OBJECT_OPAQUE)
#define VM_OBJECT_OPAQUE_CAST(a) std::static_pointer_cast<VMObjectOpaque>(a)
#define VM_OBJECT_OPAQUE_COMPARE(o0, o1) \
    (VM_OBJECT_OPAQUE_CAST(o0))->compare(o1);
#define VM_OBJECT_OPAQUE_SYMBOL(a) (VM_OBJECT_OPAQUE_CAST(a)->symbol())

class VMObjectCombinator : public VMObject {
public:
    VMObjectCombinator(const vm_subtag_t t, VM *m, const symbol_t s)
        : VMObject(VM_OBJECT_COMBINATOR, t), _machine(m), _symbol(s){};

    VMObjectCombinator(const vm_subtag_t t, VM *m, const icu::UnicodeString &n)
        : VMObject(VM_OBJECT_COMBINATOR, t),
          _machine(m),
          _symbol(m->enter_symbol(n)){};

    VMObjectCombinator(const vm_subtag_t t, VM *m, const icu::UnicodeString &n0,
                       const icu::UnicodeString &n1)
        : VMObject(VM_OBJECT_COMBINATOR, t),
          _machine(m),
          _symbol(m->enter_symbol(n0, n1)){};

    VMObjectCombinator(const vm_subtag_t t, VM *m,
                       const std::vector<icu::UnicodeString> &nn,
                       const icu::UnicodeString &n)
        : VMObject(VM_OBJECT_COMBINATOR, t),
          _machine(m),
          _symbol(m->enter_symbol(nn, n)){};

    VM *machine() const {
        return _machine;
    }

    symbol_t symbol() const override {
        return _symbol;
    }

    icu::UnicodeString text() const {
        if (_symbol == SYMBOL_NIL) {
            return "{}";
        } else {
            return _machine->get_combinator_string(_symbol);
        }
    }

    void debug(std::ostream &os) const override {
        render(os);
    }

    void render(std::ostream &os) const override {
        os << text();
    }

private:
    VM *_machine;
    symbol_t _symbol;
};

class VMObjectData : public VMObjectCombinator {
public:
    VMObjectData(VM *m, const symbol_t s)
        : VMObjectCombinator(VM_SUB_DATA, m, s){};

    VMObjectData(VM *m, const icu::UnicodeString &n)
        : VMObjectCombinator(VM_SUB_DATA, m, n){};

    VMObjectData(VM *m, const icu::UnicodeString &n0,
                 const icu::UnicodeString &n1)
        : VMObjectCombinator(VM_SUB_DATA, m, n0, n1){};

    VMObjectData(VM *m, const std::vector<icu::UnicodeString> &nn,
                 const icu::UnicodeString &n)
        : VMObjectCombinator(VM_SUB_DATA, m, nn, n){};

    VMObjectData(const VMObjectData &d)
        : VMObjectData(d.machine(), d.symbol()) {
    }

    static vm_object_t* create(VM *vm, const symbol_t s) {
        return vm_combinator_create(new VMObjectData(vm, s));
    }

    static vm_object_t* create(VM *vm, const icu::UnicodeString &s) {
        auto sym = vm->enter_symbol(s);
        return VMObjectData::create(vm, sym);
    }

    static vm_object_t* create(VM *vm, const icu::UnicodeString &s0,
                              const icu::UnicodeString &s1) {
        auto sym = vm->enter_symbol(s0, s1);
        return VMObjectData::create(vm, sym);
    }

    static vm_object_t* create(VM *vm, const UnicodeStrings &ss,
                              const icu::UnicodeString &s) {
        auto sym = vm->enter_symbol(ss, s);
        return VMObjectData::create(vm, sym);
    }

    vm_object_t* reduce(const vm_object_t *thunk) const override {
        auto tt = VM_OBJECT_ARRAY_VALUE(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];
        // auto exc   = tt[3];
        // auto c   = tt[4];

        vm_object_t* ret;
        if (tt.size() > 5) {
            vm_object_t*s rr;
            for (unsigned int i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            ret = VMObjectArray::create(rr);
        } else {
            ret = tt[4];
        }

        auto index = VM_OBJECT_INTEGER_VALUE(rti);
        auto rta = VM_OBJECT_ARRAY_CAST(rt);
        rta->set(index, ret);

        return k;
    }
};

struct Equalvm_object_t* {
    bool operator()(const vm_object_t *a0, const vm_object_t *a1) const {
        Comparevm_object_t* compare;
        return (compare(a0, a1) == 0);
    }
};
struct Lessvm_object_t* {
    bool operator()(const vm_object_t *a0, const vm_object_t *a1) const {
        Comparevm_object_t* compare;
        return (compare(a0, a1) == -1);
    }
};

// the throw combinator, used in the runtime
class VMThrow : public VMObjectCombinator {
public:
    VMThrow(VM *m) : VMObjectCombinator(VM_SUB_BUILTIN, m, "System", "throw") {
    }

    VMThrow(const VMThrow &t) : VMThrow(t.machine()) {
    }

    static vm_object_t* create(VM *m) {
        return std::make_shared<VMThrow>(m);
    }

    vm_object_t* reduce(const vm_object_t *thunk) const override {
        // when throw is reduced, it takes the exception, inserts it argument,
        // and reduces that

        auto tt = VM_OBJECT_ARRAY_VALUE(thunk);
        // auto rt  = tt[0];
        // auto rti = tt[1];
        // auto k   = tt[2];
        auto exc = tt[3];
        // auto c   = tt[4];
        auto r = tt[5];

        auto ee = VM_OBJECT_ARRAY_CAST(exc);
        ee->set(5, r);

        return exc;
    }
};

// convenience classes for defining built-in combinators
// XXX: deceprate this

class Opaque : public VMObjectOpaque {
public:
    Opaque(const vm_subtag_t t, VM *m, const icu::UnicodeString &n0,
           const icu::UnicodeString &n1)
        : VMObjectOpaque(t, m, n0, n1) {
    }

    Opaque(const vm_subtag_t t, VM *m, const symbol_t s)
        : VMObjectOpaque(t, m, s) {
    }
};

#define OPAQUE_PREAMBLE(t, c, n0, n1)              \
    c(VM *m) : Opaque(t, m, n0, n1) {              \
    }                                              \
    c(VM *m, const symbol_t s) : Opaque(t, m, s) { \
    }

// convenience classes for combinators which take and return constants

class Medadic : public VMObjectCombinator {
public:
    Medadic(const vm_subtag_t t, VM *m, const icu::UnicodeString &n0,
            const icu::UnicodeString &n1)
        : VMObjectCombinator(t, m, n0, n1) {
    }

    Medadic(const vm_subtag_t t, VM *m, const symbol_t s)
        : VMObjectCombinator(t, m, s) {
    }

    virtual vm_object_t* apply() const = 0;

    vm_object_t* reduce(const vm_object_t *thunk) const override {
        auto tt = VM_OBJECT_ARRAY_VALUE(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];

        vm_object_t* r;
        if (tt.size() > 4) {
            try {
                r = apply();
                if (r == nullptr) {
                    vm_object_t*s rr;
                    for (unsigned int i = 4; i < tt.size(); i++) {
                        rr.push_back(tt[i]);
                    }
                    r = VMObjectArray::create(rr);
                }
            } catch (vm_object_t* e) {
                auto exc = tt[3];
                auto ee = VM_OBJECT_ARRAY_VALUE(exc);

                vm_object_t*s rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray::create(rr);
            }
        } else {
            vm_object_t*s rr;
            for (unsigned int i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
        }

        // also return spurious arguments
        if (tt.size() > 5) {
            vm_object_t*s rr;
            rr.push_back(r);
            for (unsigned int i = 5; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
        }

        auto index = VM_OBJECT_INTEGER_VALUE(rti);
        auto rta = VM_OBJECT_ARRAY_CAST(rt);
        rta->set(index, r);

        return k;
    }
};

#define MEDADIC_PREAMBLE(t, c, n0, n1)              \
    c(VM *m) : Medadic(t, m, n0, n1) {              \
    }                                               \
    c(VM *m, const symbol_t s) : Medadic(t, m, s) { \
    }                                               \
    c(const c &o) : c(o.machine(), o.symbol()) {    \
    }                                               \
    static vm_object_t* create(VM *m) {              \
        return std::shared_ptr<c>(new c(m));        \
    }

class MedadicCallback : Medadic {
public:
    MedadicCallback(VM *m, const icu::UnicodeString &s0,
                    const icu::UnicodeString &s1,
                    std::function<vm_object_t*()> f)
        : Medadic(VM_SUB_BUILTIN, m, s0, s1), _value(f) {
    }

    MedadicCallback(VM *m, const symbol_t s, std::function<vm_object_t*()> f)
        : Medadic(VM_SUB_BUILTIN, m, s), _value(f) {
    }

    MedadicCallback(const MedadicCallback &o)
        : MedadicCallback(o.machine(), o.symbol(), o.value()) {
    }

    static vm_object_t* create(VM *m, const symbol_t s,
                              std::function<vm_object_t*()> f) {
        return vm_object_t*((VMObject *)new MedadicCallback(m, s, f));
    }

    std::function<vm_object_t*()> value() const {
        return _value;
    }

    vm_object_t* apply() const override {
        return (_value)();
    }

    static vm_object_t* create(VM *m, const icu::UnicodeString &s,
                              std::function<vm_object_t*()> f) {
        auto sym = m->enter_symbol(s);
        return MedadicCallback::create(m, sym, f);
    }

    static vm_object_t* create(VM *m, const icu::UnicodeString &s0,
                              const icu::UnicodeString &s1,
                              std::function<vm_object_t*()> f) {
        auto sym = m->enter_symbol(s0, s1);
        return MedadicCallback::create(m, sym, f);
    }

    static vm_object_t* create(VM *m, const UnicodeStrings &ss,
                              const icu::UnicodeString &s,
                              std::function<vm_object_t*()> f) {
        auto sym = m->enter_symbol(ss, s);
        return MedadicCallback::create(m, sym, f);
    }

private:
    std::function<vm_object_t*()> _value;
};

class Monadic : public VMObjectCombinator {
public:
    Monadic(const vm_subtag_t t, VM *m, const icu::UnicodeString &n0,
            const icu::UnicodeString &n1)
        : VMObjectCombinator(t, m, n0, n1) {
    }

    Monadic(const vm_subtag_t t, VM *m, const symbol_t s)
        : VMObjectCombinator(t, m, s) {
    }

    virtual vm_object_t* apply(const vm_object_t *arg0) const = 0;

    vm_object_t* reduce(const vm_object_t *thunk) const override {
        auto tt = VM_OBJECT_ARRAY_VALUE(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];

        vm_object_t* r;
        if (tt.size() > 5) {
            auto arg0 = tt[5];

            try {
                r = apply(arg0);
                if (r == nullptr) {
                    vm_object_t*s rr;
                    for (unsigned int i = 4; i < tt.size(); i++) {
                        rr.push_back(tt[i]);
                    }
                    r = VMObjectArray::create(rr);
                }
            } catch (vm_object_t* e) {
                auto exc = tt[3];
                auto ee = VM_OBJECT_ARRAY_VALUE(exc);

                vm_object_t*s rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray::create(rr);
            }
        } else {
            vm_object_t*s rr;
            for (unsigned int i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
        }

        // also return spurious arguments
        if (tt.size() > 6) {
            vm_object_t*s rr;
            rr.push_back(r);
            for (unsigned int i = 6; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
        }

        auto index = VM_OBJECT_INTEGER_VALUE(rti);
        auto rta = VM_OBJECT_ARRAY_CAST(rt);
        rta->set(index, r);

        return k;
    }
};

#define MONADIC_PREAMBLE(t, c, n0, n1)              \
    c(VM *m) : Monadic(t, m, n0, n1) {              \
    }                                               \
    c(VM *m, const symbol_t s) : Monadic(t, m, s) { \
    }                                               \
    c(const c &o) : c(o.machine(), o.symbol()) {    \
    }                                               \
    static vm_object_t* create(VM *m) {              \
        return std::shared_ptr<c>(new c(m));        \
    }

class MonadicCallback : public Monadic {
public:
    MonadicCallback(VM *m, const icu::UnicodeString &s0,
                    const icu::UnicodeString &s1,
                    std::function<vm_object_t*(const vm_object_t *a0)> f)
        : Monadic(VM_SUB_BUILTIN, m, s0, s1), _value(f) {
    }

    MonadicCallback(VM *m, const symbol_t s,
                    std::function<vm_object_t*(const vm_object_t *a0)> f)
        : Monadic(VM_SUB_BUILTIN, m, s), _value(f) {
    }

    MonadicCallback(const MonadicCallback &o)
        : MonadicCallback(o.machine(), o.symbol(), o.value()) {
    }

    ~MonadicCallback() {
    }

    static vm_object_t* create(
        VM *m, const symbol_t sym,
        std::function<vm_object_t*(const vm_object_t *a0)> f) {
        // return std::shared_ptr<MonadicCallback>(new MonadicCallback(m, sym,
        // f));
        return std::shared_ptr<MonadicCallback>(new MonadicCallback(m, sym, f));
    }

    static vm_object_t* create(
        VM *m, const icu::UnicodeString &s,
        std::function<vm_object_t*(const vm_object_t *a0)> f) {
        auto sym = m->enter_symbol(s);
        return MonadicCallback::create(m, sym, f);
    }

    static vm_object_t* create(
        VM *m, const icu::UnicodeString &s0, const icu::UnicodeString &s1,
        std::function<vm_object_t*(const vm_object_t *a0)> f) {
        auto sym = m->enter_symbol(s0, s1);
        return MonadicCallback::create(m, sym, f);
    }

    static vm_object_t* create(
        VM *m, const UnicodeStrings &ss, const icu::UnicodeString &s,
        std::function<vm_object_t*(const vm_object_t *a0)> f) {
        auto sym = m->enter_symbol(ss, s);
        return MonadicCallback::create(m, sym, f);
    }

    std::function<vm_object_t*(const vm_object_t *a0)> value() const {
        return _value;
    }

    vm_object_t* apply(const vm_object_t *arg0) const override {
        return (_value)(arg0);
    }

private:
    std::function<vm_object_t*(const vm_object_t *a0)> _value;
};

class Dyadic : public VMObjectCombinator {
public:
    Dyadic(const vm_subtag_t t, VM *m, const icu::UnicodeString &n0,
           const icu::UnicodeString &n1)
        : VMObjectCombinator(t, m, n0, n1) {
    }

    Dyadic(const vm_subtag_t t, VM *m, const symbol_t s)
        : VMObjectCombinator(t, m, s) {
    }

    virtual vm_object_t* apply(const vm_object_t *arg0,
                              const vm_object_t *arg1) const = 0;

    vm_object_t* reduce(const vm_object_t *thunk) const override {
        auto m = machine();
        auto rt = m->array_get(thunk, 0);
        auto rti = m->array_get(thunk, 1);
        auto k = m->array_get(thunk, 2);

        vm_object_t* r;
        if (m->array_size(thunk) > 6) {
            auto arg0 = m->array_get(thunk, 5);
            auto arg1 = m->array_get(thunk, 6);

            try {
                r = apply(arg0, arg1);
                if (r == nullptr) {
                    vm_object_t*s rr;
                    for (unsigned int i = 4;
                         i < (unsigned int)m->array_size(thunk); i++) {
                        rr.push_back(m->array_get(thunk, i));
                    }
                    r = VMObjectArray::create(rr);
                }
            } catch (vm_object_t* e) {
                auto exc = m->array_get(thunk, 3);
                auto ee = VM_OBJECT_ARRAY_VALUE(exc);

                vm_object_t*s rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray::create(rr);
            }
        } else {
            vm_object_t*s rr;
            for (unsigned int i = 4; i < (unsigned int)m->array_size(thunk);
                 i++) {
                rr.push_back(m->array_get(thunk, i));
            }
            r = VMObjectArray::create(rr);
        }

        // also return spurious arguments
        if (m->array_size(thunk) > 7) {
            vm_object_t*s rr;
            rr.push_back(r);
            for (unsigned int i = 7; i < (unsigned int)m->array_size(thunk);
                 i++) {
                rr.push_back(m->array_get(thunk, i));
            }
            r = VMObjectArray::create(rr);
        }

        auto index = VM_OBJECT_INTEGER_VALUE(rti);
        auto rta = VM_OBJECT_ARRAY_CAST(rt);
        rta->set(index, r);

        return k;
    }
};

#define DYADIC_PREAMBLE(t, c, n0, n1)              \
    c(VM *m) : Dyadic(t, m, n0, n1) {              \
    }                                              \
    c(VM *m, const symbol_t s) : Dyadic(t, m, s) { \
    }                                              \
    c(const c &o) : c(o.machine(), o.symbol()) {   \
    }                                              \
    static vm_object_t* create(VM *m) {             \
        return std::shared_ptr<c>(new c(m));       \
    }

class DyadicCallback : public Dyadic {
public:
    DyadicCallback(
        VM *m, const icu::UnicodeString &s0, const icu::UnicodeString &s1,
        std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a2)>
            f)
        : Dyadic(VM_SUB_BUILTIN, m, s0, s1), _value(f) {
    }

    DyadicCallback(
        VM *m, const symbol_t s,
        std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a2)>
            f)
        : Dyadic(VM_SUB_BUILTIN, m, s), _value(f) {
    }

    DyadicCallback(const DyadicCallback &o)
        : DyadicCallback(o.machine(), o.symbol(), o.value()) {
    }

    vm_object_t* clone() const {
        return vm_object_t*((VMObject *)new DyadicCallback(*this));
    }

    std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a2)>
    value() const {
        return _value;
    }

    vm_object_t* apply(const vm_object_t *arg0,
                      const vm_object_t *arg1) const override {
        return (_value)(arg0, arg1);
    }

    static vm_object_t* create(
        VM *m, const symbol_t sym,
        std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a2)>
            f) {
        return vm_object_t*(new DyadicCallback(m, sym, f));
    }
    static vm_object_t* create(
        VM *m, const icu::UnicodeString &s,
        std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a2)>
            f) {
        auto sym = m->enter_symbol(s);
        return DyadicCallback::create(m, sym, f);
    }

    static vm_object_t* create(
        VM *m, const icu::UnicodeString &s0, const icu::UnicodeString &s1,
        std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a2)>
            f) {
        auto sym = m->enter_symbol(s0, s1);
        return DyadicCallback::create(m, sym, f);
    }

    static vm_object_t* create(
        VM *m, const UnicodeStrings &ss, const icu::UnicodeString &s,
        std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a2)>
            f) {
        auto sym = m->enter_symbol(ss, s);
        return DyadicCallback::create(m, sym, f);
    }

private:
    std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a2)>
        _value;
};

class Triadic : public VMObjectCombinator {
public:
    Triadic(const vm_subtag_t t, VM *m, const icu::UnicodeString &n0,
            const icu::UnicodeString &n1)
        : VMObjectCombinator(t, m, n0, n1) {
    }

    Triadic(const vm_subtag_t t, VM *m, const symbol_t s)
        : VMObjectCombinator(t, m, s) {
    }

    virtual vm_object_t* apply(const vm_object_t *arg0, const vm_object_t *arg1,
                              const vm_object_t *arg2) const = 0;

    vm_object_t* reduce(const vm_object_t *thunk) const override {
        auto tt = VM_OBJECT_ARRAY_VALUE(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];

        vm_object_t* r;
        if (tt.size() > 7) {
            auto arg0 = tt[5];
            auto arg1 = tt[6];
            auto arg2 = tt[7];

            try {
                r = apply(arg0, arg1, arg2);
                if (r == nullptr) {
                    vm_object_t*s rr;
                    for (unsigned int i = 4; i < tt.size(); i++) {
                        rr.push_back(tt[i]);
                    }
                    r = VMObjectArray::create(rr);
                }
            } catch (vm_object_t* e) {
                auto exc = tt[3];
                auto ee = VM_OBJECT_ARRAY_VALUE(exc);

                vm_object_t*s rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray::create(rr);
            }
        } else {
            vm_object_t*s rr;
            for (unsigned int i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
        }

        // also return spurious arguments
        if (tt.size() > 8) {
            vm_object_t*s rr;
            rr.push_back(r);
            for (unsigned int i = 8; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
        }

        auto index = VM_OBJECT_INTEGER_VALUE(rti);
        auto rta = VM_OBJECT_ARRAY_CAST(rt);
        rta->set(index, r);

        return k;
    }
};

#define TRIADIC_PREAMBLE(t, c, n0, n1)              \
    c(VM *m) : Triadic(t, m, n0, n1) {              \
    }                                               \
    c(VM *m, const symbol_t s) : Triadic(t, m, s) { \
    }                                               \
    c(const c &o) : c(o.machine(), o.symbol()) {    \
    }                                               \
    static vm_object_t* create(VM *m) {              \
        return std::shared_ptr<c>(new c(m));        \
    }

class TriadicCallback : public Triadic {
public:
    TriadicCallback(
        VM *m, const icu::UnicodeString &s0, const icu::UnicodeString &s1,
        std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a1,
                                  const vm_object_t *a2)>
            f)
        : Triadic(VM_SUB_BUILTIN, m, s0, s1), _value(f) {
    }

    TriadicCallback(
        VM *m, const symbol_t s,
        std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a1,
                                  const vm_object_t *a2)>
            f)
        : Triadic(VM_SUB_BUILTIN, m, s), _value(f) {
    }

    TriadicCallback(const TriadicCallback &o)
        : TriadicCallback(o.machine(), o.symbol(), o.value()) {
    }

    std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a1,
                              const vm_object_t *a2)>
    value() const {
        return _value;
    }

    vm_object_t* apply(const vm_object_t *arg0, const vm_object_t *arg1,
                      const vm_object_t *arg2) const override {
        return (_value)(arg0, arg1, arg2);
    }

    static vm_object_t* create(
        VM *m, const symbol_t &sym,
        std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a1,
                                  const vm_object_t *a2)>
            f) {
        return vm_object_t*(new TriadicCallback(m, sym, f));
    }

    static vm_object_t* create(
        VM *m, const icu::UnicodeString &s,
        std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a1,
                                  const vm_object_t *a2)>
            f) {
        auto sym = m->enter_symbol(s);
        return TriadicCallback::create(m, sym, f);
    }

    static vm_object_t* create(
        VM *m, const icu::UnicodeString &s0, const icu::UnicodeString &s1,
        std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a1,
                                  const vm_object_t *a2)>
            f) {
        auto sym = m->enter_symbol(s0, s1);
        return TriadicCallback::create(m, sym, f);
    }

    static vm_object_t* create(
        VM *m, const UnicodeStrings &ss, const icu::UnicodeString &s,
        std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a1,
                                  const vm_object_t *a2)>
            f) {
        auto sym = m->enter_symbol(ss, s);
        return TriadicCallback::create(m, sym, f);
    }

private:
    std::function<vm_object_t*(const vm_object_t *a0, const vm_object_t *a1,
                              const vm_object_t *a2)>
        _value;
};

class Variadic : public VMObjectCombinator {
public:
    Variadic(const vm_subtag_t t, VM *m, const icu::UnicodeString &n0,
             const icu::UnicodeString &n1)
        : VMObjectCombinator(t, m, n0, n1) {
    }

    Variadic(const vm_subtag_t t, VM *m, const symbol_t s)
        : VMObjectCombinator(t, m, s) {
    }

    virtual vm_object_t* apply(vm_object_t*s &args) const = 0;

    vm_object_t* reduce(const vm_object_t *thunk) const override {
        auto tt = VM_OBJECT_ARRAY_VALUE(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];

        vm_object_t* r;
        if (tt.size() > 4) {
            vm_object_t*s args;
            for (unsigned int i = 5; i < tt.size(); i++) {
                args.push_back(tt[i]);
            }

            try {
                r = apply(args);
                if (r == nullptr) {
                    vm_object_t*s rr;
                    for (unsigned int i = 4; i < tt.size(); i++) {
                        rr.push_back(tt[i]);
                    }
                    r = VMObjectArray::create(rr);
                }
            } catch (vm_object_t* e) {
                auto exc = tt[3];
                auto ee = VM_OBJECT_ARRAY_VALUE(exc);

                vm_object_t*s rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray::create(rr);
            }
        } else {
            vm_object_t*s rr;
            for (unsigned int i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
        }

        auto index = VM_OBJECT_INTEGER_VALUE(rti);
        auto rta = VM_OBJECT_ARRAY_CAST(rt);
        rta->set(index, r);

        return k;
    }
};

#define VARIADIC_PREAMBLE(t, c, n0, n1)              \
    c(VM *m) : Variadic(t, m, n0, n1) {              \
    }                                                \
    c(VM *m, const symbol_t s) : Variadic(t, m, s) { \
    }                                                \
    c(const c &o) : c(o.machine(), o.symbol()) {     \
    }                                                \
    static vm_object_t* create(VM *m) {               \
        return std::shared_ptr<c>(new c(m));         \
    }

/*
class VariadicCallback : public Variadic {
public:
    VariadicCallback(VM* m, const icu::UnicodeString& s0, const
icu::UnicodeString& s1, std::function<vm_object_t*(vm_object_t*s& aa)> f)
    : Variadic(VM_SUB_BUILTIN, m, s0, s1), _value(f) {
    }

    VariadicCallback(VM* m, const symbol_t s,
                     std::function<vm_object_t*(vm_object_t*s& aa)> f)
    : Variadic(VM_SUB_BUILTIN, m, s), _value(f) {
    }

    VariadicCallback(const VariadicCallback& o)
    : VariadicCallback(o.machine(), o.symbol(), o.value()) {
    }

    static vm_object_t* create(VM* vm, const symbol_t s),
                   std::function<vm_object_t*(vm_object_t*s& aa)> f) {
        return vm_object_t*(new VariadicCallback(vm, s, f));
    }

    static vm_object_t* create(VM* m, const icu::UnicodeString& s,
                   std::function<vm_object_t*(vm_object_t*s& aa)> f) {
        auto sym = m->enter_symbol(s);
        return VariadicCallback::create(m, sym, f);
    }

    static vm_object_t* create(VM* m, const icu::UnicodeString& s0, const
icu::UnicodeString& s1, std::function<vm_object_t*(vm_object_t*s& aa)> f) {
        auto sym = m->enter_symbol(s0, s1);
        return VariadicCallback::create(m, sym, f);
    }

    static vm_object_t* create(VM* m, const UnicodeStrings& ss, const
icu::UnicodeString& s, std::function<vm_object_t*(vm_object_t*s& aa)> f) {
        auto sym = m->enter_symbol(ss, s);
        return VariadicCallback::create(m, sym, f);
    }

    std::function<vm_object_t*(vm_object_t*s& aa)> value() const {
        return _value;
    }

    vm_object_t* apply(vm_object_t*s& args) const override {
        return (_value)(args);
    }


private:
    std::function<vm_object_t*(vm_object_t*s& aa)> _value();
};
*/
// convenience classes which return continued evaluations

class Unary : public VMObjectCombinator {
public:
    Unary(const vm_subtag_t t, VM *m, const UnicodeString &n0,
          const UnicodeString &n1)
        : VMObjectCombinator(t, m, n0, n1) {
    }

    Unary(const vm_subtag_t t, VM *m, const symbol_t s)
        : VMObjectCombinator(t, m, s) {
    }

    virtual vm_object_t* apply(const vm_object_t *arg0) const = 0;

    vm_object_t* reduce(const vm_object_t *thunk) const override {
        auto tt = VM_OBJECT_ARRAY_VALUE(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];
        auto exc = tt[3];

        vm_object_t* r;
        if (tt.size() > 5) {
            auto arg0 = tt[5];

            try {
                r = apply(arg0);
                if (r == nullptr) {
                    vm_object_t*s rr;
                    for (unsigned int i = 4; i < tt.size(); i++) {
                        rr.push_back(tt[i]);
                    }
                    r = VMObjectArray::create(rr);

                    auto index = VM_OBJECT_INTEGER_VALUE(rti);
                    auto rta = VM_OBJECT_ARRAY_CAST(rt);
                    rta->set(index, r);

                    return k;
                }
            } catch (vm_object_t* e) {
                auto exc = tt[3];
                auto ee = VM_OBJECT_ARRAY_VALUE(exc);

                vm_object_t*s rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray::create(rr);
            }
        } else {
            // This seems the way to go about it.. Check Binary etc. for this.
            vm_object_t*s rr;
            for (unsigned int i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
            auto index = VM_OBJECT_INTEGER_VALUE(rti);
            auto rta = VM_OBJECT_ARRAY_CAST(rt);
            rta->set(index, r);

            return k;
        }

        vm_object_t*s kk;
        kk.push_back(rt);
        kk.push_back(rti);
        kk.push_back(k);
        kk.push_back(exc);
        kk.push_back(r);
        for (unsigned int n = 6; n < tt.size(); n++) {
            kk.push_back(tt[n]);
        }

        return VMObjectArray::create(kk);
    }
};

#define UNARY_PREAMBLE(t, c, n0, n1)              \
    c(VM *m) : Unary(t, m, n0, n1) {              \
    }                                             \
    c(VM *m, const symbol_t s) : Unary(t, m, s) { \
    }                                             \
    c(const c &o) : c(o.machine(), o.symbol()) {  \
    }                                             \
    static vm_object_t* create(VM *m) {            \
        return std::shared_ptr<c>(new c(m));      \
    }

class Binary : public VMObjectCombinator {
public:
    Binary(const vm_subtag_t t, VM *m, const icu::UnicodeString &n0,
           const icu::UnicodeString &n1)
        : VMObjectCombinator(t, m, n0, n1) {
    }

    Binary(const vm_subtag_t t, VM *m, const symbol_t s)
        : VMObjectCombinator(t, m, s) {
    }

    virtual vm_object_t* apply(const vm_object_t *arg0,
                              const vm_object_t *arg1) const = 0;

    vm_object_t* reduce(const vm_object_t *thunk) const override {
        auto tt = VM_OBJECT_ARRAY_VALUE(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];
        auto exc = tt[3];

        vm_object_t* r;
        if (tt.size() > 6) {
            auto arg0 = tt[5];
            auto arg1 = tt[6];

            try {
                r = apply(arg0, arg1);
                if (r == nullptr) {
                    vm_object_t*s rr;
                    for (unsigned int i = 4; i < tt.size(); i++) {
                        rr.push_back(tt[i]);
                    }
                    r = VMObjectArray::create(rr);

                    auto index = VM_OBJECT_INTEGER_VALUE(rti);
                    auto rta = VM_OBJECT_ARRAY_CAST(rt);
                    rta->set(index, r);

                    return k;
                }
            } catch (vm_object_t* e) {
                auto exc = tt[3];
                auto ee = VM_OBJECT_ARRAY_VALUE(exc);

                vm_object_t*s rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray::create(rr);
            }
        } else {
            vm_object_t*s rr;
            for (unsigned int i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
        }

        vm_object_t*s kk;
        kk.push_back(rt);
        kk.push_back(rti);
        kk.push_back(k);
        kk.push_back(exc);
        kk.push_back(r);
        for (unsigned int n = 7; n < tt.size(); n++) {
            kk.push_back(tt[n]);
        }

        return VMObjectArray::create(kk);
    }
};

#define BINARY_PREAMBLE(t, c, n0, n1)              \
    c(VM *m) : Binary(t, m, n0, n1) {              \
    }                                              \
    c(VM *m, const symbol_t s) : Binary(t, m, s) { \
    }                                              \
    c(const c &o) : c(o.machine(), o.symbol()) {   \
    }                                              \
    static vm_object_t* create(VM *m) {             \
        return std::shared_ptr<c>(new c(m));       \
    }

class Ternary : public VMObjectCombinator {
public:
    Ternary(const vm_subtag_t t, VM *m, const icu::UnicodeString &n0,
            const icu::UnicodeString &n1)
        : VMObjectCombinator(t, m, n0, n1) {
    }

    Ternary(const vm_subtag_t t, VM *m, const symbol_t s)
        : VMObjectCombinator(t, m, s) {
    }

    virtual vm_object_t* apply(const vm_object_t *arg0, const vm_object_t *arg1,
                              const vm_object_t *arg2) const = 0;

    vm_object_t* reduce(const vm_object_t *thunk) const override {
        auto tt = VM_OBJECT_ARRAY_VALUE(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];
        auto exc = tt[3];

        vm_object_t* r;
        if (tt.size() > 7) {
            auto arg0 = tt[5];
            auto arg1 = tt[6];
            auto arg2 = tt[7];

            try {
                r = apply(arg0, arg1, arg2);
                if (r == nullptr) {
                    vm_object_t*s rr;
                    for (unsigned int i = 4; i < tt.size(); i++) {
                        rr.push_back(tt[i]);
                    }
                    r = VMObjectArray::create(rr);

                    auto index = VM_OBJECT_INTEGER_VALUE(rti);
                    auto rta = VM_OBJECT_ARRAY_CAST(rt);
                    rta->set(index, r);

                    return k;
                }
            } catch (vm_object_t* e) {
                auto exc = tt[3];
                auto ee = VM_OBJECT_ARRAY_VALUE(exc);

                vm_object_t*s rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray::create(rr);
            }
        } else {
            vm_object_t*s rr;
            for (unsigned int i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
        }

        vm_object_t*s kk;
        kk.push_back(rt);
        kk.push_back(rti);
        kk.push_back(k);
        kk.push_back(exc);
        kk.push_back(r);
        for (unsigned int n = 8; n < tt.size(); n++) {
            kk.push_back(tt[n]);
        }

        return VMObjectArray::create(kk);
    }
};

#define TERNARY_PREAMBLE(t, c, n0, n1)              \
    c(VM *m) : Ternary(t, m, n0, n1) {              \
    }                                               \
    c(VM *m, const symbol_t s) : Ternary(t, m, s) { \
    }                                               \
    c(const c &o) : c(o.machine(), o.symbol()) {    \
    }                                               \
    static vm_object_t* create(VM *m) {              \
        return std::shared_ptr<c>(new c(m));        \
    }
