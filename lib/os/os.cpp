#include "../../src/runtime.hpp"

#include <stdlib.h>
#include <iostream>
#include <fstream>

#include <string>
#include <memory>
#include <exception>
#include <thread>

// lets hope all this C stuff can once be gone
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// from utils.... XXX
char* unicode_to_char(const icu::UnicodeString &str) {
    std::string utf8;
    str.toUTF8String(utf8);
    auto cc0 = utf8.c_str();
    auto len = strlen(cc0);
    auto cc1 = (char*) malloc(len+1);
    strncpy(cc1, cc0, len+1);
    return cc1;
}

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
    CHANNEL_FD,
} channel_tag_t;

class Channel;
typedef std::shared_ptr<Channel> ChannelPtr;

class Channel {
public:
    Channel(channel_tag_t t): _tag(t) {
    }

    virtual ~Channel() { // keep some C++ compilers happy
    }

    channel_tag_t tag() {
        return _tag;
    }

    virtual UnicodeString read() {
        throw Unsupported();
    }

    virtual UChar32 read_char() { // XXX: wait for the libicu fix?
        throw Unsupported();
    }

    virtual UnicodeString read_line() { // implemented otherwise
        throw Unsupported();
    }

    virtual UnicodeString read_all() {
        UnicodeString s; // replace with a buffer once
        while (!eof()) {
            s += read_line();
        }
        return s;
    }

    virtual void write(const UnicodeString& n) {
        throw Unsupported();
    }

    virtual void write_char(const UChar32& n) {
        throw Unsupported();
    }

    virtual void write_line(const UnicodeString& n) {
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

    virtual UnicodeString read_line() override {
        std::string str;
        std::getline(std::cin, str);
        return UnicodeString(str.c_str());
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

    virtual void write_line(const UnicodeString& s) override {
        std::cout << s << std::endl;
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

    virtual void write_line(const UnicodeString& s) override {
        std::cerr << s << std::endl;
    }

    virtual void flush() override {
        std::cerr.flush();
    }
};

class ChannelFile: public Channel {
public:
    ChannelFile(const UnicodeString& fn): Channel(CHANNEL_FILE), _fn(fn) {
        std::cerr << "creating " << fn << std::endl;
        auto cc = unicode_to_char(fn);
        _stream = std::fstream(cc, std::fstream::in | std::fstream::out); // unsafe due to NUL
        free(cc);
    }

    static ChannelPtr create(const UnicodeString& fn) {
        return ChannelPtr(new ChannelFile(fn));
    }

    virtual UnicodeString read() override {
        UnicodeString str;
        _stream >> str;
        return str;
    }

    virtual UnicodeString read_line() override {
        std::string str;
        std::getline(_stream, str);
        return UnicodeString(str.c_str());
    }

    virtual void write(const UnicodeString& s) override {
        std::cerr << "writing: " << s << std::endl;
        _stream << s;
    }

    virtual void write_line(const UnicodeString& s) override {
        _stream << s << std::endl;
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
protected:
    UnicodeString   _fn;
    std::fstream    _stream;
};

// Convenience class for file descriptors from _sockets_.
// should be removed as soon as C++ adds stream io on them.
class ChannelFD: public Channel {
public:
    ChannelFD(const int fd): Channel(CHANNEL_FD), _fd(fd) {
    }

    static ChannelPtr create(const int fd) {
        return ChannelPtr(new ChannelFD(fd));
    }

    UnicodeString read_line() override {
        const int CHUNK_SIZE = 1024;
        char* str = (char*) malloc(CHUNK_SIZE);
        int allocated = CHUNK_SIZE; 
        int count = 0;

        _eof = false;

        // XXX: replace this code once with buffered writes
        // read to a char* one byte at a time until '\n'
        int n = 0;
        char ch;
        do {
            n = ::read(_fd, (void*) &ch, (size_t) 1);
            if (n < 0) { // this signals an error
                throw "error in read";
            } else if (n == 0) { // this signals EOF
                _eof = true;
            } else { // n == 1
                if (ch == '\n') {
                } else {
                    str[count] = ch;
                    count++;
                    if (count >= allocated) {
                        str = (char*) realloc(str, allocated + CHUNK_SIZE);
                        allocated += CHUNK_SIZE;
                    }
                }
            }
        } while ((n > 0) && (ch != '\n'));

        auto s = UnicodeString::fromUTF8(StringPiece(str, count));
        delete str;
        return s;
    }

    void write_byte(const char c) {
        char ch;
        ch = c;
        int n = 0;
        do {
            n = ::write(_fd, &ch, 1); // always remember write can fail, folks
            if (n == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50)); // XXX
            }
        } while (n == 0); 
        if (n < 0) {
            throw "error in write";
        }
    }

    void write_line(const UnicodeString& s) override {
        std::string utf8;
        s.toUTF8String(utf8);

        auto len = utf8.length();
        auto str = utf8.c_str();

        for (long unsigned int i = 0; i < len; i++) {
            write_byte(str[i]);
        }

        write_byte('\n');
    }

    void close() override {
        ::close(_fd);
    }

    void flush() override {
    }

    bool eof() override {
        return _eof;
    }
    
protected:
    int  _fd;
    bool _eof;
};


/**
 * Egel's primitive input/output combinators.
 *
 * Unstable and not all combinators fully provide what their 
 * specification promises. Neither are all combinators implemented
 * according to spec.
 **/

//## OS::channel - opaque values which are input/output channels
class ChannelValue: public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_EGO, ChannelValue, "OS", "channel");

    ChannelValue(VM* m, const ChannelPtr& chan): ChannelValue(m) {
        _value = chan;
    }

    ChannelValue(const ChannelValue& chan): Opaque(VM_SUB_EGO, chan.machine(), chan.symbol()) {
        _value = chan.value();
    }

    VMObjectPtr clone() const override {
        return VMObjectPtr(new ChannelValue(*this));
    }

    static VMObjectPtr create(VM* m, const ChannelPtr& c) {
        return VMObjectPtr(new ChannelValue(m, c));
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

//## OS::cin - standard input channel
class Stdin: public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_EGO, Stdin, "OS", "stdin");

    VMObjectPtr apply() const override {
        auto cin = ChannelStreamIn::create();
        auto in  = ChannelValue(machine());
        in.set_value(cin);
        return in.clone();
    }
};

//## OS::stdout - standard output channel
class Stdout: public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_EGO, Stdout, "OS", "stdout");

    VMObjectPtr apply() const override {
        auto cout = ChannelStreamOut::create();
        auto out  = ChannelValue(machine());
        out.set_value(cout);
        return out.clone();
    }
};

//## OS::stderr - standard error channel
class Stderr: public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_EGO, Stderr, "OS", "stderr");

    VMObjectPtr apply() const override {
        auto cerr = ChannelStreamErr::create();
        auto err  = ChannelValue(machine());
        err.set_value(cerr);
        return err.clone();
    }
};


/* Input functions on standard input */

/*
//## OS::getint
// Read one line from standard input and convert it to an integer. 

class Getint: public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_EGO, Getint, "OS", "getint");

    VMObjectPtr apply() const override {
        vm_int_t n;
        std::cin >> n;
        return VMObjectInteger(n).clone();
    }
};

//## OS::getfloat
// Read one line from standard input and convert it to a 
// floating-point number. The result is unspecified if the line read 
// is not a valid representation of a floating-point number.

class Getfloat: public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_EGO, Getfloat, "OS", "getfloat");

    VMObjectPtr apply() const override {
        vm_float_t f;
        std::cin >> f;
        return VMObjectFloat(f).clone();
    }
};
*/

/* File channel creation and destruction */

//## OS::open fn - create a channel from filename 
class Open: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Open, "OS", "open");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            auto fn = VM_OBJECT_TEXT_VALUE(arg0);
            auto stream = ChannelFile::create(fn);
            auto channel = ChannelValue::create(machine(), stream);
            return channel;
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::close c - close a channel
class Close: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Close, "OS", "close");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("OS", "channel");

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            chan->close();
            return create_none();
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::read c - read a string from a channel
class Read: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Read, "OS", "read");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("OS", "channel");

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            UnicodeString str = chan->read();
            return create_text(str);
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::read_line c - read a line from a channel
class ReadLine: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, ReadLine, "OS", "read_line");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("OS", "channel");

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            UnicodeString str = chan->read_line();
            return create_text(str);
        } else {
            THROW_INVALID;
        }
    }
};

//## OS::read_all c - read entire channel content
class ReadAll: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, ReadAll, "OS", "read_all");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("OS", "channel");

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            UnicodeString str = chan->read_all();
            return create_text(str);
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::write c s - write a string s to a channel
class Write: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, Write, "OS", "write");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("OS", "channel");

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            if (arg1->tag() == VM_OBJECT_TEXT) {
                auto s = VM_OBJECT_TEXT_VALUE(arg1);
                chan->write(s);
                return create_none();
            } else {
                THROW_INVALID;
            }
        } else {
            THROW_INVALID;
        }
    }
};

//## OS::write_line c s - write a string s to a channel
class WriteLine: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, WriteLine, "OS", "write_line");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("OS", "channel");

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            if (arg1->tag() == VM_OBJECT_TEXT) {
                auto s = VM_OBJECT_TEXT_VALUE(arg1);
                chan->write_line(s);
                return create_none();
            } else {
                THROW_INVALID;
            }
        } else {
            THROW_INVALID;
        }
    }
};

//## OS::flush c - flush a channel
class Flush: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Flush, "OS", "flush");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("OS", "channel");

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            chan->flush();
            return create_none();
        } else {
            THROW_INVALID;
        }
    }
};

//## OS::eof c - tests if there is no more input
class Eof: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Eof, "OS", "eof");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("OS", "channel");

        if (CHANNEL_TEST(arg0, sym)) {
            auto chan = CHANNEL_VALUE(arg0);
            return create_bool(chan->eof());
        } else {
            THROW_INVALID;
        }
    }
};

//## OS::exit n - flush all channels and terminate process with exit code n
// (0 to indicate no errors, a small positive integer for failure.)
class Exit: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Exit, "OS", "exit");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_INTEGER) {
            auto i = VM_OBJECT_INTEGER_VALUE(arg0);
            // XXX: uh.. flushall, or something?
            exit(i);
            // play nice
            return create_none();
        } else {
            THROW_BADARGS;
        }
    }
};

//////////////////////////////////////////////////////////////////////
// Highly unstable and experimental client/server code.

//## OS::serverobject - an opaque objects which serves as a server
class ServerObject: public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_EGO, ServerObject, "OS", "serverobject");

    ServerObject(const ServerObject& so): Opaque(VM_SUB_EGO, so.machine(), so.symbol()) {
        // memcpy( (char*) &_server_address,  (char *) &so._server_address, sizeof(_server_address));
	_server_address = so._server_address;
        _portno = so._portno;
        _queue = so._queue;
        _sockfd = so._sockfd;
    }

    VMObjectPtr clone() const override {
        return VMObjectPtr(new ServerObject(*this));
    }

    int compare(const VMObjectPtr& o) override {
        // XXX: not the foggiest idea whether this words.
        // I assume file descriptors are unique.
        auto v = (std::static_pointer_cast<ServerObject>(o));
        if (_sockfd < v->_sockfd) {
            return -1;
        } else if (_sockfd > v->_sockfd) {
            return 1;
        } else {
            return 0;
        }
    }

    void bind(int port, int in) {
        _portno = port;
        _queue = in;
        _sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (_sockfd < 0) {
            throw VMObjectText::create("error opening socket");
        }

        // bzero((char *) &_server_address, sizeof(_server_address));
	_server_address = {};
        _server_address.sin_family = AF_INET;
        _server_address.sin_addr.s_addr = INADDR_ANY;
        _server_address.sin_port = htons(_portno);
        if (::bind(_sockfd, (struct sockaddr *) &_server_address,
                 sizeof(_server_address)) < 0) {
            throw VMObjectText::create("error on bind");
        }
        listen(_sockfd,_queue);
    }

    VMObjectPtr accept() {
        struct sockaddr_in address;
        socklen_t n = sizeof(address); // XXX: What is the supposed lifetime of this?
        int fd = ::accept(_sockfd,
                     (struct sockaddr *) &address,
                     &n);
        if (fd < 0) {
            throw VMObjectText::create("error opening socket");
        }
        auto cn = ChannelFD::create(fd);
        auto c  = ChannelValue(machine());
        c.set_value(cn);
        return c.clone();
    }

protected:
    struct sockaddr_in _server_address = {};
    int _portno = 0;
    int _queue = 0;
    int _sockfd = 0;
};

#define SERVER_OBJECT_TEST(o, sym) \
    ((o->tag() == VM_OBJECT_OPAQUE) && \
     (VM_OBJECT_OPAQUE_SYMBOL(o) == sym))
#define SERVER_OBJECT_CAST(o) \
    (std::static_pointer_cast<ServerObject>(o))

//## OS::accept serverobject - accept connections
class Accept: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Accept, "OS", "accept");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        static symbol_t sym = 0;
        if (sym == 0) sym = machine()->enter_symbol("OS", "serverobject");

        if (SERVER_OBJECT_TEST(arg0, sym)) {
            auto so = SERVER_OBJECT_CAST(arg0);
            auto chan = so->accept();
            return chan;
        } else {
            THROW_INVALID;
        }
    }
};

//## OS::server port in - create a serverobject 
class Server: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, Server, "OS", "server");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_INTEGER) && (arg1->tag() == VM_OBJECT_INTEGER) ) {
            auto port = VM_OBJECT_INTEGER_VALUE(arg0);
            auto in   = VM_OBJECT_INTEGER_VALUE(arg1);

            auto so = ServerObject(machine());
            so.bind(port, in);

            return so.clone();
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::client host port - create a client channel
class Client: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, Client, "OS", "client");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ( (arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_INTEGER) ) {
            auto host = VM_OBJECT_TEXT_VALUE(arg0);
            auto port = VM_OBJECT_INTEGER_VALUE(arg1);

            struct sockaddr_in server_address;
            int sockfd;

            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                throw VMObjectText::create("error opening socket");
            }

            bzero((char *) &server_address, sizeof(server_address));
            server_address.sin_family = AF_INET;
            server_address.sin_port = htons(port);

            std::string utf8;
            host.toUTF8String(utf8);
            // Convert IPv4 and IPv6 addresses from text to binary form
            if(::inet_pton(AF_INET, utf8.c_str(), &server_address.sin_addr)<=0) {
                throw VMObjectText::create("invalid address");
            }

            if (::connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
                throw VMObjectText::create("connection failed");
            }

            auto cn = ChannelFD::create(sockfd);
            auto c  = ChannelValue(machine());
            c.set_value(cn);
            return c.clone();

        } else {
            THROW_BADARGS;
        }
    }

};

extern "C" std::vector<icu::UnicodeString> egel_imports() {
    return std::vector<icu::UnicodeString>();
}

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

//    oo.push_back(VMObjectData(vm, "OS", "channel").clone());

    oo.push_back(ChannelValue(vm).clone());
    oo.push_back(Stdin(vm).clone());
    oo.push_back(Stdout(vm).clone());
    oo.push_back(Stderr(vm).clone());
    oo.push_back(Open(vm).clone());
    oo.push_back(Close(vm).clone());
    oo.push_back(Read(vm).clone());
    oo.push_back(ReadLine(vm).clone());
    oo.push_back(ReadAll(vm).clone());
    oo.push_back(Write(vm).clone());
    oo.push_back(WriteLine(vm).clone());
    oo.push_back(Flush(vm).clone());
    oo.push_back(Eof(vm).clone());
    oo.push_back(Exit(vm).clone());

// hacked TCP protocol
    oo.push_back(ServerObject(vm).clone());
    oo.push_back(Accept(vm).clone());
    oo.push_back(Server(vm).clone());
    oo.push_back(Client(vm).clone());

    return oo;
}
