LDLIBS = -lpthread
CFLAGS = -Wall -Wextra -Wpedantic -Werror
CC = clang

.PHONY: clean

all: a3

clean:
	rm -f a3