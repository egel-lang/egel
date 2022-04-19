#pragma once

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

#define VM_OBJECT_NONE_TEST(o) (o->symbol() == SYMBOL_NONE)
#define VM_OBJECT_FALSE_TEST(o) (o->symbol() == SYMBOL_FALSE)
#define VM_OBJECT_TRUE_TEST(o) (o->symbol() == SYMBOL_TRUE)
#define VM_OBJECT_TUPLE_TEST(o) (o->symbol() == SYMBOL_TUPLE)
#define VM_OBJECT_NIL_TEST(o) (o->symbol() == SYMBOL_NIL)
#define VM_OBJECT_CONS_TEST(o) (o->symbol() == SYMBOL_CONS)

/**
 * VM objects can have subtypes which are _unique_ 'magic' numbers.
 */
using vm_subtag_t = unsigned int;

const vm_subtag_t VM_SUB_DATA = 0x00;      // a combinator data object
const vm_subtag_t VM_SUB_BUILTIN = 0x01;   // a combinator internally defined
const vm_subtag_t VM_SUB_BYTECODE = 0x02;  // a bytecode combinator
const vm_subtag_t VM_SUB_COMPILED = 0x03;  // a compiled bytecode combinator
const vm_subtag_t VM_SUB_EGO = 0x04;       // a combinator from a .ego
const vm_subtag_t VM_SUB_STUB = 0x05;      // a stub combinator

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

class VMObject {
public:
    VMObject(const vm_tag_t t) : _tag(t), _subtag(0) {
    }

    VMObject(const vm_tag_t t, const vm_subtag_t st) : _tag(t), _subtag(st) {
    }

    virtual ~VMObject() {  // FIX: give a virtual destructor to keep the
                           // compiler(-s) happy
    }

    vm_tag_t tag() const {
        return _tag;
    }

    vm_subtag_t subtag() const {
        return _subtag;
    }

    bool tag_test(vm_tag_t t) const {
        return _tag == t;
    }

    bool subtag_test(vm_subtag_t t) const {
        return _subtag == t;
    }

    friend std::ostream &operator<<(std::ostream &os, const vm_object_t *a) {
        a->render(os);
        return os;
    }

    virtual vm_object_t* reduce(const vm_object_t *thunk) const = 0;

    virtual void render(std::ostream &os) const = 0;

    virtual void debug(std::ostream &os) const = 0;

    virtual symbol_t symbol() const = 0;

    icu::UnicodeString to_text() const {
        std::stringstream ss;
        render(ss);
        icu::UnicodeString u(ss.str().c_str());
        return u;
    }

private:
    vm_tag_t _tag;
    vm_subtag_t _subtag;
};

using UnicodeStrings = std::vector<icu::UnicodeString>;

// the virtual machine

class Options;
using OptionsPtr = std::shared_ptr<Options>;

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

    static OptionsPtr create() {
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

    friend std::ostream &operator<<(std::ostream &os, const OptionsPtr &m) {
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

    virtual void initialize(OptionsPtr oo) = 0;

    // symbol table manipulation
    virtual symbol_t enter_symbol(const icu::UnicodeString &n) = 0;
    virtual symbol_t enter_symbol(const icu::UnicodeString &n0,
                                  const icu::UnicodeString &n1) = 0;
    virtual symbol_t enter_symbol(const std::vector<icu::UnicodeString> &nn,
                                  const icu::UnicodeString &n) = 0;

    virtual int get_combinators_size() = 0;
    virtual icu::UnicodeString get_combinator_string(symbol_t s) = 0;

    // data table manipulation
    virtual data_t enter_data(const vm_object_t *o) = 0;
    virtual data_t define_data(const vm_object_t *o) = 0;
    virtual vm_object_t* get_data(const data_t d) = 0;
    virtual data_t get_data(const vm_object_t *d) = 0;

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
    VMObjectOpaque(const vm_subtag_t t, VM *m, const symbol_t s)
        : VMObject(VM_OBJECT_OPAQUE, t), _machine(m), _symbol(s){};

    VMObjectOpaque(const vm_subtag_t t, VM *m, const icu::UnicodeString &n)
        : VMObject(VM_OBJECT_OPAQUE, t),
          _machine(m),
          _symbol(m->enter_symbol(n)){};

    VMObjectOpaque(const vm_subtag_t t, VM *m, const icu::UnicodeString &n0,
                   const icu::UnicodeString &n1)
        : VMObject(VM_OBJECT_OPAQUE, t),
          _machine(m),
          _symbol(m->enter_symbol(n0, n1)){};

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

    // compare should implement a total order, even if the state changes..
    virtual int compare(const vm_object_t *o) = 0;

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
using vm_object_t*Set = std::set<vm_object_t*, Lessvm_object_t*>;

// a stub is used for finding objects by their string or symbol
// and for opaque default members
// XXX: to be depecrated
class VMObjectStub : public VMObjectCombinator {
public:
    VMObjectStub(VM *vm, const symbol_t s)
        : VMObjectCombinator(VM_SUB_STUB, vm, s) {
    }

    VMObjectStub(VM *vm, const icu::UnicodeString &s)
        : VMObjectCombinator(VM_SUB_STUB, vm, s) {
    }

    VMObjectStub(const VMObjectStub &d)
        : VMObjectStub(d.machine(), d.symbol()) {
    }

    static vm_object_t* create(VM *m, const symbol_t s) {
        return vm_object_create(new VMObjectStub(m, s));
    }

    static vm_object_t* create(VM *m, const UnicodeString &s) {
        return std::make_shared<VMObjectStub>(m, s);
    }

    vm_object_t* reduce(const vm_object_t *thunk) const override {
        std::cerr << "symbol = " << symbol() << std::endl;
        PANIC("reduce on stub");
        return nullptr;
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
