ODIR=./build/objs
BDIR=./build/bin

IDIR=./src
SDIR=./src
TDIR=./test

CC=gcc
LINK=gcc
DFLAGS=-g -O0 -fsanitize=address,leak,undefined
RFLAGS=-O3
CFLAGS=-I$(IDIR) -Wall $(DFLAGS)

_SRC=$(wildcard $(SDIR)/*/*.c) $(wildcard $(SDIR)/*.c)
OBJ=$(patsubst $(SDIR)/%.c,$(ODIR)/%.o,$(_SRC))

TEST=$(wildcard $(TDIR)/*.c)

DEPS=$(wildcard $(TDIR)/*.h) $(wildcard $(IDIR)/*.h)

_BIN=test
BIN=$(patsubst %,$(BDIR)/%,$(_BIN))

.PHONY: all
all: $(BIN)

$(BDIR)/test: $(OBJ) $(TEST)
	mkdir -p `dirname $@`
	$(LINK) $(CFLAGS) -o $@ $^ $(LIBS)

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -c -o $@ $<
	
.PHONY: new
new: clean all
	
.PHONY: clean
clean:
	rm -fr $(ODIR)/*

.PHONY: cleanall
cleanall:
	rm -fr $(ODIR)/* $(BDIR)/*

.PHONY: check
check: all
	./build/bin/test
