CFLAGS=-Wall -Wextra -g -std=c++11
all: mm

ifeq "$(CC)" "clang"
CFLAGS += -Weverything
endif

mm: main.cpp mm.h
	$(CC) $(CFLAGS) -o mm main.cpp -lstdc++

test: mm
	valgrind ./mm

clean:
	rm -f mm
