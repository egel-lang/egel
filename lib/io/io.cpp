#include "../../src/runtime.hpp"

#include <stdlib.h>
#include <iostream>
#include <fstream>

#include <memory>
#include <exception>

/**
 * In C++, copy constructors for streams are usually undefined
 * which makes a simple solution of just assigning or passing
 * iostream derived classes unworkable.
 *
 * These channel classes flatten C++ i/o streams hierarchy into
 * one channel class with all operations and derived classes 
 * which implement them.
 **/

class Unsupported : public std::exception {
public:
    Unsupported() :
        _message("")  {
    }

    Unsupported(const UnicodeString &m) :
        _message(m)  {
    }

    ~Unsupported() {
    }

    UnicodeString message() const {
        return _message;
    }

    friend std::ostream & operator<<(std::ostream &os, const Unsupported &e) {
        os << "unsupported(" << e.message() << ")";
        return os;
    }

private:
    UnicodeString   _message;
};

typedef enum {
    CHANNEL_STREAM_IN,
    CHANNEL_STREAM_OUT,
    CHANNEL_STREAM_ERR,
    CHANNEL_FILE,
} channel_tag_t;

class Channel;
typedef std::shared_ptr<Channel> ChannelPtr;

class Channel {
public:
    Channel(channel_tag_t t): _tag(t) {
    }

    channel_tag_t tag() {
        return _tag;
    }

    virtual UnicodeString read() {
        throw Unsupported();
    }

    virtual void write(const UnicodeString& n) {
        throw Unsupported();
    }

    virtual void close() {
    }

    virtual void flush() {
    }

    virtual bool eof() {
        return false;
    }

protected:
    channel_tag_t   _tag;
};

class ChannelStreamIn: public Channel {
public:
    ChannelStreamIn(): Channel(CHANNEL_STREAM_IN) {
    }

    static ChannelPtr create() {
        return ChannelPtr(new ChannelStreamIn());
    }

    virtual UnicodeString read() override {
        UnicodeString s;
        std::cin >> s;
        return s;
    }

    virtual bool eof() override {
        return std::cin.eof();
    }
};

class ChannelStreamOut: public Channel {
public:
    ChannelStreamOut(): Channel(CHANNEL_STREAM_OUT) {

    }

    static ChannelPtr create() {
        return ChannelPtr(new ChannelStreamOut());
    }

    virtual void write(const UnicodeString& s) override {
        std::cout << s;
    }

    virtual void flush() override {
        std::cout.flush();
    }
};

class ChannelStreamErr: public Channel {
public:
    ChannelStreamErr(): Channel(CHANNEL_STREAM_ERR) {

    }

    static ChannelPtr create() {
        return ChannelPtr(new ChannelStreamErr());
    }

    virtual void write(const UnicodeString& s) override {
        std::cerr << s;
    }

    virtual void flush() override {
        std::cerr.flush();
    }
};

class ChannelFile: public Channel {
public:
    ChannelFile(const UnicodeString& fn): Channel(CHANNEL_FILE), _fn(fn) {
        char* bf = unicode_to_char(fn);
        _stream = std::fstream(bf);
        delete bf;
    }

    static ChannelPtr create(const UnicodeString& fn) {
        return ChannelPtr(new ChannelFile(fn));
    }

    virtual UnicodeString read() override {
        UnicodeString str;
        _stream >> str;
        return str;
    }

    virtual void write(const UnicodeString& s) override {
        _stream << s;
    }

    virtual void close() override {
        _stream.close();
    }

    virtual void flush() override {
        _stream.flush();
    }

    virtual bool eof() override {
        return _stream.eof();
    }
private:
    static char* unicode_to_char(const UnicodeString& str) {
        unsigned int buffer_size = 1024; // XXX: this is always a bad idea.
        char* buffer = new char[buffer_size];
        unsigned int size = str.extract(0, str.length(), buffer, buffer_size, "UTF-8");//XXX: null, UTF-8, or platform specific?
        buffer[size] = 0;
        return buffer;
    }

protected:
    UnicodeString   _fn;
    std::fstream    _stream;
};

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
// Values which are input/output channels
class ChannelValue: public Opaque {
public:
    OPAQUE_PREAMBLE(ChannelValue, "IO", "channel");

    ChannelValue(const ChannelValue& chan): Opaque(chan.machine(), chan.symbol()) {
        _value = chan.value();
    }

    VMObjectPtr clone() const override {
        return VMObjectPtr(new ChannelValue(*this));
    }

    int compare(const VMObjectPtr& o) override {
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
        auto cerr = ChannelStreamErr::create();
        auto err  = ChannelValue(machine());
        err.set_value(cerr);
        return err.clone();
    }
};


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

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            chan->close();
            return create_nop();
        } else {
            return nullptr;
        }
    }
};

// IO.read channel
// Read a string from a channel.

class Read: public Monadic {
public:
    MONADIC_PREAMBLE(Read, "IO", "read");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("IO", "channel");

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            UnicodeString str = chan->read();
            return create_text(str);
        } else {
            return nullptr;
        }
    }
};

// IO.write chan o
// Write the string s to channel chan.

class Write: public Dyadic {
public:
    DYADIC_PREAMBLE(Write, "IO", "write");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("IO", "channel");

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            if (arg1->tag() == VM_OBJECT_TEXT) {
                auto s = VM_OBJECT_TEXT_VALUE(arg1);
                chan->write(s);
                return create_nop();
            } else {
                return nullptr;
            }
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

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            chan->flush();
            return create_nop();
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
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("IO", "channel");

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            return create_bool(chan->eof());
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

        return create_nop();
    }
};

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
        return create_text(str);
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
            return create_nop();
        } else {
            return nullptr;
        }
    }
};

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
    oo.push_back(Open(vm).clone());
    oo.push_back(Close(vm).clone());
    oo.push_back(Read(vm).clone());
    oo.push_back(Write(vm).clone());
    oo.push_back(Flush(vm).clone());
    oo.push_back(Eof(vm).clone());
    oo.push_back(Print(vm).clone());
    oo.push_back(Exit(vm).clone());

    return oo;
}
