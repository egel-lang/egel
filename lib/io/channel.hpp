/**
 * In C++, copy constructors for streams are usually undefined
 * which makes a simple solution of just assigning or passing
 * iostream derived classes unworkable.
 *
 * These channel classes flatten C++ i/o streams hierarchy into
 * one channel class with all operations and derived classes 
 * which implement them.
 **/

#include <stdlib.h>
#include <ostream>
#include <fstream>

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

    virtual void write(vm_int_t n) {
        throw Unsupported();
    }

    virtual void write(vm_float_t n) {
        throw Unsupported();
    }

    virtual void write(vm_char_t n) {
        throw Unsupported();
    }

    virtual void write(const UnicodeString& n) {
        throw Unsupported();
    }

    virtual vm_int_t read_int() {
        throw Unsupported();
    }

    virtual vm_float_t read_float() {
        throw Unsupported();
    }

    virtual vm_char_t read_char() {
        throw Unsupported();
    }

    virtual UnicodeString read_line() {
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
    ChannelStreamIn(): Channel(CHANNEL_STREAM_IN), _channel(std::cin) {
    }

    static ChannelPtr create() {
        return ChannelPtr(new ChannelStreamIn());
    }

    vm_int_t read_int() override {
        vm_int_t n;
        _channel >> n;
        return n;
    }

    virtual vm_float_t read_float() override {
        vm_float_t f;
        _channel >> f;
        return f;
    }

    virtual vm_char_t read_char() override {
        vm_char_t c;
        _channel >> c;
        return c;
    }

    virtual UnicodeString read_line() override {
        std::string line;
        std::getline(_channel, line);
        UnicodeString str(line.c_str(), -1, US_INV);
        return str;
    }

    virtual bool eof() override {
        return _channel.eof();
    }
protected:
    std::istream&   _channel;
};

class ChannelStreamOut: public Channel {
public:
    ChannelStreamOut(): Channel(CHANNEL_STREAM_OUT), _channel(std::cout) {

    }

    static ChannelPtr create() {
        return ChannelPtr(new ChannelStreamOut());
    }

    virtual void write(vm_int_t n) override {
        _channel << n;
    }

    virtual void write(vm_float_t f) override {
        _channel.precision(17); // XXX: hacked precision constant
        _channel << f;
    }

    virtual void write(vm_char_t c) override {
        _channel << (UnicodeString() + c);
    }

    virtual void write(const UnicodeString& s) override {
        _channel << s;
    }

    virtual void flush() override {
        _channel.flush();
    }

protected:
    std::ostream&       _channel;
};

class ChannelFile: public Channel {
public:
    ChannelFile(const UnicodeString& fn): Channel(CHANNEL_FILE), _fn(fn) {
        char* bf = unicode_to_char(fn);
        _channel = std::fstream(bf);
        delete bf;
    }

    static ChannelPtr create(const UnicodeString& fn) {
        return ChannelPtr(new ChannelFile(fn));
    }

    virtual vm_int_t read_int() override {
        vm_int_t n;
        _channel >> n;
        return n;
    }

    virtual vm_float_t read_float() override {
        vm_float_t f;
        _channel >> f;
        return f;
    }

    virtual vm_char_t read_char() override {
        vm_char_t c;
        _channel >> c;
        return c;
    }

    virtual UnicodeString read_line() override {
        std::string line;
        std::getline(_channel, line);
        UnicodeString str(line.c_str(), -1, US_INV);
        return str;
    }

    virtual void write(vm_int_t n) override {
        _channel << n;
    }

    virtual void write(vm_float_t f) override {
        _channel.precision(17); // XXX: hacked precision constant
        _channel << f;
    }

    virtual void write(vm_char_t c) override {
        _channel << (UnicodeString() + c);
    }

    virtual void write(const UnicodeString& s) override {
        _channel << s;
    }

    virtual void close() override {
        _channel.close();
    }

    virtual void flush() override {
        _channel.flush();
    }

    virtual bool eof() override {
        return _channel.eof();
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
    std::fstream    _channel;
};

