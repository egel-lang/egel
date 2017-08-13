#ifndef POSITION_H
#define POSITION_H

#include <iostream>
#include <sstream>
#include "utils.hpp"

class Position {
public:
    Position() {
        _resource = UnicodeString("");
        _row = 0;
        _column = 0;
    }

    Position(const UnicodeString &resource, int32_t row, int32_t column) {
        _resource = resource;
        _row = row;
        _column = column;
    }

    ~Position() {
    }

    UnicodeString resource() const {
        return _resource;
    }

    int32_t row() const {
        return _row;
    }

    int32_t column() const {
        return _column;
    }

    UnicodeString to_text() {
        std::stringstream ss;
        ss << *this;
        UnicodeString u(ss.str().c_str());
        return u;
    }

    friend std::ostream& operator<<(std::ostream& os, const Position &p) {
        os << p.resource() << ":" << p.row() << ":" << p.column() << "";
        return os;
    }

private:
    UnicodeString   _resource;
    int32_t         _row;
    int32_t         _column;
};

#endif
