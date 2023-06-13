#pragma once

#include <memory>
#include <vector>

#include "bytecode.hpp"
#include "runtime.hpp"
#include "utils.hpp"

const int BYTECODEVERSION = '0';
const int BYTECODE_TAG = 'b';
const int DATA_TAG = 'd';
const int SEPARATOR = ' ';

class Disassembler {
public:
    Disassembler(VM *m, const VMObjectPtr &o)
        : _machine(m), _object(o), _code(Code()), _data(Data()), _pc(0) {
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

    void switch_hex(std::ostream &os) {
        os << std::hex << std::noshowbase << std::setfill('0')
           << std::nouppercase;
    }

    void switch_dec(std::ostream &os) {
        os << std::dec;
    }

    void switch_char(std::ostream &os) {
        os << std::dec;
    }

    void write_int(std::ostream &os, const vm_int_t n) {
        switch_dec(os);
        os << n;
    }

    void write_i8(std::ostream &os, const uint32_t n) {
        switch_hex(os);
        os << std::setw(2) << (0xff & n);
        switch_dec(os);
    }

    void write_i16(std::ostream &os, const uint32_t n) {
        switch_hex(os);
        os << std::setw(4) << (0xffff & n);
        switch_dec(os);
    }

    void write_i32(std::ostream &os, const uint32_t n) {
        switch_hex(os);
        os << std::setw(8) << n;
        switch_dec(os);
    }

    void write_op(std::ostream &os, const opcode_t n) {
        write_i8(os, n);
    }

    void write_index(std::ostream &os, const reg_t n) {
        write_i16(os, n);
    }

    void write_register(std::ostream &os, const reg_t n) {
        write_i16(os, n);
    }

    void write_label(std::ostream &os, const label_t n) {
        write_i32(os, n);
    }

    void write_text(std::ostream &os, const icu::UnicodeString &s) {
        os << s;
    }

    void write_char(std::ostream &os, char c) {
        os.put(c);
    }

    void write_separator(std::ostream &os) {
        os.put(SEPARATOR);
    }

    void write_code(std::ostream &os, const Code &c, VM *vm) {
        _code = c;
        _pc = 0;

        std::ios_base::fmtflags old_flags = os.flags();
        std::streamsize old_prec = os.precision();
        char old_fill = os.fill();

        os << std::showbase << std::internal << std::setfill('0');

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
    }

    void write_data(std::ostream &os, const Data &d, VM *vm) {
        _data = d;
        for (unsigned int n = 0; n < _data.size(); n++) {
            write_separator(os);
            write_int(os, n);
            write_separator(os);
            auto o = vm->get_data(_data[n]);
            switch (o->tag()) {
                case VM_OBJECT_INTEGER: {
                    write_char(os, 'i');
                    write_separator(os);
                    write_text(os, o->to_text());
                } break;
                case VM_OBJECT_FLOAT: {
                    write_char(os, 'f');
                    write_separator(os);
                    write_text(os, o->to_text());
                } break;
                case VM_OBJECT_CHAR: {
                    write_char(os, 'c');
                    write_separator(os);
                    write_text(os, o->to_text());
                } break;
                case VM_OBJECT_TEXT: {
                    write_char(os, 't');
                    write_separator(os);
                    write_text(os, o->to_text());
                } break;
                case VM_OBJECT_COMBINATOR: {
                    write_char(os, 'o');
                    write_separator(os);
                    write_text(
                        os,
                        vm->symbol(o));  // output the combinator not {} or ()
                } break;
                default:
                    throw vm->create_text("weird data object");
                    break;
            }
        }
    }

    void disassemble(std::ostream &os) {
        auto o = _object;
        if (o->subtag_test(VM_SUB_BYTECODE)) {
            auto b = VMObjectBytecode::cast(o);
            write_i8(os, BYTECODEVERSION);
            write_i8(os, BYTECODE_TAG);
            write_separator(os);
            write_text(os, b->to_text());
            write_separator(os);
            write_code(os, b->code(), machine());
            write_data(os, b->data(), machine());
        } else if (o->subtag_test(VM_SUB_DATA)) {
            auto d = VMObjectData::cast(o);
            write_i8(os, BYTECODEVERSION);
            write_i8(os, DATA_TAG);
            write_separator(os);
            write_text(
                os,
                machine()->symbol(o));  // output the combinator not {} or ()
        } else {
            PANIC("disassemble failed");
        }
    }

    icu::UnicodeString disassemble() {
        std::stringstream ss;
        disassemble(ss);
        return char_to_unicode(ss.str().c_str());
    }

private:
    VM *_machine;
    VMObjectPtr _object;
    Code _code;
    Data _data;
    uint32_t _pc;
};

class Assembler {
public:
    Assembler(VM *vm, const icu::UnicodeString s) : _machine(vm), _source(s) {
    }

    ~Assembler() {
    }

    VM *machine() {
        return _machine;
    }

    int look(std::istream &in) {
        return in.peek();
    }

    bool look_at(std::istream &in, int c) {
        return look(in) == c;
    }

    void skip(std::istream &in) {
        if (!eol(in)) in.get();
    }

    void force(std::istream &in, int n) {
        if (look(in) == n) {
            skip(in);
        } else {
            throw machine()->create_text(icu::UnicodeString(": expected '") +
                                         ((char)n) + "'");
        }
    }

    bool eol(std::istream &in) {  // dude...
        return in.peek() == std::istream::traits_type::eof();
    }

    bool is_separator(std::istream &in) {
        return (look(in) == SEPARATOR);
    }

    bool is_quote(std::istream &in) {
        int c = look(in);
        return (c == '\'') || (c == '\"');
    }

    bool is_digit(std::istream &in) {
        int c = look(in);
        return (c >= '0') || (c <= '9');
    }

    char get_char(std::istream &in) {
        int c = look(in);
        skip(in);
        return c;
    }

    int char_to_val(int c) const {
        return ((c >= '0') && (c <= '9'))   ? (c - '0')
               : ((c >= 'a') && (c <= 'f')) ? (10 + c - 'a')
               : ((c >= 'A') && (c <= 'F')) ? (10 + c - 'A')
                                            : c;
    }

    int read_byte(std::istream &in) {
        int c0 = get_char(in);
        int c1 = get_char(in);
        return (char_to_val(c0) << 4) | char_to_val(c1);
    }

    void skip_white(std::istream &in) {
        while (is_separator(in)) skip(in);
    }

    char *read_quoted(std::istream &in) {
        skip(in);  // XXX make this safe once
        size_t cap = 4096;
        size_t n = 0;
        char *buffer = (char *)malloc(cap);
        while (!eol(in) && !is_quote(in)) {
            if (n > cap - 4) {
                buffer = (char *)realloc((void *)buffer, cap + 4096);
                cap += 4096;
            }
            if (look_at(in, '\\')) {
                buffer[n] = look(in);
                n++;
                skip(in);
                buffer[n] = look(in);
                n++;
                skip(in);
            } else {
                buffer[n] = look(in);
                n++;
                skip(in);
            }
        }
        skip(in);
        buffer[n] = '\0';
        return buffer;
    }

    char *read_until_separator(std::istream &in) {
        size_t cap = 4096;
        size_t n = 0;
        char *buffer = (char *)malloc(cap);
        while (!eol(in) && !is_separator(in)) {
            if (n > cap - 4) {
                buffer = (char *)realloc((void *)buffer, cap + 4096);
                cap += 4096;
            }
            buffer[n] = look(in);
            n++;
            skip(in);
        }
        buffer[n] = '\0';
        return buffer;
    }

    vm_int_t read_int(std::istream &in) {
        vm_int_t f;
        in >> f;
        return f;
    }

    vm_float_t read_float(std::istream &in) {
        vm_float_t f;
        in >> f;
        return f;
    }

    vm_char_t read_char(std::istream &in) {
        auto cc = read_until_separator(in);
        auto s = char_to_unicode(cc);
        free(cc);
        return s.char32At(0);
    }

    icu::UnicodeString read_text(std::istream &in) {
        auto cc = read_until_separator(in);
        auto s = char_to_unicode(cc);
        free(cc);
        return s;
    }

    icu::UnicodeString read_combinator(std::istream &in) {
        auto cc = read_until_separator(in);
        auto s = char_to_unicode(cc);
        free(cc);
        return s;
    }

    VMObjectPtr assemble() {
        auto chars = unicode_to_char(_source);
        std::string s(chars);
        auto in = std::stringstream(s);

        auto version = read_byte(in);
        if (version != BYTECODEVERSION)
            throw machine()->create_text("wrong bytecode version");

        auto tag = read_byte(in);
        if (tag == DATA_TAG) {
            auto t = read_combinator(in);
            return machine()->create_data(t);
        }
        // else XXX

        skip_white(in);
        auto c = read_combinator(in);

        // read in code section
        Code code;
        skip_white(in);
        while (!eol(in) && !is_separator(in)) {
            auto b = read_byte(in);
            code.push_back(b);
        }

        // read in data section
        Data data;
        skip_white(in);
        vm_int_t z;
        while (!eol(in) && is_digit(in)) {  // I hate this
            in >> z;
            if (data.size() != (unsigned long)z)
                throw machine()->create_text(
                    icu::UnicodeString("wrong data entry count ") +
                    convert_from_int(z));
            skip_white(in);
            switch (look(in)) {
                case 'i': {
                    skip(in);
                    skip(in);
                    auto n = read_int(in);
                    auto o = machine()->create_integer(n);
                    auto d = machine()->define_data(o);
                    data.push_back(d);
                } break;
                case 'f': {
                    skip(in);
                    skip(in);
                    auto f = read_float(in);
                    auto o = machine()->create_float(f);
                    auto d = machine()->define_data(o);
                    data.push_back(d);
                } break;
                case 'c': {
                    skip(in);
                    skip(in);
                    auto c = read_char(in);
                    auto o = machine()->create_char(c);
                    auto d = machine()->define_data(o);
                    data.push_back(d);
                } break;
                case 't': {
                    skip(in);
                    skip(in);
                    auto t = read_text(in);
                    auto o = machine()->create_text(t);
                    auto d = machine()->define_data(o);
                    data.push_back(d);
                } break;
                case 'o': {
                    skip(in);
                    skip(in);
                    auto t = read_combinator(in);
                    auto o = machine()->get_combinator(t);
                    auto d = machine()->define_data(o);
                    data.push_back(d);
                } break;
                default: {
                    throw machine()->create_text("panic: cannot decode data");
                } break;
            }
            skip_white(in);
        }
        free(chars);
        return VMObjectBytecode::create(_machine, code, data, c);
    }

private:
    VM *_machine;
    icu::UnicodeString _source;
};
