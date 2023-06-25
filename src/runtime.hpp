#pragma once

#include <atomic>
#include <filesystem>
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

namespace fs = std::filesystem;
// this is one stand-alone interface file external libraries can link against.
// it must be self contained except for standard c++ and unicode (which should
// be phased out).

using namespace icu;  // use stable namespace

namespace egel {

inline void assert_fail(const char *assertion, const char *file,
                        unsigned int line) {
    std::cerr << file << ':' << line << ": assertion failed " << assertion
              << '\n';
    abort();
}

inline void panic_fail(const char *message, const char *file,
                       unsigned int line) {
    std::cerr << file << ':' << line << ": panic " << message << '\n';
    abort();
}

/**
 * ASSERT macro. Always compiled assertions.
 */
#ifndef ASSERT
#define ASSERT(_e) ((_e) ? (void)0 : assert_fail(#_e, __FILE__, __LINE__));
#endif

/**
 * PANIC macro. Aborts with a message (e.g., for non-reachable statements).
 */

#define PANIC(_m) (panic_fail(_m, __FILE__, __LINE__));

#ifndef PANIC
#define PANIC(s)                     \
    {                                \
        std::cerr << s << std::endl; \
        exit(1);                     \
    }
#endif

constexpr unsigned int EGEL_FLOAT_PRECISION =
    16;  // XXX: dbl::maxdigit doesn't seem to be defined on my system?

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
using symbol_t = uint32_t;
using data_t = uint32_t;

const int SYMBOL_INT = 0;
const int SYMBOL_FLOAT = 1;
const int SYMBOL_CHAR = 2;
const int SYMBOL_TEXT = 3;

const int SYMBOL_ARRAY = 4;

const int SYMBOL_NONE = 5;
const int SYMBOL_TRUE = 6;
const int SYMBOL_FALSE = 7;

const int SYMBOL_TUPLE = 8;
const int SYMBOL_NIL = 9;
const int SYMBOL_CONS = 10;

/**
 * VM objects can have subtypes which are _unique_ 'magic' numbers.
 */
using vm_subtag_t = unsigned int;

const vm_subtag_t VM_SUB_LITERAL = 0x00;   // a combinator literal object
const vm_subtag_t VM_SUB_DATA = 0x01;      // a combinator data object
const vm_subtag_t VM_SUB_BUILTIN = 0x02;   // a combinator internally defined
const vm_subtag_t VM_SUB_BYTECODE = 0x03;  // a bytecode combinator
const vm_subtag_t VM_SUB_COMPILED = 0x04;  // a compiled bytecode combinator
const vm_subtag_t VM_SUB_EGO = 0x05;       // a combinator from a .ego
const vm_subtag_t VM_SUB_STUB = 0x06;      // a stub combinator

const vm_subtag_t VM_SUB_MODULE = 0x0a;  // opaque module combinator

const vm_subtag_t VM_SUB_PYTHON_OBJECT = 0xfe;      // Python values
const vm_subtag_t VM_SUB_PYTHON_COMBINATOR = 0xff;  // Python combinators

using vm_bool_t = bool;
using vm_int_t = int64_t;
using vm_float_t = double;
using vm_char_t = UChar32;
using vm_text_t = icu::UnicodeString;

// tagbits (used internally but declared here for vm_object_t)

typedef unsigned int vm_tagbits_t;

// an egel value

union vm_object_t {
    std::atomic<vm_tagbits_t> tagbits;
    vm_object_t *next;
};

class VMObject;
using VMObjectPtr = std::shared_ptr<VMObject>;
using VMWeakObjectPtr = std::weak_ptr<VMObject>;

// forward declarations for pretty printing
inline void render_tuple(const VMObjectPtr &tt, std::ostream &os);
inline void render_nil(const VMObjectPtr &n, std::ostream &os);
inline void render_cons(const VMObjectPtr &cc, std::ostream &os);

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

    friend std::ostream &operator<<(std::ostream &os, const VMObjectPtr &a) {
        a->render(os);
        return os;
    }

    virtual VMObjectPtr reduce(const VMObjectPtr &thunk) const = 0;

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

inline bool object_symbol_test(const VMObjectPtr &o, const symbol_t s) {
    return o->symbol() == s;
};

inline bool object_none_test(const VMObjectPtr &o) {
    return object_symbol_test(o, SYMBOL_NONE);
};

inline bool object_false_test(const VMObjectPtr &o) {
    return object_symbol_test(o, SYMBOL_FALSE);
};

inline bool object_true_test(const VMObjectPtr &o) {
    return object_symbol_test(o, SYMBOL_TRUE);
};

inline bool object_tuple_test(const VMObjectPtr &o) {
    return object_symbol_test(o, SYMBOL_TUPLE);
};

inline bool object_nil_test(const VMObjectPtr &o) {
    return object_symbol_test(o, SYMBOL_NIL);
};

inline bool object_cons_test(const VMObjectPtr &o) {
    return object_symbol_test(o, SYMBOL_CONS);
};

using VMObjectPtrs = std::vector<VMObjectPtr>;
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
    VMObjectPtr result;
    bool exception;
};

enum reducer_state_t { RUNNING, SLEEPING, HALTED };

class VM;
using VMPtr = std::shared_ptr<VM>;

using callback_t = std::function<void(VM *vm, const VMObjectPtr &)>;

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
    virtual data_t enter_data(const VMObjectPtr &o) = 0;
    virtual data_t define_data(const VMObjectPtr &o) = 0;
    virtual VMObjectPtr get_data(const data_t d) = 0;
    virtual data_t get_data(const VMObjectPtr &d) = 0;

    // reduce an expression
    virtual void reduce(const VMObjectPtr &e, const VMObjectPtr &ret,
                        const VMObjectPtr &exc, reducer_state_t *run) = 0;
    virtual void reduce(const VMObjectPtr &e, const VMObjectPtr &ret,
                        const VMObjectPtr &exc) = 0;
    virtual VMReduceResult reduce(const VMObjectPtr &e,
                                  reducer_state_t *run) = 0;
    virtual VMReduceResult reduce(const VMObjectPtr &e) = 0;

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
    virtual VMObjectPtr get_combinator(const symbol_t t) = 0;
    virtual VMObjectPtr get_combinator(const icu::UnicodeString &n) = 0;
    virtual VMObjectPtr get_combinator(const icu::UnicodeString &n0,
                                       const icu::UnicodeString &n1) = 0;
    virtual VMObjectPtr get_combinator(
        const std::vector<icu::UnicodeString> &nn,
        const icu::UnicodeString &n) = 0;

    // state
    virtual void define(
        const VMObjectPtr &o) = 0;  // define an undefined symbol
    virtual void overwrite(
        const VMObjectPtr &o) = 0;  // define or overwrite a symbol
    virtual VMObjectPtr get(const VMObjectPtr &o) = 0;  // get a defined symbol

    // expose, necessary for switch convenience
    virtual vm_tag_t get_tag(const VMObjectPtr &o) = 0;
    virtual vm_subtag_t get_subtag(const VMObjectPtr &o) = 0;

    // creation
    virtual VMObjectPtr create_integer(const vm_int_t b) = 0;
    virtual VMObjectPtr create_float(const vm_float_t b) = 0;
    virtual VMObjectPtr create_char(const vm_char_t b) = 0;
    virtual VMObjectPtr create_text(const vm_text_t b) = 0;

    virtual bool is_integer(const VMObjectPtr &o) = 0;
    virtual bool is_float(const VMObjectPtr &o) = 0;
    virtual bool is_char(const VMObjectPtr &o) = 0;
    virtual bool is_text(const VMObjectPtr &o) = 0;

    virtual vm_int_t get_integer(const VMObjectPtr &o) = 0;
    virtual vm_float_t get_float(const VMObjectPtr &o) = 0;
    virtual vm_char_t get_char(const VMObjectPtr &o) = 0;
    virtual vm_text_t get_text(const VMObjectPtr &o) = 0;

    virtual VMObjectPtr create_none() = 0;
    virtual VMObjectPtr create_true() = 0;
    virtual VMObjectPtr create_false() = 0;
    virtual VMObjectPtr create_bool(const bool b) = 0;
    virtual VMObjectPtr create_nil() = 0;
    virtual VMObjectPtr create_cons() = 0;
    virtual VMObjectPtr create_tuple() = 0;

    virtual bool is_none(const VMObjectPtr &o) = 0;
    virtual bool is_true(const VMObjectPtr &o) = 0;
    virtual bool is_false(const VMObjectPtr &o) = 0;
    virtual bool is_bool(const VMObjectPtr &o) = 0;
    virtual bool is_nil(const VMObjectPtr &o) = 0;
    virtual bool is_cons(const VMObjectPtr &o) = 0;
    virtual bool is_tuple(const VMObjectPtr &o) = 0;

    virtual VMObjectPtr create_array(const VMObjectPtrs &oo) = 0;
    virtual VMObjectPtr create_array(const unsigned int size) = 0;
    virtual bool is_array(const VMObjectPtr &o) = 0;
    virtual unsigned int array_size(const VMObjectPtr &o) = 0;
    virtual VMObjectPtr array_get(const VMObjectPtr &o, int n) = 0;
    virtual void array_set(VMObjectPtr &o, int n, const VMObjectPtr &e) = 0;
    virtual VMObjectPtrs get_array(const VMObjectPtr &o) = 0;

    virtual bool is_combinator(const VMObjectPtr &o) = 0;
    virtual bool is_opaque(const VMObjectPtr &o) = 0;
    virtual bool is_data(const VMObjectPtr &o) = 0;
    virtual bool is_bytecode(const VMObjectPtr &o) = 0;

    virtual vm_text_t symbol(const VMObjectPtr &o) = 0;

    virtual VMObjectPtr create_data(const icu::UnicodeString &s) = 0;
    virtual VMObjectPtr create_data(const icu::UnicodeString &s0,
                                    const icu::UnicodeString &s1) = 0;
    virtual VMObjectPtr create_data(const std::vector<icu::UnicodeString> &ss,
                                    const icu::UnicodeString &s) = 0;

    virtual VMObjectPtr create_medadic(const icu::UnicodeString &s,
                                       std::function<VMObjectPtr()> f) = 0;
    virtual VMObjectPtr create_medadic(const icu::UnicodeString &s0,
                                       const icu::UnicodeString &s1,
                                       std::function<VMObjectPtr()> f) = 0;
    virtual VMObjectPtr create_medadic(
        const std::vector<icu::UnicodeString> &ss, const icu::UnicodeString &s,
        std::function<VMObjectPtr()> f) = 0;

    virtual VMObjectPtr create_monadic(
        const icu::UnicodeString &s,
        std::function<VMObjectPtr(const VMObjectPtr &a0)> f) = 0;
    virtual VMObjectPtr create_monadic(
        const icu::UnicodeString &s0, const icu::UnicodeString &s1,
        std::function<VMObjectPtr(const VMObjectPtr &a0)> f) = 0;
    virtual VMObjectPtr create_monadic(
        const std::vector<icu::UnicodeString> &ss, const icu::UnicodeString &s,
        std::function<VMObjectPtr(const VMObjectPtr &a0)> f) = 0;

    virtual VMObjectPtr create_dyadic(
        const icu::UnicodeString &s,
        std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a1)>
            f) = 0;
    virtual VMObjectPtr create_dyadic(
        const icu::UnicodeString &s0, const icu::UnicodeString &s1,
        std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a1)>
            f) = 0;
    virtual VMObjectPtr create_dyadic(
        const std::vector<icu::UnicodeString> &ss, const icu::UnicodeString &s,
        std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a1)>
            f) = 0;
    /*
        virtual VMObjectPtr create_triadic(const icu::UnicodeString& s,
                                           std::function<VMObjectPtr(const
       VMObjectPtr& a0, const VMObjectPtr& a1, const VMObjectPtr& a2)> f) = 0;
        virtual VMObjectPtr create_triadic(const icu::UnicodeString& s0, const
       icu::UnicodeString& s1, std::function<VMObjectPtr(const VMObjectPtr& a0,
       const VMObjectPtr& a1, const VMObjectPtr& a2)> f) = 0; virtual
       VMObjectPtr create_triadic(const std::vector<icu::UnicodeString>& ss,
       const icu::UnicodeString& s, std::function<VMObjectPtr(const VMObjectPtr&
       a0, const VMObjectPtr& a1, const VMObjectPtr& a2)> f) = 0;

        virtual VMObjectPtr create_variadic(const icu::UnicodeString& s,
                                            std::function<VMObjectPtr(const
       VMObjectPtrs& aa)> f) = 0; virtual VMObjectPtr create_variadic(const
       icu::UnicodeString& s0, const icu::UnicodeString& s1,
                                            std::function<VMObjectPtr(const
       VMObjectPtrs& aa)> f) = 0; virtual VMObjectPtr create_variadic(const
       std::vector<icu::UnicodeString>& ss, const icu::UnicodeString& s,
                                            std::function<VMObjectPtr(const
       VMObjectPtrs& aa)> f) = 0;
    */

    // convenience (appends and strips tuple tag)
    virtual VMObjectPtr to_tuple(const VMObjectPtrs &oo) = 0;
    virtual VMObjectPtrs from_tuple(const VMObjectPtr &tt) = 0;

    // convenience (for internal usage)
    virtual bool is_list(const VMObjectPtr &o) = 0;
    virtual VMObjectPtr to_list(const VMObjectPtrs &oo) = 0;
    virtual VMObjectPtrs from_list(const VMObjectPtr &o) = 0;

    // evaluation
    virtual void eval_line(const icu::UnicodeString &in, const callback_t &main,
                           const callback_t &exc) = 0;
    virtual void eval_module(const icu::UnicodeString &fn) = 0;
    virtual void eval_command(const icu::UnicodeString &l) = 0;
    virtual void eval_main() = 0;
    virtual void eval_interactive() = 0;

    // module inspection
    virtual VMObjectPtr query_modules() = 0;
    virtual bool is_module(const VMObjectPtr &m) = 0;
    virtual VMObjectPtr query_module_name(const VMObjectPtr &m) = 0;
    virtual VMObjectPtr query_module_path(const VMObjectPtr &m) = 0;
    virtual VMObjectPtr query_module_imports(const VMObjectPtr &m) = 0;
    virtual VMObjectPtr query_module_exports(const VMObjectPtr &m) = 0;
    virtual VMObjectPtr query_module_values(const VMObjectPtr &m) = 0;

    // serialization etc
    virtual VMObjectPtr assemble(const icu::UnicodeString &s) = 0;
    virtual icu::UnicodeString disassemble(const VMObjectPtr &o) = 0;
    virtual VMObjectPtrs get_bytedata(const VMObjectPtr &o) = 0;
    virtual icu::UnicodeString serialize(const VMObjectPtr &o) = 0;
    virtual VMObjectPtr deserialize(const icu::UnicodeString &s) = 0;
    virtual VMObjectPtrs bundle(const VMObjectPtr &o) = 0;

    // machine state
    virtual VMObjectPtr query_symbols() = 0;
    virtual VMObjectPtr query_data() = 0;

    virtual int compare(const VMObjectPtr &o0, const VMObjectPtr &o1) = 0;

    virtual VMObjectPtr bad(const VMObject *o, const icu::UnicodeString &e) = 0;
    virtual VMObjectPtr bad_args(const VMObject *o, const VMObjectPtr &a0) = 0;
    virtual VMObjectPtr bad_args(const VMObject *o, const VMObjectPtr &a0,
                                 const VMObjectPtr &a1) = 0;
    virtual VMObjectPtr bad_args(const VMObject *o, const VMObjectPtr &a0,
                                 const VMObjectPtr &a1,
                                 const VMObjectPtr &a2) = 0;

    // convenience
    static std::string unicode_to_string(const icu::UnicodeString &s) {
        std::string utf8;
        s.toUTF8String(utf8);
        return utf8;
    }

    static icu::UnicodeString unicode_from_string(const std::string &s) {
        StringPiece sp(s);
        return UnicodeString::fromUTF8(sp);
    }

    static char *unicode_to_utf8_chars(const icu::UnicodeString &s) {
        std::string utf8;
        s.toUTF8String(utf8);
        char *cstr = new char[utf8.length() + 1];
        std::strcpy(cstr, utf8.c_str());
        return cstr;
    }

    static icu::UnicodeString unicode_from_utf8_chars(const char *s) {
        StringPiece sp(s);
        return UnicodeString::fromUTF8(sp);
    }

    static icu::UnicodeString unicode_escape(const icu::UnicodeString &s) {
        icu::UnicodeString s1;
        for (int i = 0; i < s.length(); i = s.moveIndex32(i, 1)) {
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

    static icu::UnicodeString unicode_unescape(const icu::UnicodeString &s) {
        return s.unescape();
    }

    static icu::UnicodeString unicode_strip_quotes(
        const icu::UnicodeString &s) {
        icu::UnicodeString copy(s);
        return copy.retainBetween(1, copy.length() - 1);
    }

    static vm_int_t unicode_to_int(const icu::UnicodeString &s) {
        char *buf = unicode_to_utf8_chars(s);
        auto i = atol(buf);
        delete[] buf;
        return i;
    }

    static vm_int_t unicode_to_hexint(const icu::UnicodeString &s) {
        int i = 0;
        int64_t n = 0;
        UChar32 c = s.char32At(i);
        while (c != 0xffff) {
            n = n * 16;
            if (c >= 48 && c <= 57) {  // 0-9
                n += (((int)(c)) - 48);
            } else if ((c >= 65 && c <= 70)) {  // A-F
                n += (((int)(c)) - 55);
            } else if (c >= 97 && c <= 102) {  // a-f
                n += (((int)(c)) - 87);
            }  // just ignore other chars (like starting 0x)
            i++;
            c = s.char32At(i);
        }
        return n;
    }

    static vm_float_t unicode_to_float(const icu::UnicodeString &s) {
        char *buf = unicode_to_utf8_chars(s);
        auto f = atof(buf);  // XXX: use stream
        delete[] buf;
        return f;
    }

    static vm_char_t unicode_to_char(const icu::UnicodeString &s) {
        auto s0 = unicode_strip_quotes(s);
        auto s1 = unicode_unescape(s0);
        return s1.char32At(0);
    }

    static vm_text_t unicode_to_text(const icu::UnicodeString &s) {
        auto s0 = unicode_strip_quotes(s);
        auto s1 = unicode_unescape(s0);
        return s1;
    }

    static icu::UnicodeString unicode_from_int(const vm_int_t &n) {
        std::stringstream ss;
        ss << n;
        icu::UnicodeString u(ss.str().c_str());
        return u;
    }

    static icu::UnicodeString unicode_from_float(const vm_float_t &f) {
        std::stringstream ss;
        ss << f;
        icu::UnicodeString u(ss.str().c_str());
        return u;
    }

    static icu::UnicodeString unicode_from_char(const vm_char_t &c) {
        std::stringstream ss;
        ss << c;
        icu::UnicodeString u(ss.str().c_str());
        auto u1 = unicode_escape(u);
        return u1;
    }

    static icu::UnicodeString unicode_from_text(const icu::UnicodeString &s) {
        auto s0 = unicode_strip_quotes(s);
        auto s1 = unicode_unescape(s0);
        return s1;
    }

private:
    static fs::path string_to_path(const icu::UnicodeString &s) {
        char *cc = unicode_to_utf8_chars(s);
        fs::path p = fs::path(cc);
        delete[] cc;
        return p;
    }

    static icu::UnicodeString path_to_string(const fs::path &p) {
        icu::UnicodeString s(p.c_str());  // XXX: utf8
        return s;
    }

public:
    static bool file_exists(const icu::UnicodeString &filename) {
        char *fn = unicode_to_utf8_chars(filename);
        bool b = fs::exists(fn);
        delete[] fn;
        return b;
    }

    static icu::UnicodeString path_absolute(const icu::UnicodeString &s) {
        auto p0 = string_to_path(s);
        auto p1 = fs::absolute(p0);
        return path_to_string(p1);
    }

    static icu::UnicodeString path_combine(const icu::UnicodeString &s0,
                                           const icu::UnicodeString &s1) {
        auto p0 = string_to_path(s0);
        auto p1 = string_to_path(s1);
        p0 /= p1;
        return path_to_string(p0);
    }

    static icu::UnicodeString read_utf8_file(
        const icu::UnicodeString &filename) {
        // XXX: this code smells
        char *fn = unicode_to_utf8_chars(filename);
        long fsize;
        UFILE *f = u_fopen(fn, "r", NULL, "UTF-8");
        ASSERT(f != NULL);

        fseek(u_fgetfile(f), 0, SEEK_END);
        fsize = ftell(u_fgetfile(f));
        u_frewind(f);

        UChar *chars = new UChar[fsize + 1];

        for (fsize = 0; !u_feof(f); ++fsize) {
            UChar c = u_fgetc(f);
            chars[fsize] = c;
        }

        chars[fsize] = 0;

        u_fclose(f);
        delete[] fn;

        icu::UnicodeString str = icu::UnicodeString(chars);
        delete[] chars;
        return str;
    }

    static void write_utf8_file(const icu::UnicodeString &filename,
                                const icu::UnicodeString &str) {
        // XXX: this code smells
        char *fn = unicode_to_utf8_chars(filename);

        // to uchar
        UErrorCode error = U_ZERO_ERROR;
        UChar *buffer = new UChar[str.length() + 1];
        int32_t size = str.extract(buffer, sizeof(buffer), error);
        buffer[size] = 0;
        ASSERT(U_SUCCESS(error));

        // write
        int32_t fsize;
        UFILE *f = u_fopen(fn, "w", NULL, "UTF-8");
        ASSERT(f != NULL);
        fsize = u_fputs(buffer, f);
        ASSERT(fsize >= 0);
        u_fclose(f);
        delete[] fn;
        delete[] buffer;
        ;
    }
};

///////////////////////////////////////////////////////////////////////////////
// stuff below is either for internal usage or for implementations which just
// need that bit of extra speed

// VM object definitions

class VMObjectLiteral : public VMObject {
public:
    VMObjectLiteral(const vm_tag_t &t) : VMObject(t) {
    }

    void debug(std::ostream &os) const override {
        render(os);
    }

    // note: reduce is defined later in this header file for portability
    VMObjectPtr reduce(const VMObjectPtr &thunk) const override;
};

class VMObjectInteger : public VMObjectLiteral {
public:
    VMObjectInteger(const vm_int_t &v)
        : VMObjectLiteral(VM_OBJECT_INTEGER), _value(v){};

    VMObjectInteger(const VMObjectInteger &l) : VMObjectInteger(l.value()) {
    }

    static VMObjectPtr create(const vm_int_t v) {
        return std::make_shared<VMObjectInteger>(v);
    }

    static bool test(const VMObjectPtr &o) {
        return o->tag() == VM_OBJECT_INTEGER;
    }

    static std::shared_ptr<VMObjectInteger> cast(const VMObjectPtr &o) {
        return std::static_pointer_cast<VMObjectInteger>(o);
    }

    static vm_int_t value(const VMObjectPtr &o) {
        return cast(o)->value();
    }

    symbol_t symbol() const override {
        return SYMBOL_INT;
    }

    void render(std::ostream &os) const override {
        os << value();
    }

    vm_int_t value() const {
        return _value;
    }

private:
    vm_int_t _value;
};

class VMObjectFloat : public VMObjectLiteral {
public:
    VMObjectFloat(const vm_float_t &v)
        : VMObjectLiteral(VM_OBJECT_FLOAT), _value(v){};

    VMObjectFloat(const VMObjectFloat &l) : VMObjectFloat(l.value()) {
    }

    static VMObjectPtr create(const vm_float_t f) {
        return std::make_shared<VMObjectFloat>(f);
    }

    static bool test(const VMObjectPtr &o) {
        return o->tag() == VM_OBJECT_FLOAT;
    }

    static std::shared_ptr<VMObjectFloat> cast(const VMObjectPtr &o) {
        return std::static_pointer_cast<VMObjectFloat>(o);
    }

    static vm_float_t value(const VMObjectPtr &o) {
        return cast(o)->value();
    }

    symbol_t symbol() const override {
        return SYMBOL_FLOAT;
    }

    void render(std::ostream &os) const override {
        os.precision(EGEL_FLOAT_PRECISION);
        // os.setf(std::ios::fixed, std::ios::floatfield);
        os.setf(std::ios::showpoint);
        os << value();
    }

    vm_float_t value() const {
        return _value;
    }

private:
    vm_float_t _value;
};

class VMObjectChar : public VMObjectLiteral {
public:
    VMObjectChar(const vm_char_t &v)
        : VMObjectLiteral(VM_OBJECT_CHAR), _value(v){};

    VMObjectChar(const VMObjectChar &l) : VMObjectChar(l.value()) {
    }

    static VMObjectPtr create(const vm_char_t v) {
        return std::make_shared<VMObjectChar>(v);
    }

    static bool test(const VMObjectPtr &o) {
        return o->tag() == VM_OBJECT_CHAR;
    }

    static std::shared_ptr<VMObjectChar> cast(const VMObjectPtr &o) {
        return std::static_pointer_cast<VMObjectChar>(o);
    }

    static vm_char_t value(const VMObjectPtr &o) {
        return cast(o)->value();
    }

    symbol_t symbol() const override {
        return SYMBOL_CHAR;
    }

    void render(std::ostream &os) const override {
        icu::UnicodeString s;
        s = VM::unicode_escape(s + value());
        os << "'" << s << "'";
    }

    vm_char_t value() const {
        return _value;
    }

private:
    vm_char_t _value;
};

class VMObjectText : public VMObjectLiteral {
public:
    VMObjectText(const icu::UnicodeString &v)
        : VMObjectLiteral(VM_OBJECT_TEXT), _value(v){};

    VMObjectText(const char *v) : VMObjectLiteral(VM_OBJECT_TEXT) {
        _value = icu::UnicodeString::fromUTF8(icu::StringPiece(v));
    };

    VMObjectText(const VMObjectText &l) : VMObjectText(l.value()) {
    }

    static VMObjectPtr create(const icu::UnicodeString &v) {
        return std::make_shared<VMObjectText>(v);
    }

    static VMObjectPtr create(const char *v) {
        return std::make_shared<VMObjectText>(v);
    }

    static bool test(const VMObjectPtr &o) {
        return o->tag() == VM_OBJECT_TEXT;
    }

    static std::shared_ptr<VMObjectText> cast(const VMObjectPtr &o) {
        return std::static_pointer_cast<VMObjectText>(o);
    }

    static icu::UnicodeString value(const VMObjectPtr &o) {
        return cast(o)->value();
    }

    symbol_t symbol() const override {
        return SYMBOL_TEXT;
    }

    char *to_char() {
        const int STRING_MAX_SIZE = 10000000;  // XXX: i hate constants
        auto len = _value.extract(0, STRING_MAX_SIZE, nullptr, (uint32_t)0);
        auto buffer = new char[len + 1];
        _value.extract(0, STRING_MAX_SIZE, buffer, len + 1);
        return buffer;
    }

    void render(std::ostream &os) const override {
        icu::UnicodeString s;
        s = VM::unicode_escape(value());
        os << '"' << s << '"';
    }

    icu::UnicodeString value() const {
        return _value;
    }

private:
    icu::UnicodeString _value;
};

class VMObjectArray : public VMObject {
public:
    VMObjectArray() : VMObject(VM_OBJECT_ARRAY) {
        _size = 0;
        _array = new VMObjectPtr[0];
    };

    VMObjectArray(const VMObjectPtrs &v) : VMObject(VM_OBJECT_ARRAY) {
        _size = v.size();
        ;
        _array = new VMObjectPtr[_size];
        for (int i = 0; i < _size; i++) {
            _array[i] = v[i];
        }
    }

    VMObjectArray(const VMObjectArray &l) : VMObject(VM_OBJECT_ARRAY) {
        _size = l._size;
        _array = new VMObjectPtr[_size];
        for (int i = 0; i < _size; i++) {
            _array[i] = l._array[i];
        }
    }

    VMObjectArray(const int size) : VMObject(VM_OBJECT_ARRAY) {
        _size = size;
        _array = new VMObjectPtr[size];
    }

    ~VMObjectArray() {
        delete[] _array;
    }

    VMObjectPtr clone() const {
        return std::make_shared<VMObjectArray>(*this);
    }

    static VMObjectPtr create(int size) {
        return std::make_shared<VMObjectArray>(size);
    }

    static VMObjectPtr create(const VMObjectPtrs &pp) {
        if (pp.size() == 1) {
            return pp[0];
        } else {
            return std::make_shared<VMObjectArray>(pp);
        }
    }

    static VMObjectPtr create(const VMObjectPtr *pp, size_t sz) {
        if (sz == 1) {
            return pp[0];
        } else {
            auto aa = std::static_pointer_cast<VMObjectArray>(create(sz));
            for (size_t n = 0; n < sz; n++) {
                aa->set(n, pp[n]);
            }
            return aa;
        }
    }

    static bool test(const VMObjectPtr &o) {
        return o->tag() == VM_OBJECT_ARRAY;
        ;
    }

    static std::shared_ptr<VMObjectArray> cast(const VMObjectPtr &o) {
        return std::static_pointer_cast<VMObjectArray>(o);
    }

    static VMObjectPtrs value(const VMObjectPtr &o) {
        return cast(o)->value();
    }

    symbol_t symbol() const override {
        return SYMBOL_ARRAY;
    }

    size_t size() const {
        return _size;
    }

    VMObjectPtr &operator[](const size_t i) {
        return _array[i];
    }

    const VMObjectPtr &operator[](const size_t i) const {
        return _array[i];
    }

    // deprecate
    VMObjectPtr get(unsigned int i) const {
        return _array[i];
    }

    // deprecate
    void set(unsigned int i, const VMObjectPtr &o) {
        _array[i] = o;
    }

    /*
        void push_back(const VMObjectPtr &o) {
            _value.push_back(o);
        }
    */

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override;

    void debug(std::ostream &os) const override {
        render(os);
    }

    void render(std::ostream &os) const override {
        if (size() == 0) {
            os << "()";
            return;
        }
        auto head = value()[0];
        if ((head != nullptr) && (head->symbol() == SYMBOL_TUPLE)) {
            render_tuple(this->clone(), os);  // XXX: still need clone...
        } else if ((head != nullptr) && (head->symbol() == SYMBOL_CONS) &&
                   (value().size() == 3)) {
            render_cons(this->clone(), os);
        } else {
            os << '(';
            bool first = true;
            for (auto &v : value()) {
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
    }

    VMObjectPtrs value() const {
        auto oo = VMObjectPtrs(_size);
        for (int i = 0; i < _size; i++) {
            oo[i] = _array[i];
        }
        return oo;
    }

private:
    VMObjectPtr *_array;
    int _size;
};

// here we can safely declare reduce
inline VMObjectPtr VMObjectLiteral::reduce(const VMObjectPtr &thunk) const {
    auto tt = VMObjectArray::cast(thunk);
    // optimize a bit for the case it's either a sole literal or an applied
    // literal
    if (tt->size() == 5) {
        auto rt = tt->get(0);
        auto rti = tt->get(1);
        auto k = tt->get(2);
        // auto exc   = tt[3];
        auto c = tt->get(4);

        auto index = VMObjectInteger::value(rti);
        auto rta = VMObjectArray::cast(rt);
        rta->set(index, c);

        return k;
    } else {
        auto rt = tt->get(0);
        auto rti = tt->get(1);
        auto k = tt->get(2);

        VMObjectPtrs vv;
        for (size_t n = 4; n < tt->size(); n++) {
            vv.push_back(tt->get(n));
        }

        auto r = VMObjectArray::create(vv);

        auto index = VMObjectInteger::value(rti);
        auto rta = VMObjectArray::cast(rt);
        rta->set(index, r);

        return k;
    }
};

inline VMObjectPtr VMObjectArray::reduce(const VMObjectPtr &thunk) const {
    auto tt = VMObjectArray::value(thunk);
    auto rt = tt[0];
    auto rti = tt[1];
    auto k = tt[2];
    auto exc = tt[3];
    auto c = tt[4];

    VMObjectPtrs vv;
    vv.push_back(rt);
    vv.push_back(rti);
    vv.push_back(k);
    vv.push_back(exc);
    auto aa = VMObjectArray::value(c);
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

    static bool test(const VMObjectPtr &o) {
        return o->tag() == VM_OBJECT_OPAQUE;
    }

    static std::shared_ptr<VMObjectOpaque> cast(const VMObjectPtr &o) {
        return std::static_pointer_cast<VMObjectOpaque>(o);
    }

    static int compare(const VMObjectPtr &o0, const VMObjectPtr &o1) {
        return cast(o0)->compare(o1);
    }

    static symbol_t symbol(const VMObjectPtr &o) {
        return cast(o)->symbol();
    }

    symbol_t symbol() const override {
        return _symbol;
    }

    icu::UnicodeString text() const {
        return _machine->get_combinator_string(_symbol);
    }

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override {
        auto tt = VMObjectArray::value(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];
        // auto exc   = tt[3];
        // auto c   = tt[4];

        VMObjectPtr ret;
        if (tt.size() > 5) {
            VMObjectPtrs rr;
            for (unsigned int i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            ret = VMObjectArray::create(rr);
        } else {
            ret = tt[4];
        }

        auto index = VMObjectInteger::value(rti);
        auto rta = VMObjectArray::cast(rt);
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
    virtual int compare(const VMObjectPtr &o) = 0;

private:
    VM *_machine;
    symbol_t _symbol;
};

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

    static bool test(const VMObjectPtr &o) {
        return o->tag() == VM_OBJECT_COMBINATOR;
    }

    static std::shared_ptr<VMObjectCombinator> cast(const VMObjectPtr &o) {
        return std::static_pointer_cast<VMObjectCombinator>(o);
    }

    static symbol_t symbol(const VMObjectPtr &o) {
        return cast(o)->symbol();
    }

    VM *machine() const {
        return _machine;
    }

    symbol_t symbol() const override {
        return _symbol;
    }

    icu::UnicodeString raw_text() const {
        return _machine->get_combinator_string(_symbol);
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

    static VMObjectPtr create(VM *vm, const symbol_t s) {
        return std::make_shared<VMObjectData>(vm, s);
    }

    static VMObjectPtr create(VM *vm, const icu::UnicodeString &s) {
        auto sym = vm->enter_symbol(s);
        return VMObjectData::create(vm, sym);
    }

    static VMObjectPtr create(VM *vm, const icu::UnicodeString &s0,
                              const icu::UnicodeString &s1) {
        auto sym = vm->enter_symbol(s0, s1);
        return VMObjectData::create(vm, sym);
    }

    static VMObjectPtr create(VM *vm, const UnicodeStrings &ss,
                              const icu::UnicodeString &s) {
        auto sym = vm->enter_symbol(ss, s);
        return VMObjectData::create(vm, sym);
    }

    static bool test(const VMObjectPtr &o) {
        return o->tag() == VM_OBJECT_COMBINATOR;
    }

    static std::shared_ptr<VMObjectData> cast(const VMObjectPtr &o) {
        return std::static_pointer_cast<VMObjectData>(o);
    }

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override {
        auto tt = VMObjectArray::value(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];
        // auto exc   = tt[3];
        // auto c   = tt[4];

        VMObjectPtr ret;
        if (tt.size() > 5) {
            VMObjectPtrs rr;
            for (unsigned int i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            ret = VMObjectArray::create(rr);
        } else {
            ret = tt[4];
        }

        auto index = VMObjectInteger::value(rti);
        auto rta = VMObjectArray::cast(rt);
        rta->set(index, ret);

        return k;
    }
};

struct CompareVMObjectPtr {
    int operator()(const VMObjectPtr &a0, const VMObjectPtr &a1) const {
        auto t0 = a0->tag();
        auto t1 = a1->tag();
        if (t0 < t1) {
            return -1;
        } else if (t1 < t0) {
            return 1;
        } else {
            switch (t0) {
                case VM_OBJECT_INTEGER: {
                    auto v0 = VMObjectInteger::value(a0);
                    auto v1 = VMObjectInteger::value(a1);
                    if (v0 < v1)
                        return -1;
                    else if (v1 < v0)
                        return 1;
                    else
                        return 0;
                } break;
                case VM_OBJECT_FLOAT: {
                    auto v0 = VMObjectFloat::value(a0);
                    auto v1 = VMObjectFloat::value(a1);
                    if (v0 < v1)
                        return -1;
                    else if (v1 < v0)
                        return 1;
                    else
                        return 0;
                } break;
                case VM_OBJECT_CHAR: {
                    auto v0 = VMObjectChar::value(a0);
                    auto v1 = VMObjectChar::value(a1);
                    if (v0 < v1)
                        return -1;
                    else if (v1 < v0)
                        return 1;
                    else
                        return 0;
                } break;
                case VM_OBJECT_TEXT: {
                    auto v0 = VMObjectText::value(a0);
                    auto v1 = VMObjectText::value(a1);
                    if (v0 < v1)
                        return -1;
                    else if (v1 < v0)
                        return 1;
                    else
                        return 0;
                } break;
                case VM_OBJECT_OPAQUE: {
                    auto s0 = VMObjectOpaque::symbol(a0);
                    auto s1 = VMObjectOpaque::symbol(a1);
                    if (s0 < s1)
                        return -1;
                    else if (s1 < s0)
                        return 1;
                    else
                        return VMObjectOpaque::compare(a0, a1);
                } break;
                case VM_OBJECT_COMBINATOR: {
                    auto v0 = VMObjectCombinator::symbol(a0);
                    auto v1 = VMObjectCombinator::symbol(a1);
                    if (v0 < v1)
                        return -1;
                    else if (v1 < v0)
                        return 1;
                    else
                        return 0;
                } break;
                case VM_OBJECT_ARRAY: {
                    auto v0 = VMObjectArray::value(a0);
                    auto v1 = VMObjectArray::value(a1);
                    auto s0 = v0.size();
                    auto s1 = v1.size();

                    if (s0 < s1)
                        return -1;
                    else if (s1 < s0)
                        return 1;
                    else {
                        for (unsigned int i = 0; i < s0; i++) {
                            auto c = operator()(v0[i], v1[i]);
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
    }
};
struct EqualVMObjectPtr {
    bool operator()(const VMObjectPtr &a0, const VMObjectPtr &a1) const {
        CompareVMObjectPtr compare;
        return (compare(a0, a1) == 0);
    }
};
struct LessVMObjectPtr {
    bool operator()(const VMObjectPtr &a0, const VMObjectPtr &a1) const {
        CompareVMObjectPtr compare;
        return (compare(a0, a1) == -1);
    }
};
using VMObjectPtrSet = std::set<VMObjectPtr, LessVMObjectPtr>;

// a stub is used for finding objects by their string or symbol
// and for opaque default members
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

    static VMObjectPtr create(VM *m, const symbol_t s) {
        return std::make_shared<VMObjectStub>(m, s);
    }

    static VMObjectPtr create(VM *m, const UnicodeString &s) {
        return std::make_shared<VMObjectStub>(m, s);
    }

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override {
        std::cerr << "symbol = " << symbol() << std::endl;
        std::cerr << "string = " << machine()->get_combinator_string(symbol())
                  << std::endl;
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

    static VMObjectPtr create(VM *m) {
        return std::make_shared<VMThrow>(m);
    }

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override {
        // when throw is reduced, it takes the exception, inserts it argument,
        // and reduces that

        auto tt = VMObjectArray::value(thunk);
        // auto rt  = tt[0];
        // auto rti = tt[1];
        // auto k   = tt[2];
        auto exc = tt[3];
        // auto c   = tt[4];
        auto r = tt[5];

        auto ee = VMObjectArray::cast(exc);
        ee->set(5, r);

        return exc;
    }
};

// the stall combinator, used in the runtime
class VMStall : public VMObjectCombinator {
public:
    VMStall(VM *m) : VMObjectCombinator(VM_SUB_BUILTIN, m, "System", "stall") {
    }

    VMStall(const VMStall &t) : VMStall(t.machine()) {
    }

    static VMObjectPtr create(VM *m) {
        return std::make_shared<VMStall>(m);
    }

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override {
        auto tt = VMObjectArray::value(thunk);
        auto rt  = tt[0];
        auto rti = tt[1];
        auto k   = tt[2];
        auto exc = tt[3];
        VMObjectPtrs rr;
        if (tt.size() > 5) {
            for (unsigned int i = 5; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
        } else {
            rr.push_back(tt[4]); // note: stall returns stall
        }
        auto r = VMObjectArray::create(rr);

        auto index = VMObjectInteger::value(rti);
        auto rta = VMObjectArray::cast(rt);
        rta->set(index, r);

        return k;
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
    } \
    static VMObjectPtr create(VM *m) {              \
        return std::shared_ptr<c>(new c(m));        \
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

    virtual VMObjectPtr apply() const = 0;

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override {
        auto tt = VMObjectArray::value(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];

        VMObjectPtr r;
        if (tt.size() > 4) {
            try {
                r = apply();
                if (r == nullptr) {
                    VMObjectPtrs rr;
                    for (unsigned int i = 4; i < tt.size(); i++) {
                        rr.push_back(tt[i]);
                    }
                    r = VMObjectArray::create(rr);
                }
            } catch (VMObjectPtr e) {
                auto exc = tt[3];
                auto ee = VMObjectArray::value(exc);

                VMObjectPtrs rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray::create(rr);
            }
        } else {
            VMObjectPtrs rr;
            for (unsigned int i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
        }

        // also return spurious arguments
        if (tt.size() > 5) {
            VMObjectPtrs rr;
            rr.push_back(r);
            for (unsigned int i = 5; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
        }

        auto index = VMObjectInteger::value(rti);
        auto rta = VMObjectArray::cast(rt);
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
    static VMObjectPtr create(VM *m) {              \
        return std::shared_ptr<c>(new c(m));        \
    }

class MedadicCallback : Medadic {
public:
    MedadicCallback(VM *m, const icu::UnicodeString &s0,
                    const icu::UnicodeString &s1,
                    std::function<VMObjectPtr()> f)
        : Medadic(VM_SUB_BUILTIN, m, s0, s1), _value(f) {
    }

    MedadicCallback(VM *m, const symbol_t s, std::function<VMObjectPtr()> f)
        : Medadic(VM_SUB_BUILTIN, m, s), _value(f) {
    }

    MedadicCallback(const MedadicCallback &o)
        : MedadicCallback(o.machine(), o.symbol(), o.value()) {
    }

    static VMObjectPtr create(VM *m, const symbol_t s,
                              std::function<VMObjectPtr()> f) {
        return VMObjectPtr((VMObject *)new MedadicCallback(m, s, f));
    }

    std::function<VMObjectPtr()> value() const {
        return _value;
    }

    VMObjectPtr apply() const override {
        return (_value)();
    }

    static VMObjectPtr create(VM *m, const icu::UnicodeString &s,
                              std::function<VMObjectPtr()> f) {
        auto sym = m->enter_symbol(s);
        return MedadicCallback::create(m, sym, f);
    }

    static VMObjectPtr create(VM *m, const icu::UnicodeString &s0,
                              const icu::UnicodeString &s1,
                              std::function<VMObjectPtr()> f) {
        auto sym = m->enter_symbol(s0, s1);
        return MedadicCallback::create(m, sym, f);
    }

    static VMObjectPtr create(VM *m, const UnicodeStrings &ss,
                              const icu::UnicodeString &s,
                              std::function<VMObjectPtr()> f) {
        auto sym = m->enter_symbol(ss, s);
        return MedadicCallback::create(m, sym, f);
    }

private:
    std::function<VMObjectPtr()> _value;
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

    virtual VMObjectPtr apply(const VMObjectPtr &arg0) const = 0;

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override {
        auto tt = VMObjectArray::value(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];

        VMObjectPtr r;
        if (tt.size() > 5) {
            auto arg0 = tt[5];

            try {
                r = apply(arg0);
                if (r == nullptr) {
                    VMObjectPtrs rr;
                    for (unsigned int i = 4; i < tt.size(); i++) {
                        rr.push_back(tt[i]);
                    }
                    r = VMObjectArray::create(rr);
                }
            } catch (VMObjectPtr e) {
                auto exc = tt[3];
                auto ee = VMObjectArray::value(exc);

                VMObjectPtrs rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray::create(rr);
            }
        } else {
            VMObjectPtrs rr;
            for (unsigned int i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
        }

        // also return spurious arguments
        if (tt.size() > 6) {
            VMObjectPtrs rr;
            rr.push_back(r);
            for (unsigned int i = 6; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
        }

        auto index = VMObjectInteger::value(rti);
        auto rta = VMObjectArray::cast(rt);
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
    static VMObjectPtr create(VM *m) {              \
        return std::shared_ptr<c>(new c(m));        \
    }

class MonadicCallback : public Monadic {
public:
    MonadicCallback(VM *m, const icu::UnicodeString &s0,
                    const icu::UnicodeString &s1,
                    std::function<VMObjectPtr(const VMObjectPtr &a0)> f)
        : Monadic(VM_SUB_BUILTIN, m, s0, s1), _value(f) {
    }

    MonadicCallback(VM *m, const symbol_t s,
                    std::function<VMObjectPtr(const VMObjectPtr &a0)> f)
        : Monadic(VM_SUB_BUILTIN, m, s), _value(f) {
    }

    MonadicCallback(const MonadicCallback &o)
        : MonadicCallback(o.machine(), o.symbol(), o.value()) {
    }

    ~MonadicCallback() {
    }

    static VMObjectPtr create(
        VM *m, const symbol_t sym,
        std::function<VMObjectPtr(const VMObjectPtr &a0)> f) {
        // return std::shared_ptr<MonadicCallback>(new MonadicCallback(m, sym,
        // f));
        return std::shared_ptr<MonadicCallback>(new MonadicCallback(m, sym, f));
    }

    static VMObjectPtr create(
        VM *m, const icu::UnicodeString &s,
        std::function<VMObjectPtr(const VMObjectPtr &a0)> f) {
        auto sym = m->enter_symbol(s);
        return MonadicCallback::create(m, sym, f);
    }

    static VMObjectPtr create(
        VM *m, const icu::UnicodeString &s0, const icu::UnicodeString &s1,
        std::function<VMObjectPtr(const VMObjectPtr &a0)> f) {
        auto sym = m->enter_symbol(s0, s1);
        return MonadicCallback::create(m, sym, f);
    }

    static VMObjectPtr create(
        VM *m, const UnicodeStrings &ss, const icu::UnicodeString &s,
        std::function<VMObjectPtr(const VMObjectPtr &a0)> f) {
        auto sym = m->enter_symbol(ss, s);
        return MonadicCallback::create(m, sym, f);
    }

    std::function<VMObjectPtr(const VMObjectPtr &a0)> value() const {
        return _value;
    }

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        return (_value)(arg0);
    }

private:
    std::function<VMObjectPtr(const VMObjectPtr &a0)> _value;
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

    virtual VMObjectPtr apply(const VMObjectPtr &arg0,
                              const VMObjectPtr &arg1) const = 0;

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override {
        auto m = machine();
        auto rt = m->array_get(thunk, 0);
        auto rti = m->array_get(thunk, 1);
        auto k = m->array_get(thunk, 2);

        VMObjectPtr r;
        if (m->array_size(thunk) > 6) {
            auto arg0 = m->array_get(thunk, 5);
            auto arg1 = m->array_get(thunk, 6);

            try {
                r = apply(arg0, arg1);
                if (r == nullptr) {
                    VMObjectPtrs rr;
                    for (unsigned int i = 4;
                         i < (unsigned int)m->array_size(thunk); i++) {
                        rr.push_back(m->array_get(thunk, i));
                    }
                    r = VMObjectArray::create(rr);
                }
            } catch (VMObjectPtr e) {
                auto exc = m->array_get(thunk, 3);
                auto ee = VMObjectArray::value(exc);

                VMObjectPtrs rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray::create(rr);
            }
        } else {
            VMObjectPtrs rr;
            for (unsigned int i = 4; i < (unsigned int)m->array_size(thunk);
                 i++) {
                rr.push_back(m->array_get(thunk, i));
            }
            r = VMObjectArray::create(rr);
        }

        // also return spurious arguments
        if (m->array_size(thunk) > 7) {
            VMObjectPtrs rr;
            rr.push_back(r);
            for (unsigned int i = 7; i < (unsigned int)m->array_size(thunk);
                 i++) {
                rr.push_back(m->array_get(thunk, i));
            }
            r = VMObjectArray::create(rr);
        }

        auto index = VMObjectInteger::value(rti);
        auto rta = VMObjectArray::cast(rt);
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
    static VMObjectPtr create(VM *m) {             \
        return std::shared_ptr<c>(new c(m));       \
    }

class DyadicCallback : public Dyadic {
public:
    DyadicCallback(
        VM *m, const icu::UnicodeString &s0, const icu::UnicodeString &s1,
        std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a2)>
            f)
        : Dyadic(VM_SUB_BUILTIN, m, s0, s1), _value(f) {
    }

    DyadicCallback(
        VM *m, const symbol_t s,
        std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a2)>
            f)
        : Dyadic(VM_SUB_BUILTIN, m, s), _value(f) {
    }

    DyadicCallback(const DyadicCallback &o)
        : DyadicCallback(o.machine(), o.symbol(), o.value()) {
    }

    /*
        VMObjectPtr clone() const {
            return VMObjectPtr((VMObject *)new DyadicCallback(*this));
        }
    */

    std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a2)>
    value() const {
        return _value;
    }

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        return (_value)(arg0, arg1);
    }

    static VMObjectPtr create(
        VM *m, const symbol_t sym,
        std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a2)>
            f) {
        return VMObjectPtr(new DyadicCallback(m, sym, f));
    }
    static VMObjectPtr create(
        VM *m, const icu::UnicodeString &s,
        std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a2)>
            f) {
        auto sym = m->enter_symbol(s);
        return DyadicCallback::create(m, sym, f);
    }

    static VMObjectPtr create(
        VM *m, const icu::UnicodeString &s0, const icu::UnicodeString &s1,
        std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a2)>
            f) {
        auto sym = m->enter_symbol(s0, s1);
        return DyadicCallback::create(m, sym, f);
    }

    static VMObjectPtr create(
        VM *m, const UnicodeStrings &ss, const icu::UnicodeString &s,
        std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a2)>
            f) {
        auto sym = m->enter_symbol(ss, s);
        return DyadicCallback::create(m, sym, f);
    }

private:
    std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a2)>
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

    virtual VMObjectPtr apply(const VMObjectPtr &arg0, const VMObjectPtr &arg1,
                              const VMObjectPtr &arg2) const = 0;

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override {
        auto tt = VMObjectArray::value(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];

        VMObjectPtr r;
        if (tt.size() > 7) {
            auto arg0 = tt[5];
            auto arg1 = tt[6];
            auto arg2 = tt[7];

            try {
                r = apply(arg0, arg1, arg2);
                if (r == nullptr) {
                    VMObjectPtrs rr;
                    for (unsigned int i = 4; i < tt.size(); i++) {
                        rr.push_back(tt[i]);
                    }
                    r = VMObjectArray::create(rr);
                }
            } catch (VMObjectPtr e) {
                auto exc = tt[3];
                auto ee = VMObjectArray::value(exc);

                VMObjectPtrs rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray::create(rr);
            }
        } else {
            VMObjectPtrs rr;
            for (unsigned int i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
        }

        // also return spurious arguments
        if (tt.size() > 8) {
            VMObjectPtrs rr;
            rr.push_back(r);
            for (unsigned int i = 8; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
        }

        auto index = VMObjectInteger::value(rti);
        auto rta = VMObjectArray::cast(rt);
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
    static VMObjectPtr create(VM *m) {              \
        return std::shared_ptr<c>(new c(m));        \
    }

class TriadicCallback : public Triadic {
public:
    TriadicCallback(
        VM *m, const icu::UnicodeString &s0, const icu::UnicodeString &s1,
        std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a1,
                                  const VMObjectPtr &a2)>
            f)
        : Triadic(VM_SUB_BUILTIN, m, s0, s1), _value(f) {
    }

    TriadicCallback(
        VM *m, const symbol_t s,
        std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a1,
                                  const VMObjectPtr &a2)>
            f)
        : Triadic(VM_SUB_BUILTIN, m, s), _value(f) {
    }

    TriadicCallback(const TriadicCallback &o)
        : TriadicCallback(o.machine(), o.symbol(), o.value()) {
    }

    std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a1,
                              const VMObjectPtr &a2)>
    value() const {
        return _value;
    }

    VMObjectPtr apply(const VMObjectPtr &arg0, const VMObjectPtr &arg1,
                      const VMObjectPtr &arg2) const override {
        return (_value)(arg0, arg1, arg2);
    }

    static VMObjectPtr create(
        VM *m, const symbol_t &sym,
        std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a1,
                                  const VMObjectPtr &a2)>
            f) {
        return VMObjectPtr(new TriadicCallback(m, sym, f));
    }

    static VMObjectPtr create(
        VM *m, const icu::UnicodeString &s,
        std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a1,
                                  const VMObjectPtr &a2)>
            f) {
        auto sym = m->enter_symbol(s);
        return TriadicCallback::create(m, sym, f);
    }

    static VMObjectPtr create(
        VM *m, const icu::UnicodeString &s0, const icu::UnicodeString &s1,
        std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a1,
                                  const VMObjectPtr &a2)>
            f) {
        auto sym = m->enter_symbol(s0, s1);
        return TriadicCallback::create(m, sym, f);
    }

    static VMObjectPtr create(
        VM *m, const UnicodeStrings &ss, const icu::UnicodeString &s,
        std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a1,
                                  const VMObjectPtr &a2)>
            f) {
        auto sym = m->enter_symbol(ss, s);
        return TriadicCallback::create(m, sym, f);
    }

private:
    std::function<VMObjectPtr(const VMObjectPtr &a0, const VMObjectPtr &a1,
                              const VMObjectPtr &a2)>
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

    virtual VMObjectPtr apply(const VMObjectPtrs &args) const = 0;

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override {
        auto tt = VMObjectArray::value(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];

        VMObjectPtr r;
        if (tt.size() > 4) {
            VMObjectPtrs args;
            for (unsigned int i = 5; i < tt.size(); i++) {
                args.push_back(tt[i]);
            }

            try {
                r = apply(args);
                if (r == nullptr) {
                    VMObjectPtrs rr;
                    for (unsigned int i = 4; i < tt.size(); i++) {
                        rr.push_back(tt[i]);
                    }
                    r = VMObjectArray::create(rr);
                }
            } catch (VMObjectPtr e) {
                auto exc = tt[3];
                auto ee = VMObjectArray::value(exc);

                VMObjectPtrs rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray::create(rr);
            }
        } else {
            VMObjectPtrs rr;
            for (unsigned int i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
        }

        auto index = VMObjectInteger::value(rti);
        auto rta = VMObjectArray::cast(rt);
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
    static VMObjectPtr create(VM *m) {               \
        return std::shared_ptr<c>(new c(m));         \
    }

/*
class VariadicCallback : public Variadic {
public:
    VariadicCallback(VM* m, const icu::UnicodeString& s0, const
icu::UnicodeString& s1, std::function<VMObjectPtr(const VMObjectPtrs& aa)> f)
    : Variadic(VM_SUB_BUILTIN, m, s0, s1), _value(f) {
    }

    VariadicCallback(VM* m, const symbol_t s,
                     std::function<VMObjectPtr(const VMObjectPtrs& aa)> f)
    : Variadic(VM_SUB_BUILTIN, m, s), _value(f) {
    }

    VariadicCallback(const VariadicCallback& o)
    : VariadicCallback(o.machine(), o.symbol(), o.value()) {
    }

    static VMObjectPtr create(VM* vm, const symbol_t s),
                   std::function<VMObjectPtr(const VMObjectPtrs& aa)> f) {
        return VMObjectPtr(new VariadicCallback(vm, s, f));
    }

    static VMObjectPtr create(VM* m, const icu::UnicodeString& s,
                   std::function<VMObjectPtr(const VMObjectPtrs& aa)> f) {
        auto sym = m->enter_symbol(s);
        return VariadicCallback::create(m, sym, f);
    }

    static VMObjectPtr create(VM* m, const icu::UnicodeString& s0, const
icu::UnicodeString& s1, std::function<VMObjectPtr(const VMObjectPtrs& aa)> f) {
        auto sym = m->enter_symbol(s0, s1);
        return VariadicCallback::create(m, sym, f);
    }

    static VMObjectPtr create(VM* m, const UnicodeStrings& ss, const
icu::UnicodeString& s, std::function<VMObjectPtr(const VMObjectPtrs& aa)> f) {
        auto sym = m->enter_symbol(ss, s);
        return VariadicCallback::create(m, sym, f);
    }

    std::function<VMObjectPtr(const VMObjectPtrs& aa)> value() const {
        return _value;
    }

    VMObjectPtr apply(const VMObjectPtrs& args) const override {
        return (_value)(args);
    }


private:
    std::function<VMObjectPtr(const VMObjectPtrs& aa)> _value();
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

    virtual VMObjectPtr apply(const VMObjectPtr &arg0) const = 0;

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override {
        auto tt = VMObjectArray::value(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];
        auto exc = tt[3];

        VMObjectPtr r;
        if (tt.size() > 5) {
            auto arg0 = tt[5];

            try {
                r = apply(arg0);
                if (r == nullptr) {
                    VMObjectPtrs rr;
                    for (unsigned int i = 4; i < tt.size(); i++) {
                        rr.push_back(tt[i]);
                    }
                    r = VMObjectArray::create(rr);

                    auto index = VMObjectInteger::value(rti);
                    auto rta = VMObjectArray::cast(rt);
                    rta->set(index, r);

                    return k;
                }
            } catch (VMObjectPtr e) {
                auto exc = tt[3];
                auto ee = VMObjectArray::value(exc);

                VMObjectPtrs rr;
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
            VMObjectPtrs rr;
            for (unsigned int i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
            auto index = VMObjectInteger::value(rti);
            auto rta = VMObjectArray::cast(rt);
            rta->set(index, r);

            return k;
        }

        VMObjectPtrs kk;
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
    static VMObjectPtr create(VM *m) {            \
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

    virtual VMObjectPtr apply(const VMObjectPtr &arg0,
                              const VMObjectPtr &arg1) const = 0;

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override {
        auto tt = VMObjectArray::value(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];
        auto exc = tt[3];

        VMObjectPtr r;
        if (tt.size() > 6) {
            auto arg0 = tt[5];
            auto arg1 = tt[6];

            try {
                r = apply(arg0, arg1);
                if (r == nullptr) {
                    VMObjectPtrs rr;
                    for (unsigned int i = 4; i < tt.size(); i++) {
                        rr.push_back(tt[i]);
                    }
                    r = VMObjectArray::create(rr);

                    auto index = VMObjectInteger::value(rti);
                    auto rta = VMObjectArray::cast(rt);
                    rta->set(index, r);

                    return k;
                }
            } catch (VMObjectPtr e) {
                auto exc = tt[3];
                auto ee = VMObjectArray::value(exc);

                VMObjectPtrs rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray::create(rr);
            }
        } else {
            VMObjectPtrs rr;
            for (unsigned int i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
        }

        VMObjectPtrs kk;
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
    static VMObjectPtr create(VM *m) {             \
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

    virtual VMObjectPtr apply(const VMObjectPtr &arg0, const VMObjectPtr &arg1,
                              const VMObjectPtr &arg2) const = 0;

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override {
        auto tt = VMObjectArray::value(thunk);
        auto rt = tt[0];
        auto rti = tt[1];
        auto k = tt[2];
        auto exc = tt[3];

        VMObjectPtr r;
        if (tt.size() > 7) {
            auto arg0 = tt[5];
            auto arg1 = tt[6];
            auto arg2 = tt[7];

            try {
                r = apply(arg0, arg1, arg2);
                if (r == nullptr) {
                    VMObjectPtrs rr;
                    for (unsigned int i = 4; i < tt.size(); i++) {
                        rr.push_back(tt[i]);
                    }
                    r = VMObjectArray::create(rr);

                    auto index = VMObjectInteger::value(rti);
                    auto rta = VMObjectArray::cast(rt);
                    rta->set(index, r);

                    return k;
                }
            } catch (VMObjectPtr e) {
                auto exc = tt[3];
                auto ee = VMObjectArray::value(exc);

                VMObjectPtrs rr;
                rr.push_back(ee[0]);
                rr.push_back(ee[1]);
                rr.push_back(ee[2]);
                rr.push_back(ee[3]);
                rr.push_back(ee[4]);
                rr.push_back(e);

                return VMObjectArray::create(rr);
            }
        } else {
            VMObjectPtrs rr;
            for (unsigned int i = 4; i < tt.size(); i++) {
                rr.push_back(tt[i]);
            }
            r = VMObjectArray::create(rr);
        }

        VMObjectPtrs kk;
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
    static VMObjectPtr create(VM *m) {              \
        return std::shared_ptr<c>(new c(m));        \
    }

// 'pretty' printing

inline void render_tuple(const VMObjectPtr &tt, std::ostream &os) {
    if (tt == nullptr) {
        os << '.';
    } else if (tt->tag() == VM_OBJECT_ARRAY) {
        os << '(';
        auto vv = VMObjectArray::value(tt);
        int sz = (int)vv.size();
        if (sz == 2) {  // NOTE, handle 'tuple 0 = (0,)' differently
            if (vv[0] == nullptr) {
                os << '.';
            } else {
                vv[0]->render(os);
            }
            os << ' ';
            if (vv[1] == nullptr) {
                os << '.';
            } else {
                vv[1]->render(os);
            }
        } else {
            for (int n = 1; n < sz; n++) {
                if (vv[n] == nullptr) {
                    os << '.';
                } else {
                    vv[n]->render(os);
                    if (n < sz - 1) {
                        os << ", ";
                    }
                }
            }
        }
        os << ')';
    } else {
        PANIC("not a tuple");
    }
};

inline void render_nil(const VMObjectPtr &n, std::ostream &os) {
    if (n == nullptr) {
        os << '.';
    } else {
        os << "{}";
    }
};

inline void render_array_raw(const VMObjectPtr &ee, std::ostream &os) {
    if (ee == nullptr) {
        os << '.';
    } else if (ee->tag() == VM_OBJECT_ARRAY) {
        auto vv = VMObjectArray::value(ee);
        os << '(';
        bool first = true;
        for (auto &v : vv) {
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
    } else {
        PANIC("array expected");
    }
};

inline bool is_well_formed_nil(const VMObjectPtr &ee) {
    if (ee->tag() == VM_OBJECT_COMBINATOR) {
        auto sym = VMObjectCombinator::symbol(ee);
        return sym == SYMBOL_NIL;
    } else {
        return false;
    }
};

inline bool is_well_formed_const(const VMObjectPtr &ee) {
    if (ee == nullptr) {
        return false;
    } else if (ee->tag() == VM_OBJECT_ARRAY) {
        auto v = VMObjectArray::value(ee);
        if (v.size() != 3) {
            return false;
        } else {
            auto head = v[0];
            if (head->tag() == VM_OBJECT_COMBINATOR) {
                auto h = VMObjectCombinator::cast(head);
                return h->symbol() == SYMBOL_CONS;
            } else {
                return false;
            }
        }
    } else {
        return false;
    }
};

inline void render_cons_elements(const VMObjectPtr &ee, std::ostream &os) {
    if (ee == nullptr) {
        os << '.';
    } else if (is_well_formed_nil(ee)) {
    } else if (is_well_formed_const(ee)) {
        auto v = VMObjectArray::value(ee);
        if ((v[2] != nullptr) && is_well_formed_nil(v[2])) {
            if (v[1] == nullptr) {
                os << ".";
            } else {
                v[1]->render(os);
            }
        } else if ((v[2] != nullptr) && is_well_formed_const(v[2])) {
            if (v[1] == nullptr) {
                os << ".";
            } else {
                v[1]->render(os);
            }
            os << ", ";
            render_cons_elements(v[2], os);
        } else {
            if (v[1] == nullptr) {
                os << ".";
            } else {
                v[1]->render(os);
            }
            os << "| ";
            if (v[2] == nullptr) {
                os << ".";
            } else {
                v[2]->render(os);
            }
        }
    } else {
        os << "|";
        ee->render(os);
    }
};

inline void render_cons(const VMObjectPtr &c, std::ostream &os) {
    os << "{";
    render_cons_elements(c, os);
    os << "}";
};

};  // namespace egel
