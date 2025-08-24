/*
 * Measure all "C"-89 built-in unsigned standard data types and determine
 * whether they use little-endian byte-ordering, big-endian byte ordering or
 * something else.
 *
 * Version 2023.270
 * Copyright (c) 2023 Guenther Brunthaler. All rights reserved.
 *
 * This source file is free software.
 * Distribution is permitted under the terms of the GPLv3.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>

static void die(char const *err_msg) {
   int e = errno;
   (void)fprintf(stderr, "%s\n", err_msg);
   if (e) (void)fprintf(stderr, "Error # %d: %s\n", e, strerror(e));
   exit(EXIT_FAILURE);
}

static void write_error(void) {
   die("Write error!");
}

static void puts_ck(char const *s) {
   if (puts(s) < 0) write_error();
}

int main(void) {
   union overlay {
      unsigned long v;
      unsigned char b[sizeof(unsigned long)];
   } u;
   u.v = 1;
   while (u.v) {
      int byte;
      for (byte = (int)(unsigned)sizeof u.b; byte--; ) {
         int bit;
         unsigned bmask = (unsigned)UCHAR_MAX & ~((unsigned)UCHAR_MAX >> 1);
         for (bit = CHAR_BIT; bit--; ) {
            if (u.b[byte] == bmask) {
            }
         }
      }
      u.v += u.v;
   }
   if (fflush(0)) write_error();
   cleanup();
}
// WORDS_BIGENDIAN
