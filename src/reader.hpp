#ifndef READER_HPP
#define READER_HPP

#include "utils.hpp"
#include "position.hpp"

class CharReader {
public:
    virtual UChar32 look() = 0;
    virtual void skip() = 0;
    virtual bool end() = 0;

    virtual void reset() = 0;

    virtual Position position() = 0;
};

class StringCharReader : public CharReader {
public:
    StringCharReader(const UnicodeString &resource, const UnicodeString &content) {
        _row = 1;
        _column = 1;
        _resource = resource;
        _content = content;
        _index = 0;
    }

    Position position() {
        return Position(_resource, _row, _column);
    }

    UnicodeString content() {
        return _content;
    }

    UChar32 look() {
        if (end()) return '\0';
        return _content.char32At(_index);
    }

    void skip() {
        if (end()) return;
        UChar32 c = look();
        _index = _content.moveIndex32(_index, 1);
        switch (c) {
            // XXX: handle MSDOS like newlines
            case '\n':
                _column = 1;
                _row++;
                break;
            default:
                _column++;
        }
    }

    bool end() {
        if (_content.char32At(_index) == 65535) return true; // make absolutely sure we always detect the end
        return _index >= _content.length();
    }

    bool eol() {
        if (end()) {
            return false;
        } else {
            return look() == '\n';
        };
    }
 
    void reset() {
        _index = 0;
        _row = 1;
        _column = 1;
    }

private:
    UnicodeString _resource;
    int32_t _row;
    int32_t _column;

    UnicodeString _content;
    int32_t _index;
};

#endif
