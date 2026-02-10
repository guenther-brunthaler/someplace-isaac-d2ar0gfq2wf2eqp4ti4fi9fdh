#! /bin/sh

# Filter removing ZEROTEST-related #ifdefs. An "#if 0" must be present.
#
# v2026.41
sed -e '/^[[:space:]]*#ifdef ZEROTEST/,/^[[:space:]]*#endif/d' \
| sed '/^[[:space:]]*#if 0/,/^[[:space:]]*$/d'
