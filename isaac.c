/*
 * isaac - cryptographically secure pseudo-random bytestream generator using
 * the ISAAC (Indirection, Shift, Accumulate, Add and Count) algorithm.
 *
 * Version 2025.236.1
 * Copyright (c) 2024-2025 Guenther Brunthaler. All rights reserved.
 *
 * This source file is free software.
 * Distribution is permitted under the terms of the GPLv3.
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>

#if UINT_MAX >= 0xffffffff
   typedef unsigned int ufast32;
#else
   typedef unsigned long ufast32;
#endif

#define DIM(array) (sizeof (array) / sizeof *(array))

int main(int argc, char **argv) {
   /* ISAAC. */
   ufast32 a; /* Entropy acumulator. */
   ufast32 b; /* Last result. */
   ufast32 c; /* Counter, guarantees minimum cycle length. */
   static ufast32 s[256]; /* Internal state or seed. */
   static ufast32 r[256]; /* Result. Next batch of generated random numbers. */
   static unsigned char be[4 * 256]; /* Big endian packed I/O buffer. */
   (void)argv;
   if (argc > 1 || fread(be, 1, sizeof s, stdin) != sizeof be) {
      (void)fprintf(
            stderr
         ,  "Feed exactly 1024 bytes as binary random seed"
            " via standard input!\n"
            "\n"
            "Then read as many pseudorandom bytes as needed from standard"
            " output (which should be a pipe) and close the pipe when done."
            " Be sure to check the return status of this program in order"
            " to dectect any errors.\n"
      );
      failed: return EXIT_FAILURE;
   }
   {
      int i, j;
      for (i = j = 0; i < (int)DIM(be); ++i) {
         ufast32 v;
         if (!(i & 3)) v = 0; else v <<= 8;
         v += be[i];
         if ((i & 3) == 3) s[j++] = v;
      }
   }
   a = b = c = 0;
   for (;;) {
      int i;
      b += ++c;
      for (i = 0; i < 256; ++i) {
         static int lshift[4] = {13, -6, 2, -16};
         int lshift_bits = lshift[i & 3];
         a ^= (lshift_bits > 0 ? a << lshift_bits : a >> -lshift_bits);
         a += s[(i + 128) & 0xff];
         {
            int x, y;
            x = s[i];
            y = s[i] = a + b + s[x >> 2 & 0xff];
            r[i] = b = x + s[y >> 10 & 0xff];
         }
      }
      {
         int i, j;
         for (i = j = 0; i < (int)DIM(be); ++i) {
            ufast32 v;
            if (!(i & 3)) v = r[j++];
            be[(i & ~3) + (3 - (i & 3))] = v & 0xff;
            v >>= 8;
         }
      }
      if (fwrite(be, 1, sizeof be, stdout) != sizeof be) {
         int e = errno;
         if (ferror(stdout) && e == EPIPE) break;
         goto failed;
      }
   }
}
