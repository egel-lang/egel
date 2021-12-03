#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP

#include <functional>
#include "utils.hpp"
#include "runtime.hpp"
#include "modules.hpp"

#include <map>
#include <vector>
#include <queue>
#include <ios>

// unfortunately it looks better to copy the entire object
// hierarchy of the runtime in order not to taint runtimes
// when serialized objects are shipped. 

typedef unsigned int objectid_t;

class SerialObject;
typedef std::shared_ptr<SerialObject> SerialObjectPtr;

class SerialObject {
public:
    SerialObject(const vm_tag_t tag)
    : _tag(tag), _id(0) {
    }

    SerialObject(const vm_tag_t tag, const objectid_t id)
    : _tag(tag), _id(id) {
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

    static bool is_integer(const SerialObjectPtr& o) {
        return o->get_tag() == VM_OBJECT_INTEGER;
    }

    static vm_int_t get_integer(const SerialObjectPtr& o);

    static SerialObjectPtr create_integer(const objectid_t id, const vm_int_t n);

    static bool is_float(const SerialObjectPtr& o) {
        return o->get_tag() == VM_OBJECT_FLOAT;
    }

    static vm_float_t get_float(const SerialObjectPtr& o);

    static SerialObjectPtr create_float(const objectid_t id, const vm_float_t f);

    static bool is_char(const SerialObjectPtr& o) {
        return o->get_tag() == VM_OBJECT_CHAR;
    }

    static vm_char_t get_char(const SerialObjectPtr& o) ;

    static SerialObjectPtr create_char(const objectid_t id, const vm_char_t c);

    static bool is_text(const SerialObjectPtr& o) {
        return o->get_tag() == VM_OBJECT_TEXT;
    }

    static vm_text_t get_text(const SerialObjectPtr& o) ;

    static SerialObjectPtr create_text(const objectid_t id, const vm_text_t t);

    static bool is_array(const SerialObjectPtr& o) {
        return o->get_tag() == VM_OBJECT_ARRAY;
    }

    static std::vector<objectid_t> get_array(const SerialObjectPtr& o) ;

    static SerialObjectPtr create_array(const objectid_t id, const std::vector<objectid_t>& ii) ;

    static bool is_combinator(const SerialObjectPtr& o) {
        return o->get_tag() == VM_OBJECT_COMBINATOR;
    }

    static icu::UnicodeString get_combinator(const SerialObjectPtr& o) ;

    static SerialObjectPtr create_combinator(const objectid_t id, const icu::UnicodeString& s) ;

private:
    vm_tag_t   _tag;
    objectid_t _id;
};

class SerialInteger: public SerialObject {
public:
    SerialInteger(const objectid_t id, const vm_int_t n)
    : SerialObject(VM_OBJECT_INTEGER, id), _value(n) {
    }

    SerialInteger(const SerialInteger& s)
    : SerialInteger(s.get_id(), s.get_value()) {
    }

    static SerialObjectPtr create(const objectid_t id, const vm_int_t n) {
        return SerialObjectPtr(new SerialInteger(id, n));
    }

    vm_int_t get_value() const {
        return _value;
    }

private:
    vm_int_t    _value;
};

inline vm_int_t SerialObject::get_integer(const SerialObjectPtr& o) {
    return std::static_pointer_cast<SerialInteger>(o)->get_value();
};

inline SerialObjectPtr SerialObject::create_integer(const objectid_t id, const vm_int_t n) {
    return SerialInteger::create(id, n);
};

class SerialFloat: public SerialObject {
public:
    SerialFloat(const objectid_t id, const vm_float_t f)
    : SerialObject(VM_OBJECT_FLOAT, id), _value(f) {
    }

    SerialFloat(const SerialFloat& s)
    : SerialFloat(s.get_id(), s.get_value()) {
    }

    static SerialObjectPtr create(const objectid_t id, const vm_float_t f) {
        return SerialObjectPtr(new SerialFloat(id, f));
    }

    vm_float_t get_value() const {
        return _value;
    }

private:
    vm_float_t    _value;
};

inline vm_float_t SerialObject::get_float(const SerialObjectPtr& o) {
    return std::static_pointer_cast<SerialFloat>(o)->get_value();
};

inline SerialObjectPtr SerialObject::create_float(const objectid_t id, const vm_float_t n) {
    return SerialFloat::create(id, n);
};

class SerialChar: public SerialObject {
public:
    SerialChar(const objectid_t id, const vm_char_t c)
    : SerialObject(VM_OBJECT_CHAR, id), _value(c) {
    }

    SerialChar(const SerialChar& s)
    : SerialChar(s.get_id(), s.get_value()) {
    }

    static SerialObjectPtr create(const objectid_t id, const vm_char_t c) {
        return SerialObjectPtr(new SerialChar(id, c));
    }

    vm_char_t get_value() const {
        return _value;
    }

private:
    vm_char_t    _value;
};

inline vm_char_t SerialObject::get_char(const SerialObjectPtr& o) {
    return std::static_pointer_cast<SerialChar>(o)->get_value();
};

inline SerialObjectPtr SerialObject::create_char(const objectid_t id, const vm_char_t n) {
    return SerialChar::create(id, n);
};

class SerialText: public SerialObject {
public:
    SerialText(const objectid_t id, const vm_text_t c)
    : SerialObject(VM_OBJECT_TEXT, id), _value(c) {
    }

    SerialText(const SerialText& s)
    : SerialText(s.get_id(), s.get_value()) {
    }

    static SerialObjectPtr create(const objectid_t id, const vm_text_t c) {
        return SerialObjectPtr(new SerialText(id, c));
    }

    vm_text_t get_value() const {
        return _value;
    }

private:
    vm_text_t    _value;
};

inline vm_text_t SerialObject::get_text(const SerialObjectPtr& o) {
    return std::static_pointer_cast<SerialText>(o)->get_value();
};

inline SerialObjectPtr SerialObject::create_text(const objectid_t id, const vm_text_t n) {
    return SerialText::create(id, n);
};

class SerialCombinator: public SerialObject {
public:
    SerialCombinator(const objectid_t id, const icu::UnicodeString& c)
    : SerialObject(VM_OBJECT_TEXT, id), _value(c) {
    }

    SerialCombinator(const SerialCombinator& s)
    : SerialCombinator(s.get_id(), s.get_value()) {
    }

    static SerialObjectPtr create(const objectid_t id, const icu::UnicodeString& c) {
        return SerialObjectPtr(new SerialCombinator(id, c));
    }

    vm_text_t get_value() const {
        return _value;
    }

private:
    vm_text_t    _value;
};

inline vm_text_t SerialObject::get_combinator(const SerialObjectPtr& o) {
    return std::static_pointer_cast<SerialCombinator>(o)->get_value();
};

inline SerialObjectPtr SerialObject::create_combinator(const objectid_t id, const icu::UnicodeString& n) {
    return SerialCombinator::create(id, n);
};

/*
inline VMObjectMap graph_to_map(const VMObjectPtr& o) {
    VMObjectsQueue work;
};
*/

/*
typedef std::vector<const VMObject*>    VMObjectsTo;
typedef std::map<const VMObject*, objectid_t> VMObjectsFrom;
typedef std::queue<const VMObject*>     VMObjectsQueue;
*/

inline icu::UnicodeString serialize_to_string(VM* m, const VMObjectPtr& o) {
    throw m->create_text("not implemented");
};

inline VMObjectPtr deserialize_from_string(VM* m, const icu::UnicodeString& o) {
    throw m->create_text("not implemented");
};

#endif