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

    virtual vm_int_t read_int() {
        throw Unsupported();
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

