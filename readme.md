% utf8(1) | UTF8 Validator

# NAME

UTF8 - UTF-8 validator and wrapped around a small UTF-8 library

# SYNOPSES

utf8 string

utf8 < file

# DESCRIPTION

	Author:     Richard James Howe / Bjoern Hoehrmann
	License:    MIT
	Repository: <https://github.com/howerj/utf8>
	Email:      howe.r.j.89@gmail.com
	Copyright:  2008-2009 Bjoern Hoehrmann
	Copyright:  2020      Richard James Howe


This UTF-8 validator/decoder library is based entirely
around the (excellent) code and description available at
<http://bjoern.hoehrmann.de/utf-8/decoder/dfa/>,

Modifications are released under the same license as the original. The
executable built is mainly just there to allow the library itself to be
tested. This is a very minimal set of UTF-8 utilities, not much is provided
but it should allow you to build upon it.

# USAGE

The test program can either check whether a string passed in as an argument
is valid UTF-8 or it can read from stdin(3) if no argument is given and do
the same, in both cases the number of code points, if any, that have been
decoded is printed out if and only if the entire input is valid.

Some example uses are, passing in a string:

	./utf8 "hello"
	5

And passing redirecting a file:

	echo "hello" > file
	./uft8 < file
	6

The built in library tests are executed before this operation begins, the
program will return a non-zero number if these built in tests fail.

# RETURN VALUE

The utf8 program returns zero on success, one on an input error and two on
an internal error.

# BUILDING

You will need make and a suitable C compiler. The only compile time option
used by the library is NDEBUG, if this is defined the built in tests will
always return success along with assertions being turned off.

# C API

The C API is spartan and has three main functions; "utf8\_decode", "utf8\_add"
and "utf8\_tests", and the minor function "utf8\_code\_point\_valid". The
functions "utf8\_code\_points" and "utf8\_next" are built around the
"utf8\_decode" function.

* "utf8\_decode" is used to decode a byte stream, byte by byte and turn
the stream into *code points*.
* "utf8\_add" is used to add a *code point* to a string, which does not
have to be NUL terminated.
* "utf8\_tests" performs a series of built in self tests, if there is a bug,
a new test can be added.

Negative is return on an error or failure, a positive value (including zero)
indicates success.

The function "utf8\_tests" contains test examples and usage for each of the
functions. All of the functions operate on binary data and not NUL terminated
ASCII strings (also known as ASCIIZ strings). It is however easy to convert
the functions to operate on the using strlen(3). Go read the source, do not
be scared, it is a small enough library.

# REFERENCES

* <http://bjoern.hoehrmann.de/utf-8/decoder/dfa/>
* <https://kunststube.net/encoding/>
* <https://stackoverflow.com/questions/27415935>
* <https://en.wikipedia.org/wiki/Code_point>
* <https://en.wikipedia.org/wiki/Unicode>
* <https://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt>
* <https://en.wikipedia.org/wiki/UTF-8>
* <https://www.cprogramming.com/tutorial/unicode.html>

# LICENSE

This project is released under the MIT license.

# LIMITATIONS

If the number of code points or byte count exceeds 2^32 - 1 then an incorrect
value may be displayed. You know why.

This is a very simple library meant to deal with UTF-8 data, and converting
strings to and from code-points. It does not deal with issues like Unicode
normalization, case conversion and a whole host of horrors that occur when
not using ASCII.


