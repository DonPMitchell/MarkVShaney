/*
 *	markov model text generator
 *	D. P. Mitchell
 */
#include <stdio.h>
#include <ctype.h>
#define PRIME 61001

typedef struct Symbol {
	struct Symbol *hashlink;
	char string[32];
} Symbol;

typedef struct List {
	struct List *next;
	int occurs;
	Symbol *s;
} List;

typedef struct Pair {
	struct Pair *hashlink;
	Symbol *s0, *s1;
	int occurs;
	List *next;
} Pair;

Symbol *shash[PRIME];
Pair   *phash[PRIME];
Symbol *period, *lastsym;

char *
toke()
{
	static char buff[32];
	register c, i;

	do
		c = getchar();
	while (isspace(c) || c == '"');
	if (c == EOF)
		return 0;
	if (ispunct(c)) {
		buff[0] = c;
		buff[1] = 0;
		buff[2] = 0;
		c = getchar();
		if (buff[0] == 047 && c == 's')
			buff[1] = c;
		else
			ungetc(c, stdin);
		return buff;
	}
	for (i = 0; i < 32; i++) {
		if (isupper(c))
			c = c - 'A' + 'a';
		buff[i] = c;
		if (!isalnum(c))
			break;
		c = getchar();
	}
/*	ungetc(c, stdin);	*/
	buff[i] = 0;
	return buff;
}

Symbol *
intern(name)
char *name;
{
	register hash;
	Symbol *sp;
	register char *cp;

	for (cp = name, hash = 0; *cp;)
		hash = ((hash << 8) + *cp++) % PRIME;
	for (sp = shash[hash]; sp; sp = sp->hashlink)
		if (strcmp(name, sp->string) == 0)
			return sp;
	sp = (Symbol *)malloc(sizeof(Symbol));
	strcpy(sp->string, name);
	sp->hashlink = shash[hash];
	shash[hash] = sp;
	return sp;
}

Pair *
pintern(s0, s1)
Symbol *s0, *s1;
{
	unsigned long hash;
	Pair *pp;

	hash = ((unsigned)s0 + (unsigned)s1 * 11) % PRIME;
	for (pp = phash[hash]; pp; pp = pp->hashlink)
		if (pp->s0 == s0 && pp->s1 == s1)
			return pp;
	pp = (Pair *)malloc(sizeof(Pair));
	pp->s0 = s0;
	pp->s1 = s1;
	pp->occurs = 0;
	pp->hashlink = phash[hash];
	phash[hash] = pp;
	pp->next = (List *)0;
	return pp;
}

void
cons(pp, s)
Pair *pp;
Symbol *s;
{
	List *lp;

	pp->occurs++;
	for (lp = pp->next; lp; lp = lp->next)
		if (lp->s == s) {
			lp->occurs++;
			return;
		}
	lp = (List *)malloc(sizeof(List));
	lp->next = pp->next;
	pp->next = lp;
	lp->s = s;
	lp->occurs = 1;
}

Symbol *
whatsnext(s0, s1)
Symbol *s0, *s1;
{
	Pair *pp;
	List *lp;
	int index, sum;

	pp = pintern(s0, s1);
	if (pp->occurs == 0)
		return 0;
	index = random() % pp->occurs;
	sum = 0;
	for (lp = pp->next; lp; lp = lp->next) {
		sum += lp->occurs;
		if (sum > index)
			break;
	}
	if (lp)
		return lp->s;
	else
		return 0;
}


printsym(s)
Symbol *s;
{
	static length;
	register char *cp;
	int slen;

	cp = s->string;
	slen = strlen(cp);
	length += slen;
	if ((length > 72 && isalpha(cp[0])) || length > 78) {
		printf("\n");
		length = slen;
	}
	if (!ispunct(cp[0]) && length != slen) {
		printf(" ");
		length++;
	}
	if (strcmp(cp, "i") == 0 || (lastsym == period && isalpha(cp[0]))) {
		printf("%c", cp[0] - 'a' + 'A');
		cp++;
	}
	printf("%s", cp);
	lastsym = s;
	fflush(stdout);
}

main()
{
	Symbol *s0, *s1, *s2;
	Symbol *start0, *start1;
	char *cp;
	Pair *pp;

	srandom(time(0));
	lastsym = period = intern(".");
	start0 = s1 = intern(toke());
	start1 = s2 = intern(toke());
	while ((cp = toke())) {
		s0 = s1;
		s1 = s2;
		s2 = intern(cp);
		pp = pintern(s0, s1);
		cons(pp, s2);
	}
	s1 = start0;
	s2 = start1;
	printsym(s1);
	printsym(s2);
	for (;;) {
		s0 = s1;
		s1 = s2;
		s2 = whatsnext(s0, s1);
		if (!s2)
			break;
		printsym(s2);
	}
	printf("\n");
}
