CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -std=c11 -D_POSIX_C_SOURCE -mpopcnt
RELEASE_FLAGS=-O3 -DNDEBUG
DEBUG_FLAGS=-ggdb3

TARGET=func_dep
INCDIR=include
SRCDIR=src
OBJDIR=bin

SRC=$(wildcard $(SRCDIR)/*.c)
OBJ=$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))

.PHONY: all, debug, clean
all: $(TARGET)

all:   CFLAGS+=$(RELEASE_FLAGS)
debug: CFLAGS+=$(DEBUG_FLAGS)
debug: $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $^ -o $@
	
$(TARGET): $(OBJ)
	$(CC) $^ -o $@
	
$(OBJDIR):
	mkdir -p $@

clean:
	$(RM) $(TARGET)
	$(RM) debug
	$(RM) -r $(OBJDIR)
