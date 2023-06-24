#include "instructions.hpp"
#include <egel/runtime>
#include <egel/bytecode>

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
                    auto pc = pc();
                    fetch_op();
                    auto x = fetch_register();
                    op_nil(pc, x);
                    }
                    break;
                case OP_MOV: {
                    auto pc = pc();
                    fetch_op();
                    auto x = fetch_register();
                    auto y = fetch_register();
                    op_mov(pc,x, y);
                    }
                    break;
                case OP_DATA: {
                    auto pc = pc();
                    fetch_op();
                    auto x = fetch_register();
                    auto i = fetch_i32();
                    op_data(pc, x, i);
                    }
                    break;
                case OP_SET: {
                    auto pc = pc();
                    fetch_op();
                    auto x = fetch_register();
                    auto y = fetch_register();
                    auto z = fetch_register();
                    op_set(pc, x, y, z);
                    }
                    break;
                case OP_SPLIT: {
                    auto pc = pc();
                    fetch_op();
                    auto x = fetch_register();
                    auto y = fetch_register();
                    auto z = fetch_register();
                    op_split(pc, x, y, z);
                    }
                    break;
                case OP_ARRAY: {
                    auto pc = pc();
                    fetch_op();
                    auto x = fetch_register();
                    auto y = fetch_register();
                    auto z = fetch_register();
                    op_array(pc, x, y, z);
                    }
                    break;
                case OP_TAKEX: {
                    auto pc = pc();
                    fetch_op();
                    auto x = fetch_register();
                    auto y = fetch_register();
                    auto z = fetch_register();
                    auto i = fetch_index();
                    op_takex(pc, x,y,z,i);
                    }
                    break;
                case OP_CONCATX: {
                    auto pc = pc();
                    fetch_op();
                    auto x = fetch_register();
                    auto y = fetch_register();
                    auto z = fetch_register();
                    auto i = fetch_index();
                    op_concatx(pc, x,y,z,i);
                    }
                    break;
                case OP_TEST: {
                    auto pc = pc();
                    fetch_op();
                    auto x = fetch_register();
                    auto y = fetch_register();
                    op_test(pc, x,y);
                    }
                    break;
                case OP_TAG: {
                    auto pc = pc();
                    fetch_op();
                    auto x = fetch_register();
                    auto y = fetch_register();
                    op_tag(pc, x,y);
                    }
                    break;
                case OP_FAIL: {
                    auto pc = pc();
                    fetch_op();
                    auto l = fetch_label();
                    op_fail(pc, x,y);
                    }
                    break;
                case OP_RETURN: {
                    auto pc = pc();
                    fetch_op();
                    auto x = fetch_register();
                    op_return(pc, x);
                    }
                    break;
            }
        }
    }
private:
    Code _code;
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

    virtual void data_entry(uint32_t key, const VMObjectPtr& value) {
    }

    void pass() {
        for (size_t i = 0; i < _data.size(); i++) {
            data_entry(i, _data[i];
        }
    }
private:
    Data _data;
};

// derive number of registers, labels
class AnalyzeBytecode: public BytecodePass {
public:
    AnalyzeBytecode(const VMObjectPtr &o) : BytecodePass(o), _max(0) {
    }

    void max(reg_t x) {
        if (_max < x) _max = x;
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
class AnalyzeData: public DataPass {
};


