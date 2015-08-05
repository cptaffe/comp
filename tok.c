
enum {
	kKvar, // var b = 4
	kKfunc, // func main()
	kKconst, // const a = 2, b u8 = 3
};

typedef struct {
	int line, col;
	enum {
		kTIdent,
		kTKeyword,
		kTOperator,
		kTNum,
		kTChar,
		kTStr,
		kTEOF,
		kTError,
	} type;
	// kTStr, kTIdent, kTNum, KTError
	char *str;
} Tok;

void pprintTok(Tok *t) {
	const char *types[] = {
		"identifier",
		"keyword",
		"operator",
		"number",
		"character",
		"string",
		"end of file",
		"error"
	};
	printf("%s (%d:%d) %s\n", types[t->type], t->line, t->col, t->str);
}

static Tok *makeTok() {
	Tok *tok = calloc(sizeof(Tok), 1);
	if (tok == NULL) {
		printf("Allocating Tok failed, exiting...\n");
		exit(1);
	}
	return tok;
}

Tok *makeTokStr(int line, int col, int type, char *str) {
	Tok *tok = makeTok();
	*tok = (Tok){
		.line = line,
		.col = col,
		.type = type,
		.str = str
	};
	return tok;
}
