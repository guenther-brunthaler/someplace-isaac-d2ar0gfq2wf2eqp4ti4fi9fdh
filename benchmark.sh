#! /bin/sh
# v2025.236.2

BENCH_SRC=isaac.c
RESULT_COLLECTION=timings.txt
SEEDSZ=1024
BLKSZ=102400
BLKS=1024000

usch='tag:xworld.mine.nu,2009:'
r1=$usch'usr-local-bin-xworld-jv3gwuidf2ezyr5vbqavqtxyh.git'
external_helpers="times2ms:$r1"

# No need to modify below this line.
export LC_NUMERIC=C # Unclear why this is needed.
unset LC_ALL # Avoid overriding LC_NUMERIC.
self=`basename -- "$0"`
bench_exe=${BENCH_SRC%.*}
exec 8> /dev/null
for tool in $external_helpers
do
	uri=${tool#*:}
	tool=${tool%":$uri"}
	command -v "$tool" >& 8 2>& 1 && continue
	echo "You need the '$tool' utility in your \$PATH!"
	echo "Get it from the Git repository with abstract URI"
	echo
	echo "$uri"
	echo
	echo "which probably resolves to the URL of some currently"
	echo "popular free hosting service for Git."
	false || exit
done >& 2
make -s "$bench_exe"
(
	dd if=/dev/zero bs=$SEEDSZ count=1 2>& 8 \
	| ./"$bench_exe" \
	| dd bs=$BLKSZ count=$BLKS 2>& 8 \
	>& 8
	times
) | times2ms \
| { tr '\n' ' ' && date -u +'%Y%m%dz%H%M%S'; } \
| { tr '\n' ' ' && cksum "$BENCH_SRC"; } \
| { tr '\n' ' ' && cksum "$self"; } \
| tee -a "$RESULT_COLLECTION"
exec 8>& -
