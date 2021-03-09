/*
 * ==================================================
 *
 *       Filename:  sfi.c
 *    Description:  Simple file importer written in C 
 *
 *        Created:  01/25/2021 09:57:15 PM
 *       Compiler:  cc
 *         Author:  Andrea Galletti ()
 *
 *       Copyright (c) 2021 Andrea Galletti
 *       See LICENSE for further informations
 *
 * ==================================================
 */
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef VERSION
#define VERSION "`No version information`"
#endif
#define LENGTH(x) sizeof(x)/sizeof(x[0])

typedef const char * (*Parser)(const char *, const char *, int);

static void eprintf(const char *format, ...);					/* print formatted err str and exit(1) */
static void print_chunk(const char *start, const char *end);			/* print the string chunk */
static void process_import(const char *buffer);					/* recursive file importing and processing */
static void * erecalloc(void *p, size_t size);					/* calloc or realloc if void *p is not NULL */
static char * read_file(const char *file);
static char * copy_chunk(const char *start, const char *end);			/* copy in new buffer the passed chunk */
static const char * find_string(const char *buffer, const char *to_find);
static const char * doreplace(const char *start, const char *end, int newchunk);/* do a simple string replace */

static const char *wrappers[2] = { "{{", "}}" };
static const char *replace[][2] = {
	{ "\\{",	"{" },
	{ "\\}",	"}" }
};
static const Parser parsers[] = { doreplace };

char *
usage() {
	return "Usage %s [OPTION] [file]\n"
		"-v	print the version and exit\n"
		"-h	print this help and exit\n";
}

void
eprintf(const char *format, ...) {
	va_list ap;
	
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void *
erecalloc(void *p, size_t size) {
	void *res;
	if (p) {
		res = realloc(p, size);
	}
	else {
		res = calloc(1, size);
	}
	if (!res)
		eprintf("fatal; could not malloc() %u bytes\n", size);
	return res;
}

char *
read_file(const char *file) {
	int readed;
	size_t len = 0, bsize = 2 * BUFSIZ;
	char *buffer = NULL;
	FILE *source = stdin;

	if (file && !(source = fopen(file, "r")))
		eprintf("Cannot open file `%s`\n", file);
	/* allocate base buffer space */
	buffer = erecalloc(buffer, bsize);
	while ((readed = fread(&buffer[len], 1, BUFSIZ, source))) {
		len += readed;
		if (len + BUFSIZ + 1 > bsize) {
			/* increment buffer size if needed */
			bsize += BUFSIZ;
			buffer = erecalloc(buffer, bsize);
		}
	}
	/* check fread errors */
	if(ferror(source)){
		eprintf("fread: file: %s: %s\n", file, strerror(errno));
	}
	buffer[len] = '\0';
	fclose(source);
	return buffer;
}

const char *
find_string(const char *buffer, const char *to_find) {
	size_t len;
	const char *start, *end;
	len = strlen(to_find);
	end = buffer + strlen(buffer);
	/* iterate over char pointers */
	for (start = buffer; start < end; start++) {
		if (strncmp(to_find, start, len) == 0) {
			/* check if the first char is escaped */
			if (start != buffer && *(start-1) == '\\')
				continue;
			return start;
		}
	}
	return NULL;
}

char *
copy_chunk(const char *start, const char *end) {
	char *buffer;
	size_t blen = (end - start) * sizeof(char),
	       bsize = blen/BUFSIZ * BUFSIZ + BUFSIZ;
	if (blen > 0) {
		/* copy the string chunk */
		buffer = erecalloc(NULL, bsize);
		memcpy(buffer, start, blen);
		buffer[blen] = '\0';
		return buffer;
	}
	return NULL;
}

void
print_chunk(const char *start, const char *end) {
	char *buffer = copy_chunk(start, end);
	/* print the chunk and free */
	if (buffer) {
		fputs(buffer, stdout);
		free(buffer);
	}
}

const char *
doreplace(const char *start, const char *end, int newchunk) {
	const char *found = NULL, *cursor = NULL;
	int i, len = LENGTH(replace), pos;

	for (i = 0; i < len; i++) {
		found = find_string(start, replace[i][0]);
		if (found && (found < cursor || !cursor)) {
			cursor = found;
			pos = i;
		}
	}
	if (cursor && newchunk == 1)
		return cursor;
	if (cursor && newchunk == 0) {
		fputs(replace[pos][1], stdout);
		return cursor + strlen(replace[pos][0]);
	}
	return NULL;
}

void
process_import(const char *buffer) {
	const char *start = buffer, *end = NULL, *found, *cursor;
	char *file, *filename;
	int i, j;
	Parser cur_parser;
	for (i = 0; *start != '\0'; i++) {
		end = find_string(start, wrappers[i % 2]);
		if (!end) {
			end = start + strlen(start);
		}
		else if (start != buffer && i % 2 > 0) {
			/* skip previous wrapper from start*/
			start += strlen(wrappers[(i - 1) % 2]);
			filename = copy_chunk(start, end);
			file = read_file(filename);
			/* recursion here!! */
			process_import(file);
			free(filename);
			free(file);
			/* skip current wrapper from end */
			start = end += strlen(wrappers[i % 2]);
		}

		do {
			cursor = found = NULL;
			/* iterate over parsers and choose the nearest to start */
			for (j = 0; j < LENGTH(parsers); j++) {
				found = parsers[j](start, end, 1);
				if (found && (!cursor || found  < cursor)) {
					cur_parser = parsers[j];
					cursor = found;
				}
			}
			if (cursor) {
				print_chunk(start, cursor);
				start = cur_parser(start, end, 0);
			}
		}
		while (found);

		print_chunk(start, end);
		start = end;
	}
}

int
main(int argc, char *argv[]) {
	int i;
        char *program_name = argv[0];
	char *filename = NULL;
	
	/* filter arguments */
	for(i=1; i < argc; i++) {
		if (!strcmp("-v", argv[i])) {
			printf("Simple template engine %s (C) Andrea Galletti\n",
				VERSION);
			return EXIT_SUCCESS;
		}
		else if (!strcmp("-h", argv[i])) {
			printf(usage(), program_name);
			return EXIT_SUCCESS;
		} 
		else if (i == argc-1 && argv[i][0] != '-') {
			filename = argv[i];
		}
		else {
			fprintf(stderr, usage(), argv[0]);
			eprintf("\nInvalid parameter: %s\n", argv[i]);
		}
	}
	process_import(read_file(filename));
	return EXIT_SUCCESS;
}
