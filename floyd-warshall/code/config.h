#ifndef CONFIG_H
#define CONFIG_H

#define MAX_WEIGHT 100
#define MAX_LINE 1000
#define TEST 1

/**
 * error codes
 */
#define ERROR -1
#define SUCCESS 0

#define ERR_OPEN 1
#define ERR_FORMAT 2
#define ERR_DIM_MISMATCH 3
#define ERR_ARGS 4

/**
 * execution modes
 */
#define SERIAL 0

/**
 * thread configuration
 */
#define CHECKOUT_BLOCK 4

/**
 * global test result variable
 */
#include <stdint.h>
int *result;

#endif
