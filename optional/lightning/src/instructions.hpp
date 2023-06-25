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

inline void vm_object_ptr_destruct(VMObjectPtr* p) {
    p->~VMObjectPtr();
};

inline void vm_object_ptr_assign(VMObjectPtr* p0, VMObjectPtr *p1) {
    *p0 = *p1;
};

// OP_NIL x,  x := null
inline void op_nil(VM *vm, VMObjectPtr *x) {
    *x = nullptr;
};

// OP_MOV x y, x := y
inline void op_mov(VM *vm, VMObjectPtr *x, VMObjectPtr *y) {
    *x = *y;
};

// OP_DATA x i32, x := data(i32)
inline void op_data(VM* vm, VMObjectPtr *x, int i) {
    *x = vm->get_data(i);
};

// OP_ARRAY x y z, x := [ y, y+1,.., z ]
inline void op_array(VM* vm, VMObjectPtr *x, VMObjectPtr* y, int n) {
    auto a0 = VMObjectArray::create(n);
    auto a1 = VMObjectArray::cast(a0);
    for (int i = 0; i < n; i++) {
        a1->set(i, y[i]);
    }
    *x = a1;
};

// OP_SET x y z, x[val(y)] = z
inline void op_set(VM* vm, VMObjectPtr* x, VMObjectPtr *y, VMObjectPtr *z) {
    int n = VMObjectInteger::cast(*y)->value();
    auto a = VMObjectArray::cast(*x);
    a->set(n, *z);
};

// OP_TAKEX x y z i16, x,..,y = z[i],..,z[i+y-x], ~flag if fail
inline void op_takex(VM* vm, VMObjectPtr* x, int n0, VMObjectPtr* z, int n1, bool *flag) {
    auto a = VMObjectArray::cast(*z);
    if (a->size() < n0 + n1) {
        *flag = false;
    } else {
        for (int i = 0; i < n0; i++) {
            x[i] = a->get(n1+i);
        }
    }
};

// OP_SPLIT x y z, x,..,y = z[0],..,z[y-x], flag if not exact
inline void op_split(VM* vm, VMObjectPtr* x, int n, VMObjectPtr* z, bool* flag) {
    auto a = VMObjectArray::cast(*z);
    if (a->size() != n) {
        *flag = false;
    } else {
        for (int i = 0; i < n; i++) {
            x[i] = a->get(i);
        }
    }
};

// OP_CONCATX x y z i16, x := y ++ drop i z
inline void op_concat_x(VM* vm, VMObjectPtr *x, VMObjectPtr *y, VMObjectPtr *z, int n) {
    auto y0 = VMObjectArray::cast(*y);
    auto z0 = VMObjectArray::cast(*z);

    int n0 = y0->size();
    int n1 = z0->size();

    if (n1 <= n) {
        vm_object_ptr_assign(x, y);
    } else {
        auto q0 = VMObjectArray::create(n0+n1-n);
        auto q1 = VMObjectArray::cast(q0);
        for (int i = 0; i < n0; i++) {
            q1->set(i, y0->get(i));
        }
        for (int i = n0; i < n0+n1-n; i++) {
            q1->set(i, z0->get(i+n-n0));
        }
        *x = q1; 
    }
};

// OP_TEST x y, flag := (x == y)
inline void op_test(VM* vm, VMObjectPtr *x, VMObjectPtr *y, bool* flag) {
    EqualVMObjectPtr equals;
    bool b = equals(*x, *y);
    *flag = b;
};

// OP_TAG x y, flag := (x->tag() == y)
inline void op_tag(VM* vm, VMObjectPtr *x, VMObjectPtr *y, bool* flag) {
    auto s0 = (*x)->symbol(); // XXX, wut?
    auto s1 = (*y)->symbol();
    bool b = ( s0 == s1 );
    *flag = b;
};

// OP_RETURN x
inline void op_return(VM*, VMObjectPtr *ret, VMObjectPtr *x) {
    vm_object_ptr_assign(ret, x);
};

}
