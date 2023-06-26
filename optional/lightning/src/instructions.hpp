#pragma once

#include <egel/runtime.hpp>

extern "C" {

using namespace egel;

const int VM_OBJECT_PTR_SIZE = sizeof(VMObjectPtr);

// create registers
inline VMObjectPtr* alloc_vm_object_ptr() {
    return static_cast<VMObjectPtr*>(malloc(VM_OBJECT_PTR_SIZE));
};

inline void vm_object_ptr_construct(VMObjectPtr* p) {
    new (p) VMObjectPtr();
};

inline void vm_object_ptr_construct_n(VMObjectPtr* p, int n) {
    for (int i = 0; i < n; i++) {
        new (&(p[i])) VMObjectPtr();
    }
};

inline void vm_object_ptr_destruct(VMObjectPtr* p) {
    p->~VMObjectPtr();
};

inline void vm_object_ptr_destruct_n(VMObjectPtr* p, int n) {
    for (int i = 0; i < n; i++) {
        p[i].~VMObjectPtr();
    }
};

inline void vm_object_ptr_assign(VMObjectPtr* p0, VMObjectPtr *p1) {
    *p0 = *p1;
};

// OP_NIL x,  x := null
inline void op_nil(VM *vm, VMObjectPtr *a, int x) {
    a[x] = nullptr;
};

// OP_MOV x y, x := y
inline void op_mov(VM *vm, VMObjectPtr *a, int x, int y) {
    a[x] = a[y];
};

// OP_DATA x i32, x := data(i32)
inline void op_data(VM* vm, VMObjectPtr *a, int x, int i) {
    a[x] = vm->get_data(i);
};

// OP_ARRAY x y z, x := [ y, y+1,.., z ]
inline void op_array(VM* vm, VMObjectPtr *a, int x, int y, int n) {
    auto a0 = VMObjectArray::create(n);
    auto a1 = VMObjectArray::cast(a0);
    for (int i = 0; i < n; i++) {
        a1->set(i, a[y+i]);
    }
    a[x] = a1;
};

// OP_SET x y z, x[val(y)] = z
inline void op_set(VM* vm, VMObjectPtr* a, int x, int  y, int z) {
    int n = VMObjectInteger::cast(a[y])->value();
    auto a0 = VMObjectArray::cast(a[x]);
    a0->set(n, a[z]);
};

// OP_TAKEX x y z i16, x,..,y = z[i],..,z[i+y-x], ~flag if fail
inline void op_takex(VM* vm, VMObjectPtr* a, int x, int n0, int z, int n1, bool *flag) {
    auto a0 = VMObjectArray::cast(a[z]);
    if (a0->size() < n0 + n1) {
        *flag = false;
    } else {
        for (int i = 0; i < n0; i++) {
            a[x+i] = a0->get(n1+i);
        }
    }
};

// OP_SPLIT x y z, x,..,y = z[0],..,z[y-x], flag if not exact
inline void op_split(VM* vm, VMObjectPtr* a, int x, int n, int z, bool* flag) {
    auto a0 = VMObjectArray::cast(a[z]);
    if (a0->size() != n) {
        *flag = false;
    } else {
        for (int i = 0; i < n; i++) {
            a[x+i] = a0->get(i);
        }
    }
};

// OP_CONCATX x y z i16, x := y ++ drop i z
inline void op_concat_x(VM* vm, VMObjectPtr *a, int x, int y, int z, int n) {
    auto y0 = VMObjectArray::cast(a[y]);
    auto z0 = VMObjectArray::cast(a[z]);

    int n0 = y0->size();
    int n1 = z0->size();

    if (n1 <= n) {
        a[x] = a[y];
    } else {
        auto q0 = VMObjectArray::create(n0+n1-n);
        auto q1 = VMObjectArray::cast(q0);
        for (int i = 0; i < n0; i++) {
            q1->set(i, y0->get(i));
        }
        for (int i = n0; i < n0+n1-n; i++) {
            q1->set(i, z0->get(i+n-n0));
        }
        a[x] = q1; 
    }
};

// OP_TEST x y, flag := (x == y)
inline void op_test(VM* vm, VMObjectPtr *a, int x, int y, bool* flag) {
    EqualVMObjectPtr equals;
    bool b = equals(a[x], a[y]);
    *flag = b;
};

// OP_TAG x y, flag := (x->tag() == y)
inline void op_tag(VM* vm, VMObjectPtr *a, int x, int y, bool* flag) {
    auto s0 = (a[x])->symbol(); // XXX, wut?
    auto s1 = (a[y])->symbol();
    bool b = ( s0 == s1 );
    *flag = b;
};

// OP_RETURN x
inline void op_return(VM*, VMObjectPtr *a, int x, VMObjectPtr *ret) {
    *ret = a[x];
};

}
