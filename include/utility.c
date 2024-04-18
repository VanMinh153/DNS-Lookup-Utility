#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include "utility.h"

/*
@function: Convert a string to port number available for user server applications.
          Available port number for user server applications is [1024, 49151]
@parameter [IN] str: a string containing port number
           [OUT] port: port number retrieve from str
@return: true if success
         false if fail
*/
bool str_to_port(char *str, int *port) {
  int n = 0;
  char overcheck = '\0';
  sscanf(str, "%5d %c", &n, &overcheck);
  if (overcheck != '\0') return false;
  if (n >= 1024 && n <= 49151) {
    *port = n;
    return true;
  }
  return false;
}
// Clear stdin buffer
void clear_stdin() {
  char chr = '\0';
  do {
    chr = getchar();
  } while (chr != EOF && chr != '\n');
  return;
}
/*
@function: Removes whitespace at the beginning and end of a string
@parameter [IN]: a string will to handle
@return: pointer to string after handle
         NULL if string only has space or tab character
*/
char *clear_f(char *str) {
  if (str == NULL) return NULL;
  if (strlen(str) == 0) return str;

  char *p1 = str;
  char *p2 = str + strlen(str) - 1;  // p2 points to the last character of str

  while (p1 < str + strlen(str) && (*p1 == ' ' || *p1 == '\t' || *p1 == '\n'))
    p1++;
  while (p2 > str && (*p2 == ' ' || *p2 == '\t' || *p2 == '\n'))
    p2--;

  if (p1 > p2) {
    return p1;
  }
  p2++;
  *p2 = '\0';
  return p1;
}
// get current time. Support for make log_file
char time_str[30];
char *get_time() {
    time_t current_time;
    struct tm *time_st;
    time(&current_time);
    time_st = localtime(&current_time);
    strftime(time_str, sizeof(time_str), "[%d/%m/%Y %H:%M:%S]", time_st);
    return time_str;
}
// Get last character of a string
char get_last_char(char *str) {
  if (str == NULL) return '\0';
  if (strlen(str) == 0) return '\0';
  else return str[strlen(str) - 1];
}
// Check if all characters in a string are alphanumeric
bool isalnum_f(char *str) {
  for (int i = 0; i < strlen(str); i++) {
    if (!isalnum(str[i])) return false;
  }
  return true;
}