
#include "c.h"

int main() {
	// testing purposes.
	compGen("***      var c haha = \"hello, world\";")();
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vector.c"
#include "map.c"
#include "tok.c"
#include <stdarg.h>
#include "lex.c"
#include "parse.c"
#include <sys/mman.h>
#include <unistd.h>
#include "gen.c"

void (*compGen(char *source))(void) {
	return gen(parse(lex(source)));
}
