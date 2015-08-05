
// Takes a string of source text,
// returns a vector of tokens.
Vector *lex(char *source);

typedef struct {
	Vector *toks;
	char *src;
	int i, len, lemit;
	int line, col;
} State;

static State *makeState(char *src) {
	State *s = calloc(sizeof(State), 1);
	if (s != NULL) {
		(*s).toks = makeArrayVectorVector();
		(*s).src = src;
		return s;
	}
	// Idiomatic (for make) death on allocation error
	printf("Allocating State failed, exiting...\n");
	exit(1);
}

static int lnext(State *s) {
	int c = s->src[s->i];
	if (c != '\0') s->i++;
	if (c == '\n') {
		s->col = 0;
		s->line++;
	}
	s->col++;
	return c;
}

static int lpeek(State *s) {
	return s->src[s->i];
}

static void lback(State *s) {
	s->col--;
	s->i--;
}

static char *lstr(State *s) {
	int len = s->i - s->lemit;
	char *str = calloc(sizeof(char), len + 1);
	strncpy(str, &s->src[s->lemit], len);
	return str;
}

static char *l_emit(State *s) {
	char *str = lstr(s);
	s->lemit = s->i;
	return str;
}

static int llen(State *s) {
	return s->i - s->lemit;
}

static void lemit(State *s, int type) {
	Tok *t = makeTokStr(s->line, s->col - llen(s), type, l_emit(s));
	pprintTok(t);
	pushVector(s->toks, t);
}

static char *ldump(State *s) {
	return l_emit(s);
}

struct Error {
	const char *msg; // brief message
	const char *comment; // long explanation
};

static void lerror(State *s, char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char *str;
	vasprintf(&str, fmt, args);
	va_end(args);
	Tok *t = makeTokStr(s->line, s->col, kTError, str);
	pprintTok(t);
	pushVector(s->toks, t);
}

static void lwhile(State *s, int (*func)(int)) {
	while (func(lnext(s))) {}
	lback(s);
}

int isWhitespace(int c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int isLetter(int c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int isDigit(int c) {
	return c >= '0' && c <= '9';
}

int isOperator(int c) {
	// simple for now
	return c == '+' || c == '-' || c == '*' || c == '/';
}

void *startState(State *s);

void *varInfixExpressionState(State *s) {
	lwhile(s, isOperator);
	lemit(s, kTOperator);
	return NULL;
}

void *varExpressionState(State *s) {
	int c = lnext(s);
	if (isDigit(c) || c == '+' || c == '-') {
		return varInfixExpressionState;
	} else if (isLetter(c)) {
		// type declaration, variable assignment, or function/method call.
		lwhile(s, isLetter);
		lemit(s, kTIdent);
		lwhile(s, isWhitespace);
		ldump(s);
		// operators are infix's problem
		if (isOperator(lnext(s))) {
			return varInfixExpressionState;
		}
		return NULL;
	} else {
		lerror(s, "Unknown character encountered '%s'", ldump(s));
		return NULL;
	}
}

void *varValueState(State *s) {
	// value assignment (optional, defaults to 0)
	if (lnext(s) == '=') {
		lemit(s, kTOperator);
		// optional whitespace
		lwhile(s, isWhitespace);
		ldump(s);
		// value
		return varExpressionState;
	} else lback(s);
	return startState;
}

void *varTypeState(State *s) {
	// variable type (mandatory in global scope)
	if (isLetter(lnext(s))) {
		// lex type ident
		lwhile(s, isLetter);
		lemit(s, kTIdent);
		// optional whitespace
		lwhile(s, isWhitespace);
		ldump(s);
		return varValueState;
	} else {
		lback(s);
		lerror(s, "Global variable declarations require types");
		return startState;
	}
}

void *varNameState(State *s) {
	// variable name
	if (isLetter(lnext(s))) {
		// lex type ident
		lwhile(s, isLetter);
		lemit(s, kTIdent);
		// optional whitespace
		lwhile(s, isWhitespace);
		ldump(s);
		return varTypeState;
	} else {
		lback(s);
		lerror(s, "Var declaration requires identifier");
		return startState;
	}
}

void *varState(State *s) {
	// optional whitespace
	lwhile(s, isWhitespace);
	ldump(s);

	return varNameState;
}

struct Keyword {
	// keyword string
	const char *word;
	// subsequent lexing state
	void *state;
} keywords[] = {
	{
		.word = "var",
		.state = varState
	}, {
		.word = "func",
		.state = NULL
	}, {
		.word = "const",
		.state = NULL
	}
};

void *keywordState(State *s) {
	lwhile(s, isLetter);
	char *str = lstr(s);
	for (int i = 0; i < (sizeof(keywords) / sizeof(struct Keyword)); i++) {
		if (strcmp(keywords[i].word, str) == 0) {
			// have a keyword
			lemit(s, kTKeyword);
			return keywords[i].state;
		}
	}
	lerror(s, "Unknown keyword '%s'", ldump(s));
	return NULL;
}

void *startState(State *s) {
	// Start state
	int c = lnext(s);
	if (c == '\0') return NULL;
	if (isLetter(c)) {
		// is a keyword, 'var', 'func', 'const'.
		return keywordState;
	} else if (isWhitespace(c)) {
		// is whitespace
		lwhile(s, isWhitespace);
		ldump(s);
		return startState;
	} else {
		lerror(s, "Unexpected char '%s', expected alphabetic", ldump(s));
		return startState;
	}
}

Vector *lex(char *source) {
	State *s = makeState(source);
	void *func = (void *) startState;
	while (func != NULL) {
		func = ((void *(*)(State *)) func)(s);
	}
	return s->toks;
}
