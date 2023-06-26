#include "instructions.hpp"

#include <egel/runtime.hpp>
#include <egel/bytecode.hpp>

#include <lightning.h>

using namespace egel;

class BytecodePass {
public:
    BytecodePass(VM *m, const Code &c)
        : _machine(m), _code(c), _pc(0) {
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
        : _machine(m), _data(d) {
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

    bool has_label(uint32_t pc) {
        return _labels.count(pc) > 0;
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

    data_t get_vm_data(uint32_t entry) {
        return _map[entry];
    }

    virtual void data_entry(uint32_t key, const VMObjectPtr& value) {
        auto entry = machine()->get_data(value);
        _map[key] = entry;
    }
private:
    std::map<int, int>  _map;
};

class EmitNative: public BytecodePass {
public:
    EmitNative(VM* m, const VMObjectPtr &o) : BytecodePass(m, o), _data(PushData(m, o)), _analyzebytecode(m,o), _proc(nullptr) {
    }

    void* get_procedure() {
        return _proc;
    }

    void emit_label(uint32_t pc) {
        if (_analyzebytecode.has_label(pc)) {
            auto l = jit_label();
            _labels[(int)pc] = l;
        }
    }

    void patch_jumps() {
        for (auto it = _jumps.begin(); it != _jumps.end(); ++it) {
            auto jump = it->first;
            auto pc = it->second;
            auto l = _labels[(int) pc];
            jit_patch_at(jump, l);
         }
    }

    virtual void op_nil(uint32_t pc, reg_t x) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_R0);  // VM*
        jit_pushargr(JIT_R1);  // registers
        jit_pushargi((int) x); // x
        jit_finishi((void*)::op_nil);
    }

    virtual void op_mov(uint32_t pc, reg_t x, reg_t y) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_R0);  // VM*
        jit_pushargr(JIT_R1);  // registers
        jit_pushargi((int) x); // x
        jit_pushargi((int) y); // y
        jit_finishi((void*)::op_mov);
    }

    virtual void op_data(uint32_t pc, reg_t x, uint32_t d) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_R0);  // VM*
        jit_pushargr(JIT_R1);  // registers
        jit_pushargi((int) x); // x
        jit_pushargi((int) d); // d
        jit_finishi((void*)::op_data);
    }

    virtual void op_set(uint32_t pc, reg_t x, reg_t y, reg_t z) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_R0);  // VM*
        jit_pushargr(JIT_R1);  // registers
        jit_pushargi((int) x); // x
        jit_pushargi((int) y); // y
        jit_pushargi((int) z); // z
        jit_finishi((void*)::op_set);
    }

    virtual void op_split(uint32_t pc, reg_t x, reg_t y, reg_t z) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_R0);  // VM*
        jit_pushargr(JIT_R1);  // registers
        jit_pushargi((int) x); // x
        jit_pushargi((int) y); // y
        jit_pushargi((int) z); // z
        jit_finishi((void*)::op_split);
    }

    virtual void op_array(uint32_t pc, reg_t x, reg_t y, reg_t z) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_R0);  // VM*
        jit_pushargr(JIT_R1);  // registers
        jit_pushargi((int) x); // x
        jit_pushargi((int) y); // y
        jit_pushargi((int) z); // z
        jit_finishi((void*)::op_array);
    }

    virtual void op_takex(uint32_t pc, reg_t x, reg_t y, reg_t z, uint16_t i) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_R0);  // VM*
        jit_pushargr(JIT_R1);  // registers
        jit_pushargi((int) x); // x
        jit_pushargi((int) y); // y
        jit_pushargi((int) z); // z
        jit_pushargi((int) i); // z
        jit_finishi((void*)::op_takex);
    }

    virtual void op_concatx(uint32_t pc, reg_t x, reg_t y, reg_t z, uint16_t i) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_R0);  // VM*
        jit_pushargr(JIT_R1);  // registers
        jit_pushargi((int) x); // x
        jit_pushargi((int) y); // y
        jit_pushargi((int) z); // z
        jit_pushargi((int) i); // i
        jit_finishi((void*)::op_concatx);
    }

    virtual void op_test(uint32_t pc, reg_t x, reg_t y) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_R0);  // VM*
        jit_pushargr(JIT_R1);  // registers
        jit_pushargi((int) x); // x
        jit_pushargi((int) y); // y
        jit_finishi((void*)::op_test);
    }

    virtual void op_tag(uint32_t pc, reg_t x, reg_t y) override {
        emit_label(pc);
        jit_prepare();
        jit_pushargr(JIT_R0);  // VM*
        jit_pushargr(JIT_R1);  // registers
        jit_pushargi((int) x); // x
        jit_pushargi((int) y); // y
        jit_finishi((void*)::op_tag);
    }

    virtual void op_fail(uint32_t pc, label_t l) override {
        emit_label(pc);
        jit_ldxi(JIT_R5, JIT_R2, 0); // load flag to R5
        auto j = jit_beqi(JIT_R5, (int) true); // branch on true
        _jumps[j] = (int) l;
    }

    virtual void op_return(uint32_t pc, reg_t x) override {
        emit_label(pc);
    }

    void emit() {
        _analyzebytecode.pass();
        _data.pass();

        static bool initialized = false; // XXX: not thread safe
        if (!initialized) {
            initialized = true;
            ::init_jit(nullptr);
        }

        _jit = jit_new_state();
        jit_prolog();

        auto regs = _analyzebytecode.register_count();

        // reserve space in the stack for registers and flag
        auto regs_offset = jit_allocai(regs * sizeof(VMObjectPtr));
        auto flag_offset = jit_allocai(sizeof(bool));


        // R1 = registers
        jit_addi(JIT_R1, JIT_FP, regs_offset);
        // R2 = flag
        jit_addi(JIT_R2, JIT_FP, flag_offset);
        // R0 = VM*
        auto a0 = jit_arg();
        jit_getarg(JIT_R0, a0);
        auto a1 = jit_arg();
        // R3 = thunk
        jit_getarg(JIT_R3, a1);
        auto a3 = jit_arg();
        // R4 = return
        jit_getarg(JIT_R4, a1);

        // set up registers
        jit_prepare();
        jit_pushargr(JIT_R1);
        jit_pushargi(regs);
        jit_finishi((void*)vm_object_ptr_construct_n);

        pass();

        // patch all jumps
        patch_jumps();

        // destroy registers
        jit_prepare();
        jit_pushargr(JIT_R1);
        jit_pushargi(regs);
        jit_finishi((void*)vm_object_ptr_destruct_n);

        jit_ret();
        jit_epilog();

        // emit the native code
        _proc = jit_emit();

        // clean up
        jit_clear_state();
        jit_destroy_state();
        // finish_jit(); // we don't call this
    }

private:
    void* _proc;
    jit_state* _jit;
    std::map<int, jit_node_t*> _labels;
    std::map<jit_node_t*, int> _jumps;
    PushData _data;
    AnalyzeBytecode _analyzebytecode;
};
