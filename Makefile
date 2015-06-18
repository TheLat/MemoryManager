CFLAGS=-Wall -Wextra -g -std=c++11
all: mm

ifeq "$(CXX)" "clang"
CFLAGS += -Weverything
endif

mm: main.cpp mm.h
	$(CXX) $(CFLAGS) -o mm main.cpp -lstdc++

test: mm
	valgrind ./mm

clean:
	rm -f mm
