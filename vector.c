
// Interface for what defines a vector.
typedef struct {
	void (*Push)(void *self, void *elem);
	void *(*Pop)(void *self);
	void *(*Access)(void *self, int i);
	int (*Len)(void *self);
} VectorInterface;

// Wrapper type to interact with the interface.
typedef struct {
	void *self;
	VectorInterface interface;
} Vector;

Vector *makeVector(void *self, VectorInterface interface) {
	Vector *vec = calloc(sizeof(Vector), 1);
	if (vec != NULL) {
		(*vec).self = self;
		(*vec).interface = interface;
		return vec;
	}
	// Idiomatic (for make) death on allocation error
	printf("Allocating Vector failed, exiting...\n");
	exit(1);
}

void pushVector(Vector *vec, void *elem) {
	return vec->interface.Push(vec->self, elem);
}

void *popVector(Vector *vec) {
	return vec->interface.Pop(vec->self);
}

int lenVector(Vector *vec) {
	return vec->interface.Len(vec->self);
}

void *accessVector(Vector *vec, int i) {
	// bounds checking
	if (i > 0 && i < lenVector(vec)) {
		return vec->interface.Access(vec->self, i);
	} else {
		return NULL;
	}
}

// Dynamic Array implementation of the interface.
typedef struct {
	void **vec;
	int nelem;
	int nused;
} ArrayVector;

ArrayVector *makeArrayVector() {
	ArrayVector *v = calloc(sizeof(ArrayVector), 1);
	if (v != NULL) {
		(*v).vec = calloc(sizeof(void *), 1);
		if (v->vec != NULL) {
			return v;
		}
	}
	// Idiomatic (for make) death on allocation error
	printf("Allocating ArrayVector failed, exiting...\n");
	exit(1);
}

static void upsizeArrayVector(ArrayVector *v) {
	v->nelem++;
	realloc(v->vec, v->nelem * sizeof(void *));
}

static void downsizeArrayVector(ArrayVector *v) {
	v->nelem--;
	realloc(v->vec, v->nelem * sizeof(void *));
}

// O(n) Push because of possible realloc memcpy.
// Very cheap on large memory page relocation.
// http://blog.httrack.com/blog/2014/04/05/a-story-of-realloc-and-laziness/
void pushArrayVector(ArrayVector *v, void *elem) {
	if (v->nused >= v->nelem) {
		upsizeArrayVector(v);
	}
	v->vec[v->nused] = elem;
	v->nused++;
}

// O(1) Pop
void *popArrayVector(ArrayVector *v) {
	if (v->nelem > v->nused) {
		downsizeArrayVector(v);
	}
	v->nused--;
	return v->vec[v->nused];
}

// O(1) Access
void *accessArrayVector(ArrayVector *v, int i) {
	return v->vec[i];
}

// O(1) Len
int lenArrayVector(ArrayVector *v) {
	return v->nused;
}

VectorInterface arrayVectorAsVectorInterface(ArrayVector *v) {
	return (VectorInterface) {
		// Cast typed pointers to void pointers
		.Push   = (void (*)(void *, void *)) pushArrayVector,
		.Pop    = (void *(*)(void *)) popArrayVector,
		.Access = (void *(*)(void *, int)) accessArrayVector,
		.Len    = (int (*)(void *)) lenArrayVector,
	};
}

Vector *makeArrayVectorVector() {
	ArrayVector *v = makeArrayVector();
	return makeVector((void *) v, arrayVectorAsVectorInterface(v));
}

// Linked List implementation of interface.
struct ListVectorElem {
	void *elem;
	struct ListVectorElem *prev;
};

typedef struct {
	struct ListVectorElem *end;
	int len;
} ListVector;

static struct ListVectorElem *makeListVectorElem(void *elem) {
	struct ListVectorElem *v = calloc(sizeof(struct ListVectorElem), 1);
	if (v != NULL) {
		(*v).elem = elem;
		return v;
	}
	// Idiomatic (for make) death on allocation error
	printf("Allocating ListVectorElem failed, exiting...\n");
	exit(1);
}

ListVector *makeListVector() {
	ListVector *v = calloc(sizeof(ListVector), 1);
	if (v != NULL) {
		return v;
	}
	// Idiomatic (for make) death on allocation error
	printf("Allocating ListVector failed, exiting...\n");
	exit(1);
}

// O(1) Push
void pushListVector(ListVector *v, void *elem) {
	void *prev = v->end;
	v->end = makeListVectorElem(elem);
	v->end->prev = prev;
	v->len++;
}

// O(1) Pop
void *popListVector(ListVector *v) {
	void *c = v->end;
	v->end = v->end->prev;
	v->len--;
	return c;
}

static void *accessListVectorElem(struct ListVectorElem *v, int i) {
	if (i > 0) {
		return accessListVectorElem(v->prev, i-1);
	} else {
		return v;
	}
}

// Lazy O(n) space Access recursive algorithm
void *accessListVector(ListVector *v, int i) {
	return accessListVectorElem(v->end, i);
}

// O(1) Len
int lenListVector(ListVector *v) {
	return v->len;
}

VectorInterface listVectorAsVectorInterface(ListVector *v) {
	return (VectorInterface) {
		// Cast typed pointers to void pointers
		.Push   = (void (*)(void *, void *)) pushListVector,
		.Pop    = (void *(*)(void *)) popListVector,
		.Access = (void *(*)(void *, int)) accessListVector,
		.Len    = (int (*)(void *)) lenListVector,
	};
}

Vector *makeListVectorVector() {
	ListVector *v = makeListVector();
	return makeVector((void *) v, listVectorAsVectorInterface(v));
}
