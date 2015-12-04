TARGET=libfpu.so
CC=gcc
CFLAGS=-Wall
LDFLAGS=-lm
SRC=fpu.c fadd.c fmul.c finv.c

$(TARGET): $(SRC)
	$(CC) -shared -fPIC -o $(TARGET) $(CFLAGS) $(SRC)	$(LDFLAGS)

.PHONY: clean
clean:
	rm -rf $(TARGET)
