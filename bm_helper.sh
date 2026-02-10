#! /bin/false

# Do not run directly. Invoke via "make benchmark" instead.
#
# Append benchmark results to <RESULT_COLLECTION>.
#
# v2026.41

BENCH_SRC=isaac.c
RESULT_COLLECTION=timings.txt
SEEDSZ=1024
BLKSZ=102400
BLKS=1024000

usch='tag:xworld.mine.nu,2009:'
r1=$usch'usr-local-bin-xworld-jv3gwuidf2ezyr5vbqavqtxyh.git'
external_helpers="times2ms:$r1 lsb_release uname cksum date tr tee awk file"

# No need to modify below this line.
unset LC_ALL # Avoid overriding LC_NUMERIC.
exec 8> /dev/null
ok=true
for tool in $external_helpers
do
	uri=${tool#*:}
	if test "$uri" != "$tool"
	then
		tool=${tool%":$uri"}
	else
		uri=
	fi
	command -v "$tool" >& 8 2>& 1 && continue
	echo; echo "You need the '$tool' utility in your \$PATH!"
	if test "$uri"
	then
		echo "Get it from the Git repository with abstract URI"
		echo
		echo "$uri"
		echo
		echo "which probably resolves to the URL of some currently"
		echo "popular free hosting service for Git."
	fi
	ok=false
done >& 2
$ok || exit
export LC_NUMERIC=C # Unclear why this is needed.
self=`basename -- "$0"`
bench_exe=${BENCH_SRC%.*}
test -f "$bench_exe" || exit
(
	dd if=/dev/zero bs=$SEEDSZ count=1 2>& 8 \
	| ./"$bench_exe" \
	| dd bs=$BLKSZ count=$BLKS 2>& 8 \
	>& 8
	times
) \
| times2ms \
| {
	cat
	date -u +'dt=%Y%m%dz%H%M%S'
	printf env=
	{
		# Platform hash.
		lsb_release -a 2>& 8
		uname -m
		file --brief ./"$bench_exe" \
		| sed 's/\( E\{0,1\}ABI[0-9]*\).*/\1/; t; d'
	} \
	| cksum \
	| awk -v m=4 '{
		if ((l = n = length($1)) > m) n = m
		print substr($1, 1 + l - n)
	}'
	printf src= && cksum "$BENCH_SRC" | cut -d ' ' -f 1-2 | tr ' ' :
	printf bm= && cksum "$self" | cut -d ' ' -f 1-2 | tr ' ' :
} \
| tr '\n' ' ' \
| tee -a "$RESULT_COLLECTION"
exec 8>& -
