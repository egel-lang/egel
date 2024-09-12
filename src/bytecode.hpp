#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <tuple>
#include <vector>

#include "runtime.hpp"
#include "reader.hpp"
#include "lexical.hpp"

namespace egel {

class VMObjectBytecode;

using reg_t = uint32_t;
using index_t = uint16_t;
using label_t = uint32_t;

/*----------------------------------------------------------------------
name            args	    description
----------------------------------------------------------------------*/
enum opcode_t {
    OP_NIL,      //  x           x := null
    OP_MOV,      //  x y         x := y
    OP_DATA,     //  x i32       x := data(i32)
    OP_SET,      //  x y z       x[val(y)] = z
    OP_TAKEX,    //  x y z i16   x,..,y = z[i],..,z[i+y-x], ~flag if fail
    OP_SPLIT,    //  x y z       x,..,y = z[0],..,z[y-x], flag if not exact
    OP_ARRAY,    //  x y z       x := [ y, y+1,.., z ]
    OP_CONCATX,  //  x y z i16   x := y ++ drop i z
    OP_TEST,     //  x y         flag := (x == y)
    OP_TAG,      //  x y         flag := (x, or x[0], == y)
    OP_FAIL,     //  l           pc := l, if flag
    OP_RETURN,   //  x           return x
};

using Code = std::vector<uint8_t>;
using Data = std::vector<uint32_t>;  // XXX this is overkil after a change to a
                                     // data section

using Labels = std::map<label_t, uint32_t>;

constexpr auto OP_SIZE = 1;
constexpr auto OP_REG_SIZE = (sizeof(reg_t));
constexpr auto OP_INT_SIZE = (sizeof(uint32_t));
constexpr auto OP_LABEL_SIZE = (sizeof(label_t));
constexpr auto OP_INDEX_SIZE = (sizeof(index_t));

class Coder {
public:
    Coder(VM *m)
        : _machine(m),
          _code(Code()),
          _data(Data()),
          _label_counter(0),
          _register_counter(0),
          _index_counter(0),
          _labels(Labels()) {
    }

    VM *machine() {
        return _machine;
    }

    Code code() {
        // can only be called once
        return _code;
    }

    Data data() {
        return _data;
    }

    void reset() {
        _code = Code();
        _data = Data();
        _label_counter = 0;
        _register_counter = 0;
        _index_counter = 0;
        _labels = Labels();
    }

    label_t generate_label() {
        label_t l = _label_counter;
        _label_counter++;
        return l;
    }

    reg_t generate_index() {
        reg_t r = _index_counter;
        _index_counter++;
        return r;
    }

    reg_t generate_register() {
        reg_t r = _register_counter;
        _register_counter++;
        return r;
    }

    reg_t peek_register() const {
        return _register_counter;
    }

    void restore_register(reg_t r) {
        _register_counter = r;
    }

    // primitive byte emit
    void emit_i8(uint8_t b) {
        _code.push_back(b);
    }

    void emit_i16(uint16_t n) {
        // don't use typecasts, this is portable
        _code.push_back((n >> 8) & 0xFF);
        _code.push_back(n & 0xFF);
    }

    void emit_i32(uint32_t n) {
        _code.push_back((n >> 24) & 0xFF);
        _code.push_back((n >> 16) & 0xFF);
        _code.push_back((n >> 8) & 0xFF);
        _code.push_back(n & 0xFF);
    }

    void emit_op(const opcode_t op) {
        emit_i8(op);
    }

    void emit_reg(const reg_t r) {
        emit_i32(r);
    }

    void emit_lbl(const label_t l) {
        emit_i32(l);
    }

    void emit_idx(const index_t i) {
        emit_i16((uint16_t)i);
    }

    // bytecode emit
    void emit_op_nil(const reg_t x) {
        emit_op(OP_NIL);
        emit_reg(x);
    }

    void emit_op_mov(const reg_t x, const reg_t y) {
        emit_op(OP_MOV);
        emit_reg(x);
        emit_reg(y);
    }

    void emit_op_data(const reg_t x, const uint32_t i32) {
        emit_op(OP_DATA);
        emit_reg(x);
        emit_i32(i32);
    }

    void emit_op_set(const reg_t x, const reg_t y, const reg_t z) {
        emit_op(OP_SET);
        emit_reg(x);
        emit_reg(y);
        emit_reg(z);
    }

    void emit_op_takex(const reg_t x, const reg_t y, const reg_t z,
                       const index_t i) {
        emit_op(OP_TAKEX);
        emit_reg(x);
        emit_reg(y);
        emit_reg(z);
        emit_idx(i);
    }

    void emit_op_split(const reg_t x, const reg_t y, const reg_t z) {
        emit_op(OP_SPLIT);
        emit_reg(x);
        emit_reg(y);
        emit_reg(z);
    }

    void emit_op_array(const reg_t x, const reg_t y, const reg_t z) {
        emit_op(OP_ARRAY);
        emit_reg(x);
        emit_reg(y);
        emit_reg(z);
    }

    void emit_op_concatx(const reg_t x, const reg_t y, const reg_t z,
                         const index_t i) {
        emit_op(OP_CONCATX);
        emit_reg(x);
        emit_reg(y);
        emit_reg(z);
        emit_idx(i);
    }

    void emit_op_test(const reg_t x, const reg_t y) {
        emit_op(OP_TEST);
        emit_reg(x);
        emit_reg(y);
    }

    void emit_op_tag(const reg_t x, const reg_t y) {
        emit_op(OP_TAG);
        emit_reg(x);
        emit_reg(y);
    }

    void emit_op_fail(const label_t l) {
        emit_op(OP_FAIL);
        emit_lbl(l);
    }

    void emit_op_return(const reg_t x) {
        emit_op(OP_RETURN);
        emit_reg(x);
    }

    void emit_label(const label_t l) {
        _labels[l] = _code.size();
    }

    uint32_t emit_data(const VMObjectPtr &o) {
        auto d = machine()->enter_data(o);
        _data.push_back(d);
        // optimize once
        return _data.size() - 1;
    }

    void relabel() {
        uint32_t pc = 0;

        while (pc < _code.size()) {
            switch (_code[pc]) {
                case OP_NIL:
                    pc += OP_SIZE + 1 * OP_REG_SIZE;
                    break;
                case OP_MOV:
                    pc += OP_SIZE + 2 * OP_REG_SIZE;
                    break;
                case OP_DATA:
                    pc += OP_SIZE + 1 * OP_REG_SIZE + OP_INT_SIZE;
                    break;
                case OP_SET:
                    pc += OP_SIZE + 3 * OP_REG_SIZE;
                    break;
                case OP_TAKEX:
                case OP_CONCATX:
                    pc += OP_SIZE + 3 * OP_REG_SIZE + OP_INDEX_SIZE;
                    break;
                case OP_SPLIT:
                case OP_ARRAY:
                    pc += OP_SIZE + 3 * OP_REG_SIZE;
                    break;
                case OP_TEST:
                case OP_TAG:
                    pc += OP_SIZE + 2 * OP_REG_SIZE;
                    break;
                case OP_FAIL: {
                    pc += OP_SIZE;
                    uint32_t l0 = ((_code[pc] << 24) | (_code[pc + 1] << 16) |
                                   (_code[pc + 2] << 8) | _code[pc + 3]);
                    uint32_t l1 = _labels[l0];
                    _code[pc + 0] = ((l1 >> 24) & 0xFF);
                    _code[pc + 1] = ((l1 >> 16) & 0xFF);
                    _code[pc + 2] = ((l1 >> 8) & 0xFF);
                    _code[pc + 3] = (l1 & 0xFF);
                    pc += OP_LABEL_SIZE;
                } break;
                case OP_RETURN:
                    pc += OP_SIZE + 1 * OP_REG_SIZE;
                    break;
                default:
                    PANIC("relabel case");
                    break;
            }
        }
    }

private:
    VM *_machine;
    Code _code;
    Data _data;
    int _label_counter;
    int _register_counter;
    int _index_counter;
    Labels _labels;
};

#define FETCH_i8(c, pc) \
    c[pc];              \
    pc += 1
#define FETCH_i16(c, pc)        \
    ((c[pc] << 8) | c[pc + 1]); \
    pc += 2
#define FETCH_i32(c, pc)                                                \
    ((c[pc] << 24) | (c[pc + 1] << 16) | (c[pc + 2] << 8) | c[pc + 3]); \
    pc += 4

#define LOOK_op(c, pc) c[pc++]
#define FETCH_op(c, pc) FETCH_i8(c, pc)
#define FETCH_reg(c, pc) FETCH_i32(c, pc)
#define FETCH_idx(c, pc) FETCH_i16(c, pc)
#define FETCH_lbl(c, pc) FETCH_i32(c, pc)

class Registers {
public:
    Registers() {
    }

    const VMObjectPtr get(const reg_t n) {
        if (n < MAX_REGISTERS) {
            return _fast[n];
        } else {
            return _fallback[n];
        }
    }

    void set(const reg_t n, const VMObjectPtr &o) {
        if (n < MAX_REGISTERS) {
            _fast[n] = o;
        } else {
            _fallback[n] = o;
        }
    }

    const VMObjectPtr operator[](const reg_t n) {
        return get(n);
    }

private:
    static const int MAX_REGISTERS = 64;

    VMObjectPtr _fast[MAX_REGISTERS];
    std::map<reg_t, VMObjectPtr> _fallback;
};

// forward declaration
inline void write_assembly(std::ostream &os, const VMObjectBytecode& o);

class VMObjectBytecode : public VMObjectCombinator {
public:
    VMObjectBytecode(VM *m, const Code &c, const Data &d, const symbol_t s)
        : VMObjectCombinator(VM_SUB_BYTECODE, m, s), _code(c), _data(d) {};

    VMObjectBytecode(VM *m, const Code &c, const Data &d,
                     const icu::UnicodeString &n)
        : VMObjectCombinator(VM_SUB_BYTECODE, m, n), _code(c), _data(d) {};

    VMObjectBytecode(VM *m, const Code &c, const Data &d,
                     const icu::UnicodeString &n0, const icu::UnicodeString &n1)
        : VMObjectCombinator(VM_SUB_BYTECODE, m, n0, n1), _code(c), _data(d) {};

    VMObjectBytecode(VM *m, const Code &c, const Data &d,
                     const UnicodeStrings &nn, const icu::UnicodeString &n)
        : VMObjectCombinator(VM_SUB_BYTECODE, m, nn, n), _code(c), _data(d) {};

    VMObjectBytecode(const VMObjectBytecode &d)
        : VMObjectBytecode(d.machine(), d.code(), d.data(), d.symbol()) {
    }

    static VMObjectPtr create(VM *m, const Code &c, const Data &d,
                              const UnicodeStrings &nn,
                              const icu::UnicodeString &n) {
        return std::make_shared<VMObjectBytecode>(m, c, d, nn, n);
    }

    static VMObjectPtr create(VM *m, const Code &c, const Data &d,
                              const icu::UnicodeString &s) {
        return std::make_shared<VMObjectBytecode>(m, c, d, s);
    }

    static std::shared_ptr<VMObjectBytecode> cast(const VMObjectPtr &o) {
        return std::static_pointer_cast<VMObjectBytecode>(o);
    }

    void debug(std::ostream &os) const override {
        write_assembly(os, *this);
    }

    Code code() const {
        return _code;
    }

    Data data() const {
        return _data;
    }

    data_t get_data(uint32_t n) const {
        return _data[n];
    }

    VMObjectPtrs get_data_list() const {
        VMObjectPtrs oo;
        for (unsigned int n = 0; n < _data.size(); n++) {
            oo.push_back(machine()->get_data(_data[n]));
        }
        return oo;
    }

    VMObjectPtr reduce(const VMObjectPtr &thunk) const override {
        std::cerr << "WARNING: unjitted combinator" << std::endl;
        Registers reg;
        uint32_t pc = 0;
        reg.set(0, thunk);
        bool flag = false;

        EqualVMObjectPtr equals;

        while (true) {
            switch (LOOK_op(_code, pc)) {
                case OP_NIL: {
                    //  x           x := null
                    reg_t x = FETCH_reg(_code, pc);

                    reg.set(x, nullptr);
                } break;
                case OP_MOV: {
                    //  x y         x := y
                    reg_t x = FETCH_reg(_code, pc);
                    reg_t y = FETCH_reg(_code, pc);

                    reg.set(x, reg[y]);
                } break;
                case OP_DATA: {
                    //  x i32       x := data(i32)
                    reg_t x = FETCH_reg(_code, pc);
                    int32_t i32 = FETCH_i32(_code, pc);

                    reg.set(x, machine()->get_data(_data[i32]));
                } break;
                case OP_SET: {
                    //  x y z       x[val(y)] := z
                    reg_t x = FETCH_reg(_code, pc);
                    reg_t y = FETCH_reg(_code, pc);
                    reg_t z = FETCH_reg(_code, pc);

                    auto x0 = reg[x];
                    auto y0 = reg[y];
                    auto z0 = reg[z];

                    ASSERT(machine()->is_array(x0));
                    ASSERT(y0->tag() == VM_OBJECT_INTEGER);

                    auto xv = VMObjectArray::cast(x0);
                    auto yv = machine()->get_integer(y0);

                    xv->set(yv, z0);
                } break;
                case OP_TAKEX: {
                    //  x y z i     x,..,y = z[i],..,z[i+y-x], flag fail
                    reg_t x = FETCH_reg(_code, pc);
                    reg_t y = FETCH_reg(_code, pc);
                    reg_t z = FETCH_reg(_code, pc);
                    index_t i = FETCH_idx(_code, pc);

                    auto z0 = reg[z];
                    if (machine()->is_array(z0)) {
                        auto zz = VMObjectArray::cast(z0);
                        flag =
                            (((int)y - (int)x + 1) <= (int)zz->size() - (int)i);
                        if (flag) {
                            for (reg_t n = x; n <= y; n++) {
                                reg.set(n, zz->get(n - x + i));
                            }
                        } else {
                            // XXX: insert prefetch code here once
                        }
                    } else {
                        flag = false;
                    }
                } break;
                case OP_SPLIT: {
                    //  x y z       x,..,y = z[0],..,z[y-x], flag not exact
                    reg_t x = FETCH_reg(_code, pc);
                    reg_t y = FETCH_reg(_code, pc);
                    reg_t z = FETCH_reg(_code, pc);

                    auto z0 = reg[z];
                    if (machine()->is_array(z0)) {
                        auto zz = VMObjectArray::cast(z0);
                        flag = (((int)y - (int)x + 1) == (int)zz->size());
                        if (flag) {
                            for (reg_t n = x; n <= y; n++) {
                                reg.set(n, zz->get(n - x));
                            }
                        } else {
                            // XXX: insert prefetch code here once
                        }
                    } else {
                        flag = false;
                    }
                } break;
                case OP_ARRAY: {
                    //  x y z       x := [ y, y+1,.., z ]
                    reg_t x = FETCH_reg(_code, pc);
                    reg_t y = FETCH_reg(_code, pc);
                    reg_t z = FETCH_reg(_code, pc);

                    size_t sz = (size_t)z - y + 1;
                    if (sz > 0) {  // we do generate empty arrays sometimes
                        auto oo =
                            VMObjectArray::cast(VMObjectArray::create(sz));
                        for (reg_t n = y; n <= z; n++) {
                            oo->set(n - y, reg[n]);
                        }
                        reg.set(x, oo);
                    } else {
                        auto oo = VMObjectArray::cast(VMObjectArray::create(0));
                        reg.set(x, oo);
                    }
                } break;
                case OP_CONCATX: {
                    //  x y z i     x := y ++ drop i z
                    reg_t x = FETCH_reg(_code, pc);
                    reg_t y = FETCH_reg(_code, pc);
                    reg_t z = FETCH_reg(_code, pc);
                    index_t i = FETCH_idx(_code, pc);

                    auto y0 = reg[y];
                    auto z0 = reg[z];
                    if ((machine()->is_array(y0)) &&
                        (machine()->is_array(z0))) {
                        auto yc = VMObjectArray::cast(y0);
                        auto zc = VMObjectArray::cast(z0);

                        size_t sz = yc->size() + zc->size() - i;

                        if (i < zc->size()) {  // there are members in z to be
                                               // copied

                            if (sz > 1) {
                                auto oo = VMObjectArray::cast(
                                    VMObjectArray::create(sz));
                                size_t l = 0;
                                for (size_t n = 0; n < yc->size(); n++) {
                                    oo->set(l, yc->get(n));
                                    l++;
                                }
                                for (size_t n = i; n < zc->size(); n++) {
                                    oo->set(l, zc->get(n));
                                    l++;
                                }
                                reg.set(x, oo);
                            } else {
                                auto o = zc->get(i);
                                reg.set(x, o);
                            }
                        } else {  // optimize for `drop i z = {}` case
                            if (yc->size() == 1) {  // XXX: move to reg.set?
                                reg.set(x, yc->get(0));
                            } else {
                                reg.set(x, y0);
                            }
                        }
                    } else if (machine()->is_array(z0)) {
                        auto zc = VMObjectArray::cast(z0);
                        size_t sz = 1 + zc->size() - i;

                        if (i < zc->size()) {  // there are members in z to be
                                               // copied
                            if (sz > 1) {
                                auto oo = VMObjectArray::cast(
                                    VMObjectArray::create(sz));
                                size_t l = 0;
                                oo->set(l, y0);
                                l++;
                                for (size_t n = i; n < zc->size(); n++) {
                                    oo->set(l, zc->get(n));
                                    l++;
                                }
                                reg.set(x, oo);
                            } else {
                                auto o = zc->get(i);
                                reg.set(x, o);
                            }
                        } else {  // optimize for `drop i z = {}` case
                            reg.set(x, y0);
                        }
                    } else {
                        PANIC("two arrays expected");
                        return nullptr;
                    }
                } break;
                case OP_TEST: {
                    //  x y         flag := (x == y)
                    reg_t x = FETCH_reg(_code, pc);
                    reg_t y = FETCH_reg(_code, pc);

                    auto x0 = reg[x];
                    auto y0 = reg[y];
                    flag = equals(x0, y0);
                } break;
                case OP_TAG: {
                    //  x y         flag := (x, or x[0], == y)
                    reg_t x = FETCH_reg(_code, pc);
                    reg_t y = FETCH_reg(_code, pc);

                    auto x0 = reg[x];
                    auto y0 = reg[y];

                    auto s0 = x0->symbol();
                    auto s1 = y0->symbol();

                    flag = (s0 == s1);
                } break;
                case OP_FAIL: {
                    //  l           pc := l, if ~flag
                    label_t l = FETCH_lbl(_code, pc);

                    if (!flag) pc = l;
                    flag = false;
                } break;
                case OP_RETURN: {
                    //  x           return x
                    reg_t x = FETCH_reg(_code, pc);

                    return reg[x];
                } break;
            }
        }
    }

private:
    Code _code;
    Data _data;
};

struct opcode_text_t {
    opcode_t op;
    const char *text;
};

constexpr auto STRING_OP_NIL = "nil";
constexpr auto STRING_OP_MOV = "mov";
constexpr auto STRING_OP_DATA = "data";
constexpr auto STRING_OP_SET = "set";
constexpr auto STRING_OP_TAKEX = "takex";
constexpr auto STRING_OP_SPLIT = "split";
constexpr auto STRING_OP_ARRAY = "array";
constexpr auto STRING_OP_CONCATX = "concatx";
constexpr auto STRING_OP_TEST = "test";
constexpr auto STRING_OP_TAG = "tag";
constexpr auto STRING_OP_FAIL = "fail";
constexpr auto STRING_OP_RETURN = "return";

class Disassembler {
public:
    Disassembler(const VMObjectBytecode &o): _name(o.text()), _code(o.code()), _data(o.data()), _vm(o.machine()), _pc(0) {}

    Disassembler(const VMObjectPtr &o): Disassembler(*VMObjectBytecode::cast(o)) {
    }

    const char *opcode_to_text(const opcode_t op) {
        static constexpr opcode_text_t opcode_text_table[]{
            {
                OP_NIL,
                STRING_OP_NIL,
            },
            {
                OP_MOV,
                STRING_OP_MOV,
            },
            {
                OP_DATA,
                STRING_OP_DATA,
            },
            {
                OP_SET,
                STRING_OP_SET,
            },
            {
                OP_TAKEX,
                STRING_OP_TAKEX,
            },
            {
                OP_SPLIT,
                STRING_OP_SPLIT,
            },
            {
                OP_ARRAY,
                STRING_OP_ARRAY,
            },
            {
                OP_CONCATX,
                STRING_OP_CONCATX,
            },
            {
                OP_TEST,
                STRING_OP_TEST,
            },
            {
                OP_TAG,
                STRING_OP_TAG,
            },
            {
                OP_FAIL,
                STRING_OP_FAIL,
            },
            {
                OP_RETURN,
                STRING_OP_RETURN,
            },
        };

        for (int n = 0; n <= OP_RETURN; n++) {
            if (opcode_text_table[n].op == op) {
                return opcode_text_table[n].text;
            }
        }
        PANIC("couldn't decode opcode");
        return nullptr;
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
        return fetch_i32();
    }

    label_t fetch_label() {
        return fetch_i32();
    }

    void write_space(std::ostream &os) {
        os << ' ';
    }

    void write_newline(std::ostream &os) {
        os << std::endl;
    }

    void write_op(std::ostream &os, const opcode_t op) {
        os << opcode_to_text(op);
    }

    void write_i32(std::ostream &os, const uint32_t n) {
        os << n;
    }

    void write_0xi32(std::ostream &os, const uint32_t n) {
        os << std::hex << "0x" << n << std::dec;
    }

    void write_0xi32_filled(std::ostream &os, const uint32_t n) {
        os << std::hex << "0x" << n << std::dec;
    }
    void write_index(std::ostream &os, const index_t i) {
        os << i;
    }

    void write_register(std::ostream &os, const reg_t i) {
        os << 'r' << i;
    }

    void write_label(std::ostream &os, const label_t l) {
        write_0xi32(os, (uint32_t)l);
    }

    void write(std::ostream &os) {
        std::ios_base::fmtflags old_flags = os.flags();
        std::streamsize old_prec = os.precision();
        char old_fill = os.fill();

        // write header
        os << "bytecode 01" << std::endl;

        // write name
        os << "  " << _name << std::endl;

        // write code
        //os << std::showbase << std::internal << std::setfill('0');
        reset();
        os << "code" << std::endl;
        while (!is_end()) {
            os << "  ";
            write_0xi32_filled(os, pc());
            write_space(os);
            switch (look()) {
                case OP_NIL:
                    write_op(os, fetch_op());
                    write_space(os);
                    write_register(os, fetch_register());
                    break;
                case OP_MOV:
                    write_op(os, fetch_op());
                    write_space(os);
                    write_register(os, fetch_register());
                    write_space(os);
                    write_register(os, fetch_register());
                    break;
                case OP_DATA:
                    write_op(os, fetch_op());
                    write_space(os);
                    write_register(os, fetch_register());
                    write_space(os);
                    {
                        auto i = fetch_i32();
                        write_i32(os, i);
                        // os << "   ; " << get_data(i);
                    }
                    break;
                case OP_SET:
                case OP_SPLIT:
                case OP_ARRAY:
                    write_op(os, fetch_op());
                    write_space(os);
                    write_register(os, fetch_register());
                    write_space(os);
                    write_register(os, fetch_register());
                    write_space(os);
                    write_register(os, fetch_register());
                    break;
                case OP_TAKEX:
                case OP_CONCATX:
                    write_op(os, fetch_op());
                    write_space(os);
                    write_register(os, fetch_register());
                    write_space(os);
                    write_register(os, fetch_register());
                    write_space(os);
                    write_register(os, fetch_register());
                    write_space(os);
                    write_index(os, fetch_index());
                    break;
                case OP_TEST:
                case OP_TAG:
                    write_op(os, fetch_op());
                    write_space(os);
                    write_register(os, fetch_register());
                    write_space(os);
                    write_register(os, fetch_register());
                    break;
                case OP_FAIL:
                    write_op(os, fetch_op());
                    write_space(os);
                    write_label(os, fetch_label());
                    break;
                case OP_RETURN:
                    write_op(os, fetch_op());
                    write_space(os);
                    write_register(os, fetch_register());
                    break;
            }
            write_newline(os);
        }

        // write data
        os << "data" << std::endl;
        for (unsigned int n = 0; n < _data.size(); n++) {
            os << "  ";
            auto o = _vm->get_data(_data[n]);
            switch (o->tag()) {
                case VM_OBJECT_INTEGER:
                    os << "i ";
                    break;
                case VM_OBJECT_FLOAT:
                    os << "f ";
                    break;
                case VM_OBJECT_COMPLEX:
                    os << "z ";
                    break;
                case VM_OBJECT_CHAR:
                    os << "c ";
                    break;
                case VM_OBJECT_TEXT:
                    os << "t ";
                    break;
                case VM_OBJECT_COMBINATOR:
                    os << "o ";
                    break;
                default:
                    PANIC("error in data section");
            }
            os << n << ' ' << _data[n] << " " << o << std::endl;
        }
        // write end
        os << "end" << std::endl;

        os.flags(old_flags);
        os.precision(old_prec);
        os.fill(old_fill);
    }

    icu::UnicodeString disassemble() {                                                                                                  
        std::stringstream ss;                                                                                                           
        write(ss);                                                                                                                
        return VM::unicode_from_utf8_chars(ss.str().c_str());                                                                           
    } 
private:
    icu::UnicodeString _name;
    Code _code;
    Data _data;
    VM* _vm;
    uint32_t _pc;
};

inline void write_assembly(std::ostream &os, const VMObjectBytecode& o) {
    Disassembler d(o);
    d.write(os);
};

inline icu::UnicodeString disassemble(const VMObjectPtr &o) {
    if (o->subtag_test(VM_SUB_DATA)) {
        std::stringstream ss;                                                                                                           
        ss << "data 01 " << VMObjectData::cast(o)->raw_text() << " end" << std::endl;
        return VM::unicode_from_utf8_chars(ss.str().c_str());                                                                           
    } else if (o->subtag_test(VM_SUB_BYTECODE)) {
        Disassembler d(o);
        return d.disassemble();
    } else {
        PANIC("cannot disassemble");
        return "";
    }
};

class Assembler {
public:
    Assembler(VM *vm, const icu::UnicodeString s) : _machine(vm), _source(s) {
        StringCharReader r = StringCharReader("unknown", _source);
        _tokenreader = tokenize_from_reader(r);
        sanitize(_tokenreader);
    }

    ~Assembler() {
    }

    VM *machine() {
        return _machine;
    }

    Token look(int n = 0) {
        return _tokenreader.look(n);
    }

    token_t tag(int n = 0) {
        return look(n).tag();
    }

    icu::UnicodeString look_text(int n = 0) {
        return look(n).text();
    }

    Position position() {
        return look().position();
    }

    void skip() {
        // std::cerr << "skipped: " << look() << std::endl;
        _tokenreader.skip();
    }

    void check_token(token_t t) {
        if (tag() != t) {
            Position p = position();
            throw ErrorSyntactical(p, token_text(t) + " expected");
        };
    }

    void force_token(token_t t) {
        check_token(t);
        skip();
    }
 
    bool is_string(const icu::UnicodeString &s) {
        return s == look_text();
    }

    void force_string(const icu::UnicodeString& s) {
        if (is_string(s)) {
            skip();
        } else {
            Position p = position();
            throw ErrorSyntactical(p, s + " expected");
        }
    }

    icu::UnicodeString fetch_combinator() {
        Position p = position();
        icu::UnicodeString s;
        if ((tag() == TOKEN_UPPERCASE) || (tag() == TOKEN_LOWERCASE) || (tag() == TOKEN_OPERATOR)) {
            s += look_text();
            skip();
        } else {
            throw ErrorSyntactical(p, "combinator expected");
        }
        while (tag() == TOKEN_DCOLON) {
            s += "::";
            skip();
            if ((tag() == TOKEN_UPPERCASE) || (tag() == TOKEN_LOWERCASE) || (tag() == TOKEN_OPERATOR)) {
                s += look_text();
                skip();
            } else {
                throw ErrorSyntactical(p, "combinator expected");
            }
        }
        return s;
    }


    reg_t fetch_register() {
        auto s = look_text();
        if (s.startsWith('r')) {
            skip();
            s.removeBetween(0,1);
            return VM::unicode_to_int(s);
        } else {
            Position p = position();
            throw ErrorSyntactical(p, "register expected");
        }
    }

    uint16_t fetch_i16() {
        auto s = look_text();
        skip();
        return VM::unicode_to_int(s);
    }

    uint32_t fetch_i32() {
        auto s = look_text();
        skip();
        return VM::unicode_to_int(s);
    }

    label_t fetch_label() {
        auto s = look_text();
        skip();
        return VM::unicode_to_hexint(s);
    }

    vm_int_t fetch_integer() {
        auto s = look_text();
        skip();
        return VM::unicode_to_int(s);
    }

    vm_float_t fetch_float() {
        auto s = look_text();
        skip();
        return VM::unicode_to_float(s);
    }

    vm_complex_t fetch_complex() {
        auto s0 = look_text();
        skip();
        auto s1 = look_text();
        skip();
        if (s1.trim() == "+") {
            s1 = "+" + look_text();
            skip();
        }
        return VM::unicode_to_complex(s0+s1);
    }

    vm_char_t fetch_char() {
        auto s = look_text();
        skip();
        return VM::unicode_to_char(s);
    }

    vm_text_t fetch_text() {
        auto s = look_text();
        skip();
        return VM::unicode_to_text(s);
    }

    VMObjectPtr assemble() {
        if (is_string("data")) {
            force_string("data");
            force_string("01");
            auto name = fetch_combinator();
            force_string("end");
            return machine()->create_data(name);
        }

        force_string("bytecode");
        force_string("01");
        
        auto name = fetch_combinator();
        force_string("code");

        Coder coder(_machine);
        while(!is_string("data")) {
            skip();
            Position p = position();
            if (is_string(STRING_OP_NIL)) {
                skip();
                auto r0 = fetch_register();
                coder.emit_op_nil(r0);
            } else if (is_string(STRING_OP_MOV)) {
                skip();
                auto r0 = fetch_register();
                auto r1 = fetch_register();
                coder.emit_op_mov(r0,r1);
            } else if (is_string(STRING_OP_DATA)) {
                skip();
                auto r0 = fetch_register();
                auto i0 = fetch_i32();
                coder.emit_op_data(r0,i0);
            } else if (is_string(STRING_OP_SET)) {
                skip();
                auto r0 = fetch_register();
                auto r1 = fetch_register();
                auto r2 = fetch_register();
                coder.emit_op_set(r0,r1,r2);
            } else if (is_string(STRING_OP_TAKEX)) {
                skip();
                auto r0 = fetch_register();
                auto r1 = fetch_register();
                auto r2 = fetch_register();
                auto i0 = fetch_i16();
                coder.emit_op_takex(r0,r1,r2,i0);
            } else if (is_string(STRING_OP_SPLIT)) {
                skip();
                auto r0 = fetch_register();
                auto r1 = fetch_register();
                auto r2 = fetch_register();
                coder.emit_op_split(r0,r1,r2);
            } else if (is_string(STRING_OP_ARRAY)) {
                skip();
                auto r0 = fetch_register();
                auto r1 = fetch_register();
                auto r2 = fetch_register();
                coder.emit_op_array(r0,r1,r2);
            } else if (is_string(STRING_OP_CONCATX)) {
                skip();
                auto r0 = fetch_register();
                auto r1 = fetch_register();
                auto r2 = fetch_register();
                auto i0 = fetch_i16();
                coder.emit_op_concatx(r0,r1,r2,i0);
            } else if (is_string(STRING_OP_TEST)) {
                skip();
                auto r0 = fetch_register();
                auto r1 = fetch_register();
                coder.emit_op_test(r0,r1);
            } else if (is_string(STRING_OP_TAG)) {
                skip();
                auto r0 = fetch_register();
                auto r1 = fetch_register();
                coder.emit_op_tag(r0,r1);
            } else if (is_string(STRING_OP_FAIL)) {
                skip();
                auto l0 = fetch_label();
                coder.emit_op_fail(l0);
            } else if (is_string(STRING_OP_RETURN)) {
                skip();
                auto r0 = fetch_register();
                coder.emit_op_return(r0);
            } else {
                throw ErrorSyntactical(p, "instruction expected");
            }
        }
        auto code = coder.code();

        force_string("data");

        Data data;
        while(!is_string("end")) {
            if (is_string("i")) {
                skip();
                skip();
                skip();
                auto i = fetch_integer();
                auto o = machine()->create_integer(i);
                auto d = machine()->define_data(o);
                data.push_back(d);
            } else if (is_string("f")) {
                skip();
                skip();
                skip();
                auto f = fetch_float();
                auto o = machine()->create_float(f);
                auto d = machine()->define_data(o);
                data.push_back(d);
            } else if (is_string("z")) {
                skip();
                skip();
                skip();
                auto z = fetch_complex();
                auto o = machine()->create_complex(z);
                auto d = machine()->define_data(o);
                data.push_back(d);
            } else if (is_string("c")) {
                skip();
                skip();
                skip();
                auto c = fetch_char();
                auto o = machine()->create_char(c);
                auto d = machine()->define_data(o);
                data.push_back(d);
            } else if (is_string("t")) {
                skip();
                skip();
                skip();
                auto t = fetch_text();
                auto o = machine()->create_text(t);
                auto d = machine()->define_data(o);
                data.push_back(d);
            } else if (is_string("o")) {
                skip();
                skip();
                skip();
                auto t = fetch_combinator();
                auto o = machine()->get_combinator(t);
                auto d = machine()->define_data(o);
                data.push_back(d);
            } else {
                auto p = position();
                throw ErrorSyntactical(p, "data expected");
            }
        }

        force_string("end");
        return VMObjectBytecode::create(_machine, code, data, name);
    }

private:
    VM *_machine;
    icu::UnicodeString _source;
    Tokens _tokenreader;
};


inline VMObjectPtr assemble(VM *vm, const icu::UnicodeString &s) {
    Assembler a(vm, s);
    return a.assemble();
};

}  // namespace egel
