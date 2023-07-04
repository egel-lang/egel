#pragma once

#include "runtime.hpp"
#include "bytecode.hpp"

extern "C" {
#if __has_include(<lightning/lightning.h>)
    #include <lightning/lightning.h>
#else
    #include <lightning.h>
#endif
}

using namespace egel;

//#define TRACE_JIT(x)    x;
#define TRACE_JIT(x)    ;

extern "C" {

using namespace egel;

// used for debugging jit code
inline void marker() {
    TRACE_JIT(std::cerr << "marker" << std::endl);
};

// create registers
inline VMObjectPtr* alloc_vm_object_ptr() {
    TRACE_JIT(std::cerr << "alloc " << std::endl);
    return static_cast<VMObjectPtr*>(malloc(sizeof(VMObjectPtr)));
};

inline void vm_object_ptr_construct(VMObjectPtr* p) {
    TRACE_JIT(std::cerr << "construct " << p << std::endl);
    new (p) VMObjectPtr();
};

inline void vm_object_ptr_construct_n(VMObjectPtr* p, int n) {
    TRACE_JIT(std::cerr << "construct_n " << p << " " << n << std::endl);
    for (int i = 0; i < n; i++) {
        new (&(p[i])) VMObjectPtr();
    }
};

inline void vm_object_ptr_destruct(VMObjectPtr* p) {
    TRACE_JIT(std::cerr << "destruct " << std::endl);
    p->~VMObjectPtr();
};

inline void vm_object_ptr_destruct_n(VMObjectPtr* p, int n) {
    TRACE_JIT(std::cerr << "destruct_n " << p << ", " << n << std::endl);
    for (int i = 0; i < n; i++) {
        p[i].~VMObjectPtr();
    }
};

// LOAD 
inline void load(VM *vm, VMObjectPtr *a, VMObjectPtr *x) {
    TRACE_JIT(std::cerr << "LOAD " << vm << ", " << a << ", " << x << std::endl);
    a[0] = *x;;
};

// DEBUG
inline void debug(VM *vm, VMObjectPtr *a, int n, int* flag) {
    std::cerr << "############## DEBUG START ######################" << std::endl;
    for(int i = 0; i < n; i++) {
        if (a[i] == nullptr) {
            std::cerr << "r[" << i << "] = . " << std::endl;
        } else {
            std::cerr << "r[" << i << "] = " << a[i] << std::endl;
        }
    }
    std::cerr << "flag(" << *flag << ", " << (((bool)*flag)?"true":"false") << ")" << std::endl;
    std::cerr << "############## DEBUG END ########################" << std::endl;
};

// DEBUG 
inline void debug_registers(void* v0, void* v1, void* v2, void* r0, void* r1, void* r2, void* fp) {
    std::cerr << "############## REGISTERS ########################" << std::endl;
    std::cerr << "V0 " << v0 << std::endl;
    std::cerr << "V1 " << v1 << std::endl;
    std::cerr << "V2 " << v2 << std::endl;
    std::cerr << "R0 " << r0 << std::endl;
    std::cerr << "R1 " << r1 << std::endl;
    std::cerr << "R2 " << r2 << std::endl;
    std::cerr << "FP " << fp << std::endl;
    std::cerr << "############## REGISTERS ########################" << std::endl;
};

// OP_NIL x,  x := null
inline void op_nil(VM *vm, VMObjectPtr *a, int x) {
    TRACE_JIT(std::cerr << "OP_NIL r" << x << std::endl);
    a[x] = nullptr;
};

// OP_MOV x y, x := y
inline void op_mov(VM *vm, VMObjectPtr *a, int x, int y) {
    TRACE_JIT(std::cerr << "OP_MOV r" << x << ", r" << y << std::endl);
    a[x] = a[y];
};

// OP_DATA x i32, x := data(i32)
inline void op_data(VM* vm, VMObjectPtr *a, int x, int i) {
    TRACE_JIT(std::cerr << "OP_DATA r" << x << ", i" << i << std::endl);
    a[x] = vm->get_data(i);
};

// OP_ARRAY x y z, x := [ y, y+1,.., z ]
inline void op_array(VM* vm, VMObjectPtr *a, int x, int y, int z) {
    TRACE_JIT(std::cerr << "OP_ARRAY r" << x << ", r" << y << ", r" << z << std::endl);
  //  if (y <= z) {
        auto n = (z - y) + 1;
        auto a0 = VMObjectArray::create(n);
        auto a1 = VMObjectArray::cast(a0);
        for (int i = 0; i < n; i++) {
            a1->set(i, a[y+i]);
        }
        a[x] = a1;
  // }
};

// OP_SET x y z, x[val(y)] = z
inline void op_set(VM* vm, VMObjectPtr* a, int x, int  y, int z) {
    TRACE_JIT(std::cerr << "OP_MOV r" << x << ", r" << y << ", r" << z << std::endl);
    int n = VMObjectInteger::cast(a[y])->value();
    auto a0 = VMObjectArray::cast(a[x]);
    a0->set(n, a[z]);
};

// OP_TAKEX x y z i16, x,..,y = z[i],..,z[i+y-x], ~flag if fail
inline void op_takex(VM* vm, VMObjectPtr* a, int x, int y, int z, int i, void**flag) {
    TRACE_JIT(std::cerr << "OP_TAKEX r" << x << ", r" << y << ", r" << z << ", i" << i << std::endl);
    int n = (y - x) + 1;
    auto a0 = VMObjectArray::cast(a[z]);
    if (((int)a0->size()) < i + n) {
        *flag = (void*) false;
    } else {
        *flag = (void*) true;
        for (int j = 0; j < n; j++) {
            a[x+j] = a0->get(i+j);
        }
    }
};

// OP_SPLIT x y z, x,..,y = z[0],..,z[y-x], flag if not exact
inline void op_split(VM* vm, VMObjectPtr* a, int x, int y, int z, void** flag) {
    TRACE_JIT(std::cerr << "OP_SPLIT r" << x << ", r" << y << ", r" << z << std::endl);
    auto n = (y - x) + 1;
    auto a0 = VMObjectArray::cast(a[z]);
    if (((int)a0->size()) != n) {
        *flag = (void*) false;
    } else {
        *flag = (void*) true;
        for (int i = 0; i < n; i++) {
            a[x+i] = a0->get(i);
        }
    }
};

// OP_CONCATX x y z i16, x := y ++ drop i z
inline void op_concatx(VM* vm, VMObjectPtr *a, int x, int y, int z, int n) {
    TRACE_JIT(std::cerr << "OP_CONCATX r" << x << ", r" << y << ", r" << z << ", i" << n << std::endl);

/*
    if (a[z] == nullptr) {
        a[x] = a[y];
        return;
    }
*/
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
inline void op_test(VM* vm, VMObjectPtr *a, int x, int y, void** flag) {
    TRACE_JIT(std::cerr << "OP_TEST r" << x << ", r" << y << std::endl);
    EqualVMObjectPtr equals;
    bool b = equals(a[x], a[y]);
    TRACE_JIT(std::cerr << ((b)?"true":"false") << std::endl);
    *flag = (void*) b; // cast to word size
};

// OP_TAG x y, flag := (x->tag() == y)
inline void op_tag(VM* vm, VMObjectPtr *a, int x, int y, void** flag) {
    TRACE_JIT(std::cerr << "OP_TAG r" << x << ", r" << y << std::endl);
    auto s0 = (a[x])->symbol(); // XXX, wut?
    auto s1 = (a[y])->symbol();
    bool b = ( s0 == s1 );
    *flag = (void*) b; // cast to word size
};

// OP_RETURN x
inline void op_return(VM*, VMObjectPtr *a, int x, VMObjectPtr *ret) {
    TRACE_JIT(std::cerr << "OP_RETURN r" << x << std::endl);
    //*ret = a[x];
    ret[0] = a[x];
};

}; // extern "C"

namespace egel {

using namespace egel;

class BytecodePass {
public:
    BytecodePass(VM *m, const Code &c)
        : _code(c), _pc(0), _machine(m) {
    }

    BytecodePass(VM* m, const VMObjectPtr& o) {
        ASSERT(m->is_bytecode(o));
        auto b = VMObjectBytecode::cast(o);
        _machine = m;
        _code = b->code();
        _pc = 0;
    }

    VM *machine() {
        return _machine;
    }

    void reset() {
        _pc = 0;
    }

    uint32_t pc() const {
        return _pc;
    }

    bool is_end() const {
        return _pc >= _code.size();
    }

    opcode_t look() const {
        return (opcode_t)_code[_pc];
    }

    uint8_t fetch_i8() {
        uint8_t n = _code[_pc];
        _pc += 1;
        return n;
    }

    uint16_t fetch_i16() {
        uint16_t n = ((_code[_pc] << 8) | _code[_pc + 1]);
        _pc += 2;
        return n;
    }

    uint32_t fetch_i32() {  // XXX: depends on size of machine register?
        uint32_t n = ((_code[_pc] << 24) | (_code[_pc + 1] << 16) |
                      (_code[_pc + 2] << 8) | _code[_pc + 3]);
        _pc += 4;
        return n;
    }

    opcode_t fetch_op() {
        return (opcode_t)fetch_i8();
    }

    reg_t fetch_index() {
        return fetch_i16();
    }

    reg_t fetch_register() {
        return fetch_i16();
    }

    label_t fetch_label() {
        return fetch_i32();
    }

    virtual void op_nil(uint32_t pc, reg_t x) {
    }

    virtual void op_mov(uint32_t pc, reg_t x, reg_t y) {
    }

    virtual void op_data(uint32_t pc, reg_t x, uint32_t d) {
    }

    virtual void op_set(uint32_t pc, reg_t x, reg_t y, reg_t z) {
    }

    virtual void op_split(uint32_t pc, reg_t x, reg_t y, reg_t z) {
    }

    virtual void op_array(uint32_t pc, reg_t x, reg_t y, reg_t z) {
    }

    virtual void op_takex(uint32_t pc, reg_t x, reg_t y, reg_t z, uint16_t i) {
    }

    virtual void op_concatx(uint32_t pc, reg_t x, reg_t y, reg_t z, uint16_t i) {
    }

    virtual void op_test(uint32_t pc, reg_t x, reg_t y) {
    }

    virtual void op_tag(uint32_t pc, reg_t x, reg_t y) {
    }

    virtual void op_fail(uint32_t pc, label_t l) {
    }

    virtual void op_return(uint32_t pc, reg_t x) {
    }

    void pass() {
        reset();

        while (!is_end()) {
            switch (look()) {
                case OP_NIL: {
                    auto p = pc();
                    fetch_op();
                    auto x = fetch_register();
                    op_nil(p, x);
                    }
                    break;
                case OP_MOV: {
                    auto p = pc();
                    fetch_op();
                    auto x = fetch_register();
                    auto y = fetch_register();
                    op_mov(p,x, y);
                    }
                    break;
                case OP_DATA: {
                    auto p = pc();
                    fetch_op();
                    auto x = fetch_register();
                    auto i = fetch_i32();
                    op_data(p, x, i);
                    }
                    break;
                case OP_SET: {
                    auto p = pc();
                    fetch_op();
                    auto x = fetch_register();
                    auto y = fetch_register();
                    auto z = fetch_register();
                    op_set(p, x, y, z);
                    }
                    break;
                case OP_SPLIT: {
                    auto p = pc();
                    fetch_op();
                    auto x = fetch_register();
                    auto y = fetch_register();
                    auto z = fetch_register();
                    op_split(p, x, y, z);
                    }
                    break;
                case OP_ARRAY: {
                    auto p = pc();
                    fetch_op();
                    auto x = fetch_register();
                    auto y = fetch_register();
                    auto z = fetch_register();
                    op_array(p, x, y, z);
                    }
                    break;
                case OP_TAKEX: {
                    auto p = pc();
                    fetch_op();
                    auto x = fetch_register();
                    auto y = fetch_register();
                    auto z = fetch_register();
                    auto i = fetch_index();
                    op_takex(p, x,y,z,i);
                    }
                    break;
                case OP_CONCATX: {
                    auto p = pc();
                    fetch_op();
                    auto x = fetch_register();
                    auto y = fetch_register();
                    auto z = fetch_register();
                    auto i = fetch_index();
                    op_concatx(p, x,y,z,i);
                    }
                    break;
                case OP_TEST: {
                    auto p = pc();
                    fetch_op();
                    auto x = fetch_register();
                    auto y = fetch_register();
                    op_test(p, x,y);
                    }
                    break;
                case OP_TAG: {
                    auto p = pc();
                    fetch_op();
                    auto x = fetch_register();
                    auto y = fetch_register();
                    op_tag(p, x,y);
                    }
                    break;
                case OP_FAIL: {
                    auto p = pc();
                    fetch_op();
                    auto l = fetch_label();
                    op_fail(p, l);
                    }
                    break;
                case OP_RETURN: {
                    auto p = pc();
                    fetch_op();
                    auto x = fetch_register();
                    op_return(p, x);
                    }
                    break;
            }
        }
    }
private:
    Code _code;
    uint32_t _pc;
    VM* _machine;
};

// for orthogonality reasons
class DataPass {
public:
    DataPass(VM *m, const Data &d)
        : _data(d), _machine(m) {
    }

    DataPass(VM* m, const VMObjectPtr& o) {
        ASSERT(m->is_bytecode(o));
        auto b = VMObjectBytecode::cast(o);
        _machine = m;
        _data = b->data();
    }

    VM *machine() {
        return _machine;
    }

    size_t size() {
        return _data.size();
    }

    virtual void data_entry(uint32_t key, uint32_t value) {
    }

    void pass() {
        for (size_t i = 0; i < _data.size(); i++) {
            data_entry(i, _data[i]);
        }
    }
private:
    Data _data;
    VM*  _machine;
};

// derive number of registers, labels
class AnalyzeBytecode: public BytecodePass {
public:
    AnalyzeBytecode(VM* m, const VMObjectPtr &o) : BytecodePass(m, o), _max(0) {
    }

    void max(reg_t x) {
        if (_max < x) _max = x;
    }

    reg_t register_count() {
        return _max + 1;
    }

    std::set<label_t> labels() {
        return _labels;
    }

    virtual void op_nil(uint32_t pc, reg_t x) override {
        max(x);
    }

    virtual void op_mov(uint32_t pc, reg_t x, reg_t y) override {
        max(x);
    }

    virtual void op_data(uint32_t pc, reg_t x, uint32_t d) override {
        max(x);
    }

    virtual void op_set(uint32_t pc, reg_t x, reg_t y, reg_t z) override {
        max(x);
    }

    virtual void op_split(uint32_t pc, reg_t x, reg_t y, reg_t z) override {
        max(x);
        max(y);
    }

    virtual void op_array(uint32_t pc, reg_t x, reg_t y, reg_t z) override {
        max(x);
    }

    virtual void op_takex(uint32_t pc, reg_t x, reg_t y, reg_t z, uint16_t i) override {
        max(x);
        max(y);
    }

    virtual void op_concatx(uint32_t pc, reg_t x, reg_t y, reg_t z, uint16_t i) override {
        max(x);
    }

    virtual void op_test(uint32_t pc, reg_t x, reg_t y) override {
    }

    virtual void op_tag(uint32_t pc, reg_t x, reg_t y) override {
    }

    virtual void op_fail(uint32_t pc, label_t l) override {
        _labels.insert(l);
    }

    virtual void op_return(uint32_t pc, reg_t x) override {
        max(x);
    }

private:
    reg_t _max;
    std::set<label_t> _labels;
};

// map locals to globals
class PushData: public DataPass {
public:
    PushData(VM* m, const VMObjectPtr &o) : DataPass(m, o) {
    }

    data_t get_data(uint32_t entry) {
        return _map[entry];
    }

    virtual void data_entry(uint32_t key, uint32_t value) override {
        _map[key] = value;
    }

private:
    std::map<int, int>  _map;
};

class EmitNative: public BytecodePass {
public:
    EmitNative(VM* m, const VMObjectPtr &o) : BytecodePass(m, o), _proc(nullptr), _data(PushData(m, o)), _analyzebytecode(m,o) {
    }

    void* get_procedure() {
        return _proc;
    }

    void forward_labels() {
        // for every bytecode label declare and store a forward label
        auto ll = _analyzebytecode.labels();
        for (auto &l : ll) {
            auto l0 = jit_forward();
            _labels[(int)l] = l0;
        }
    }

    void emit_label(uint32_t pc) {
        auto ll = _analyzebytecode.labels();
        if (ll.count(pc) > 0) {
        //TRACE_JIT(std::cerr << "emitted label for " << (void*)pc << std::endl);
            jit_link(_labels[(int)pc]);
        }
    }

    virtual void op_nil(uint32_t pc, reg_t x) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_V0);  // VM*
        jit_pushargr(JIT_V1);  // registers
        jit_pushargi((int) x); // x
        jit_finishi((void*)::op_nil);
    }

    virtual void op_mov(uint32_t pc, reg_t x, reg_t y) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_V0);  // VM*
        jit_pushargr(JIT_V1);  // registers
        jit_pushargi((int) x); // x
        jit_pushargi((int) y); // y
        jit_finishi((void*)::op_mov);
    }

    virtual void op_data(uint32_t pc, reg_t x, uint32_t d) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_V0);  // VM*
        jit_pushargr(JIT_V1);  // registers
        jit_pushargi((int) x); // x
        auto g = _data.get_data(d); // get the global
        jit_pushargi((int) g); // d
        jit_finishi((void*)::op_data);
    }

    virtual void op_set(uint32_t pc, reg_t x, reg_t y, reg_t z) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_V0);  // VM*
        jit_pushargr(JIT_V1);  // registers
        jit_pushargi((int) x); // x
        jit_pushargi((int) y); // y
        jit_pushargi((int) z); // z
        jit_finishi((void*)::op_set);
    }

    virtual void op_split(uint32_t pc, reg_t x, reg_t y, reg_t z) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_V0);  // VM*
        jit_pushargr(JIT_V1);  // registers
        jit_pushargi((int) x); // x
        jit_pushargi((int) y); // y
        jit_pushargi((int) z); // z
        jit_pushargr(JIT_V2);  // flag
        jit_finishi((void*)::op_split);
    }

    virtual void op_array(uint32_t pc, reg_t x, reg_t y, reg_t z) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_V0);  // VM*
        jit_pushargr(JIT_V1);  // registers
        jit_pushargi((int) x); // x
        jit_pushargi((int) y); // y
        jit_pushargi((int) z); // z
        jit_finishi((void*)::op_array);
    }

    virtual void op_takex(uint32_t pc, reg_t x, reg_t y, reg_t z, uint16_t i) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_V0);  // VM*
        jit_pushargr(JIT_V1);  // registers
        jit_pushargi((int) x); // x
        jit_pushargi((int) y); // y
        jit_pushargi((int) z); // z
        jit_pushargi((int) i); // i
        jit_pushargr(JIT_V2);  // flag
        jit_finishi((void*)::op_takex);
    }

    virtual void op_concatx(uint32_t pc, reg_t x, reg_t y, reg_t z, uint16_t i) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_V0);  // VM*
        jit_pushargr(JIT_V1);  // registers
        jit_pushargi((int) x); // x
        jit_pushargi((int) y); // y
        jit_pushargi((int) z); // z
        jit_pushargi((int) i); // i
        jit_finishi((void*)::op_concatx);
    }

    virtual void op_test(uint32_t pc, reg_t x, reg_t y) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_V0);  // VM*
        jit_pushargr(JIT_V1);  // registers
        jit_pushargi((int) x); // x
        jit_pushargi((int) y); // y
        jit_pushargr(JIT_V2);  // flag
        jit_finishi((void*)::op_test);
    }

    virtual void op_tag(uint32_t pc, reg_t x, reg_t y) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_V0);  // VM*
        jit_pushargr(JIT_V1);  // registers
        jit_pushargi((int) x); // x
        jit_pushargi((int) y); // y
        jit_pushargr(JIT_V2);  // flag
        jit_finishi((void*)::op_tag);
    }

    virtual void op_fail(uint32_t pc, label_t l) override {
        emit_label(pc);
        jit_ldr(JIT_R0, JIT_V2); // load flag to R0
        auto j = jit_beqi(JIT_R0, (int) false); // branch on false
        jit_patch_at(j, _labels[(int)l]);
    }

    virtual void op_return(uint32_t pc, reg_t x) override {
        emit_label(pc);
        jit_addi(JIT_R0, JIT_FP, _return_offset);
        jit_ldr(JIT_R1, JIT_R0);
        jit_prepare();
        jit_pushargr(JIT_V0);  // VM*
        jit_pushargr(JIT_V1);  // registers
        jit_pushargi((int) x); // x
        jit_pushargr(JIT_R1);  // return
        jit_finishi((void*)::op_return);

        auto j = jit_jmpi(); // branch on true
        jit_patch_at(j, _cleanup);
    }

    void emit_marker() {
        jit_prepare();
        jit_finishi((void*)marker);
    }

    void emit_debug() {
        jit_prepare();
        jit_pushargr(JIT_V0);  // VM*
        jit_pushargr(JIT_V1);  // registers
        jit_pushargi(_reg_n);  // reg count
        jit_pushargr(JIT_V2);  // flag
        jit_finishi((void*)::debug);
    }

    void emit_debug_registers() {
        jit_prepare();
        jit_pushargr(JIT_V0);  // V0
        jit_pushargr(JIT_V1);  // V1
        jit_pushargr(JIT_V2);  // V2
        jit_pushargr(JIT_R0);  // R0
        jit_pushargr(JIT_R1);  // R1
        jit_pushargr(JIT_R2);  // R2
        jit_pushargr(JIT_FP);  // FP
        jit_finishi((void*)::debug_registers);
    }

    void* emit() {
        _analyzebytecode.pass();
        _data.pass();

        static bool initialized = false; // XXX: not thread safe
        if (!initialized) {
            initialized = true;
            init_jit(nullptr);
        }

        _jit = jit_new_state();
        jit_prolog();

        // get the arguments
        // V0 = VM*
        auto a0 = jit_arg();
        jit_getarg(JIT_V0, a0);
        // V1 = thunk
        auto a1 = jit_arg();
        jit_getarg(JIT_V1, a1);
        // V2 = return
        auto a2 = jit_arg();
        jit_getarg(JIT_V2, a2);
        _reg_n = _analyzebytecode.register_count();

        // reserve space in the stack for registers and flag
        _regs_offset   = jit_allocai(_reg_n * sizeof(VMObjectPtr));
        _flag_offset   = jit_allocai(sizeof(void*)); // stores a bool but use word size
        _return_offset = jit_allocai(sizeof(VMObjectPtr*));

        // store return (free V2)
        jit_addi(JIT_R0, JIT_FP, _return_offset);
        jit_str(JIT_R0, JIT_V2);

        // set up registers
        jit_addi(JIT_R0, JIT_FP, _regs_offset);
        jit_prepare();
        jit_pushargr(JIT_R0);
        jit_pushargi(_reg_n);
        jit_finishi((void*)vm_object_ptr_construct_n);

        // set register 0 to the thunk (free V1)
        jit_addi(JIT_R0, JIT_FP, _regs_offset);
        jit_prepare();
        jit_pushargr(JIT_V0); // VM
        jit_pushargr(JIT_R0); // registers
        jit_pushargr(JIT_V1); // thunk
        jit_finishi((void*)load);


        // V0 = VM*, V1 = registers, V2 = flag
        jit_addi(JIT_V1, JIT_FP, _regs_offset);
        jit_addi(JIT_V2, JIT_FP, _flag_offset);

        // create forward labels for jump
        forward_labels();

        _cleanup = jit_forward();
        TRACE_JIT(emit_debug());

        pass();

        // destroy registers
        jit_link(_cleanup);
        TRACE_JIT(emit_debug());
        jit_prepare();
        jit_pushargr(JIT_V1);
        jit_pushargi(_reg_n);
        jit_finishi((void*)vm_object_ptr_destruct_n);

        // jit_ret();
        jit_epilog();

        // emit the native code
        _proc = jit_emit();

        // clean up
        jit_clear_state();
        // jit_destroy_state(); // this destroys the jit and all emitted code
        // finish_jit(); // we don't call this

        return _proc;
    }

private:
    void* _proc;
    jit_state* _jit;
    std::map<int, jit_node_t*> _labels;
    PushData _data;
    AnalyzeBytecode _analyzebytecode;

    int _reg_n = 0;
    int _regs_offset   = 0;
    int _flag_offset   = 0;
    int _return_offset = 0;

    jit_node_t* _cleanup;
};

class VMObjectLightning : public VMObjectBytecode {
public:
    VMObjectLightning(VM *m, const Code &c, const Data &d, const icu::UnicodeString &n, void* p) :
    VMObjectBytecode(m, c, d, n), _proc(p) {
    }

    static VMObjectPtr create(VM *m, const Code &c, const Data &d, const icu::UnicodeString &n, void* p) {
        return std::make_shared<VMObjectLightning>(m, c, d, n, p);
    }

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override { 
        VMObjectPtr r;
        // VM, thunk, result
        TRACE_JIT(std::cerr << "native call " << to_text() << " on " << _proc << std::endl);

        void(*f)(VM*, VMObjectPtr*, VMObjectPtr*) = nullptr;;
        reinterpret_cast<void*&>(f) = _proc; // undefined behavior

        auto tt = const_cast<VMObjectPtr&>(thunk); // lose the const specifier

        TRACE_JIT(std::cerr << "arguments: vm(" << (void*)machine() << ") thunk(" << (void*)&tt << ") return(" << (void*)&r << ")" << std::endl);
        f(machine(), &tt, &r);

        TRACE_JIT(std::cerr << "call returned: " << r << std::endl);
        return r;

    };

private:
    void* _proc;
};

inline void try_compile(VM* m,const VMObjectPtr& o) {
    if (m->is_bytecode(o)) {
        TRACE_JIT(std::cerr << "compiling " << o->to_text() << std::endl);

        auto e = EmitNative(m, o);
        auto p = e.emit();

        auto b = VMObjectBytecode::cast(o);
        auto l = VMObjectLightning::create(m, b->code(), b->data(), b->to_text(), p);

        m->overwrite(l);
    }
};

inline void emit_jit(VM* m, std::vector<VMObjectPtr> oo) {
    for (auto& o: oo) {
        try_compile(m, o);
    }
};

}
