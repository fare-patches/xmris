#include <stdio.h>
char *win_pref_file(const char *name);
int win_lock(FILE *stream, int acquire);
int win_replace(const char *source, const char *destination);
