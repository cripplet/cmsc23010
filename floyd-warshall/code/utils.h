#ifndef UTILS_H
#define UTILS_H

void fail(int, char *, char *);
void debug(char *, char *);
void notice(char *, char *);

void usage(char *);

void fw_log(int, char *, char *, char *, int, int, double);

int **initialize_matrix(int n, int[n][n]);

#endif
