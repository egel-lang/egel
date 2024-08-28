#include <stdlib.h>

#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <filesystem>

#include "../../src/runtime.hpp"

// lets hope all this C stuff can once be gone
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

using namespace egel;
// from utils.... XXX
char* unicode_to_char(const icu::UnicodeString& str) {
    std::string utf8;
    str.toUTF8String(utf8);
    auto cc0 = utf8.c_str();
    auto len = strlen(cc0);
    auto cc1 = (char*)malloc(len + 1);
    strncpy(cc1, cc0, len + 1);
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
    Unsupported() : _message("") {
    }

    Unsupported(const UnicodeString& m) : _message(m) {
    }

    ~Unsupported() {
    }

    UnicodeString message() const {
        return _message;
    }

    friend std::ostream& operator<<(std::ostream& os, const Unsupported& e) {
        os << "unsupported(" << e.message() << ")";
        return os;
    }

private:
    UnicodeString _message;
};

enum channel_tag_t {
    CHANNEL_STREAM_IN,
    CHANNEL_STREAM_OUT,
    CHANNEL_STREAM_ERR,
    CHANNEL_FILE,
    CHANNEL_FD,
};

class Channel;
typedef std::shared_ptr<Channel> ChannelPtr;

class Channel {
public:
    Channel(channel_tag_t t) : _tag(t) {
    }

    virtual ~Channel() {  // keep some C++ compilers happy
    }

    channel_tag_t tag() {
        return _tag;
    }

    virtual UnicodeString read() {
        throw Unsupported();
    }

    virtual int read_byte() {
        throw Unsupported();
    }

    virtual UChar32 read_char() {  // XXX: wait for the libicu fix?
        throw Unsupported();
    }

    virtual UnicodeString read_line() {  // implemented otherwise
        throw Unsupported();
    }

    virtual UnicodeString read_all() {
        UnicodeString s;  // replace with a buffer once
        s = read_line();
        while (!eof()) { // XXX: wtf is this code?
            auto s0 = read_line();
            if (!eof()) s += "\n" + s0;
        }
        return s;
    }

    virtual void write(const UnicodeString& n) {
        throw Unsupported();
    }

    virtual void write_byte(const int n) {
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
    channel_tag_t _tag;
};

class ChannelStreamIn : public Channel {
public:
    ChannelStreamIn() : Channel(CHANNEL_STREAM_IN) {
    }

    static ChannelPtr create() {
        return ChannelPtr(new ChannelStreamIn());
    }

    virtual UnicodeString read() override {
        UnicodeString s;
        std::cin >> s;
        return s;
    }

    virtual int read_byte() override {
        // XXX: not always supported to read bytes, stream may be in text mode
        // too
        return std::cin.get();
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

class ChannelStreamOut : public Channel {
public:
    ChannelStreamOut() : Channel(CHANNEL_STREAM_OUT) {
    }

    static ChannelPtr create() {
        return ChannelPtr(new ChannelStreamOut());
    }

    virtual void write(const UnicodeString& s) override {
        std::cout << s;
    }

    virtual void write_byte(const int n) override {
        std::cout.put((char)n);
    }

    virtual void write_line(const UnicodeString& s) override {
        std::cout << s << std::endl;
    }

    virtual void flush() override {
        std::cout.flush();
    }
};

class ChannelStreamErr : public Channel {
public:
    ChannelStreamErr() : Channel(CHANNEL_STREAM_ERR) {
    }

    static ChannelPtr create() {
        return ChannelPtr(new ChannelStreamErr());
    }

    virtual void write(const UnicodeString& s) override {
        std::cerr << s;
    }

    virtual void write_byte(const int n) override {
        std::cerr.put((char)n);
    }

    virtual void write_line(const UnicodeString& s) override {
        std::cerr << s << std::endl;
    }

    virtual void flush() override {
        std::cerr.flush();
    }
};

class ChannelFile : public Channel {
public:
    ChannelFile(const UnicodeString& fn, std::ios_base::openmode m)
        : Channel(CHANNEL_FILE), _fn(fn) {
        auto cc = unicode_to_char(fn);
        _stream = std::fstream(cc, m);  // unsafe due to NUL
        free(cc);
    }

    static ChannelPtr create(const UnicodeString& fn,
                             std::ios_base::openmode m) {
        return ChannelPtr(new ChannelFile(fn, m));
    }

    virtual UnicodeString read() override {
        UnicodeString str;
        _stream >> str;
        return str;
    }

    virtual int read_byte() override {
        return _stream.get();
    }

    virtual UnicodeString read_line() override {
        std::string str;
        std::getline(_stream, str);
        return UnicodeString(str.c_str());
    }

    virtual void write(const UnicodeString& s) override {
        _stream << s;
    }

    virtual void write_byte(const int n) override {
        _stream.put((char)n);
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
    UnicodeString _fn;
    std::fstream _stream;
};

// Convenience class for file descriptors from _sockets_.
// should be removed as soon as C++ adds stream io on them.
class ChannelFD : public Channel {
public:
    ChannelFD(const int fd) : Channel(CHANNEL_FD), _fd(fd) {
    }

    ~ChannelFD() {
        ::close(_fd);
    }

    static ChannelPtr create(const int fd) {
        return ChannelPtr(new ChannelFD(fd));
    }

    virtual UnicodeString read_line() override {
        const int CHUNK_SIZE = 1024;
        char* str = (char*)malloc(CHUNK_SIZE);
        int allocated = CHUNK_SIZE;
        int count = 0;

        _eof = false;

        // XXX: replace this code once with buffered writes
        // read to a char* one byte at a time until '\n'
        int n = 0;
        char ch;
        do {
            n = ::read(_fd, (void*)&ch, (size_t)1);
            if (n < 0) {  // this signals an error
                throw "error in read";
            } else if (n == 0) {  // this signals EOF
                _eof = true;
            } else {  // n == 1
                if (ch == '\n') {
                } else {
                    str[count] = ch;
                    count++;
                    if (count >= allocated) {
                        str = (char*)realloc(str, allocated + CHUNK_SIZE);
                        allocated += CHUNK_SIZE;
                    }
                }
            }
        } while ((n > 0) && (ch != '\n'));

        auto s = UnicodeString::fromUTF8(StringPiece(str, count));
        free(str);
        return s;
    }

    virtual void write_byte(const int b) override {
        char ch;
        ch = b;
        int n = 0;
        do {
            n = ::write(_fd, &ch, 1);  // always remember write can fail, folks
            if (n == 0) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(50));  // XXX
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
    int _fd;
    bool _eof;
};

/**
 * Egel's primitive input/output combinators.
 *
 * Unstable and not all combinators fully provide what their
 * specification promises. Neither are all combinators implemented
 * according to spec.
 **/

class ChannelValue : public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_EGO, ChannelValue, "OS", "channel");
DOCSTRING("OS::channel - opaque values which are input/output channels");

    ChannelValue(VM* m, const ChannelPtr& chan) : ChannelValue(m) {
        _value = chan;
    }

    ChannelValue(const ChannelValue& chan)
        : Opaque(VM_SUB_EGO, chan.machine(), chan.symbol()) {
        _value = chan.value();
    }

    static VMObjectPtr create(VM* m, const ChannelPtr& c) {
        return VMObjectPtr(new ChannelValue(m, c));
    }

    int compare(const VMObjectPtr& o) override {
        auto v = (std::static_pointer_cast<ChannelValue>(o))->value();
        if (_value < v)
            return -1;
        else if (v < _value)
            return 1;
        else
            return 0;
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

class Stdin : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_EGO, Stdin, "OS", "stdin");
DOCSTRING("OS::cin - standard input channel");

    VMObjectPtr apply() const override {
        auto cin = ChannelStreamIn::create();
        auto in = ChannelValue::create(machine(), cin);
        return in;
    }
};

class Stdout : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_EGO, Stdout, "OS", "stdout");
DOCSTRING("OS::stdout - standard output channel");

    VMObjectPtr apply() const override {
        auto cout = ChannelStreamOut::create();
        auto out = ChannelValue::create(machine(), cout);
        return out;
    }
};

class Stderr : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_EGO, Stderr, "OS", "stderr");
DOCSTRING("OS::stderr - standard error channel");

    VMObjectPtr apply() const override {
        auto cerr = ChannelStreamErr::create();
        auto err = ChannelValue::create(machine(), cerr);
        return err;
    }
};

/* File channel creation and destruction */

class OpenIn : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, OpenIn, "OS", "open_in");
DOCSTRING("OS::open_in fn - create a channel from filename");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            auto fn = machine()->get_text(arg0);

            // throw an error when the file doesn't exist
            auto len = fn.extract(0, 2048, nullptr, (uint32_t)0);  // XXX: I hate constants
            auto buffer = new char[len + 1];
            fn.extract(0, 2048, buffer, len + 1);
            auto p = fs::path(buffer);
            delete[] buffer;
            auto b = fs::exists(p); 

            if (!b) throw machine()->create_text("file not found: " + fn);


            auto stream = ChannelFile::create(fn, std::fstream::in);
            auto channel = ChannelValue::create(machine(), stream);
            return channel;
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class OpenOut : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, OpenOut, "OS", "open_out");
DOCSTRING("OS::open_out fn - create a channel from filename");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            auto fn = machine()->get_text(arg0);
            auto stream = ChannelFile::create(fn, std::fstream::out);
            auto channel = ChannelValue::create(machine(), stream);
            return channel;
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Close : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Close, "OS", "close");
DOCSTRING("OS::close c - close a channel");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (ChannelValue::is_type(arg0)) {
            auto chan = ChannelValue::cast(arg0)->value();
            chan->close();
            return machine()->create_none();
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Read : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Read, "OS", "read");
DOCSTRING("OS::read c - read a string from a channel");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (ChannelValue::is_type(arg0)) {
            auto chan = ChannelValue::cast(arg0)->value();
            UnicodeString str = chan->read();
            return machine()->create_text(str);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class ReadByte : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, ReadByte, "OS", "read_byte");
DOCSTRING("OS::read_byte c - read a byte from a channel");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (ChannelValue::is_type(arg0)) {
            auto chan = ChannelValue::cast(arg0)->value();
            int b = chan->read_byte();
            return machine()->create_integer(b);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class ReadLine : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, ReadLine, "OS", "read_line");
DOCSTRING("OS::read_line c - read a line from a channel");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (ChannelValue::is_type(arg0)) {
            auto chan = ChannelValue::cast(arg0)->value();
            UnicodeString str = chan->read_line();
            return machine()->create_text(str);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class ReadAll : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, ReadAll, "OS", "read_all");
DOCSTRING("OS::read_all c - read entire channel content");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (ChannelValue::is_type(arg0)) {
            auto chan = ChannelValue::cast(arg0)->value();
            UnicodeString str = chan->read_all();
            return machine()->create_text(str);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Write : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, Write, "OS", "write");
DOCSTRING("OS::write c s - write a string to a channel");

    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
        if (ChannelValue::is_type(arg0)) {
            auto chan = ChannelValue::cast(arg0)->value();
            if (machine()->is_text(arg1)) {
                auto s = machine()->get_text(arg1);
                chan->write(s);
                return machine()->create_none();
            } else {
                throw machine()->bad_args(this, arg0, arg1);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class WriteByte : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, WriteByte, "OS", "write_byte");
DOCSTRING("OS::write_byte c b - write a byte to a channel");

    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
        if (ChannelValue::is_type(arg0)) {
            auto chan = ChannelValue::cast(arg0)->value();
            if (machine()->is_integer(arg1)) {
                auto b = machine()->get_integer(arg1);
                chan->write_byte(b);
                return machine()->create_none();
            } else {
                throw machine()->bad_args(this, arg0, arg1);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class WriteLine : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, WriteLine, "OS", "write_line");
DOCSTRING("OS::write_line c s - write a string s to a channel");

    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
        if (ChannelValue::is_type(arg0)) {
            auto chan = ChannelValue::cast(arg0)->value();
            if (machine()->is_text(arg1)) {
                auto s = machine()->get_text(arg1);
                chan->write_line(s);
                return machine()->create_none();
            } else {
                throw machine()->bad_args(this, arg0, arg1);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class Flush : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Flush, "OS", "flush");
DOCSTRING("OS::flush c - flush a channel");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (ChannelValue::is_type(arg0)) {
            auto chan = ChannelValue::cast(arg0)->value();
            chan->flush();
            return machine()->create_none();
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Eof : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Eof, "OS", "eof");
DOCSTRING("OS::eof c - tests if there is no more input");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (ChannelValue::is_type(arg0)) {
            auto chan = ChannelValue::cast(arg0)->value();
            return machine()->create_bool(chan->eof());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Flock : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Flock, "OS", "flock");
DOCSTRING("OS::flock f n - create a filesystem lock file (not process safe)");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        auto m = machine();

        if (m->is_text(arg0)) {
            auto s = m->get_text(arg0);
            auto fn = unicode_to_char(s);

            auto fd = open(fn, O_WRONLY | O_CREAT,
                           0644);  // channels not fds so just open a file
            flock(fd, LOCK_EX);
            free(fn);
            auto cn = ChannelFD::create(fd);
            auto c = ChannelValue::create(
                m, cn);  // lock will be released when object destroyed
            return c;
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//  (0 to indicate no errors, a small positive integer for failure.)
class Exit : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Exit, "OS", "exit");
DOCSTRING("OS::exit n - flush all channels and terminate process with exit code n");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            // XXX: uh.. flushall, or something?
            exit(i);
            // play nice
            return machine()->create_none();
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Exec : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Exec, "OS", "exec");
DOCSTRING("OS::exec c - system exec command");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            auto cmd = VM::unicode_to_utf8_chars(s);
            
            std::array<char, 128> buffer;
            std::string result;
            std::unique_ptr<FILE, void(*)(FILE*)> pipe(popen(cmd, "r"),
                [](FILE * f) -> void
                {
                    // wrapper to ignore the return value from pclose() is needed with newer versions of gnu g++
                    std::ignore = pclose(f);
                });
            //std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
            if (!pipe) {
                throw std::runtime_error("popen() failed!");
            }
            while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
                result += buffer.data();
            }
            auto r = VM::unicode_from_utf8_chars(result.c_str());
            return machine()->create_text(r);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

char get_key() {
#if defined(_WIN32) || defined(_WIN64)
    return _getch();  // Windows: Waits for a key press without echoing it
#elif defined(__unix__) || defined(__unix) || defined(__APPLE__) && defined(__MACH__)
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);           // Get current terminal settings
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);         // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);  // Apply new settings

    char ch;
    std::cin.get(ch);                         // Wait for key press

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);  // Restore old settings
    return ch;
#endif
};

class GetKey : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_EGO, GetKey, "OS", "get_key");
DOCSTRING("OS::get_key - key press from console");

    VMObjectPtr apply() const override {
        auto ch = (vm_char_t) get_key();
        return machine()->create_char(ch);
    }
};

class OSModule: public CModule {
public:
    icu::UnicodeString name() const override {
        return "os";
    }

    icu::UnicodeString docstring() const override {
        return "The 'os' module defines basic input/output combinators.";
    }

    std::vector<VMObjectPtr> exports(VM *vm) override {
        std::vector<VMObjectPtr> oo;
    //    oo.push_back::create(VMObjectData(vm, "OS", "channel"));

    // oo.push_back(ChannelValue::create(vm));
    oo.push_back(VMObjectStub::create(vm, "<OS::channel>"));
    oo.push_back(Stdin::create(vm));
    oo.push_back(Stdout::create(vm));
    oo.push_back(Stderr::create(vm));
    oo.push_back(OpenIn::create(vm));
    oo.push_back(OpenOut::create(vm));
    oo.push_back(Close::create(vm));
    oo.push_back(Read::create(vm));
    oo.push_back(ReadByte::create(vm));
    oo.push_back(ReadLine::create(vm));
    oo.push_back(ReadAll::create(vm));
    oo.push_back(Write::create(vm));
    oo.push_back(WriteByte::create(vm));
    oo.push_back(WriteLine::create(vm));
    oo.push_back(Flush::create(vm));
    oo.push_back(Eof::create(vm));
    oo.push_back(Flock::create(vm));
    oo.push_back(Exit::create(vm));
    oo.push_back(Exec::create(vm));
    oo.push_back(GetKey::create(vm));


        return oo;
    }
};

extern "C" CModule* egel_module() {
    return new OSModule();
};
