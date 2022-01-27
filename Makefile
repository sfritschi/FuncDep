CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -std=c11 -march=native -D_POSIX_C_SOURCE

TARGET=func_dep
SOURCE=src
HEADER=include

.PHONY: all, debug, clean
all=${TARGET}

${TARGET}: ${SOURCE}/${TARGET}.c
	${CC} -O3 -DNDEBUG ${CFLAGS} -o $@ $^ -I${HEADER}/

debug: ${SOURCE}/${TARGET}.c
	${CC} -g ${CFLAGS} -o $@ $^ -I${HEADER}/

clean:
	${RM} ${TARGET}
	${RM} debug
