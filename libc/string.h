#ifndef STRINGS_H
#define STRINGS_H

#include <stddef.h>

void int_to_ascii(int n, char str[]);
void hex_to_ascii(int n, char str[]);
void reverse(char s[]);
size_t strlen(char s[]);
void str_pop_back(char s[]);
void append(char s[], char n);
int strcmp(char s1[], char s2[]);
int strncmp(const char *s1, const char *s2, size_t n);
void strcpy(char *dest, char *src);
char tolower(char c);
int strcmpi(char s1[], char s2[]);

#endif
