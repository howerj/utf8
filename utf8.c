/* 
	Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
	See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
	Modifications are made by Richard James Howe <howe.r.j@gmail.com>
	and released under the same license.
	See <https://github.com/howerj/utf8> for this projects repository.

	UTF-8 Validation, Decoding and Encoding functions.
 
	TODO
		- Unit tests
		- Functions for adding a code point to a string

	Note that we could add all kinds of functions to this library,
	but that simplicity is a virtue, this library is intended to
	do the bare minimum necessary for manipulating UTF-8 strings.

	Some potentially useful functions could include:
	- Escape/Unescape functions for turning encoded constants like
	'\u2023' into their respective unicode.
	- Functions that operate on arrays of code points to convert
	to/from strings.
*/

#include "utf8.h"
#include <assert.h>
#include <limits.h>

enum { ACCEPT, REJECT }; /* internal use only */

int utf8_decode(unsigned long *state, unsigned long *codep, const unsigned char byte) {
	assert(state);
	assert(codep);
	static const unsigned char utf8d[] = {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
		8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
		0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
		0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
		0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
		1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
		1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
		1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
	};
	const unsigned long type = utf8d[byte];
	*codep = (*state != ACCEPT) ?  (byte & 0x3fu) | (*codep << 6) : (0xfful >> type) & (byte);
	*state = utf8d[256ul + *state*16ul + type];
	return *state == 1 ? -1 : (int)*state;
}

int utf8_code_points(const char *s, size_t *count) {
	assert(s);
	assert(count);
	unsigned long codepoint = 0, state = 0;
	for (*count = 0; *s; ++s)
		if (!utf8_decode(&state, &codepoint, *s))
			*count += 1;
	return state == ACCEPT ? 0 : -1;
}

int utf8_next(char **s, unsigned long *codep) {
	assert(s);
	assert(codep);
	char *sp = *s;
	unsigned long state = 0;
	*codep = 0;
	for (; *sp; ++sp) {
		const int r = utf8_decode(&state, codep, *sp);
		if (r < 0)
			return -1;
		if (r == 0)
			break;
	}
	*s = sp;
	return 0;
}

int utf8_code_point_valid(unsigned long codep) {
	if (codep <= 0x10FFFF) /* not quite right <https://stackoverflow.com/questions/27415935> */
		return 0; /* valid */
	return -1;
}

static size_t bytes(unsigned long codep) {
	if (codep < 0x80)
		return 1;
	if (codep < 0x800)
		return 2;
	if (codep < 0x10000)
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
	if (l == 0 || (l - 1) < b)
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

int utf8_tests(void) {
#ifdef NDEBUG
	return 0;
#endif
	if (CHAR_BIT != 8)
		return -1;
	return 0;
}

