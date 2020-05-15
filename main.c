#include "utf8.h"
#include <stdio.h>

int main(int argc, char **argv) {
	FILE *e = stderr, *o = stdout;

	if (utf8_tests() < 0) {
		(void)fprintf(e, "internal tests failed\n");
		return 1;
	}

	if (argc == 1) { /* TODO: Read from stdin and validate as we go */
		return 0;
	}

	if (argc == 2) {
		size_t count = 0;
		if (utf8_code_points(argv[1], &count) < 0) {
			if (fputs("invalid\n", o) < 0)
				return -1;
			return 1;
		}
		if (fprintf(o, "%lu\n", (unsigned long)count) < 0)
			return -1;
		return 0;
	}

	static const char *help = "usage: %s [string]\n\n\
Simple UTF-8 validator, see <http://bjoern.hoehrmann.de/utf-8/decoder/dfa/>\n\
for original source and <https://github.com/howerj/utf8> for this programs\n\
source.\n\n";
	(void)fprintf(e, help, argv[0]);
	return 1;
}

