
// Interface for what defines a map.
typedef struct {
	void (*Store)(void *self, const char *key, void *value);
	void *(*Retrieve)(void *self, const char *key);
} MapInterface;

// Wrapper type to interact with interface.
typedef struct {
	void *self;
	MapInterface interface;
} Map;

Map *makeMap(void *self, MapInterface interface) {
	Map *m = calloc(sizeof(Map), 1);
	if (m != NULL) {
		(*m).self = self;
		(*m).interface = interface;
		return m;
	}
	// Idiomatic (for make) death on allocation error
	printf("Allocating Map failed, exiting...\n");
	exit(1);
}

void storeMap(Map *m, const char *key, void *value) {
	return m->interface.Store(m->self, key, value);
}

void *retrieveMap(Map *m, const char *key) {
	return m->interface.Retrieve(m->self, key);
}

typedef struct {
	const char *key;
	void *value;
} Entry;

Entry *makeEntry(const char *key, void *value) {
	Entry *e = calloc(sizeof(Entry), 1);
	if (e != NULL) {
		(*e).key = key;
		(*e).value = value;
		return e;
	}
	// Idiomatic (for make) death on allocation error
	printf("Allocating Entry failed, exiting...\n");
	exit(1);
}

enum {
	kHashMapBucketSize = 0x1000
};

typedef struct {
	Entry **entries;
} HashMap;

HashMap *makeHashMap() {
	HashMap *m = calloc(sizeof(HashMap), 1);
	if (m != NULL) {
		(*m).entries = calloc(sizeof(Entry*), kHashMapBucketSize);
	}
	// Idiomatic (for make) death on allocation error
	printf("Allocating HashMap failed, exiting...\n");
	exit(1);
}

// multiplicative hash function
static int stringHash(const char *key) {
	int hash = 0;
	for (int i = 0; key[i]; i++) {
		// Integer overflow simulates % p.
		hash = ((hash * 31) + key[i]);
	}
	return hash;
}

void storeHashMap(HashMap *m, const char *key, void *value) {
	m->entries[stringHash(key) % kHashMapBucketSize] = makeEntry(key, value);
}

void *retrieveHashMap(HashMap *m, const char *key) {
	Entry *e = m->entries[stringHash(key) % kHashMapBucketSize];
	if (e != NULL) {
		return (*e).value;
	} else {
		return NULL;
	}
}

MapInterface hashMapAsMapInterface(HashMap *m) {
	return (MapInterface) {
		.Store    = (void(*)(void *, const char *, void *)) storeHashMap,
		.Retrieve = (void *(*)(void *, const char *)) retrieveHashMap
	};
}

Map *makeHashMapMap() {
	HashMap *m = makeHashMap();
	return makeMap((void *) m, hashMapAsMapInterface(m));
}
