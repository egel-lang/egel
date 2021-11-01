#ifndef UTILS_HPP
#define UTILS_HPP

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include "unicode/ustdio.h"
#include "unicode/uchar.h"
#include "unicode/unistr.h"
#include "unicode/ustream.h"
#include "unicode/stringpiece.h"

typedef unsigned int uint_t;

// debugging macros

void assert_fail (const char *assertion, const char *file, uint_t line);
void panic_fail (const char *message, const char *file, uint_t line);

/** 
 * ASSERT macro. Always compiled assertions.
 */
#define ASSERT(_e) \
    ( (_e)?(void)0: assert_fail(#_e, __FILE__, __LINE__) );

/** 
 * PANIC macro. Aborts with a message (e.g., for non-reachable statements).
 */
// play nice with runtime.hpp
#ifdef PANIC
#undef PANIC
#endif

#define PANIC(_m) \
    ( panic_fail(_m, __FILE__, __LINE__) );

/** 
 * DEBUG macro. An assertion only compiled in for debug builds.
 */
#if DEBUG
#define DEBUG_ASSERT(_e) \
    ( (_e)?(void)0: assert_fail(#_e, __FILE__, __LINE__) );
#else
#define DEBUG_ASSERT(_e)           ((void)0);
#endif

typedef std::vector<icu::UnicodeString> UnicodeStrings;
// unicode routines

/** 
 ** Converts a icu::UnicodeString to a character array.
 **
 ** @param str  the string
 **
 ** @return a character array (heap allocated)
 **/
char* unicode_to_char(const icu::UnicodeString &str);

/** 
 ** Converts a icu::UnicodeString to a UChar array.
 **
 ** @param str  the string
 **
 ** @return a UChar array (heap allocated)
 **/
UChar* unicode_to_uchar(const icu::UnicodeString &str);

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
icu::UnicodeString unicode_concat(const icu::UnicodeString& s0, const icu::UnicodeString& s1);

/**
 ** Strip the leading and trailing quote of a Unicode string
 **
 ** @param s  the unicode string
 **
 ** @return a stripped string
 **/
icu::UnicodeString unicode_strip_quotes(const icu::UnicodeString& s);

/**
 ** Escape certain characters in a Unicode string.
 **
 ** @param s  the unicode string
 **
 ** @return the escaped string
 **/
icu::UnicodeString unicode_escape(const icu::UnicodeString& s);

/**
 ** Unescape certain characters in a Unicode string.
 **
 ** @param s  the unicode string
 **
 ** @return the unescaped string
 **/
icu::UnicodeString unicode_unescape(const icu::UnicodeString& s);

/**
 ** Determine if a string ends with a certain suffix.
 **
 ** @param s  the unicode string to be searched
 ** @param sf the unicode string suffix
 **
 ** @return the unescaped string
 **/
bool unicode_endswith(const icu::UnicodeString& s, const icu::UnicodeString& sf);

// convenience routines text to, and from, literals

/**
 ** Parse and convert an integer. like 42.
 **
 ** @param s  the unicode string
 **
 ** @return the integer recognized
 **/
int64_t convert_to_int(const icu::UnicodeString& s);

/**
 ** Parse and convert a hexadecimal integer. like 0xa3.
 **
 ** @param s  the unicode string
 **
 ** @return the integer recognized
 **/
int64_t convert_to_hexint(const icu::UnicodeString& s);

/**
 ** Parse and convert a float. like 3.14.
 **
 ** @param s  the unicode string
 **
 ** @return the integer recognized
 **/
double convert_to_float(const icu::UnicodeString& s);

/**
 ** Parse and convert a char string. like 'a'.
 **
 ** @param s  the unicode string
 **
 ** @return the char recognized
 **/
UChar32 convert_to_char(const icu::UnicodeString& s);

/**
 ** Parse and convert an integer. like 42.
 **
 ** @param s  the unicode string
 **
 ** @return the integer recognized
 **/
icu::UnicodeString convert_from_int(const int64_t& s);

/**
 ** Parse and convert a float. like 3.14.
 **
 ** @param s  the unicode string
 **
 ** @return the integer recognized
 **/
icu::UnicodeString convert_from_float(const double& s);

/**
 ** Parse and convert a char string. like 'a'.
 **
 ** @param s  the unicode string
 **
 ** @return the char recognized
 **/
icu::UnicodeString convert_from_char(const UChar32& s);

/**
 ** Parse and convert a text string. like "hello!".
 **
 ** @param s  the unicode string
 **
 ** @return the integer recognized
 **/
icu::UnicodeString convert_to_text(const icu::UnicodeString& s);

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
void file_write(const icu::UnicodeString &filename, const icu::UnicodeString &str);

/** 
 ** Convenience. Checks whether a given file exists. (Note the possible race conditions.)
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
icu::UnicodeString path_absolute(const icu::UnicodeString& s);

/** 
 ** Convenience. Combine two paths into a new path. OS specific.
 **
 ** @param s0     the first path
 ** @param s1     the second path
 **
 ** @return the combined path
 **/
icu::UnicodeString path_combine(const icu::UnicodeString& s0, const icu::UnicodeString& s1);

#endif
