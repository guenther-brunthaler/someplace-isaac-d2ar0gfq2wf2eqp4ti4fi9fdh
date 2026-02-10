char const SHORT_DESCRIPTION[] = \
   "isaac - cryptographically secure pseudo-random bytestream generator\n"
   "using the ISAAC (Indirection, Shift, Accumulate, Add and Count)"
   " algorithm.\n"
;
char const VERSION[] = \
   "Version 2026.41\n"
   "Copyright (c) 2024-2026 Guenther Brunthaler. All rights reserved.\n"
   "\n"
   "This source file is free software.\n"
   "Distribution is permitted under the terms of the GPLv3.\n"
;

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>

#if CHAR_BIT != 8
   #error "Sorry, but this implementation only works with 8-bit bytes!"
#endif

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

/* The array size (of 32 bit words) ISAAC uses for its input and result. */
#define SIZE 256

int main(int argc, char **argv) {
   /* Little endian packed I/O buffer. */
   static unsigned char le[32 / CHAR_BIT * SIZE];
   (void)argv;
   if (argc > 1 || fread(le, 1, sizeof le, stdin) != sizeof le) {
      (void)fprintf(
            stderr
         ,  "%s\n"
            "Feed exactly 1024 octets (= 8-bit bytes) as binary"
            " random seed via standard input!\n"
            "\n"
            "Then read as many cryptographically strong pseudorandom"
            " octets as needed from this program's standard output"
            " (which should be a pipe) and stop reading (or close the"
            " pipe) when done.\n"
            "\n"
            "Be sure to check the return status of this program in"
            " order to dectect any errors.\n"
            "\n"
            "The seed must contain enough entropy for your"
            " application's security requirements.\n"
            "\n"
            "If you use this program for hashing the seed, throw away"
            " the first 8 blocks of 1024 octets because they might"
            " not be very random yet. The generator needs some time"
            " to 'warm up' in this case. This is not necessary if the"
            " seed already has high entropy.\n"
            "\n"
            "This implementation will interpret the seed as an array"
            " of unsigned little-endian 32-bit integers before"
            " passing them on to the ISAAC algorithm, and the"
            " algorithm's output will be written to standard output"
            " in the same format.\n"
            " \n%s", SHORT_DESCRIPTION, VERSION
      );
      failed: return EXIT_FAILURE;
   }
   {
      static ufast32 s[SIZE]; /* Internal state or seed. */
      {
         /* Little-endian le[] to native-endian s[] input conversion. */
         int i, j;
         ufast32 v;
         for (i = j = 0; i < (int)DIM(le); ++i) {
            if ((i & 3) == 0) v = 0;
            v |= le[i] << ((i & 3) << 3);
            if ((i & 3) == 3) s[j++] = v;
         }
      }
      /* ISAAC. */
      {
         ufast32 a; /* Entropy acumulator. */
         ufast32 b; /* Last result. */
         ufast32 c; /* Counter, guarantees minimum cycle length. */
         /* Result. Next batch of generated random numbers. */
         static ufast32 r[SIZE];
         for (a = b = c = 0; ; ) {
            int i;
            c = LOWER32(c + 1);
            b = LOWER32(b + c);
            for (i = 0; i < SIZE; ++i) {
               static int lshift[4] = {13, -6, 2, -16};
               int lshift_bits = lshift[i & 3];
               assert(a <= BITMASK_32);
               a ^= (
                  lshift_bits > 0
                  ? LOWER32(a << lshift_bits)
                  : a >> -lshift_bits
               );
               assert(a <= BITMASK_32);
               a = LOWER32(a + s[LOWER8(i + (SIZE >> 1))]);
               {
                  ufast32 x, y;
                  assert(s[i] <= BITMASK_32);
                  y = s[i] = LOWER32(a + b + s[LOWER8((x = s[i]) >> 2)]);
                  b = r[i] = LOWER32(x + s[LOWER8(y >> 10)]);
               }
            }
            {
               /* Native-endian r[] to little-endian le[] output conversion. */
               int i, j;
               ufast32 v;
               for (i = j = 0; i < (int)DIM(le); ++i) {
                  if ((i & 3) == 0) v = r[j++];
                  le[i] = LOWER8(v);
                  v >>= 8;
               }
            }
            if (fwrite(le, 1, sizeof le, stdout) != sizeof le) {
               int e = errno;
               if (ferror(stdout) && e == EPIPE) break;
               goto failed;
            }
         }
      }
   }
}
