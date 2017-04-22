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

typedef std::vector<UnicodeString> UnicodeStrings;
// unicode routines

/** 
 ** Converts a UnicodeString to a character array.
 **
 ** @param str  the string
 **
 ** @return a character array (heap allocated)
 **/
char* unicode_to_char(const UnicodeString &str);

/** 
 ** Converts a UnicodeString to a UChar array.
 **
 ** @param str  the string
 **
 ** @return a UChar array (heap allocated)
 **/
UChar* unicode_to_uchar(const UnicodeString &str);

/**
 ** Converts an unsigned int to Unicode.
 **
 ** @param n  the number
 **
 ** @return a Unicode string
 **/
UnicodeString unicode_convert_uint(uint_t n);

/**
 ** Concatenate two unicode strings
 **
 ** @param s0  the first unicode string
 ** @param s1  the second unicode string
 **
 ** @return the concatenation, a Unicode string
 **/
UnicodeString unicode_concat(const UnicodeString& s0, const UnicodeString& s1);

/**
 ** Strip the leading and trailing quote of a Unicode string
 **
 ** @param s  the unicode string
 **
 ** @return a stripped string
 **/
UnicodeString unicode_strip_quotes(const UnicodeString& s);

/**
 ** Escape certain characters in a Unicode string.
 **
 ** @param s  the unicode string
 **
 ** @return the escaped string
 **/
UnicodeString unicode_escape(const UnicodeString& s);

/**
 ** Unescape certain characters in a Unicode string.
 **
 ** @param s  the unicode string
 **
 ** @return the unescaped string
 **/
UnicodeString unicode_unescape(const UnicodeString& s);

// convenience routines text to, and from, literals

/**
 ** Parse and convert an integer. like 42.
 **
 ** @param s  the unicode string
 **
 ** @return the integer recognized
 **/
uint64_t convert_to_int(const UnicodeString& s);

/**
 ** Parse and convert a float. like 3.14.
 **
 ** @param s  the unicode string
 **
 ** @return the integer recognized
 **/
double convert_to_float(const UnicodeString& s);

/**
 ** Parse and convert a char string. like 'a'.
 **
 ** @param s  the unicode string
 **
 ** @return the char recognized
 **/
UChar32 convert_to_char(const UnicodeString& s);

/**
 ** Parse and convert a float. like "hello!".
 **
 ** @param s  the unicode string
 **
 ** @return the integer recognized
 **/
UnicodeString convert_to_text(const UnicodeString& s);

// convenience io-routines

/** 
 ** Convenience. Reads a file into a string from a given filename.
 **
 ** @param filename filename
 **
 ** @return the contents of the file
 **/
UnicodeString file_read(const UnicodeString &filename);

/** 
 ** Convenience. Writes a string to a file.
 **
 ** @param filename     the filename
 ** @param str          the string
 **/
void file_write(const UnicodeString &filename, const UnicodeString &str);

/** 
 ** Convenience. Checks whether a given file exists. (Note the possible race conditions.)
 **
 ** @param filename     the filename
 **
 ** @return true iff the file exists
 **/
bool file_exists(const UnicodeString &filename);

/** 
 ** Convenience. Combine two paths into a new path. OS specific.
 **
 ** @param p0     the first path
 ** @param p1     the second path
 **
 ** @return the combined path
 **/
UnicodeString path_combine(const UnicodeString& p0, const UnicodeString& p1);

#endif
