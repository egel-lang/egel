#pragma once

#include <iostream>
#include <sstream>

#include "unicode/unistr.h"
#include "unicode/ustream.h"

namespace egel {

class Position {
public:
    Position() {
        _resource = icu::UnicodeString("");
        _row = 0;
        _column = 0;
    }

    Position(const icu::UnicodeString &resource, int32_t row, int32_t column) {
        _resource = resource;
        _row = row;
        _column = column;
    }

    ~Position() {
    }

    icu::UnicodeString resource() const {
        return _resource;
    }

    int32_t row() const {
        return _row;
    }

    int32_t column() const {
        return _column;
    }

    icu::UnicodeString to_text() {
        std::stringstream ss;
        ss << *this;
        icu::UnicodeString u(ss.str().c_str());
        return u;
    }

    friend std::ostream &operator<<(std::ostream &os, const Position &p) {
        os << p.resource() << ":" << p.row() << ":" << p.column() << "";
        return os;
    }

private:
    icu::UnicodeString _resource;
    int32_t _row;
    int32_t _column;
};

};  // namespace egel
