#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include "utils.hpp"
#include "runtime.hpp"
#include "bytecode.hpp"

const int BYTECODEVERSION = 0x00;
const int SEPARATOR       = ' ';

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

    void switch_char(std::ostream& os) {
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

    void write_char(std::ostream& os, char c) {
	    os.put(c);
    }

    void write_separator(std::ostream& os) {
	    os.put(SEPARATOR);
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
            switch (data_object()->tag()) {
                case VM_OBJECT_INTEGER: {
                    write_char(os, 'i');
                    write_separator(os);
                    write_text(os, data_object()->to_text());
                }
                break;
                case VM_OBJECT_FLOAT: {
                    write_char(os, 'f');
                    write_separator(os);
                    write_text(os, data_object()->to_text());
                }
                break;
                case VM_OBJECT_CHAR: {
                    write_char(os, 'c');
                    write_separator(os);
                    write_text(os, data_object()->to_text());
                }
                break;
                case VM_OBJECT_TEXT: {
                    write_char(os, 't');
                    write_separator(os);
                    write_text(os, data_object()->to_text());
                }
                break;
                case VM_OBJECT_COMBINATOR: {
                    write_char(os, 'o');
                    write_separator(os);
                    write_text(os, vm->symbol(data_object())); // output the combinator not {} or ()
                }
                break;
                default:
                PANIC("cannot dis object")
                break;
            }
            data_skip();
        }
    }

    void disassemble(std::ostream& os) {
	    auto o = _object;
        if (o->subtag_test(VM_SUB_BYTECODE)) {
            auto b = VMObjectBytecode::cast(o);
            write_i8(os, BYTECODEVERSION);
		    write_separator(os);
		    write_text(os, b->to_text());
		    write_separator(os);
		    write_code(os, b->code(), b->machine());
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
    VMObjectPtr   _object;
    Code          _code;
    uint32_t      _pc;
    data_vector_t _data;
    int		      _data_index;
};


class Assembler {
public:
    Assembler(VM* vm, const icu::UnicodeString s): 
        _machine(vm), _source(s) {
    }

    ~Assembler() {
    }

    int look(std::istream& in) {
        return in.peek();
    }

    bool look_at(std::istream& in, int c) {
        return look(in) == c;
    }

    void skip(std::istream& in) {
        in.get();
    }

    void force(std::istream& in, int n) {
        if (look(in) == n) {
            skip(in);
        } else {
            PANIC("failed to disassemble");
        }
    }

    bool eol(std::istream& in) const {
        return in.eof();
    }

    bool is_separator(std::istream& in) {
        return look(in) == SEPARATOR;
    }

    bool is_quote(std::istream& in) {
        int c = look(in);
        return (c == '\'') || (c == '\"');
    }

    char read_char(std::istream& in) {
        int c = look(in); skip(in); return c;
    }

    int char_to_val(int c) const {
        return ((c >= '0') && (c <= '9'))? (c - '0'):
                ((c >= 'a') && (c <= 'f'))? (c - 'a'):
                ((c >= 'A') && (c <= 'F'))? (c - 'A'): c; 
    }

    int read_byte(std::istream& in) {
        if (is_separator(in)) return -1;
        int c0 = read_char(in); int c1 = read_char(in);
        return (char_to_val(c0) << 16) | char_to_val(c1);
    }

    char* read_quoted(std::istream& in) {
        skip(in); // XXX make this safe once
        size_t cap = 4096;
        size_t n = 0;
        char* buffer = (char*) malloc(cap);
        while (!eol(in) && !is_quote(in)) {
            if (n > cap - 4) {
                buffer = (char*) realloc((void*)buffer, cap+4096);
                cap += 4096;
            }
            if (look_at(in, '\\')) {
                buffer[n] = look(in); n++; skip(in);
                buffer[n] = look(in); n++; skip(in);
            } else {
                buffer[n] = look(in); n++; skip(in);
            }
        }
        skip(in);
        buffer[n] = '\0';
        return buffer;
    }

    char* read_until_separator(std::istream& in) {
        size_t cap = 4096;
        size_t n = 0;
        char* buffer = (char*) malloc(cap);
        while (!eol(in) && !is_separator(in)) {
            if (n > cap - 4) {
                buffer = (char*) realloc((void*)buffer, cap+4096);
                cap += 4096;
            } 
            buffer[n] = look(in); n++; skip(in);
        }
        buffer[n] = '\0';
        return buffer;
    }

    icu::UnicodeString read_combinator(std::istream& in) {
        auto cc = read_until_separator(in);
        auto s = char_to_unicode(cc);
        free(cc);
        return s;
    }

    VMObjectPtr assemble() {
        auto chars = unicode_to_char(_source);
        std::string s(chars);
        auto in = std::stringstream(s);
        force(in, BYTECODEVERSION);
        force(in, SEPARATOR);
        auto c = read_combinator(in);

        Code code;
        while (!is_separator(in)) {
            auto b = read_byte(in);
            code.push_back(b);
        }

        while (!eol(in) && is_separator(in)) {
// XXX
        }
        free(chars);
        return VMObjectBytecode::create(_machine, code, c);
    }

private:
    VM*                 _machine;
    icu::UnicodeString  _source;
};

#endif