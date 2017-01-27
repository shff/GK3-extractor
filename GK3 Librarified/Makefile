CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic -Wmissing-declarations -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wredundant-decls -Wnested-externs -Winline -Winline -Wstrict-prototypes -fwrapv

all: gk3 test clean

libgk3.so: gk3.c
	@clang $(CFLAGS) -shared -undefined dynamic_lookup -O2 -flto gk3.c -lz -o libgk3.so

gk3: main.c libgk3.so
	@clang $(CFLAGS) $^ -o $@

test:
	@./gk3 TE1.SCN

clean:
	@rm ./gk3
