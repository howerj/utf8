#include "utf8.h"
#include <stdio.h>
#include <string.h>

enum { OK, INPUT_ERROR, INTERNAL_ERROR, };

int main(int argc, char **argv) {
	FILE *in = stdin, *e = stderr, *o = stdout;

	unsigned long version = 0;
	if (utf8_version(&version) < 0) {
		(void)fprintf(e, "version not set by build system\n");
		return INTERNAL_ERROR;
	}

	if (utf8_tests() < 0) {
		(void)fprintf(e, "internal tests failed\n");
		return INTERNAL_ERROR;
	}

	if (argc == 1) {
		unsigned long state = 0, codep = 0;
		unsigned long long count = 0, bytes = 0;
		for (int ch = 0; (ch = fgetc(in)) != EOF; bytes++) {
			const int r = utf8_decode(&state, &codep, ch);
			if (r < 0) {
				(void)fprintf(e, "invalid UTF-8 at/before byte %ld\n", (unsigned long)bytes);
				return INPUT_ERROR;
			}
			count += r == 0;
		}
		if (fprintf(o, "%ld\n", (unsigned long)count) < 0)
			return INPUT_ERROR;
		return 0;
	}

	if (argc == 2) {
		size_t count = 0;
		if (utf8_code_points(argv[1], strlen(argv[1]), &count) < 0) {
			if (fputs("invalid\n", o) < 0)
				return INTERNAL_ERROR;
			return INPUT_ERROR;
		}
		if (fprintf(o, "%lu\n", (unsigned long)count) < 0)
			return INTERNAL_ERROR;
		return 0;
	}

	static const char *help = "usage: %s [string]\n\n\
Simple UTF-8 validator, see <http://bjoern.hoehrmann.de/utf-8/decoder/dfa/>\n\
for original source and <https://github.com/howerj/utf8> for this programs\n\
source.\n\n";
	(void)fprintf(e, help, argv[0]);
	return INPUT_ERROR;
}

