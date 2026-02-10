.POSIX:
# v2026.41

# You may redefine these settings via the "make" command line or export them
# as environment variables and (in the latter case only) also
# "export MAKEFLAGS=e". Or just edit this file.

CFLAGS = -D NDEBUG -O
LDFLAGS = -s

# No need to redefine these.
TARGETS = isaac endian_swap32

.PHONY: all clean test

all: $(TARGETS)

clean:
	-rm $(TARGETS)

test: isaac
	@if \
		dd if=/dev/zero bs=1024 count=1 2> /dev/null \
		| ./isaac \
		| dd bs=1024 count=1024 2> /dev/null \
		| cksum \
		| cmp -s - expected_test_results.txt; \
	then \
		echo "Test SUCCESSFUL!"; \
	else \
		echo "Test FAILED!"; \
	fi

benchmark:
	@make -s clean
	@(unset CPPFLAGS CFLAGS LDFLAGS && make -s)
	@sh bm_helper.sh
