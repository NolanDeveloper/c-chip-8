#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

extern void
die(const char * format, ...) {
    va_list vargs;
    va_start(vargs, format);
    vfprintf(stderr, format, vargs);
    fprintf(stderr, ".\n");
    va_end(vargs);
    exit(1);
}
