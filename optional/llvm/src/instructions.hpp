#pragma once

#include <egel/runtime>

enum opcode_t {
    OP_FAIL,     //  l           pc := l, if flag
    OP_RETURN,   //  x           return x
};

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

inline void vm_object_ptr_assign(VMObjectPtr* p, const VMObjectPtr &o) {
    *p = o;
};

// OP_NIL x,  x := null
inline void op_nil(VM *vm, VMOjectPtr *x) {
    *x = nullptr;
};

// OP_MOV x y, x := y
inline void op_mov(VM *vm, VMOjectPtr *x, VMObjectPtr *y) {
    *x = *y;
};

// OP_DATA x i32, x := data(i32)
inline void op_data(VM* vm, VMObjectPtr *x, int i) {
    *x = vm->get_data(i);
};

// OP_ARRAY x y z, x := [ y, y+1,.., z ]
inline void op_array(VM* vm, VMObjectPtr *x, VMObjectPtr* y, int n) {
    auto a0 = egel::VMObjectArray::create(n);
    auto a1 = egel::VMObjectArray::cast(a1);
    for (int i = 0; i < n; i++) {
        a1->set(i, *(y[n]));
    }
    *x = a1;
};

// OP_SET x y z, x[val(y)] = z
inline void op_set(VM* vm, VMObjectPtr* x, VMObjectPtr *y, VMObjectPtr *z) {
    int n = egel::VMObjectInteger::cast(*y)->value();
    int a = egel::VMObjectArray::cast(*x);
    a->set(n, *z);
};

// OP_TAKEX x y z i16, x,..,y = z[i],..,z[i+y-x], ~flag if fail
inline void op_takex(VM* vm, VMObjectPtr* x, int n0, VMObjectPtr* z, int n1, bool *flag) {
    auto a = egel::VMObjectArray::cast(*z);
    if (a->size() < n0 + n1) {
        *flag = false;
    } else {
        for (int i = 0; i < n0; i++) {
            *(x[i]) = a->get(n1+i);
        }
    }
};

// OP_SPLIT x y z, x,..,y = z[0],..,z[y-x], flag if not exact
inline void op_split(VM* vm, VMObjectPtr* x, int n, VMObjectPtr* z, bool* flag) {
    auto a = egel::VMObjectArray::cast(*z);
    if (a->size() != n) {
        *flag = false;
    } else {
        for (int i = 0; i < n; i++) {
            *(x[i]) = a->get(i);
        }
    }
};

// OP_CONCATX x y z i16, x := y ++ drop i z
inline void op_concat_x(VM* vm, VMOjectPtr *x, VMObjectPtr *y, VMObjectPtr *z, int i) {
};

// OP_TEST x y, flag := (x == y)
inline void op_test(VM* vm, VMOjectPtr *x, VMObjectPtr *y, bool* flag) {
};

// OP_TAG x y, flag := (x->tag() == y)
inline void op_tag(VM* vm, VMOjectPtr *x, VMObjectPtr *y, bool* flag) {
};

