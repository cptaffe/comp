
// Takes a vector of trees,
// returns an executable page.
void (*gen(Vector *vec))(void);

// x86_64 instructions
enum {
	kIRet = 0xc3,
};

void (*gen(Vector *vec))(void) {
	const unsigned long int pageSize = sysconf(_SC_PAGESIZE);

	unsigned char *mem = (unsigned char *) mmap(NULL, pageSize,
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

	mem[0] = kIRet; // return instruction

	mprotect(mem, pageSize, PROT_EXEC);
	return (void (*)(void)) mem;
}
