#ifndef BYTECODE_HPP
#define BYTECODE_HPP

#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>
#include <tuple>
#include "runtime.hpp"

class VMObjectBytecode;

typedef uint16_t    reg_t;
typedef uint16_t    index_t;
typedef uint32_t    label_t;

/*----------------------------------------------------------------------
name            args	    description
----------------------------------------------------------------------*/
typedef enum {
OP_NIL,     //  x           x := null
OP_MOV,     //  x y         x := y
OP_DATA,    //  x i32       x := data(i32)
OP_SET,     //  x y z       x[val(y)] = z
OP_TAKEX,   //  x y z i16   x,..,y = z[i],..,z[i+y-x], ~flag if fail
OP_SPLIT,   //  x y z       x,..,y = z[0],..,z[y-x], flag if not exact
OP_ARRAY,   //  x y z       x := [ y, y+1,.., z ]
OP_CONCATX, //  x y z i16   x := y ++ drop i z
OP_TEST,    //  x y         flag := (x == y)
OP_TAG,     //  x y         flag := (x, or x[0], == y)
OP_FAIL,    //  l           pc := l, if flag
OP_RETURN,  //  x           return x
} opcode_t;

typedef std::vector<uint8_t>        Code;
typedef std::map<label_t, uint32_t> Labels;

#define OP_SIZE         1
#define OP_REG_SIZE     (sizeof(reg_t))
#define OP_INT_SIZE     (sizeof(uint32_t))
#define OP_LABEL_SIZE   (sizeof(label_t))
#define OP_INDEX_SIZE   (sizeof(index_t))

typedef struct {
    opcode_t    op;
    const char* text;
} opcode_text_t;

#define  STRING_OP_NIL      "nil"
#define  STRING_OP_MOV      "mov"
#define  STRING_OP_DATA     "data"
#define  STRING_OP_SET      "set"
#define  STRING_OP_TAKEX    "takex"
#define  STRING_OP_SPLIT    "split"
#define  STRING_OP_ARRAY    "array"
#define  STRING_OP_CONCATX  "concatx"
#define  STRING_OP_TEST     "test"
#define  STRING_OP_TAG      "tag"
#define  STRING_OP_FAIL     "fail"
#define  STRING_OP_RETURN   "return"

#define VM_OBJECT_BYTECODE_CAST(o) std::static_pointer_cast<VMObjectBytecode>(o)

class CodePrinter {
public:
    CodePrinter(const Code& code)
        : _code(code), _pc(0) {
    }


    const char* opcode_to_text(const opcode_t op) {

    static constexpr opcode_text_t opcode_text_table[] {
        { OP_NIL, STRING_OP_NIL, },
        { OP_MOV, STRING_OP_MOV, },
        { OP_DATA, STRING_OP_DATA, },
        { OP_SET, STRING_OP_SET, },
        { OP_TAKEX, STRING_OP_TAKEX, },
        { OP_SPLIT, STRING_OP_SPLIT, },
        { OP_ARRAY, STRING_OP_ARRAY, },
        { OP_CONCATX, STRING_OP_CONCATX, },
        { OP_TEST, STRING_OP_TEST, },
        { OP_TAG, STRING_OP_TAG, },
        { OP_FAIL, STRING_OP_FAIL, },
        { OP_RETURN, STRING_OP_RETURN, },
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
        return (opcode_t) _code[_pc];
    }

    uint8_t fetch_i8() {
        uint8_t n = _code[_pc];
        _pc += 1;
        return n;
    }

    uint16_t fetch_i16() {
        uint16_t n = ( (_code[_pc] << 8) | _code[_pc+1] );
        _pc += 2;
        return n;
    }

    uint32_t fetch_i32() { // XXX: depends on size of machine register?
        uint32_t n = ( (_code[_pc] << 24) | (_code[_pc+1] << 16) | (_code[_pc+2] << 8) |  _code[_pc+3] );
        _pc += 4;
        return n;
    }

    opcode_t fetch_op() {
        return (opcode_t) fetch_i8();
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

    void write_space(std::ostream& os) {
        os << ' ';
    }

    void write_newline(std::ostream& os) {
        os << std::endl;
    }

    void write_op(std::ostream& os, const opcode_t op) {
        os << opcode_to_text(op);
    }

    void write_i32(std::ostream& os, const uint32_t n) {
        os << n;
    }

    void write_0xi32(std::ostream& os, const uint32_t n) {
        os << std::hex << std::setw(6) << n << std::dec;
    }

    void write_index(std::ostream& os, const index_t i) {
        os << 'i' << i;
    }

    void write_register(std::ostream& os, const reg_t i) {
        os << 'r' << i;
    }

    void write_label(std::ostream& os, const label_t l) {
        write_0xi32(os, (uint32_t) l);
    }

    void write(std::ostream& os, VM* vm) {

        std::ios_base::fmtflags old_flags = os.flags();
        std::streamsize         old_prec  = os.precision();
        char                    old_fill  = os.fill();

        os  << std::showbase
            << std::internal
            << std::setfill('0');

        reset();

        while (!is_end()) {
            write_0xi32(os, pc());
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
                    os << "   ; " << vm->get_data(i);
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

        os.flags(old_flags);
        os.precision(old_prec);
        os.fill(old_fill);
    }

private:
    Code        _code;
    uint32_t    _pc;
};

typedef std::tuple<uint16_t, VMObjectPtr>  data_tuple_t;
typedef std::vector<data_tuple_t>	   data_vector_t;

class Coder {
public:
    Coder(): 
        _code(Code()),
        _label_counter(0),
        _register_counter(0),
        _index_counter(0),
        _labels(Labels()) {
    }

    Code code() {
        // can only be called once
        relabel();
        return _code;
    }

    void reset() {
        _code = Code();
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
        _code.push_back( (n >> 8) & 0xFF );
        _code.push_back( n & 0xFF );
    }

    void emit_i32(uint32_t n) {
        _code.push_back( (n >> 24) & 0xFF );
        _code.push_back( (n >> 16) & 0xFF );
        _code.push_back( (n >> 8) & 0xFF );
        _code.push_back( n & 0xFF );
    }

    void emit_op(const opcode_t op) {
        emit_i8(op);
    }

    void emit_reg(const reg_t r) {
        emit_i16(r);
    }

    void emit_lbl(const label_t l) {
        emit_i32(l);
    }

    void emit_idx(const index_t i) {
        emit_i16((uint16_t) i);
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

    void emit_op_takex(const reg_t x, const reg_t y, const reg_t z, const index_t i) {
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

    void emit_op_concatx(const reg_t x, const reg_t y, const reg_t z, const reg_t i) {
        emit_op(OP_CONCATX);
        emit_reg(x);
        emit_reg(y);
        emit_reg(z);
        emit_reg(i);
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

    void relabel() {

        uint32_t    pc = 0;

        while (pc < _code.size()) {
            switch (_code[pc]) {
            case OP_NIL:
                pc += OP_SIZE + 1*OP_REG_SIZE;
                break;
            case OP_MOV:
                pc += OP_SIZE + 2*OP_REG_SIZE;
                break;
            case OP_DATA:
                pc += OP_SIZE + 1*OP_REG_SIZE + OP_INT_SIZE;
                break;
            case OP_SET:
                pc += OP_SIZE + 3*OP_REG_SIZE;
                break;
            case OP_TAKEX:
            case OP_CONCATX:
                pc += OP_SIZE + 3*OP_REG_SIZE + OP_INDEX_SIZE;
                break;
            case OP_SPLIT:
            case OP_ARRAY:
                pc += OP_SIZE + 3*OP_REG_SIZE;
                break;
            case OP_TEST:
            case OP_TAG:
                pc += OP_SIZE + 2*OP_REG_SIZE;
                break;
            case OP_FAIL: {
                pc += OP_SIZE;
                uint32_t l0 = ( (_code[pc] << 24) | (_code[pc+1] << 16) | (_code[pc+2] << 8) |  _code[pc+3] );
                uint32_t l1 = _labels[l0];
                _code[pc+0] = ( (l1 >> 24) & 0xFF );
                _code[pc+1] = ( (l1 >> 16) & 0xFF );
                _code[pc+2] = ( (l1 >> 8) & 0xFF );
                _code[pc+3] = ( l1 & 0xFF );
                pc += OP_LABEL_SIZE;
                }
                break;
            case OP_RETURN:
                pc += OP_SIZE + 1*OP_REG_SIZE;
                break;
            }
        }
    }

private:
    Code        _code;
    int         _label_counter;
    int         _register_counter;
    int         _index_counter;
    Labels      _labels;
};

#define FETCH_i8(c, pc)     c[pc]; pc += 1
#define FETCH_i16(c, pc)    ( (c[pc] << 8) | c[pc+1] ); pc += 2
#define FETCH_i32(c, pc)    ( (c[pc] << 24) | (c[pc+1] << 16) | (c[pc+2] << 8) |  c[pc+3] ); pc += 4

#define LOOK_op(c, pc)      c[pc++]
#define FETCH_op(c, pc)     FETCH_i8(c, pc)
#define FETCH_reg(c,pc)     FETCH_i16(c, pc)
#define FETCH_idx(c,pc)     FETCH_i16(c, pc)
#define FETCH_lbl(c,pc)     FETCH_i32(c, pc)

class Registers {
public:
    Registers() {
        //_fast.reserve(MAX_REGISTERS);
    }

    const VMObjectPtr get(const reg_t n) {
        if (n < MAX_REGISTERS) {
            return _fast[n];
        } else {
            return _fallback[n];
        }
    }

    void set(const reg_t n, const VMObjectPtr& o) {
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

    //std::vector<VMObjectPtr>        _fast;
    VMObjectPtr                     _fast[MAX_REGISTERS];
    std::map<reg_t, VMObjectPtr>    _fallback;
};


class VMObjectBytecode: public VMObjectCombinator {
public:

    VMObjectBytecode(VM* m, const Code& c, const symbol_t s)
        : VMObjectCombinator(VM_SUB_BYTECODE, m, s), _code(c) {
    };
    
    VMObjectBytecode(VM* m, const Code& c, const icu::UnicodeString& n)
        : VMObjectCombinator(VM_SUB_BYTECODE, m, n), _code(c) {
    };
    
    VMObjectBytecode(VM* m, const Code& c, const icu::UnicodeString& n0, const icu::UnicodeString& n1)
        : VMObjectCombinator(VM_SUB_BYTECODE, m, n0, n1), _code(c) {
    };
    
    VMObjectBytecode(VM* m, const Code& c, const UnicodeStrings& nn, const icu::UnicodeString& n)
        : VMObjectCombinator(VM_SUB_BYTECODE, m, nn, n), _code(c) {
    };
    
    VMObjectBytecode(const VMObjectBytecode& d)
        : VMObjectBytecode(d.machine(), d.code(), d.symbol()) {
    }
    
    VMObjectPtr clone() const override {
        return VMObjectPtr(new VMObjectBytecode(*this));
    }

    void debug(std::ostream& os) const override {
        os << text() << " (" << tag() << ", " << subtag() << ")" << std::endl << "begin" << std::endl;
        CodePrinter cp(_code);
        cp.write(os, machine());
        os << "end" << std::endl;
    }

    Code code() const {
        return _code;
    }

    icu::UnicodeString disassemble() const {
        std::stringstream ss;
        ss << std::hex;

        int sz = _code.size();
        for( int i = 0; i < sz; ++i ) {
            ss << std::setw(2) << std::setfill('0') << (int)_code[i];
        }

        icu::UnicodeString u(ss.str().c_str());
        return u;
    }

    void assemble(const icu::UnicodeString& hex) {
        uint8_t byte;

        auto len = hex.extract(0, 10000, nullptr, (uint32_t) 0); // XXX: I hate constants
        auto buffer = new char[len+1];
        hex.extract(0, 10000, buffer, len+1);

        for(int j = 0; j < len / 2; j++) {
            sscanf(buffer + j * 2, "%02hhX", &byte);
            _code.push_back(byte);
        }

        delete[] buffer;
    }

    VMObjectPtr reduce(const VMObjectPtr& thunk) const override {
        Registers  reg;

        uint32_t pc = 0;
        reg.set(0, thunk);
        bool flag = false;

        EqualVMObjectPtr equals;

        while (true) {

#ifdef DEBUG
            // code for debuggin
            std::cout << "eval: ";
            render(std::cout);
            std::cout << std::endl;
            for (reg_t n = 0; n < 128; n++) {
                if (reg[n] != nullptr) {
                    std::cout << n << "\t" << reg[n] << std::endl;
                }
            }
#endif

            switch (LOOK_op(_code, pc)) {
            case OP_NIL: {
                //  x           x := null
                reg_t       x = FETCH_reg(_code,pc);

                reg.set(x, nullptr);

                }
                break;
            case OP_MOV: {
                //  x y         x := y
                reg_t       x = FETCH_reg(_code,pc);
                reg_t       y = FETCH_reg(_code,pc);

                reg.set(x, reg[y]);

                }
                break;
            case OP_DATA: {
                //  x i32       x := data(i32)
                reg_t       x = FETCH_reg(_code,pc);
                int32_t     i32 = FETCH_i32(_code,pc);

                reg.set(x, machine()->get_data(i32));

                }
                break;
            case OP_SET: {
                //  x y z       x[val(y)] := z
                reg_t       x = FETCH_reg(_code,pc);
                reg_t       y = FETCH_reg(_code,pc);
                reg_t       z = FETCH_reg(_code,pc);

                auto x0 = reg[x];
                auto y0 = reg[y];
                auto z0 = reg[z];

                ASSERT(x0->tag() == VM_OBJECT_ARRAY);
                ASSERT(y0->tag() == VM_OBJECT_INTEGER);

                auto xv = VM_OBJECT_ARRAY_CAST(x0);
                auto yv = VM_OBJECT_INTEGER_VALUE(y0);

                xv->set(yv, z0);

                }
                break;
            case OP_TAKEX: {
                //  x y z i     x,..,y = z[i],..,z[i+y-x], flag fail
                reg_t       x = FETCH_reg(_code,pc);
                reg_t       y = FETCH_reg(_code,pc);
                reg_t       z = FETCH_reg(_code,pc);
                index_t     i = FETCH_idx(_code,pc);

                auto z0 = reg[z];
                if (z0->tag() == VM_OBJECT_ARRAY) {
                    auto zz = VM_OBJECT_ARRAY_CAST(z0);
                    flag = (( (int) y - (int) x + 1) <= (int) zz->size() - (int) i);
                    if (flag) {
                        for (reg_t n = x; n <= y; n++) {
                            reg.set(n, zz->get(n-x+i));
                        }
                    } else {
                        // XXX: insert prefetch code here once
                    }
                } else {
                    flag = false;
                }

                }
                break;
            case OP_SPLIT: {
                //  x y z       x,..,y = z[0],..,z[y-x], flag not exact
                reg_t       x = FETCH_reg(_code,pc);
                reg_t       y = FETCH_reg(_code,pc);
                reg_t       z = FETCH_reg(_code,pc);

                auto z0 = reg[z];
                if (z0->tag() == VM_OBJECT_ARRAY) {
                    auto zz = VM_OBJECT_ARRAY_CAST(z0);
                    flag = (( (int) y - (int) x + 1) == (int) zz->size() );
                    if (flag) {
                        for (reg_t n = x; n <= y; n++) {
                            reg.set(n, zz->get(n-x));
                        }
                    } else {
                        // XXX: insert prefetch code here once
                    }
                } else {
                    flag = false;
                }

                }
                break;
            case OP_ARRAY: {
                //  x y z       x := [ y, y+1,.., z ]
                reg_t       x = FETCH_reg(_code,pc);
                reg_t       y = FETCH_reg(_code,pc);
                reg_t       z = FETCH_reg(_code,pc);

                auto xx0 = VMObjectArray::create(1+z-y);
                auto xx1 = VM_OBJECT_ARRAY_CAST(xx0);
                for (reg_t n = y; n <= z; n++) {
                //    xx1->push_back(reg[n]);
                    xx1->set(n-y, reg[n]);
                }
                reg.set(x, xx1);

                }
                break;
            case OP_CONCATX: {
                //  x y z i     x := y ++ drop i z
                reg_t       x = FETCH_reg(_code,pc);
                reg_t       y = FETCH_reg(_code,pc);
                reg_t       z = FETCH_reg(_code,pc);
                index_t     i = FETCH_idx(_code,pc);

                auto y0 = reg[y];
                auto z0 = reg[z];
                if ((y0->tag() == VM_OBJECT_ARRAY) &&
                    (z0->tag() == VM_OBJECT_ARRAY) ) {
                    auto yc = VM_OBJECT_ARRAY_CAST(y0);
                    auto zc = VM_OBJECT_ARRAY_CAST(z0);

                    if ( i < zc->size()) { // XXX: rewrite to use the cast
                        auto yy = VM_OBJECT_ARRAY_VALUE(y0);
                        auto zz = VM_OBJECT_ARRAY_VALUE(z0);

                        auto xx = VMObjectPtrs();

                        for (auto& y1:yy) xx.push_back(y1);
                        for (int n = (int) i; n < (int) zz.size(); n++) xx.push_back(zz[n]);

                        if (xx.size() == 1) { // XXX: move to reg.set?
                            reg.set(x, xx[0]);
                        } else {
                            reg.set(x, machine()->create_array(xx));
                        }
                    } else { // optimize for `drop i z = {}` case
                        if (yc->size() == 1) { // XXX: move to reg.set?
                            reg.set(x, yc->get(0));
                        } else {
                            reg.set(x, y0);
                        }
                    }
                } else {
                    PANIC("two arrays expected");
                    return nullptr;
                }

                }
                break;
            case OP_TEST: {
                //  x y         flag := (x == y)
                reg_t       x = FETCH_reg(_code,pc);
                reg_t       y = FETCH_reg(_code,pc);

                auto x0 = reg[x];
                auto y0 = reg[y];
                flag = equals(x0, y0);

                }
                break;
            case OP_TAG: {
                //  x y         flag := (x, or x[0], == y)
                reg_t       x = FETCH_reg(_code,pc);
                reg_t       y = FETCH_reg(_code,pc);

                auto x0 = reg[x];
                auto y0 = reg[y];

                auto s0 = x0->symbol();
                auto s1 = y0->symbol();

                flag = (s0 == s1);

                }
                break;
            case OP_FAIL: {
                //  l           pc := l, if ~flag
                label_t     l = FETCH_lbl(_code,pc);

                if (!flag) pc = l;
                flag = false;

                }
                break;
            case OP_RETURN: {
                //  x           return x
                reg_t       x = FETCH_reg(_code,pc);

                return reg[x];
                }
                break;
            }
        }
    }
private:
    Code  _code;
};

class Disassembler {
public:
    Disassembler(const VMObjectPtr& o)
        : _object(o), _pc(0) {
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
        return (opcode_t) _code[_pc];
    }

    uint8_t fetch_i8() {
        uint8_t n = _code[_pc];
        _pc += 1;
        return n;
    }

    uint16_t fetch_i16() {
        uint16_t n = ( (_code[_pc] << 8) | _code[_pc+1] );
        _pc += 2;
        return n;
    }

    uint32_t fetch_i32() { // XXX: depends on size of machine register?
        uint32_t n = ( (_code[_pc] << 24) | (_code[_pc+1] << 16) | (_code[_pc+2] << 8) |  _code[_pc+3] );
        _pc += 4;
        return n;
    }

    opcode_t fetch_op() {
        return (opcode_t) fetch_i8();
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

    void switch_hex(std::ostream& os) {
        os << std::hex << std::noshowbase << std::setfill('0') << std::nouppercase;
    }

    void switch_dec(std::ostream& os) {
	os << std::dec;
    }

    void write_i8(std::ostream& os, const uint32_t n) {
	switch_hex(os);
        os << std::setw(2) << (0xff & n);
        switch_dec(os);
    }

    void write_i16(std::ostream& os, const uint32_t n) {
	switch_hex(os);
        os << std::setw(4) << (0xffff & n);
        switch_dec(os);
    }

    void write_i32(std::ostream& os, const uint32_t n) {
	switch_hex(os);
        os << std::setw(8) << n;
        switch_dec(os);
    }

    void write_op(std::ostream& os, const opcode_t n) {
	write_i8(os, n);
    }

    void write_index(std::ostream& os, const reg_t n) {
	write_i16(os, n);
    }

    void write_register(std::ostream& os, const reg_t n) {
	write_i16(os, n);
    }

    void write_label(std::ostream& os, const label_t n) {
	write_i32(os, n);
    }

    void write_text(std::ostream& os, const icu::UnicodeString& s) {
	os << s;
    }

    void write_separator(std::ostream& os) {
	os << "#";
    }

    void data_push(const reg_t i, const VMObjectPtr& o) {
	_data.push_back(data_tuple_t(i, o));
    }

    bool data_end() const {
	return ( (unsigned int) _data_index >= (unsigned int) _data.size());
    }

    void data_skip() {
	_data_index++;
    }

    reg_t data_index() const {
	return std::get<0>(_data[_data_index]);
    }

    VMObjectPtr data_object() const {
	return std::get<1>(_data[_data_index]);
    }

    void write_code(std::ostream& os, const Code& code, VM* vm) {
	_code       = code;
	_data       = data_vector_t();
        _data_index = 0;	
	_pc         = 0;

        std::ios_base::fmtflags old_flags = os.flags();
        std::streamsize         old_prec  = os.precision();
        char                    old_fill  = os.fill();

        os  << std::showbase << std::internal << std::setfill('0');

        reset();
        while (!is_end()) {
            switch (look()) {
            case OP_NIL:
                write_op(os, fetch_op());
                write_register(os, fetch_register());
                break;
            case OP_MOV:
                write_op(os, fetch_op());
                write_register(os, fetch_register());
                write_register(os, fetch_register());
                break;
            case OP_DATA:
                write_op(os, fetch_op());
                write_register(os, fetch_register());
                {
                    auto i = fetch_i32();
                    write_i32(os, i);
                    data_push(i, vm->get_data(i));
                }
                break;
            case OP_SET:
            case OP_SPLIT:
            case OP_ARRAY:
                write_op(os, fetch_op());
                write_register(os, fetch_register());
                write_register(os, fetch_register());
                write_register(os, fetch_register());
                break;
            case OP_TAKEX:
            case OP_CONCATX:
                write_op(os, fetch_op());
                write_register(os, fetch_register());
                write_register(os, fetch_register());
                write_register(os, fetch_register());
                write_index(os, fetch_index());
                break;
            case OP_TEST:
            case OP_TAG:
                write_op(os, fetch_op());
                write_register(os, fetch_register());
                write_register(os, fetch_register());
                break;
            case OP_FAIL:
                write_op(os, fetch_op());
                write_label(os, fetch_label());
                break;
            case OP_RETURN:
                write_op(os, fetch_op());
                write_register(os, fetch_register());
                break;
            }
        }

        os.flags(old_flags);
        os.precision(old_prec);
        os.fill(old_fill);

	while(!data_end()) {
            write_separator(os);
	    write_i32(os, data_index());
            write_separator(os);
	    write_text(os, data_object()->to_text());
	    data_skip();
	}
    }

    void disassemble(std::ostream& os) {
	auto o = _object;
	switch (_object->tag()) {
        case VM_OBJECT_INTEGER:
            write_i8(os, VM_OBJECT_INTEGER);
		    write_separator(os);
		    write_text(os, o->to_text());
		    break;
        case VM_OBJECT_FLOAT:
            write_i8(os, VM_OBJECT_FLOAT);
            write_separator(os);
            write_text(os, o->to_text());
            break;
        case VM_OBJECT_CHAR:
            write_i8(os, VM_OBJECT_CHAR);
            write_separator(os);
            write_text(os, o->to_text());
            break;
        case VM_OBJECT_TEXT:
            write_i8(os, VM_OBJECT_TEXT);
            write_separator(os);
            write_text(os, o->to_text());
            break;
        case VM_OBJECT_OPAQUE:
            write_i8(os, VM_OBJECT_OPAQUE);
            write_separator(os);
            write_text(os, "<opaque>"); // XXX
		    break;
        case VM_OBJECT_COMBINATOR: {
		    auto c = VM_OBJECT_COMBINATOR_CAST(o);
		    switch (c->subtag()) {
	        case VM_SUB_DATA:
                write_i8(os, VM_OBJECT_COMBINATOR);
		        write_separator(os);
                write_i16(os, VM_SUB_DATA);
		        write_separator(os);
		        write_text(os, c->to_text());
		        break;
	         case VM_SUB_BYTECODE: {
		        auto b = VM_OBJECT_BYTECODE_CAST(c);
                write_i8(os, VM_OBJECT_COMBINATOR);
		        write_separator(os);
                write_i16(os, VM_SUB_BYTECODE);
		        write_separator(os);
		        write_text(os, b->to_text());
		        write_separator(os);
		        write_code(os, b->code(), b->machine());
                }
		        break;
		    default:
		        break;
		    }
		    return;
		    break;
            }
        case VM_OBJECT_ARRAY: {
            write_i8(os, VM_OBJECT_ARRAY);
		    auto aa = VM_OBJECT_ARRAY_CAST(o);
		    for (int n = 0; n < aa->size(); n++) {
		        write_separator(os);
		        write_text(os, aa->get(n)->to_text());
		    }
		    break;
	        }
            default:
		    break;
        }
    }

    icu::UnicodeString disassemble() {
        std::stringstream ss;
	disassemble(ss);
        icu::UnicodeString u(ss.str().c_str());
        return u;
    }

private:
    VMObjectPtr   _object;
    Code          _code;
    uint32_t      _pc;
    data_vector_t _data;
    int		  _data_index;
};


#endif
