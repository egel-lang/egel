#include "../../src/runtime.hpp"

#include "channel.hpp"

#include <stdlib.h>
#include <ostream>
#include <fstream>
#include <memory>
#include <exception>

/**
 * Egel's primitive input/output combinators.
 *
 * Unstable and not all combinators fully provide what their 
 * specification promises. Neither are all combinators implemented
 * according to spec.
 *
 * The spec was somewhat shamelessly adapted from ocaml's primitive
 * IO.
 * See https://ocaml.org/.
 **/

// IO.channel
// Values which are input/output streams
class ChannelValue: public Opaque {
public:
    OPAQUE_PREAMBLE(ChannelValue, "IO", "channel");

    ChannelValue(const ChannelValue& chan): Opaque(chan.machine(), chan.symbol()) {
        _value = chan.value();
    }

    VMObjectPtr clone() const override {
        return VMObjectPtr(new ChannelValue(*this));
    }

    int compare(const VMObjectPtr& o) {
        auto v = (std::static_pointer_cast<ChannelValue>(o))->value();
        if (_value < v) return -1;
        else if (v < _value) return 1;
        else return 0;
    }

    void set_value(ChannelPtr cp) {
        _value = cp;
    }

    ChannelPtr value() const {
        return _value;
    }

protected:
    ChannelPtr _value;
};

#define CHANNEL_TEST(o, sym) \
    ((o->tag() == VM_OBJECT_OPAQUE) && \
     (VM_OBJECT_OPAQUE_SYMBOL(o) == sym))
#define CHANNEL_VALUE(o) \
    ((std::static_pointer_cast<ChannelValue>(o))->value())

// IO.cin
// Standard input.
class Stdin: public Medadic {
public:
    MEDADIC_PREAMBLE(Stdin, "IO", "stdin");

    VMObjectPtr apply() const override {
        auto cin = ChannelStreamIn::create();
        auto in  = ChannelValue(machine());
        in.set_value(cin);
        return in.clone();
    }
};

// IO.stdout
// Standard output.
class Stdout: public Medadic {
public:
    MEDADIC_PREAMBLE(Stdout, "IO", "stdout");

    VMObjectPtr apply() const override {
        auto cout = ChannelStreamOut::create();
        auto out  = ChannelValue(machine());
        out.set_value(cout);
        return out.clone();
    }
};

// IO.stderr
// Standard error.
class Stderr: public Medadic {
public:
    MEDADIC_PREAMBLE(Stderr, "IO", "stderr");

    VMObjectPtr apply() const override {
        auto cerr = ChannelStreamOut::create();
        auto err  = ChannelValue(machine());
        err.set_value(cerr);
        return err.clone();
    }
};


// IO.exit n
// Flush all pending writes on stdout and stderr, and terminate the 
// process and return the status code to the operating system.
// (0 to indicate no errors, a small positive integer for failure.)

class Exit: public Monadic {
public:
    MONADIC_PREAMBLE(Exit, "IO", "exit");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_INTEGER) {
            auto i = VM_OBJECT_INTEGER_VALUE(arg0);
            // XXX: uh.. flushall, or something?
            exit(i);
            // play nice
            return machine()->get_data_string("System", "nop");
        } else {
            return nullptr;
        }
    }
};

// IO.print o0 .. on
// Print objects on standard output; don't escape characters or 
// strings when they are the argument. May recursively print large
// objects leading to stack explosion.
class Print: public Variadic {
public:
    VARIADIC_PREAMBLE(Print, "IO", "print");

    VMObjectPtr apply(const VMObjectPtrs& args) const override {

        static VMObjectPtr nop = nullptr;
        if (nop == nullptr) nop = machine()->get_data_string("System", "nop");

        UnicodeString s;
        for (auto& arg:args) {
            if (arg->tag() == VM_OBJECT_INTEGER) {
                s += arg->to_text();
            } else if (arg->tag() == VM_OBJECT_FLOAT) {
                s += arg->to_text();
            } else if (arg->tag() == VM_OBJECT_CHAR) {
                s += VM_OBJECT_CHAR_VALUE(arg);
            } else if (arg->tag() == VM_OBJECT_TEXT) {
                s += VM_OBJECT_TEXT_VALUE(arg);
            } else {
                return nullptr;
            }
        }
        std::cout << s;

        return nop;
    }
};

/* Input functions on standard input */

// IO.getline
// Read characters from standard input
// until a newline character is encountered. Return the string of all
// characters read, without the newline character at the end.

class Getline: public Medadic {
public:
    MEDADIC_PREAMBLE(Getline, "IO", "getline");

    VMObjectPtr apply() const override {
        std::string line;
        std::getline(std::cin, line);
        UnicodeString str(line.c_str());
        return VMObjectText(str).clone();
    }
};


/*
// IO.getint
// Read one line from standard input and convert it to an integer. 

class Getint: public Medadic {
public:
    MEDADIC_PREAMBLE(Getint, "IO", "getint");

    VMObjectPtr apply() const override {
        vm_int_t n;
        std::cin >> n;
        return VMObjectInteger(n).clone();
    }
};

// IO.getfloat
// Read one line from standard input and convert it to a 
// floating-point number. The result is unspecified if the line read 
// is not a valid representation of a floating-point number.

class Getfloat: public Medadic {
public:
    MEDADIC_PREAMBLE(Getfloat, "IO", "getfloat");

    VMObjectPtr apply() const override {
        vm_float_t f;
        std::cin >> f;
        return VMObjectFloat(f).clone();
    }
};
*/

/* File channel creation and destruction */

// IO.open s
// Open the named file, and return a new channel on
// that file, positionned at the beginning of the file. The file is
// truncated to zero length if it already exists. It is created if it
// does not already exists. 

class Open: public Monadic {
public:
    MONADIC_PREAMBLE(Open, "IO", "open");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto fn = VM_OBJECT_TEXT_VALUE(arg0);
            auto stream = ChannelFile::create(fn);
            auto channel  = ChannelValue(machine());
            channel.set_value(stream);
            return channel.clone();
        } else {
            return nullptr;
        }
    }
};

// IO.close channel
// Close the given channel. Anything can happen if any of the
// functions above is called on a closed channel.

class Close: public Monadic {
public:
    MONADIC_PREAMBLE(Close, "IO", "close");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("IO", "channel");

        static VMObjectPtr nop = nullptr;
        if (nop == nullptr) nop = machine()->get_data_string("System", "nop");

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            chan->close();
            return nop;
        } else {
            return nullptr;
        }
    }
};

// IO.flush channel
// Flush the buffer associated with the given output channel, 
// performing all pending writes on that channel. Interactive programs
// must be careful about flushing std_out and std_err at the right time.

class Flush: public Monadic {
public:
    MONADIC_PREAMBLE(Flush, "IO", "flush");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("IO", "channel");

        static VMObjectPtr nop = nullptr;
        if (nop == nullptr) nop = machine()->get_data_string("System", "nop");

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            chan->flush();
            return nop;
        } else {
            return nullptr;
        }
    }
};

// IO.eof channel
// True if there is no more input, false otherwise.

class Eof: public Monadic {
public:
    MONADIC_PREAMBLE(Eof, "IO", "eof");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        static VMObjectPtr _true = nullptr;
        if (_true == nullptr) _true = machine()->get_data_string("System", "true");

        static VMObjectPtr _false = nullptr;
        if (_false == nullptr) _false = machine()->get_data_string("System", "false");

        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("IO", "channel");

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            if (chan->eof()) {
                return _true;
            } else {
                return _false;
            }
        } else {
            return nullptr;
        }
    }
};

// IO.write chan o
// Write the primitive object on the given output channel. Do not escape
// characters or strings. May recursively write an object leading to 
// stack explosion.

class Write: public Dyadic {
public:
    DYADIC_PREAMBLE(Write, "IO", "write");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("IO", "channel");

        static VMObjectPtr nop = nullptr;
        if (nop == nullptr) nop = machine()->get_data_string("System", "nop");

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            if (arg1->tag() == VM_OBJECT_INTEGER) {
                auto i = VM_OBJECT_INTEGER_VALUE(arg1);
                chan->write(i);
                return nop;
            } else if (arg1->tag() == VM_OBJECT_FLOAT) {
                auto f = VM_OBJECT_FLOAT_VALUE(arg1);
                chan->write(f);
                return nop;
            } else if (arg1->tag() == VM_OBJECT_CHAR) {
                auto c = VM_OBJECT_CHAR_VALUE(arg1);
                chan->write(c);
                return nop;
            } else if (arg1->tag() == VM_OBJECT_TEXT) {
                auto s = VM_OBJECT_TEXT_VALUE(arg1);
                chan->write(s);
                return nop;
            } else {
                return nullptr;
            }
            chan->flush();
            return nop;
        } else {
            return nullptr;
        }
    }
};

/*
class Readint: public Monadic {
public:
    MONADIC_PREAMBLE(Readint, "IO", "readint");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("IO", "channel");

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            try {
                auto n = chan->readint();
                return VMObjectInteger(n).clone();
            } catch (exception e) {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }
};

class Readfloat: public Monadic {
public:
    MONADIC_PREAMBLE(Readfloat, "IO", "readfloat");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("IO", "channel");

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            try {
                auto f = chan->readfloat();
                return VMObjectFloat(f).clone();
            } catch (exception e) {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }
};
*/

class Readline: public Monadic {
public:
    MONADIC_PREAMBLE(Readline, "IO", "readline");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("IO", "channel");

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            try {
                std::string line;
                std::getline(std::cin, line);
                UnicodeString str(line.c_str());
                return VMObjectText(str).clone();
            } catch (std::exception e) {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }
};

// IO.output_byte channel i
// Write one 8-bit integer (as the single character with that code) on
// the given output channel. The given integer is taken modulo 256.

// IO.seek_out channel i
// seek_out chan pos sets the current writing position to pos for
// channel chan. This works only for regular files. On files of other
// kinds (such as terminals, pipes and sockets), the behavior is
// unspecified.

// IO.pos_out channel
// Return the current writing position for the given channel.


// IO.out_channel_length channel
// Return the total length (number of characters) of the given
// channel. This works only for regular files. On files of other
// kinds, the result is meaningless.

/* General input functions */

// IO.open_in s
// Open the named file for reading, and return a new input channel on
// that file, positionned at the beginning of the file. Raise
// sys__Sys_error if the file could not be opened.

// IO.open_in_bin s
// Same as open_in, but the file is opened in binary mode, so that no
// translation takes place during reads. On operating systems that do
// not distinguish between text mode and binary mode, this function
// behaves like open_in.

// IO.open_in_gen flags i s
// open_in_gen mode rights filename opens the file named filename for
// reading, as above. The extra arguments mode and rights specify the
// opening mode and file permissions (see sys__open). open_in and
// open_in_bin are special cases of this function.

// IO.open_descriptor_in i
// open_descriptor_in fd returns a buffered input channel reading from
// the file descriptor fd. The file descriptor fd must have been
// previously opened for reading, else the behavior is undefined.

// IO.input_byte channel
// Same as input_char, but return the 8-bit integer representing the
// character. Raise End_of_file if an end of file was reached.

// IO.seek_in channel n
// seek_in chan pos sets the current reading position to pos for
// channel chan. This works only for regular files. On files of other
// kinds, the behavior is unspecified.

// IO.pos_in channel
// Return the current reading position for the given channel.

// IO.in_channel_length channel
// Return the total length (number of characters) of the given
// channel. This works only for regular files. On files of other
// kinds, the result is meaningless.

extern "C" std::vector<UnicodeString> egel_imports() {
    return std::vector<UnicodeString>();
}

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(VMObjectData(vm, "IO", "channel").clone());

    oo.push_back(ChannelValue(vm).clone());
    oo.push_back(Stdin(vm).clone());
    oo.push_back(Stdout(vm).clone());
    oo.push_back(Stderr(vm).clone());
    oo.push_back(Exit(vm).clone());
    oo.push_back(Print(vm).clone());
    oo.push_back(Getline(vm).clone());
    oo.push_back(Open(vm).clone());
    oo.push_back(Close(vm).clone());
    oo.push_back(Flush(vm).clone());
    oo.push_back(Eof(vm).clone());
    oo.push_back(Write(vm).clone());
    oo.push_back(Readline(vm).clone());

    return oo;
}
