TARGET=csim
CC=gcc
CFLAGS=-Wall -std=c99 -O3 -D_POSIX_C_SOURCE=199309L
LDFLAGS=-lm
SRC_MAIN=main.c
SRC_UTILS=utils.c
SRC_SIM=simulator.c
FPUDIR=../fpu/
OBJS=main.o utils.o simulator.o fadd.o fmul.o finv.o fpu.o
HEADER_SIM=simulator.h
HEADER_UTILS=utils.h

$(TARGET): $(OBJS)
	cd ../fpu; make
	$(CC) -I$(FPUDIR) -o $(TARGET) $(CFLAGS) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) -I$(FPUDIR) -c $(CFLAGS) $<

.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJS)
