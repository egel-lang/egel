export module utils;

import <stdio.h>;
import <stdlib.h>;
import <unistd.h>;
import <iostream>;
import <vector>;
import <sstream>;
import <string.h>;
import <filesystem>;

import "unicode/ustdio.h";
import "unicode/uchar.h";
import "unicode/unistr.h";
import "unicode/ustream.h";
import "unicode/stringpiece.h";

using uint_t = unsigned int;

// debugging macros

void assert_fail(const char *assertion, const char *file, uint_t line);
void panic_fail(const char *message, const char *file, uint_t line);

/**
 * ASSERT macro. Always compiled assertions.
 */
#define ASSERT(_e) ((_e) ? (void)0 : assert_fail(#_e, __FILE__, __LINE__));

/**
 * PANIC macro. Aborts with a message (e.g., for non-reachable statements).
 */
// play nice with runtime.hpp
#ifdef PANIC
#undef PANIC
#endif

#define PANIC(_m) (panic_fail(_m, __FILE__, __LINE__));

/**
 * DEBUG macro. An assertion only compiled in for debug builds.
 */
#if DEBUG
#define DEBUG_ASSERT(_e) \
    ((_e) ? (void)0 : assert_fail(#_e, __FILE__, __LINE__));
#else
#define DEBUG_ASSERT(_e) ((void)0);
#endif

using UnicodeStrings = std::vector<icu::UnicodeString>;
// unicode routines

/**
 ** Converts a icu::UnicodeString to a character array.
 **
 ** @param str  the string
 **
 ** @return a character array (heap allocated)
 **/
char *unicode_to_char(const icu::UnicodeString &str);

/**
 ** Converts a character array to a Unicode string.
 **
 ** @param cc  the UTF8 character array
 **
 ** @return a character array (heap allocated)
 **/
icu::UnicodeString char_to_unicode(const char *cc);

/**
 ** Converts a icu::UnicodeString to a UChar array.
 **
 ** @param str  the string
 **
 ** @return a UChar array (heap allocated)
 **/
UChar *unicode_to_uchar(const icu::UnicodeString &str);

/**
 ** Converts an unsigned int to Unicode.
 **
 ** @param n  the number
 **
 ** @return a Unicode string
 **/
icu::UnicodeString unicode_convert_uint(uint_t n);

/**
 ** Concatenate two unicode strings
 **
 ** @param s0  the first unicode string
 ** @param s1  the second unicode string
 **
 ** @return the concatenation, a Unicode string
 **/
icu::UnicodeString unicode_concat(const icu::UnicodeString &s0,
                                  const icu::UnicodeString &s1);

/**
 ** Strip the leading and trailing quote of a Unicode string
 **
 ** @param s  the unicode string
 **
 ** @return a stripped string
 **/
icu::UnicodeString unicode_strip_quotes(const icu::UnicodeString &s);

/**
 ** Escape certain characters in a Unicode string.
 **
 ** @param s  the unicode string
 **
 ** @return the escaped string
 **/
icu::UnicodeString unicode_escape(const icu::UnicodeString &s);

/**
 ** Unescape certain characters in a Unicode string.
 **
 ** @param s  the unicode string
 **
 ** @return the unescaped string
 **/
icu::UnicodeString unicode_unescape(const icu::UnicodeString &s);

/**
 ** Determine if a string ends with a certain suffix.
 **
 ** @param s  the unicode string to be searched
 ** @param sf the unicode string suffix
 **
 ** @return the unescaped string
 **/
bool unicode_endswith(const icu::UnicodeString &s,
                      const icu::UnicodeString &sf);

// convenience routines text to, and from, literals

/**
 ** Parse and convert an integer. like 42.
 **
 ** @param s  the unicode string
 **
 ** @return the integer recognized
 **/
int64_t convert_to_int(const icu::UnicodeString &s);

/**
 ** Parse and convert a hexadecimal integer. like 0xa3.
 **
 ** @param s  the unicode string
 **
 ** @return the integer recognized
 **/
int64_t convert_to_hexint(const icu::UnicodeString &s);

/**
 ** Parse and convert a float. like 3.14.
 **
 ** @param s  the unicode string
 **
 ** @return the integer recognized
 **/
double convert_to_float(const icu::UnicodeString &s);

/**
 ** Parse and convert a char string. like 'a'.
 **
 ** @param s  the unicode string
 **
 ** @return the char recognized
 **/
UChar32 convert_to_char(const icu::UnicodeString &s);

/**
 ** Parse and convert an integer. like 42.
 **
 ** @param s  the unicode string
 **
 ** @return the integer recognized
 **/
icu::UnicodeString convert_from_int(const int64_t &s);

/**
 ** Parse and convert a float. like 3.14.
 **
 ** @param s  the unicode string
 **
 ** @return the integer recognized
 **/
icu::UnicodeString convert_from_float(const double &s);

/**
 ** Parse and convert a char string. like 'a'.
 **
 ** @param s  the unicode string
 **
 ** @return the char recognized
 **/
icu::UnicodeString convert_from_char(const UChar32 &s);

/**
 ** Parse and convert a text string. like "hello!".
 **
 ** @param s  the unicode string
 **
 ** @return the integer recognized
 **/
icu::UnicodeString convert_to_text(const icu::UnicodeString &s);

// convenience io-routines

/**
 ** Convenience. Reads a file into a string from a given filename.
 **
 ** @param filename filename
 **
 ** @return the contents of the file
 **/
icu::UnicodeString file_read(const icu::UnicodeString &filename);

/**
 ** Convenience. Writes a string to a file.
 **
 ** @param filename     the filename
 ** @param str          the string
 **/
void file_write(const icu::UnicodeString &filename,
                const icu::UnicodeString &str);

/**
 ** Convenience. Checks whether a given file exists. (Note the possible race
 *conditions.)
 **
 ** @param filename     the filename
 **
 ** @return true iff the file exists
 **/
bool file_exists(const icu::UnicodeString &filename);

/**
 ** Convenience. Get the absolute path. OS specific.
 **
 ** @param s      the path
 **
 ** @return the absolute path
 **/
icu::UnicodeString path_absolute(const icu::UnicodeString &s);

/**
 ** Convenience. Combine two paths into a new path. OS specific.
 **
 ** @param s0     the first path
 ** @param s1     the second path
 **
 ** @return the combined path
 **/
icu::UnicodeString path_combine(const icu::UnicodeString &s0,
                                const icu::UnicodeString &s1);

#define STRING_MAX_SIZE 65536

// C routines
void assert_fail(const char *assertion, const char *file, uint_t line) {
    std::cerr << file << ':' << line << ": assertion failed " << assertion
              << '\n';
    abort();
}

void panic_fail(const char *message, const char *file, uint_t line) {
    std::cerr << file << ':' << line << ": panic " << message << '\n';
    abort();
}

UChar *read_utf8_file(const char *filename) {
    // FIXME: discard BOM when encountered?
    long fsize;
    UFILE *f = u_fopen(filename, "r", NULL, "UTF-8");
    ASSERT(f != NULL);

    fseek(u_fgetfile(f), 0, SEEK_END);
    fsize = ftell(u_fgetfile(f));
    u_frewind(f);

    UChar *str = new UChar[fsize + 1];
    // UChar* str = (UChar*) malloc(sizeof(UChar) * (fsize + 1));

    for (fsize = 0; !u_feof(f); ++fsize) {
        UChar c = u_fgetc(f);
        str[fsize] = c;
    }

    str[fsize] = 0;

    u_fclose(f);

    return str;
}

void write_utf8_file(const char *filename, UChar *str) {
    int32_t size;
    UFILE *f = u_fopen(filename, "w", NULL, "UTF-8");
    ASSERT(f != NULL);
    size = u_fputs(str, f);
    ASSERT(size >= 0);
    u_fclose(f);
}

bool exists_file(const char *filename) {
    // note race conditions when files are created/deleted. whatever...
    return (access(filename, F_OK) != -1);
}

// Unicode routines

char *unicode_to_char(const icu::UnicodeString &str) {
    /* old code - no utf8 conversion
    auto len = str.extract(0, STRING_MAX_SIZE, nullptr, (uint32_t) 0);
    auto buffer = new char[len+1];
    str.extract(0, STRING_MAX_SIZE, buffer, len+1);
    return buffer;
    */

    std::string utf8;
    str.toUTF8String(utf8);
    auto cc0 = utf8.c_str();
    auto len = strlen(cc0);
    auto cc1 = (char *)malloc(len + 1);
    strncpy(cc1, cc0, len + 1);
    return cc1;
}

icu::UnicodeString char_to_unicode(const char *cc) {
    return icu::UnicodeString(cc);
}

UChar *unicode_to_uchar(const icu::UnicodeString &str) {
    UErrorCode error = U_ZERO_ERROR;
    UChar *buffer = new UChar[str.length() + 1];
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

icu::UnicodeString unicode_concat(const icu::UnicodeString &s0,
                                  const icu::UnicodeString &s1) {
    icu::UnicodeString s;
    s += s0;
    s += s1;
    return s;
}

int64_t convert_to_int(const icu::UnicodeString &s) {
    char *buf = unicode_to_char(s);
    auto i = atol(buf);
    delete[] buf;
    return i;
}

int64_t convert_to_hexint(const icu::UnicodeString &s) {
    int i = 0;
    int64_t n = 0;
    UChar32 c = s.char32At(i);
    while (c != 0xffff) {
        n = n * 16;
        if (c >= 48 && c <= 57) {  // 0-9
            n += (((int)(c)) - 48);
        } else if ((c >= 65 && c <= 70)) {  // A-F
            n += (((int)(c)) - 55);
        } else if (c >= 97 && c <= 102) {  // a-f
            n += (((int)(c)) - 87);
        }  // just ignore other chars (like starting 0x)
        i++;
        c = s.char32At(i);
    }
    return n;
}

double convert_to_float(const icu::UnicodeString &s) {
    char *buf = unicode_to_char(s);
    auto f = atof(buf);
    delete[] buf;
    return f;
}

UChar32 convert_to_char(const icu::UnicodeString &s) {
    auto s0 = unicode_strip_quotes(s);
    auto s1 = unicode_unescape(s0);
    return s1.char32At(0);
}

icu::UnicodeString convert_to_text(const icu::UnicodeString &s) {
    auto s0 = unicode_strip_quotes(s);
    auto s1 = unicode_unescape(s0);
    return s1;
}

icu::UnicodeString convert_from_int(const int64_t &n) {
    std::stringstream ss;
    ss << n;
    icu::UnicodeString u(ss.str().c_str());
    return u;
}

icu::UnicodeString convert_from_float(const double &f) {
    std::stringstream ss;
    ss << f;
    icu::UnicodeString u(ss.str().c_str());
    return u;
}

icu::UnicodeString convert_from_char(const UChar32 &c) {
    std::stringstream ss;
    ss << c;
    icu::UnicodeString u(ss.str().c_str());
    auto u1 = unicode_escape(u);
    return u1;
}

icu::UnicodeString convert_from_text(const icu::UnicodeString &s) {
    auto s1 = unicode_escape(s);
    return s1;
}

icu::UnicodeString unicode_strip_quotes(const icu::UnicodeString &s) {
    icu::UnicodeString copy(s);
    return copy.retainBetween(1, copy.length() - 1);
}

icu::UnicodeString unicode_escape(const icu::UnicodeString &s) {
    icu::UnicodeString s1;
    int i = 0;
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

icu::UnicodeString unicode_unescape(const icu::UnicodeString &s) {
    return s.unescape();
}

bool unicode_endswith(const icu::UnicodeString &s,
                      const icu::UnicodeString &suffix) {
    return s.endsWith(suffix);
}

// basic I/O routines

namespace fs = std::filesystem;

fs::path string_to_path(const icu::UnicodeString &s) {
    char *cc = unicode_to_char(s);
    fs::path p = fs::u8path(cc);
    delete[] cc;
    return p;
}

icu::UnicodeString path_to_string(const fs::path &p) {
    icu::UnicodeString s(p.c_str());  // XXX: utf8
    return s;
}

icu::UnicodeString path_absolute(const icu::UnicodeString &s) {
    auto p0 = string_to_path(s);
    auto p1 = fs::absolute(p0);
    return path_to_string(p1);
}

icu::UnicodeString path_combine(const icu::UnicodeString &s0,
                                const icu::UnicodeString &s1) {
    auto p0 = string_to_path(s0);
    auto p1 = string_to_path(s1);
    p0 /= p1;
    return path_to_string(p0);
}

icu::UnicodeString file_read(const icu::UnicodeString &filename) {
    char *fn = unicode_to_char(filename);
    UChar *chars = read_utf8_file(fn);
    delete[] fn;
    icu::UnicodeString str = icu::UnicodeString(chars);
    delete[] chars;
    return str;
}

void file_write(const icu::UnicodeString &filename,
                const icu::UnicodeString &str) {
    char *fn = unicode_to_char(filename);
    UChar *s = unicode_to_uchar(str);
    write_utf8_file(fn, s);
    delete[] fn;
    delete[] s;
}

bool file_exists(const icu::UnicodeString &filename) {
    char *fn = unicode_to_char(filename);
    bool b = exists_file(fn);
    delete[] fn;
    return b;
}
