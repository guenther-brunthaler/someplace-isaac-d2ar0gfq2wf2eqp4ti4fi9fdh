/*
 * isaac - cryptographically secure pseudo-random bytestream generator using
 * the ISAAC (Indirection, Shift, Accumulate, Add and Count) algorithm.
 *
 * Version 2025.236.3
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

#define BITMASK_8 0xffu
#define LOWER8(x) ((x) & BITMASK_8)

#define BITMASK_32 0xffffffffu
#if UINT_MAX >= BITMASK_32
   typedef unsigned int ufast32;
   #define UFAST32_MAX UINT_MAX
#elif ULONG_MAX >= BITMASK_32
   typedef unsigned long ufast32;
   #define UFAST32_MAX ULONG_MAX
#else
   #error "C89 requires <unsigned long> to be at least 32 bit wide!"
#endif

/* Don't trust the compiler to optimize useless masking away. Some compilers
\* are more clever than others. And some compilers are embarrassingly dumb. */
#if UFAST32_MAX == BITMASK_32
   #define LOWER32(x) (x)
#else
   #define LOWER32(x) ((x) & BITMASK_32)
#endif

#define DIM(array) (sizeof (array) / sizeof *(array))

int main(int argc, char **argv) {
   /* ISAAC. */
   static unsigned char be[4 * 256]; /* Big endian packed I/O buffer. */
   (void)argv;
   if (argc > 1 || fread(be, 1, sizeof be, stdin) != sizeof be) {
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
      static ufast32 s[256]; /* Internal state or seed. */
      {
         int i, j;
         for (i = j = 0; i < (int)DIM(be); ++i) {
            ufast32 v;
            if (!(i & 3)) v = 0; else v <<= 8;
            v += be[i];
            if ((i & 3) == 3) s[j++] = v;
         }
      }
      {
         ufast32 a; /* Entropy acumulator. */
         ufast32 b; /* Last result. */
         ufast32 c; /* Counter, guarantees minimum cycle length. */
         /* Result. Next batch of generated random numbers. */
         static ufast32 r[256];
         for (a = b = c = 0; ; ) {
            int i;
            c = LOWER32(c + 1);
            b = LOWER32(b + c);
            for (i = 0; i < 256; ++i) {
               static int lshift[4] = {13, -6, 2, -16};
               int lshift_bits = lshift[i & 3];
               assert(a <= BITMASK_32);
               a ^= (
                  lshift_bits > 0
                  ? LOWER32(a << lshift_bits)
                  : a >> -lshift_bits
               );
               assert(a <= BITMASK_32);
               a = LOWER32(a + s[LOWER8(i + 128)]);
               {
                  ufast32 x, y;
                  assert(s[i] <= BITMASK_32);
                  y = s[i] = LOWER32(a + b + s[LOWER8((x = s[i]) >> 2)]);
                  b = r[i] = LOWER32(x + s[LOWER8(y >> 10)]);
               }
            }
            {
               int i, j;
               for (i = j = 0; i < (int)DIM(be); ++i) {
                  ufast32 v;
                  if (!(i & 3)) v = r[j++];
                  be[(i & ~3) + (3 - (i & 3))] = LOWER8(v);
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
   }
}
