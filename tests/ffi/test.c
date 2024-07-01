#include <stdio.h>
#include <stdbool.h>

void id_void() {
    printf("id_void\n");
};

bool id_bool(bool b) {
    printf("id_bool: '%d'\n", b);
    return b;
};

char id_char(char c) {
    printf("id_char: '%c'\n", c);
    return c;
};

char id_byte(char b) {
    printf("id_byte: '%d'\n", b);
    return b;
};

short id_short(short n) {
    printf("id_short: '%d'\n", n);
    return n;
};

int id_int(int n) {
    printf("id_int: '%d'\n", n);
    return n;
};

long id_long(long n) {
    printf("id_long: '%ld'\n", n);
    return n;
};

long long id_long_long(long long n) {
    printf("id_long_long: '%lld'\n", n);
    return n;
};

size_t id_size_t(size_t s) {
    printf("id_size_t: '%zu'\n", s);
    return s;
};

ssize_t id_ssize_t(ssize_t s) {
    printf("id_ssize_t: '%zd'\n", s);
    return s;
};

void* id_void_p(void* p) {
    printf("id_void_p: '%p'\n", p);
    return p;
};

char* id_char_p(char* cc) {
    printf("id_char_p: '%s'\n", cc);
    return cc;
};

