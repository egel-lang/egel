#include <sstream>
#include <string.h>
#include "utils.hpp"
#include <experimental/filesystem>

#define STRING_MAX_SIZE 65536

// C routines
void assert_fail(const char *assertion, const char *file, uint_t line) {
    std::cerr << file << ':' << line << ": assertion failed " << assertion << '\n';
    abort();
}

void panic_fail(const char *message, const char *file, uint_t line) {
    std::cerr << file << ':' << line << ": panic " << message << '\n';
    abort();
}

UChar* read_utf8_file(const char* filename) {
    // FIXME: discard BOM when encountered?
    long fsize;
    UFILE* f = u_fopen(filename, "r", NULL, "UTF-8");
    ASSERT(f!=NULL);

    fseek(u_fgetfile(f), 0, SEEK_END);
    fsize = ftell(u_fgetfile(f));
    u_frewind(f);

    UChar* str = new UChar[fsize+1];  
    // UChar* str = (UChar*) malloc(sizeof(UChar) * (fsize + 1));

    for (fsize = 0; !u_feof(f); ++fsize) {
        UChar c = u_fgetc(f);
        str[fsize] = c;
    }

    str[fsize] = 0;

    u_fclose(f);

    return str;
}

void write_utf8_file(const char* filename, UChar* str) {
    int32_t size;
    UFILE* f = u_fopen(filename, "w", NULL, "UTF-8");
    ASSERT(f!=NULL);
    size = u_fputs(str, f);
    ASSERT(size >= 0);
    u_fclose(f);
}

bool exists_file(const char* filename) {
    // note race conditions when files are created/deleted. whatever...
    return ( access( filename, F_OK ) != -1 );
}

// Unicode routines

char* unicode_to_char(const icu::UnicodeString &str) {
    auto len = str.extract(0, STRING_MAX_SIZE, nullptr, (uint32_t) 0);
    auto buffer = new char[len+1];
    str.extract(0, STRING_MAX_SIZE, buffer, len+1);
    return buffer;
}

UChar* unicode_to_uchar(const icu::UnicodeString &str) {
    UErrorCode error = U_ZERO_ERROR;
    UChar* buffer = new UChar[str.length()+1];
    int32_t size = str.extract(buffer, sizeof(buffer), error);
    buffer[size] = 0;
#ifdef DEBUG
    ASSERT(size == str.length() + 1);
#endif
    ASSERT(U_SUCCESS(error));
    return buffer;
}

icu::UnicodeString unicode_convert_uint(uint_t n) {
    std::stringstream ss;
    ss << n;
    icu::UnicodeString u(ss.str().c_str());
    return u;
}

icu::UnicodeString unicode_concat(const icu::UnicodeString& s0, const icu::UnicodeString& s1) {
    icu::UnicodeString s;
    s += s0;
    s += s1;
    return s;
}

int64_t convert_to_int(const icu::UnicodeString& s) {
    char* buf = unicode_to_char(s);
    auto i = atol(buf);
    delete buf;
    return i;
}

int64_t convert_to_hexint(const icu::UnicodeString& s) {
    int i = 0;
    int64_t n = 0;
    UChar32 c = s.char32At(i);
    while (c != 0xffff) {
        n = n * 16;
        if (c >= 48 && c <= 57) {  // 0-9
            n += (((int)(c)) - 48);
        } else if ((c >= 65 && c <= 70))  { // A-F
            n += (((int)(c)) - 55);
        } else if (c >= 97 && c <= 102) {  // a-f
            n += (((int)(c)) - 87);
        } // just ignore other chars (like starting 0x)
        i++;
        c = s.char32At(i);
    }
    return n;
}

double convert_to_float(const icu::UnicodeString& s) {
    char* buf = unicode_to_char(s);
    auto f = atof(buf);
    delete buf;
    return f;
}

UChar32 convert_to_char(const icu::UnicodeString& s) {
    auto s0 = unicode_strip_quotes(s);
    auto s1 = unicode_unescape(s0);
    return s1.char32At(0);
}

icu::UnicodeString convert_to_text(const icu::UnicodeString& s) {
    auto s0 = unicode_strip_quotes(s);
    auto s1 = unicode_unescape(s0);
    return s1;
}

icu::UnicodeString convert_from_int(const int64_t& n) {
    std::stringstream ss;
    ss << n;
    icu::UnicodeString u(ss.str().c_str());
    return u;
}

icu::UnicodeString convert_from_float(const double& f) {
    std::stringstream ss;
    ss << f;
    icu::UnicodeString u(ss.str().c_str());
    return u;
}

icu::UnicodeString convert_from_char(const UChar32& c) {
    std::stringstream ss;
    ss << c;
    icu::UnicodeString u(ss.str().c_str());
    auto u1 = unicode_escape(u);
    return u1;
}

icu::UnicodeString convert_from_text(const icu::UnicodeString& s) {
    auto s1 = unicode_escape(s);
    return s1;
}

icu::UnicodeString unicode_strip_quotes(const icu::UnicodeString& s) {
    icu::UnicodeString copy(s);
    return copy.retainBetween(1, copy.length() -1);
}

icu::UnicodeString unicode_escape(const icu::UnicodeString& s) {
    icu::UnicodeString s1;
    int i=0;
    int len = s.length();
    for (i = 0; i < len; i++) {
        UChar32 c = s.char32At(i);
        switch (c) {
        case '\a':
            s1 += "\\a";
            break;
        case '\b':
            s1 += "\\b";
            break;
        case '\t':
            s1 += "\\t";
            break;
        case '\n':
            s1 += "\\n";
            break;
        case '\v':
            s1 += "\\v";
            break;
        case '\f':
            s1 += "\\f";
            break;
        case '\r':
            s1 += "\\r";
            break;
        case '\"':
            s1 += "\\\"";
            break;
        case '\'':
            s1 += "\\'";
            break;
        case '\\':
            s1 += "\\\\";
            break;
        default:
            s1 += c;
            break;
        }
    }
    return s1;
}

icu::UnicodeString unicode_unescape(const icu::UnicodeString& s) {
    return s.unescape();
}

bool unicode_endswith(const icu::UnicodeString& s, const icu::UnicodeString& suffix) {
    return s.endsWith(suffix);
}

// basic I/O routines

namespace fs = std::experimental::filesystem;

fs::path string_to_path(const icu::UnicodeString& s) {
    char* cc = unicode_to_char(s);
    fs::path p = fs::u8path(cc);
    delete cc;
    return p;
}

icu::UnicodeString path_to_string(const fs::path& p) {
    icu::UnicodeString s(p.c_str()); // XXX: utf8
    return s;
}

icu::UnicodeString path_absolute(const icu::UnicodeString& s) {
    auto p0 = string_to_path(s);
    auto p1 = fs::absolute(p0);
    return path_to_string(p1);
}

icu::UnicodeString path_combine(const icu::UnicodeString& s0, const icu::UnicodeString& s1) {
    auto p0 = string_to_path(s0);
    auto p1 = string_to_path(s1);
    p0 /= p1;
    return path_to_string(p0);
}

icu::UnicodeString file_read(const icu::UnicodeString &filename) {
    char* fn = unicode_to_char(filename);
    UChar* chars = read_utf8_file(fn);
    delete[] fn;
    icu::UnicodeString str = icu::UnicodeString(chars);
    delete[] chars;
    return str;
}

void file_write(const icu::UnicodeString &filename, const icu::UnicodeString &str) {
    char* fn = unicode_to_char(filename);
    UChar* s  = unicode_to_uchar(str);
    write_utf8_file(fn, s);
    delete[] fn;
    delete[] s;
}

bool file_exists(const icu::UnicodeString &filename) {
    char* fn = unicode_to_char(filename);
    bool b = exists_file(fn);
    delete[] fn;
    return b;
}


