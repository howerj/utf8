/* 
	Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
	See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
	Modifications are made by Richard James Howe <howe.r.j@gmail.com>
	and released under the same license.
	See <https://github.com/howerj/utf8> for this projects repository.

	UTF-8 Validation, Decoding and Encoding functions.
*/

#ifndef UTF8_H
#define UTF8_H

#include <stddef.h>

/* all functions return negative on failure */
int utf8_decode(unsigned long *state, unsigned long *codep, const unsigned char byte); /* returns: 0 = done, 1 = need more, -1 = error */
int utf8_code_points(const char *s, size_t *count);
int utf8_next(char **s, unsigned long *codep);
int utf8_code_point_valid(unsigned long codep); /* -1 = invalid, 0 = valid */
int utf8_add(char **s, size_t *length, unsigned long codep);
int utf8_tests(void);

#endif
