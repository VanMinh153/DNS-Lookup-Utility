#include <stdbool.h>
#ifndef UTILITY_H
#define UTILITY_H

extern char str_time[30];

bool str_to_port(char* str, int* port);
void clear_stdin();
char* clear_f(char* str);
char* get_time();
char get_last_char(char* str);
bool isalnum_f(char* str);

#endif // UTILITY_H
