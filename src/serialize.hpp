#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <stack>
#include <vector>

#include "modules.hpp"
#include "runtime.hpp"
#include "utils.hpp"

// unfortunately it looks better to copy the entire object
// hierarchy of the runtime in order not to taint runtimes
// when serialized objects are shipped.

using objectid_t = unsigned int;

class SerialObject;
using SerialObjectPtr = std::shared_ptr<SerialObject>;

class SerialObject {
public:
    SerialObject(const vm_tag_t tag) : _tag(tag), _id(0) {
    }

    SerialObject(const vm_tag_t tag, const objectid_t id) : _tag(tag), _id(id) {
    }

    virtual ~SerialObject() {
    }

    vm_tag_t get_tag() const {
        return _tag;
    }

    objectid_t get_id() const {
        return _id;
    }

    void set_id(const objectid_t id) {
        _id = id;
    }

    static bool is_integer(const SerialObjectPtr &o) {
        return o->get_tag() == VM_OBJECT_INTEGER;
    }

    static vm_int_t get_integer(const SerialObjectPtr &o);

    static SerialObjectPtr create_integer(const objectid_t id,
                                          const vm_int_t n);

    static bool is_float(const SerialObjectPtr &o) {
        return o->get_tag() == VM_OBJECT_FLOAT;
    }

    static vm_float_t get_float(const SerialObjectPtr &o);

    static SerialObjectPtr create_float(const objectid_t id,
                                        const vm_float_t f);

    static bool is_char(const SerialObjectPtr &o) {
        return o->get_tag() == VM_OBJECT_CHAR;
    }

    static vm_char_t get_char(const SerialObjectPtr &o);

    static SerialObjectPtr create_char(const objectid_t id, const vm_char_t c);

    static bool is_text(const SerialObjectPtr &o) {
        return o->get_tag() == VM_OBJECT_TEXT;
    }

    static vm_text_t get_text(const SerialObjectPtr &o);

    static SerialObjectPtr create_text(const objectid_t id, const vm_text_t t);

    static bool is_array(const SerialObjectPtr &o) {
        return o->get_tag() == VM_OBJECT_ARRAY;
    }

    static std::vector<objectid_t> get_array(const SerialObjectPtr &o);

    static SerialObjectPtr create_array(const objectid_t id,
                                        const std::vector<objectid_t> &ii);

    static bool is_combinator(const SerialObjectPtr &o) {
        return o->get_tag() == VM_OBJECT_COMBINATOR;
    }

    static icu::UnicodeString get_combinator(const SerialObjectPtr &o);

    static SerialObjectPtr create_combinator(const objectid_t id,
                                             const icu::UnicodeString &s);

private:
    vm_tag_t _tag;
    objectid_t _id;
};

class SerialInteger : public SerialObject {
public:
    SerialInteger(const objectid_t id, const vm_int_t n)
        : SerialObject(VM_OBJECT_INTEGER, id), _value(n) {
    }

    SerialInteger(const SerialInteger &s)
        : SerialInteger(s.get_id(), s.get_value()) {
    }

    static SerialObjectPtr create(const objectid_t id, const vm_int_t n) {
        return SerialObjectPtr(new SerialInteger(id, n));
    }

    vm_int_t get_value() const {
        return _value;
    }

private:
    vm_int_t _value;
};

inline vm_int_t SerialObject::get_integer(const SerialObjectPtr &o) {
    return std::static_pointer_cast<SerialInteger>(o)->get_value();
};

inline SerialObjectPtr SerialObject::create_integer(const objectid_t id,
                                                    const vm_int_t n) {
    return SerialInteger::create(id, n);
};

class SerialFloat : public SerialObject {
public:
    SerialFloat(const objectid_t id, const vm_float_t f)
        : SerialObject(VM_OBJECT_FLOAT, id), _value(f) {
    }

    SerialFloat(const SerialFloat &s) : SerialFloat(s.get_id(), s.get_value()) {
    }

    static SerialObjectPtr create(const objectid_t id, const vm_float_t f) {
        return SerialObjectPtr(new SerialFloat(id, f));
    }

    vm_float_t get_value() const {
        return _value;
    }

private:
    vm_float_t _value;
};

inline vm_float_t SerialObject::get_float(const SerialObjectPtr &o) {
    return std::static_pointer_cast<SerialFloat>(o)->get_value();
};

inline SerialObjectPtr SerialObject::create_float(const objectid_t id,
                                                  const vm_float_t n) {
    return SerialFloat::create(id, n);
};

class SerialChar : public SerialObject {
public:
    SerialChar(const objectid_t id, const vm_char_t c)
        : SerialObject(VM_OBJECT_CHAR, id), _value(c) {
    }

    SerialChar(const SerialChar &s) : SerialChar(s.get_id(), s.get_value()) {
    }

    static SerialObjectPtr create(const objectid_t id, const vm_char_t c) {
        return SerialObjectPtr(new SerialChar(id, c));
    }

    vm_char_t get_value() const {
        return _value;
    }

private:
    vm_char_t _value;
};

inline vm_char_t SerialObject::get_char(const SerialObjectPtr &o) {
    return std::static_pointer_cast<SerialChar>(o)->get_value();
};

inline SerialObjectPtr SerialObject::create_char(const objectid_t id,
                                                 const vm_char_t n) {
    return SerialChar::create(id, n);
};

class SerialText : public SerialObject {
public:
    SerialText(const objectid_t id, const vm_text_t c)
        : SerialObject(VM_OBJECT_TEXT, id), _value(c) {
    }

    SerialText(const SerialText &s) : SerialText(s.get_id(), s.get_value()) {
    }

    static SerialObjectPtr create(const objectid_t id, const vm_text_t c) {
        return SerialObjectPtr(new SerialText(id, c));
    }

    vm_text_t get_value() const {
        return _value;
    }

private:
    vm_text_t _value;
};

inline vm_text_t SerialObject::get_text(const SerialObjectPtr &o) {
    return std::static_pointer_cast<SerialText>(o)->get_value();
};

inline SerialObjectPtr SerialObject::create_text(const objectid_t id,
                                                 const vm_text_t n) {
    return SerialText::create(id, n);
};

class SerialArray : public SerialObject {
public:
    SerialArray(const objectid_t id, const std::vector<objectid_t> &c)
        : SerialObject(VM_OBJECT_ARRAY, id), _value(c) {
    }

    SerialArray(const SerialArray &s) : SerialArray(s.get_id(), s.get_value()) {
    }

    static SerialObjectPtr create(const objectid_t id,
                                  const std::vector<objectid_t> &c) {
        return SerialObjectPtr(new SerialArray(id, c));
    }

    std::vector<objectid_t> get_value() const {
        return _value;
    }

private:
    std::vector<objectid_t> _value;
};

inline std::vector<objectid_t> SerialObject::get_array(
    const SerialObjectPtr &o) {
    return std::static_pointer_cast<SerialArray>(o)->get_value();
};

inline SerialObjectPtr SerialObject::create_array(
    const objectid_t id, const std::vector<objectid_t> &n) {
    return SerialArray::create(id, n);
};

class SerialCombinator : public SerialObject {
public:
    SerialCombinator(const objectid_t id, const icu::UnicodeString &c)
        : SerialObject(VM_OBJECT_COMBINATOR, id), _value(c) {
    }

    SerialCombinator(const SerialCombinator &s)
        : SerialCombinator(s.get_id(), s.get_value()) {
    }

    static SerialObjectPtr create(const objectid_t id,
                                  const icu::UnicodeString &c) {
        return SerialObjectPtr(new SerialCombinator(id, c));
    }

    vm_text_t get_value() const {
        return _value;
    }

private:
    vm_text_t _value;
};

inline vm_text_t SerialObject::get_combinator(const SerialObjectPtr &o) {
    return std::static_pointer_cast<SerialCombinator>(o)->get_value();
};

inline SerialObjectPtr SerialObject::create_combinator(
    const objectid_t id, const icu::UnicodeString &n) {
    return SerialCombinator::create(id, n);
};

using SerialObjectPtrs = std::vector<SerialObjectPtr>;

using VMObjectsTo = std::vector<VMObjectPtr>;
using VMObjectsFrom = std::map<VMObjectPtr, objectid_t>;
using VMObjectsStack = std::stack<VMObjectPtr>;
using VMObjectsSet = std::set<VMObjectPtr>;

inline SerialObjectPtrs to_dag(VM *m, const VMObjectPtr &o) {
    VMObjectsStack work0;
    work0.push(o);

    VMObjectsSet visited;
    VMObjectsStack work1; // terminal nodes
    VMObjectsStack work2; // array nodes

    while (!work0.empty()) {
        auto o = work0.top();
        work0.pop();
        if (!visited.contains(o)) {
            visited.insert(o);
            if (m->is_array(o)) {
                work2.push(o);
                auto n = m->array_size(o);
                for (unsigned int i = 0; i < n; i++) {
                    auto o0 = m->array_get(o, i);
                    work0.push(o0);
                }
            } else {
                work1.push(o);
            }
        }
    }
    visited.clear();

    VMObjectsFrom from;
    SerialObjectPtrs dag;

    while (!work1.empty()) {
        auto o = work1.top();
        work1.pop();
        auto sz = dag.size();
        switch (o->tag()) {
            case VM_OBJECT_INTEGER: {
                auto n = m->get_integer(o);
                auto s = SerialObject::create_integer(sz, n);
                dag.push_back(s);
                from[o] = sz;
            } break;
            case VM_OBJECT_FLOAT: {
                auto f = m->get_float(o);
                auto s = SerialObject::create_float(sz, f);
                dag.push_back(s);
                from[o] = sz;
            } break;
            case VM_OBJECT_CHAR: {
                auto c = m->get_char(o);
                auto s = SerialObject::create_char(sz, c);
                dag.push_back(s);
                from[o] = sz;
            } break;
            case VM_OBJECT_TEXT: {
                auto t = m->get_text(o);
                auto s = SerialObject::create_text(sz, t);
                dag.push_back(s);
                from[o] = sz;
            } break;
            case VM_OBJECT_COMBINATOR: {
                auto t = m->symbol(o);
                auto s = SerialObject::create_combinator(sz, t);
                dag.push_back(s);
                from[o] = sz;
            } break;
            case VM_OBJECT_ARRAY: {
                auto oo = m->get_array(o);
                std::vector<objectid_t> ss;
                for (auto &o0 : oo) {
                    ss.push_back(from[o0]);
                }
                auto s = SerialObject::create_array(sz, ss);
                dag.push_back(s);
                from[o] = sz;
            } break;
            case VM_OBJECT_OPAQUE:
                throw m->create_text(
                    "cannot serialize opaque");  // change this once
                break;
        }
    }

    // XXX
    while (!work2.empty()) {
        auto o = work2.top();
        work2.pop();
        auto sz = dag.size();
        switch (o->tag()) {
            case VM_OBJECT_INTEGER: {
                auto n = m->get_integer(o);
                auto s = SerialObject::create_integer(sz, n);
                dag.push_back(s);
                from[o] = sz;
            } break;
            case VM_OBJECT_FLOAT: {
                auto f = m->get_float(o);
                auto s = SerialObject::create_float(sz, f);
                dag.push_back(s);
                from[o] = sz;
            } break;
            case VM_OBJECT_CHAR: {
                auto c = m->get_char(o);
                auto s = SerialObject::create_char(sz, c);
                dag.push_back(s);
                from[o] = sz;
            } break;
            case VM_OBJECT_TEXT: {
                auto t = m->get_text(o);
                auto s = SerialObject::create_text(sz, t);
                dag.push_back(s);
                from[o] = sz;
            } break;
            case VM_OBJECT_COMBINATOR: {
                auto t = m->symbol(o);
                auto s = SerialObject::create_combinator(sz, t);
                dag.push_back(s);
                from[o] = sz;
            } break;
            case VM_OBJECT_ARRAY: {
                auto oo = m->get_array(o);
                std::vector<objectid_t> ss;
                for (auto &o0 : oo) {
                    ss.push_back(from[o0]);
                }
                auto s = SerialObject::create_array(sz, ss);
                dag.push_back(s);
                from[o] = sz;
            } break;
            case VM_OBJECT_OPAQUE:
                throw m->create_text(
                    "cannot serialize opaque");  // change this once
                break;
        }
    }

    return dag;
};

inline VMObjectPtr from_dag(VM *m, const SerialObjectPtrs &dag) {
    std::map<objectid_t, VMObjectPtr> map;

    for (auto &d : dag) {
        switch (d->get_tag()) {
            case VM_OBJECT_INTEGER: {
                auto n = SerialObject::get_integer(d);
                auto o = m->create_integer(n);
                map[d->get_id()] = o;
            } break;
            case VM_OBJECT_FLOAT: {
                auto f = SerialObject::get_float(d);
                auto o = m->create_float(f);
                map[d->get_id()] = o;
            } break;
            case VM_OBJECT_CHAR: {
                auto c = SerialObject::get_char(d);
                auto o = m->create_char(c);
                map[d->get_id()] = o;
            } break;
            case VM_OBJECT_TEXT: {
                auto t = SerialObject::get_text(d);
                auto o = m->create_text(t);
                map[d->get_id()] = o;
            } break;
            case VM_OBJECT_COMBINATOR: {
                auto t = SerialObject::get_combinator(d);
                auto o = m->get_combinator(t);
                map[d->get_id()] = o;
            } break;
            case VM_OBJECT_ARRAY: {
                VMObjectPtrs oo;
                auto nn = SerialObject::get_array(d);
                for (auto &n : nn) {
                    oo.push_back(map[n]);
                }
                map[d->get_id()] = m->create_array(oo);
            } break;
            default:
                throw m->create_text("unhandled deserialization case");
        }
    }
    return map[dag.size() - 1];
}

inline void dag_serialize(VM *m, const SerialObjectPtrs &dag,
                          std::ostream &os) {
    os << "[" << std::endl;
    for (auto &s : dag) {
        switch (s->get_tag()) {
            case VM_OBJECT_INTEGER: {
                auto n = SerialObject::get_integer(s);
                os << s->get_id() << ": i " << n << std::endl;
            } break;
            case VM_OBJECT_FLOAT: {
                auto f = SerialObject::get_float(s);
                os << s->get_id() << ": f " << f << std::endl;
            } break;
            case VM_OBJECT_CHAR: {
                auto c = SerialObject::get_char(s);
                icu::UnicodeString cc;
                cc = VM::unicode_escape(cc + c);
                os << s->get_id() << ": c '" << cc << "'" << std::endl;
            } break;
            case VM_OBJECT_TEXT: {
                auto t = SerialObject::get_text(s);
                auto cc = VM::unicode_escape(t);
                os << s->get_id() << ": t \"" << cc << "\"" << std::endl;
            } break;
            case VM_OBJECT_COMBINATOR: {
                auto t = SerialObject::get_combinator(s);
                os << s->get_id() << ": o " << t << std::endl;
            } break;
            case VM_OBJECT_ARRAY: {
                auto ss = SerialObject::get_array(s);
                os << s->get_id() << ": a [";
                for (auto &s : ss) {
                    os << " " << s;
                }
                os << " ]" << std::endl;
            } break;
            case VM_OBJECT_OPAQUE: {
                throw m->create_text(
                    "cannot serialize opaque");  // change this once
            } break;
        }
    }
    os << "]" << std::endl;
};

inline bool is_char(std::istream &is, char c) {
    auto p = is.peek();
    return p == c;
};

inline int get_char(std::istream &is) {
    return is.get();
};
inline void skip_char(std::istream &is) {
    get_char(is);
};

inline void forced_char(VM *m, std::istream &is, char c) {
    if (is_char(is, c)) {
        skip_char(is);
    } else {
        throw m->create_text("deserialization error");
    }
};

inline void skip_white(VM *m, std::istream &is) {
    while (is_char(is, ' ') || is_char(is, '\n')) {
        skip_char(is);
    }
};

inline vm_char_t parse_char(VM *m, std::istream &is) {
    forced_char(m, is, '\'');
    char buffer[32];
    int n = 0;
    while (!is_char(is, '\'')) {
        auto c = (char)get_char(is);
        if (c == '\\') {
            buffer[n] = c;
            n++;
            c = get_char(is);
            buffer[n] = c;
            n++;
        } else {
            buffer[n] = c;
            n++;
        }
    }
    forced_char(m, is, '\'');
    buffer[n] = '\0';

    auto s = char_to_unicode(buffer);
    auto u = unicode_unescape(s);
    return u.char32At(0);
};

inline icu::UnicodeString parse_text(VM *m, std::istream &is) {
    forced_char(m, is, '\"');
    int n = 0;
    int capacity = 8192;
    char *buffer = (char *)malloc(capacity);
    while (!is_char(is, '\"')) {
        // realloc if necessary
        if (n >= (capacity - 4)) {  // be generous
            capacity += capacity;
            buffer = (char *)realloc(buffer, capacity);
        }
        auto c = (char)get_char(is);
        if (c == '\\') {
            buffer[n] = c;
            n++;
            c = get_char(is);
            buffer[n] = c;
            n++;
        } else {
            buffer[n] = c;
            n++;
        }
    }
    forced_char(m, is, '\"');
    buffer[n] = '\0';

    auto s = char_to_unicode(buffer);
    auto u = unicode_unescape(s);
    return u;
};

inline icu::UnicodeString parse_symbol(VM *m, std::istream &is) {
    int n = 0;
    int capacity = 8192;
    char *buffer = (char *)malloc(capacity);
    while (!is_char(is, '\n')) {
        // realloc if necessary
        if (n >= (capacity - 4)) {  // be generous
            capacity += capacity;
            buffer = (char *)realloc(buffer, capacity);
        }
        auto c = (char)get_char(is);
        if (c == '\\') {
            buffer[n] = c;
            n++;
            c = get_char(is);
            buffer[n] = c;
            n++;
        } else {
            buffer[n] = c;
            n++;
        }
    }
    buffer[n] = '\0';

    auto s = char_to_unicode((const char *)buffer);
    free(buffer);
    return s;
};

inline std::vector<objectid_t> parse_array(VM *m, std::istream &is) {
    std::vector<objectid_t> oo;

    forced_char(m, is, '[');
    skip_white(m, is);
    while (!is_char(is, ']')) {
        objectid_t o;
        is >> o;
        oo.push_back(o);
        skip_white(m, is);
    }
    forced_char(m, is, ']');

    return oo;
}

inline SerialObjectPtrs dag_deserialize(VM *m, std::istream &is) {
    SerialObjectPtrs ss;

    forced_char(m, is, '[');
    skip_white(m, is);

    while (!is_char(is, ']')) {
        objectid_t o;
        is >> o;
        skip_white(m, is);
        forced_char(m, is, ':');
        skip_white(m, is);
        auto c = get_char(is);
        skip_white(m, is);
        switch (c) {
            case 'i': {
                vm_int_t n;
                is >> n;
                auto s = SerialObject::create_integer(o, n);
                ss.push_back(s);
            } break;
            case 'f': {
                vm_float_t f;
                is >> f;
                auto s = SerialObject::create_float(o, f);
                ss.push_back(s);
            } break;
            case 'c': {
                vm_char_t c;
                c = parse_char(m, is);
                auto s = SerialObject::create_char(o, c);
                ss.push_back(s);
            } break;
            case 't': {
                vm_text_t t;
                t = parse_text(m, is);
                auto s = SerialObject::create_text(o, t);
                ss.push_back(s);
            } break;
            case 'o': {
                icu::UnicodeString t;
                t = parse_symbol(m, is);
                auto s = SerialObject::create_combinator(o, t);
                ss.push_back(s);
            } break;
            case 'a': {
                icu::UnicodeString t;
                auto oo = parse_array(m, is);
                auto s = SerialObject::create_array(o, oo);
                ss.push_back(s);
            } break;
            default:
                throw m->create_text("deserialization error");
                break;
        }
        skip_white(m, is);
    }

    forced_char(m, is, ']');

    return ss;
};

inline icu::UnicodeString dag_text(VM *m, const SerialObjectPtrs &dag) {
    std::stringstream ss;
    dag_serialize(m, dag, ss);
    icu::UnicodeString u(ss.str().c_str());
    return u;
};

// no idea why this is needed, copied from the intertubes
class membuf : public std::basic_streambuf<char> {
public:
    membuf(const uint8_t *p, size_t l) {
        setg((char *)p, (char *)p, (char *)p + l);
    }
};

inline SerialObjectPtrs dag_from_text(VM *m, const icu::UnicodeString &s) {
    auto buffer = (uint8_t *)unicode_to_char(s);
    membuf sbuf(buffer, (size_t)(buffer + sizeof(buffer)));
    std::istream in(&sbuf);
    auto ss = dag_deserialize(m, in);
    free(buffer);
    return ss;
};

inline icu::UnicodeString serialize_to_string(VM *m, const VMObjectPtr &o) {
    auto dag = to_dag(m, o);
    auto s = dag_text(m, dag);
    return s;
};

inline VMObjectPtr deserialize_from_string(VM *m, const icu::UnicodeString &s) {
    auto dag = dag_from_text(m, s);
    auto o = from_dag(m, dag);
    return o;
};

inline VMObjectPtrs bundle(VM *m, const VMObjectPtr &o) {
    VMObjectsStack work0;
    work0.push(o);

    VMObjectsSet visited;
    VMObjectsStack work1;

    // first run through the dag and collect all data and bytecode objects
    while (!work0.empty()) {
        auto o = work0.top();
        work0.pop();
        if (!visited.contains(o)) {
            visited.insert(o);
            if (m->is_data(o)) {
                work1.push(o);
            } else if (m->is_bytecode(o)) {
                work1.push(o);
            } else if (m->is_array(o)) {
                auto n = m->array_size(o);
                for (unsigned int i = 0; i < n; i++) {
                    auto o0 = m->array_get(o, i);
                    work0.push(o0);
                }
            }
        }
    }
    visited.clear();

    // then close over all bytecode combinators
    VMObjectPtrs oo;
    while (!work1.empty()) {
        auto o = work1.top();
        work1.pop();
        if (!visited.contains(o)) {
            visited.insert(o);
            oo.push_back(o);
            if (m->is_bytecode(o)) {
                auto oo0 = m->get_bytedata(o);
                for (auto o0 : oo0) {
                    if (m->is_data(o0)) {
                        work1.push(o0);
                    } else if (m->is_bytecode(o0)) {
                        work1.push(o0);
                    }
                }
            }
        }
    }

    return oo;
}
