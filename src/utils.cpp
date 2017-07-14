#include <sstream>
#include "utils.hpp"

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
        str[fsize] = u_fgetc(f);
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

char* unicode_to_char(const UnicodeString &str) {
    uint_t buffer_size = 256; // XXX: this is always a bad idea
    char* buffer = new char[buffer_size];
    uint_t size = str.extract(0, str.length(), buffer, buffer_size, "UTF-8");//FIXME: null, UTF-8, or platform specific?
    ASSERT(size < buffer_size);
    buffer[size] = 0;
    return buffer;
}

UChar* unicode_to_uchar(const UnicodeString &str) {
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

UnicodeString unicode_convert_uint(uint_t n) {
    std::stringstream ss;
    ss << n;
    UnicodeString u(ss.str().c_str());
    return u;
}

UnicodeString unicode_concat(const UnicodeString& s0, const UnicodeString& s1) {
    UnicodeString s;
    s += s0;
    s += s1;
    return s;
}

uint64_t convert_to_int(const UnicodeString& s) {
    char* i = unicode_to_char(s);
    return atol(i);
}

double convert_to_float(const UnicodeString& s) {
    char* f = unicode_to_char(s);
    return atof(f);
}

UChar32 convert_to_char(const UnicodeString& s) {
    auto s0 = unicode_strip_quotes(s);
    auto s1 = unicode_unescape(s0);
    return s1.char32At(0);
}

UnicodeString convert_to_text(const UnicodeString& s) {
    auto s0 = unicode_strip_quotes(s);
    auto s1 = unicode_unescape(s0);
    return s1;
}

UnicodeString convert_from_int(const uint64_t n) {
    std::stringstream ss;
    ss << n;
    UnicodeString u(ss.str().c_str());
    return u;
}

UnicodeString convert_from_float(const double f) {
    std::stringstream ss;
    ss << f;
    UnicodeString u(ss.str().c_str());
    return u;
}

UnicodeString convert_from_char(const UChar32 c) {
    std::stringstream ss;
    ss << c;
    UnicodeString u(ss.str().c_str());
    auto u1 = unicode_escape(u);
    return u1;
}

UnicodeString convert_from_text(const UnicodeString& s) {
    auto s1 = unicode_escape(s);
    return s1;
}

UnicodeString unicode_strip_quotes(const UnicodeString& s) {
    UnicodeString copy(s);
    return copy.retainBetween(1, copy.length() -1);
}

UnicodeString unicode_escape(const UnicodeString& s) {
    UChar* s0  = unicode_to_uchar(s);
    UnicodeString s1;
    int i=0;
    while (s0[i] != 0) {
        switch (s0[i]) {
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
        default:
            s1 += (s0[i]);
            break;
        }
        i++;
    }
    return s1;
}

UnicodeString unicode_unescape(const UnicodeString& s) {
    return s.unescape();
}

bool unicode_endswith(const UnicodeString& s, const UnicodeString& suffix) {
    return s.endsWith(suffix);
}

// basic I/O routines

UnicodeString file_read(const UnicodeString &filename) {
    char* fn = unicode_to_char(filename);
    UChar* chars = read_utf8_file(fn);
    delete[] fn;
    UnicodeString str = UnicodeString(chars);
    delete[] chars;
    return str;
}

void file_write(const UnicodeString &filename, const UnicodeString &str) {
    char* fn = unicode_to_char(filename);
    UChar* s  = unicode_to_uchar(str);
    write_utf8_file(fn, s);
    delete[] fn;
    delete[] s;
}

bool file_exists(const UnicodeString &filename) {
    char* fn = unicode_to_char(filename);
    bool b = exists_file(fn);
    delete[] fn;
    return b;
}

UnicodeString path_combine(const UnicodeString& p0, const UnicodeString& p1) {
    // XXX: OS specific so this should once be generalized.
    if (p0.endsWith(UnicodeString("/"))) {
        return p0 + p1;
    } else {
        return p0 + '/' + p1;
    }
}

