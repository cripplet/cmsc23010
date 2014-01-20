#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "io.h"
#include "utils.h"

#define MAX_BUF 2

char *get_line(FILE *);

/**
 * Reads in the given file, and returns a pointer to a newly-allocated N x N array.
 *
 * The expected format of the file is:
 *
 * N
 * 0 ... X
 * . ... .
 * X ... 0
 *
 * Where X is the distance between node N - 1 and 0 (0-indexed).
 */
int **read_array(char *filename, int *dim_ptr) {
	FILE *fp = fopen(filename, "r");
	if(fp == NULL) {
		fail(ERR_OPEN, "read_array", "Cannot open file.");
	}

	// get N, which is on the first line of the text file
	char *line = get_line(fp);
	int dim = (int) strtol(line, NULL, 10);
	if(dim == 0) {
		fail(ERR_FORMAT, "read_array", "Invalid file format.");
	}

	int **arr = calloc(dim, sizeof(int *));
	char *tok;
	int j;
	// assuming the matrix is correctly formatted if the previous check was passed
	for(int i = 0; i < dim; i++) {
		arr[i] = calloc(dim, sizeof(int));
		line = get_line(fp);
		j = 0;
		for(tok = strtok(line, " "); tok; tok = strtok(NULL, " ")) {
			if(j < dim) {
				arr[i][j++] = (int) strtol(tok, NULL, 10);
			}
		}
	}

	fclose(fp);

	if(dim_ptr != NULL) {
		*dim_ptr = dim;
	}

	return(arr);
}

void write_array(char *filename, int dim, int **arr) {
	FILE *fp = fopen(filename, "wb");
	if(fp == NULL) {
		fail(ERR_OPEN, "write_array", "Cannot open file.");
	}
	fprintf(fp, "%i\n", dim);
	for(int i = 0; i < dim; i++) {
		fprintf(fp, "%i", arr[i][0]);
		for(int j = 1; j < dim; j++) {
			fprintf(fp, " %i", arr[i][j]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

char *get_line(FILE *fp) {
	if(MAX_BUF < 2) {
		fail(ERROR, "get_line", "MAX_BUF is too small.");
	}

	// keeps track of the current line being constructed
	char *line = calloc(MAX_BUF, sizeof(char));
	int line_size = MAX_BUF;

	char buf[MAX_BUF];

	// read from the file MAX_BUF bytes at a time
	while(fgets(buf, MAX_BUF, fp) != NULL) {
		if(strlen(buf) + strlen(line) + 1 >= line_size) {
			line_size += strlen(buf);
			line = (char *) realloc(line, line_size * sizeof(char));
		}
		strcat(line, buf);

		// return if the return character is found
		if(strstr(line, "\n") != NULL) {
			// strip the last newline character
			strtok(line, "\n");
			break;
		}
	}
	return(line);
}
