CFLAGS = -O2 -flto -std=c99 -Wall -Wextra -Werror -pedantic -Wmissing-declarations -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wredundant-decls -Wnested-externs -Winline -Winline -Wstrict-prototypes -fwrapv

all: gk3

gk3: gk3.c
	@clang $(CFLAGS) -lz $^ -o $@

test:
	@./gk3 WOD202P.SHP

clean:
	@rm -f ./gk3
