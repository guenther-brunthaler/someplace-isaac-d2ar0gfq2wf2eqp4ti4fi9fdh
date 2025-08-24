/*
 * ISAAC as described in the PDF by Jean-Philippe Aumasson 2007-01
 *
 * ANSI-C implementation:
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

#if UINT_MAX >= 0xfffffffful
   typedef unsigned int uword32;
   #define UWORD32_C(c) c##u
   #define PRId_UWORD32 "%u"
   #define PRIx_UWORD32 "%x"
#elif ULONG_MAX >= 0xfffffffful
   typedef unsigned long uword32;
   #define UWORD32_C(c) c##ul
   #define PRId_UWORD32 "%lu"
   #define PRIx_UWORD32 "%lx"
#else
   #error "An ANSI/ISO C89-compliant C compiler is required!"
#endif

typedef struct {
   uword32 a; /* Entropy accumulator. */
   uword32 b; /* Last output word. */
   uword32 c; /* Cyclic counter. */
   uword32 s[256];
} isaac_state;

static void isaac(isaac_state *prng, uword32 (*result)[256]) {
   uword32 a = prng->a, b = prng->b, *s = prng->s;
   int i;
   for (i = 0; i < 256; ++i) {
      static int lshift[4] = {13, -6, 2, -16};
      int lshift_bits = lshift[i & 3];
      a ^= (lshift_bits > 0 ? a << lshift_bits : a >> -lshift_bits);
      a += s[i + 128 & 0xff];
      int x = s[i];
      int y = s[i] = a + b + s[x >> 2 & 0xff];
      (*result)[i] = b = x + s[y >> 10 & 0xff];
   }
   prng->a = a; prng->b = b;
}

static void tea_encrypt(uword32 v[2], uword32 const k[4]) {
   uword32 v0 = v[0], v1 = v[1], sum = 0, i;
   uword32 const delta = UWORD32_C(0x9e3779b9);
   uword32 const k0 = k[0], k1 = k[1], k2 = k[2], k3 = k[3];
   for (i = 0; i < 32; ++i) {
      sum += delta;
      v0 += (v1 << 4) + k0 ^ v1 + sum ^ (v1 >> 5) + k1;
      v1 += (v0 << 4) + k2 ^ v0 + sum ^ (v0 >> 5) + k3;
   }
   v[0] = (uword32)v0;
   v[1] = (uword32)v1;
}

static uword32 xorshift32(uword32 x32) {
   x32 ^= x32 << 13;
   x32 ^= x32 >> 17;
   return x32 ^= x32 << 5;
}

static void cleanup(void);

static void die(char const *err_msg) {
   int e = errno;
   (void)fprintf(stderr, "%s\n", err_msg);
   if (e) (void)fprintf(stderr, "Error # %d: %s\n", e, strerror(e));
   cleanup();
   exit(EXIT_FAILURE);
}

static void *realloc_ck(void *old, size_t new_size) {
   void *p;
   if (!(p = realloc(old, new_size))) {
      die(
         old ? "Could not resize memory buffer!"
         : "Could not allocate memory buffer!"
      );
   }
   return p;
}

/* Allocates a memory block of size <bytes> and returns the address of a
 * pointer variable (will also be allocated) pointing to the block. The
 * pointer variable will be registered in a global list. When the function is
 * called with <bytes> == 0, all registered pointer variables are examined and
 * the buffers they currently point to will be released. The associated
 * pointer variable registrations will also be released. A registered pointer
 * variable may also contain a null pointer: This indicates that the buffer
 * has already been released. */
static void **tracked_malloc(size_t bytes) {
   struct buffer_dsc {
      struct buffer_dsc *link;
      void *buffer;
   } *r;
   static struct buffer_dsc *rlist;
   if (bytes) {
      r = realloc_ck(0, sizeof *r);
      r->link = rlist;
      rlist = r;
      r->buffer = realloc_ck(r->buffer = 0, bytes);
      return &r->buffer;
   }
   while (r = rlist) {
      rlist = r->link;
      free(r->buffer);
      free(r);
   }
   return 0;
}

static void cleanup(void) {
   (void)tracked_malloc(0);
}

static void write_error(void) {
   die("Write error!");
}

static void printf_ck(char const *format, ...) {
   va_list args;
   va_start(args, format);
   if (vprintf(format, args) < 0) write_error();
   va_end(args);
}

typedef struct isaac_prng isaac_prng;

struct isaac_prng {
   uword32 out[256];
   int i;
};

static isaac_prng *new_prng(void) {
   return *tracked_malloc(sizeof(isaac_prng));
}

static void isaac_init(isaac_prng *rng, void const *seed, size_t seed_bytes) {
}

static uword32 isaac_rand(isaac_prng *rng) {
}

int main(void) {
  if (fflush(0)) write_error();
  cleanup();
}
