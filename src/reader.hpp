#pragma once

#include <vector>

#include "position.hpp"
#include "unicode/unistr.h"
#include "unicode/ustream.h"
#include "runtime.hpp"

class CharReader {
public:
    virtual UChar32 look() = 0;
    virtual UChar32 look(int n) = 0;
    virtual void skip() = 0;
    virtual bool end() = 0;
    virtual bool eol() = 0;

    virtual void reset() = 0;

    virtual Position position() = 0;
};

class StringCharReader : public CharReader {
public:
    StringCharReader(const icu::UnicodeString &resource,
                     const icu::UnicodeString &content) {
        _row = 1;
        _column = 1;
        _index = 0;
        _resource = resource;
        _content = content;
        fill_buffer();
    }

    Position position() override {
        return Position(_resource, _row, _column);
    }

    icu::UnicodeString content() {
        return _content;
    }

    UChar32 look() override {
        if (end()) return '\0';
        return _buffer[_index];
    }

    UChar32 look(int n) override {  // need LL(2) in the lexer for '-1', to be
        // decaprecated at some point
        return _buffer[_index + n];
    }

    void skip() override {
        if (end()) return;
        UChar32 c = look();
        _index++;
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

    bool end() override {
        if (_index >= _length) return true;
        if (_buffer[_index] == 65535)
            return true;  // make absolutely sure we always detect the end
        return false;
    }

    bool eol() override {
        if (end()) {
            return false;
        } else {
            return look() == '\n';
        };
    }

    void reset() override {
        _index = 0;
        _row = 1;
        _column = 1;
    }

protected:
    void fill_buffer() {
        for (int i = 0; i < _content.length(); i = _content.moveIndex32(i, 1)) {
            auto c = _content.char32At(i);
            _buffer.push_back(c);
        }
        _length = _buffer.size();
    }

private:
    int32_t _row;
    int32_t _column;
    int32_t _index;
    int32_t _length;

    icu::UnicodeString _resource;
    icu::UnicodeString _content;
    std::vector<UChar32> _buffer;
};
