/* v2025.236 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>

#if CHAR_BIT != 8
   #error "Sorry, but this implementation only works with 8-bit bytes!"
#endif

#define DIM(array) (sizeof(array) / sizeof *(array))
#define IDIM(array) ((int)(unsigned)DIM(array))

int main(void) {
   int i;
   for (i = 0; ; ++i) {
      static char buf[32 / CHAR_BIT];
      if (i < IDIM(buf)) {
         int c;
         if ((c = getchar()) == EOF) {
            if (ferror(stdin)) {
               perror("Read error");
               failed:
               return EXIT_FAILURE;
            }
            assert(feof(stdin));
            if (i != 0) {
               (void)fputs(
                  "Unexpected trailing input octets have not been converted"
                  " and are lost!\n"
                  , stderr
               );
            }
            break;
         }
         buf[i] = (char)c;
      } else {
         if (putchar(buf[(IDIM(buf) << 1) - 1 - i]) == EOF) {
            #ifdef EPIPE
               if (errno == EPIPE) break;
            #endif
            wr_err: perror("Write error");
            goto failed;
         }
         if (i == (IDIM(buf) << 1) - 1) i = -1;
      }
   }
   if (fflush(NULL)) goto wr_err;
}
