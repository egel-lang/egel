#include "../../../src/runtime.hpp"

// type test
template<typename T> struct typetest {};

template<> struct typetest<vm_int_t> {
    static inline bool func(const VMObjectPtr& o) {
        return o->tag() == VM_OBJECT_INTEGER;
    }
};

template<> struct typetest<vm_float_t> {
    static inline bool func(const VMObjectPtr& o) {
        return o->tag() == VM_OBJECT_FLOAT;
    }
};

template<> struct typetest<vm_text_t> {
    static inline bool func(const VMObjectPtr& o) {
        return o->tag() == VM_OBJECT_TEXT;
    }
};

template<> struct typetest<vm_char_t> {
    static inline bool func(const VMObjectPtr& o) {
        return o->tag() == VM_OBJECT_CHAR;
    }
};

template<> struct typetest<vm_ptr_t> {
    static inline bool func(const VMObjectPtr& o) {
        return o->tag() == VM_OBJECT_POINTER;
    }
};

// value coercion from object
template<typename T> struct value_from {};

template<> struct value_from<vm_int_t> {
    static inline vm_int_t func(const VMObjectPtr& o) {
        return VM_OBJECT_INTEGER_VALUE(o);
    }
};

template<> struct value_from<vm_float_t> {
    static inline vm_float_t func(const VMObjectPtr& o) {
        return VM_OBJECT_FLOAT_VALUE(o);
    }
};

template<> struct value_from<vm_text_t> {
    static inline vm_text_t func(const VMObjectPtr& o) {
        return VM_OBJECT_TEXT_VALUE(o);
    }
};

template<> struct value_from<vm_char_t> {
    static inline vm_char_t func(const VMObjectPtr& o) {
        return VM_OBJECT_CHAR_VALUE(o);
    }
};

template<> struct value_from<vm_ptr_t> {
    static inline vm_ptr_t func(const VMObjectPtr& o) {
        return VM_OBJECT_POINTER_VALUE(o);
    }
};

// value coercion to object
template<typename T> struct value_to {};

template<> struct value_to<vm_int_t> {
    static inline VMObjectPtr func(const vm_int_t& v) {
        return VMObjectInteger::create(v);
    }
};

template<> struct value_to<vm_float_t> {
    static inline VMObjectPtr func(const vm_float_t& v) {
        return VMObjectFloat::create(v);
    }
};

template<> struct value_to<vm_text_t> {
    static inline VMObjectPtr func(const vm_text_t& v) {
        return VMObjectText::create(v);
    }
};

template<> struct value_to<vm_char_t> {
    static inline VMObjectPtr func(const vm_char_t& v) {
        return VMObjectChar::create(v);
    }
};

template<> struct value_to<vm_ptr_t> {
    static inline VMObjectPtr func(const vm_ptr_t& v) {
        return VMObjectPointer::create(v);
    }
};

// ffi call class construction
template<typename R> class ffi0: public Medadic {
public:
    ffi0<R>(VM* m, const UnicodeString& n0, const UnicodeString& n1, R(*c)()):
        Medadic(m, n0, n1), _call(c) {
    }

    ffi0<R>(VM* m, const symbol_t s, R(*c)()): Medadic(m, s), _call(c) {
    }

    ffi0<R>(const ffi0<R>& o) : ffi0<R>(o.machine(), o.symbol(), o._call) {
    }

    VMObjectPtr clone() const {
        return VMObjectPtr(new ffi0<R>(*this));
    }

    VMObjectPtr apply() const override {
        auto r0 = _call();
        auto r1 = value_to<R>::func(r0);
        return r1;
    }

protected:
    R (*_call)();
};

template<typename R, typename A0> class ffi1: public Monadic {
public:
    ffi1<R, A0>(VM* m, const UnicodeString& n0, const UnicodeString& n1, R(*c)(const A0)):
        Monadic(m, n0, n1), _call(c) {
    }

    ffi1<R, A0>(VM* m, const symbol_t s, R(*c)(const A0)): Monadic(m, s), _call(c) {
    }

    ffi1<R, A0>(const ffi1<R, A0>& o) : ffi1<R, A0>(o.machine(), o.symbol(), o._call) {
    }

    VMObjectPtr clone() const {
        return VMObjectPtr(new ffi1(*this));
    }

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (!(typetest<A0>::func(arg0))) return nullptr;
        auto a0 = value_from<A0>::func(arg0);
        auto r0 = _call(a0);
        auto r1 = value_to<R>::func(r0);
        return r1;
    }

protected:
    R (*_call)(const A0);
};

template<typename R, typename A0, typename A1> class ffi2: public Dyadic {
public:
    ffi2<R, A0, A1>(VM* m, const UnicodeString& n0, const UnicodeString& n1, R(*c)(const A0, const A1)):
        Dyadic(m, n0, n1), _call(c) {
    }

    ffi2<R, A0, A1>(VM* m, const symbol_t s, R(*c)(const A0, A1)): Dyadic(m, s), _call(c) {
    }

    ffi2<R, A0, A1>(const ffi2<R, A0, A1>& o) : ffi2<R, A0, A1>(o.machine(), o.symbol(), o._call) {
    }

    VMObjectPtr clone() const {
        return VMObjectPtr(new ffi2(*this));
    }

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if (!(typetest<A0>::func(arg0))) return nullptr;
        if (!(typetest<A1>::func(arg1))) return nullptr;
        auto a0 = value_from<A0>::func(arg0);
        auto a1 = value_from<A1>::func(arg1);
        auto r0 = _call(a0, a1);
        auto r1 = value_to<R>::func(r0);
        return r1;
    }

protected:
    R (*_call)(const A0, const A1);
};

template<typename R, typename A0, typename A1, typename A2> class ffi3: public Triadic {
public:
    ffi3<R, A0, A1, A2>(VM* m, const UnicodeString& n0, const UnicodeString& n1, R(*c)(const A0, const A1, A2)):
        Triadic(m, n0, n1), _call(c) {
    }

    ffi3<R, A0, A1, A2>(VM* m, const symbol_t s, R(*c)(const A0, A1, A2)): Triadic(m, s), _call(c) {
    }

    ffi3<R, A0, A1, A2>(const ffi3<R, A0, A1, A2>& o) : ffi3<R, A0, A1, A2>(o.machine(), o.symbol(), o._call) {
    }

    VMObjectPtr clone() const {
        return VMObjectPtr(new ffi3(*this));
    }

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1, const VMObjectPtr& arg2) const override {
        if (!(typetest<A0>::func(arg0))) return nullptr;
        if (!(typetest<A1>::func(arg1))) return nullptr;
        if (!(typetest<A2>::func(arg2))) return nullptr;
        auto a0 = value_from<A0>::func(arg0);
        auto a1 = value_from<A1>::func(arg1);
        auto a2 = value_from<A2>::func(arg2);
        auto r0 = _call(a0, a1, a2);
        auto r1 = value_to<R>::func(r0);
        return r1;
    }

protected:
    R (*_call)(const A0, const A1);
};

