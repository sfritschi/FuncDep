CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -std=c11 -march=native -D_POSIX_C_SOURCE

TARGET=func_dep
SOURCE=src
HEADER=include

.PHONY: all, clean
all=${TARGET}

${TARGET}: ${SOURCE}/${TARGET}.c
	${CC} ${CFLAGS} -o $@ $^ -I${HEADER}/

clean:
	${RM} ${TARGET}
