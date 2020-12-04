/* Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

Modifications are made by Richard James Howe <howe.r.j@gmail.com> and released
under the same license. See <https://github.com/howerj/utf8> for this projects
repository.

UTF-8 Validation, Decoding and Encoding functions.

Note that we could add all kinds of functions to this library, but that
simplicity is a virtue, this library is intended to do the bare minimum
necessary for manipulating UTF-8 strings. */

#include "utf8.h"
#include <assert.h>
#include <limits.h>

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

#ifndef UTF8_VERSION
#define UTF8_VERSION (0) /* should be set by build system, 0 = errors */
#endif

enum { ACCEPT = 0, REJECT = 12, }; /* internal use only */

int utf8_decode(unsigned long *state, unsigned long *codep, const unsigned char byte) {
	assert(state);
	assert(codep);

	static const unsigned char utf8d[] = {
		/* The first part of the table maps bytes to character classes that
		to reduce the size of the transition table and create bit-masks. */
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
		10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

		/* The second part is a transition table that maps a combination
		of a state of the automaton and a character class to a state. */
		0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
		12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
		12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
		12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
		12,36,12,12,12,12,12,12,12,12,12,12,
	};

	const unsigned long type = utf8d[byte];
	*codep = *state != ACCEPT ? (byte & 0x3fu) | (*codep << 6) : (0xff >> type) & byte;
	*state = utf8d[256ul + *state + type];
	return *state == REJECT ? -1 : (int)*state;
}

int utf8_code_points(const char *s, const size_t slen, size_t *count) {
	assert(s);
	assert(count);
	int r = 0;
	unsigned long codepoint = 0, state = 0;
	size_t c = 0;
	*count = 0;
	for (size_t i = 0; i < slen; i++) {
		r = utf8_decode(&state, &codepoint, s[i]);
		if (r == 0)
			c++;
		if (r < 0)
			return -1;
	}
	*count = c;
	return r;
}

int utf8_next(char **s, size_t *slen, unsigned long *codep) {
	assert(s);
	assert(codep);
	assert(slen);
	int r = 0;
	char *sp = *s;
	unsigned long state = 0;
	size_t sz = *slen, i = 0;
	*codep = 0;
	for (i = 0; i < sz; i++) {
		r = utf8_decode(&state, codep, sp[i]);
		if (r < 0)
			return -1;
		if (r == 0) {
			i++;
			break;
		}
	}
	*s = &sp[i];
	*slen -= i;
	return r;
}

int utf8_code_point_valid(const unsigned long codep) {
	if (codep <= 0x10FFFFul) /* not quite right <https://stackoverflow.com/questions/27415935> */
		return 0; /* valid */
	return -1;
}

static size_t bytes(const unsigned long codep) {
	if (codep < 0x80ul)
		return 1;
	if (codep < 0x800ul)
		return 2;
	if (codep < 0x10000ul)
		return 3;
	return 4;
}

/* 00000000 -- 0000007F: 0xxxxxxx
 * 00000080 -- 000007FF: 110xxxxx 10xxxxxx
 * 00000800 -- 0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx
 * 00010000 -- 001FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
int utf8_add(char **s, size_t *length, unsigned long codep) {
	assert(s);
	assert(length);
	if (utf8_code_point_valid(codep) < 0)
		return -1;
	char *sp = *s;
	const size_t l = *length, b = bytes(codep);
	if (l == 0 || l < b)
		return -1;
	*length -= b;
	switch (b) {
	case 1:
		sp[0] = codep;
		break;
	case 2:
		sp[0] = 0xC0 | (0x1f & (codep >>  6));
		sp[1] = 0x80 | (0x3f & (codep >>  0));
		break;
	case 3:
		sp[0] = 0xE0 | (0x0f & (codep >> 12));
		sp[1] = 0x80 | (0x3f & (codep >>  6));
		sp[2] = 0x80 | (0x3f & (codep >>  0));
		break;
	case 4:
		sp[0] = 0xF0 | (0x07 & (codep >> 18));
		sp[1] = 0x80 | (0x3f & (codep >> 12));
		sp[2] = 0x80 | (0x3f & (codep >>  6));
		sp[3] = 0x80 | (0x3f & (codep >>  0));
		break;
	}
	sp += b;
	*s = sp;
	return 0;
}

#define ERROR (-__LINE__)

/* NB. It would be nifty to test failure cases as well. */
int utf8_tests(void) {
	BUILD_BUG_ON(CHAR_BIT != 8);
#ifdef NDEBUG
	return 0;
#endif
	/* decoding a single code point */
	size_t sz = 0;
	if (utf8_code_points((char*)(unsigned char[]){ 0x41, }, 1, &sz) < 0) return ERROR; /* LATIN CAPITAL LETTER A */ 
	if (sz != 1) return ERROR;
	if (utf8_code_points((char*)(unsigned char[]){ 0xC3, 0xB6, }, 2, &sz) < 0) return ERROR; /* LATIN SMALL LETTER O WITH DIAERESIS */
	if (sz != 1) return ERROR;
	if (utf8_code_points((char*)(unsigned char[]){ 0xD0, 0x96, }, 2, &sz) < 0) return ERROR; /* CYRILLIC CAPITAL LETTER ZHE */
	if (sz != 1) return ERROR;
	if (utf8_code_points((char*)(unsigned char[]){ 0xE2, 0x82, 0xAC, }, 3, &sz) < 0) return ERROR; /* EURO SIGN */
	if (sz != 1) return ERROR;
	if (utf8_code_points((char*)(unsigned char[]){ 0xF0, 0x9D, 0x84, 0x9E, }, 4, &sz) < 0) return ERROR; /* MUSICAL SYMBOL G CLEF */
	if (sz != 1) return ERROR;
	/* decoding multiple code points A + Clef + A + oe */
	if (utf8_code_points((char*)(unsigned char[]){ 0x41, 0xF0, 0x9D, 0x84, 0x9E, 0x41, 0xC3, 0xB6, }, 8, &sz) < 0) return ERROR;
	if (sz != 4) return ERROR;

	/* moving along a string extracting code points */
	unsigned char s0[] = {
		0x41, /* code point is 0x41 */
		0xC3, 0xB6, /* code point is 0xF6 */
		0xD0, 0x96, /* code point is 0x416 */
		0xE2, 0x82, 0xAC, /* code point is 0x20AC */
		0xF0, 0x9D, 0x84, 0x9E, /* code point is 0x1D11E */
	};
	unsigned long codep = 0;
	char *s0p = (char*)s0;
	size_t s0len = sizeof (s0);
	if (utf8_next(&s0p, &s0len, &codep) < 0) return ERROR;
	if (codep != 0x41ul) return ERROR;
	if (s0len != (sizeof (s0) - 1)) return ERROR;
	if (utf8_next(&s0p, &s0len, &codep) < 0) return ERROR;
	if (codep != 0xF6ul) return ERROR;
	if (s0len != (sizeof (s0) - 3)) return ERROR;
	if (utf8_next(&s0p, &s0len, &codep) < 0) return ERROR;
	if (codep != 0x416ul) return ERROR;
	if (s0len != (sizeof (s0) - 5)) return ERROR;
	if (utf8_next(&s0p, &s0len, &codep) < 0) return ERROR;
	if (codep != 0x20ACul) return ERROR;
	if (s0len != (sizeof (s0) - 8)) return ERROR;
	if (utf8_next(&s0p, &s0len, &codep) < 0) return ERROR;
	if (codep != 0x1D11Eul) return ERROR;
	if (s0len != (sizeof (s0) - 12)) return ERROR;
	if (utf8_next(&s0p, &s0len, &codep) < 0) return ERROR;
	if (codep != 0) return ERROR;
	if (s0len != 0) return ERROR;
	if (utf8_next(&s0p, &s0len, &codep) < 0) return ERROR;
	if (codep != 0) return ERROR;
	if (s0len != 0) return ERROR;

	/* check constructing UTF-8 string from code points */
	unsigned char s1[12] = { 0, };
	char *s1p = (char*)s1;
	size_t s1len = sizeof (s1);
	if (utf8_add(&s1p, &s1len, 0x41ul) < 0) return ERROR;
	if (utf8_add(&s1p, &s1len, 0xF6ul) < 0) return ERROR;
	if (utf8_add(&s1p, &s1len, 0x416ul) < 0) return ERROR;
	if (utf8_add(&s1p, &s1len, 0x20ACul) < 0) return ERROR;
	if (utf8_add(&s1p, &s1len, 0x1D11Eul) < 0) return ERROR;
	BUILD_BUG_ON(sizeof (s1) != sizeof (s0));
	for (size_t i = 0; i < sizeof (s1); i++)
		if (s0[i] != s1[i])
			return ERROR;
	return 0;
}

int utf8_version(unsigned long *version) {
	assert(version);
	*version = UTF8_VERSION;
	return UTF8_VERSION == 0 ? -1 : 0;
}
