#pragma once

// this is written to use mimalloc as a slab allocator but that's for later

#include <atomic>

#include "unicode/uchar.h"
#include "unicode/unistr.h"
#include "unicode/ustdio.h"
#include "unicode/ustream.h"
#include "unicode/ustring.h"

#include "runtime.hpp"

const unsigned int VM_TAG_BITS = 3;
const vm_tagbits_t VM_TAG_MASK = (1 << VM_TAG_BITS) - 1;
const vm_tagbits_t VM_RC_ONE = 1 << VM_TAG_BITS;

inline vm_tag_t vm_tagbits_tag(const vm_tagbits_t bb) const {
    return (vm_tag_t)(bb & VM_TAG_MASK);
};

inline vm_tagbits_t vm_tagbits_rc(const vm_tagbits_t bb) const {
    return bb >> VM_TAG_BITS;
};

inline vm_tagbits_t vm_tagbits_inc(const vm_tagbits_t bb) {
    return bb + VM_RC_ONE;
};

inline vm_tagbits_t vm_tagbits_dec(const vm_tagbits_t bb) {
    return bb - VM_RC_ONE;
};

inline vm_tag_t vm_object_tag(const vm_object_t* p) const {
    return vm_tagbits_tag(p->tagbits);
};

inline unsigned int vm_object_rc(const vm_object_t* p) const {
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

inline void* vm_combinator_value(const vm_object_t* p) {
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

inline symbol_t vm_object_symbol(vm_object_t* p) {
    if (vm_is_int(p)) {
        return SYMBOL_INT;
    } else if (vm_is_float(p)) {
        return SYMBOL_FLOAT;
    } else if (vm_is_char(p)) {
        return SYMBOL_CHAR;
    } else if (vm_is_text(p)) {
        return SYMBOL_TEXT
    } else if (vm_is_opaque(p)) {
        return vm_opaque_value(p)->symbol();
    } if (vm_is_combinator(p)) {
        return vm_combinator_value(p)->symbol();
    } else {
        return 0;
    }
};
