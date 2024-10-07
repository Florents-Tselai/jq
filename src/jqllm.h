#ifndef JQLLM_H
#define JQLLM_H

#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "jv.h"
#include "curl/curl.h"
#include "jq.h"
#include "jv.h"
#include "util.h"
#include "math.h"


#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include "jq.h"
#include "jv.h"
#include <dirent.h>
#include <errno.h>
#include <assert.h>

#include "curl/curl.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>

/*
* #########################################################################################################
*                                       fmgr
*
* The following is inspired by Postgres' fmgrh.h.
*
* When the programmer runs `jq '... | llm_xxx(a, b,...)',
* behind the scenes there's a function that does the real work
* and looks like this:
*
* jv f_llm_xxxx(jq_state *jq, jv input, jv a, jv b, ...) {
*      .....
* }
* #########################################################################################################
*/

#define SPQL_FUNCTIONS_ARGS_0 jq_state *jq, jv input
#define SPQL_FUNCTIONS_ARGS_1 SPQL_FUNCTIONS_ARGS_0, jv a
#define SPQL_FUNCTIONS_ARGS_2 SPQL_FUNCTIONS_ARGS_1, jv b
#define SPQL_FUNCTIONS_ARGS_3 SPQL_FUNCTIONS_ARGS_2, jv c
#define SPQL_FUNCTIONS_ARGS_4 SPQL_FUNCTIONS_ARGS_3, jv d
#define SPQL_FUNCTIONS_ARGS_5 SPQL_FUNCTIONS_ARGS_4, jv e


#define FUNC_NARGS_0 1
#define FUNC_NARGS_1 2
#define FUNC_NARGS_2 3
#define FUNC_NARGS_3 4
#define FUNC_NARGS_4 5

#define JQLLM_BUILTIN_FUNC(func, name, nargs) {(func), name, (nargs)}


/*
* #########################################################################################################
*                                       HTTP
* #########################################################################################################
*/

#define DEFAULT_USERAGENT "jqllm/v0.1.0a1"
jv f_http_4(SPQL_FUNCTIONS_ARGS_4);

/*
* #########################################################################################################
*                                       Vector
* #########################################################################################################
*/

typedef jv vector;

#define VECTOR_MAX_DIM 16000


#define ndims(v) jv_array_length(v)
#define vector_free(v) jv_free(v)
#define vector_get(v, i) jv_number_value(jv_array_get(jv_copy(v), (i)))

static inline void check_dims(vector a, vector b) {
    assert(ndims(a) == ndims(b));
}

/* Macro to iterate over a vector. inspired by jv_array_foreach */
#define vector_foreach(v, i, x) \
        for (int jv_len__ = jv_array_length(jv_copy(v)), i=0, jv_j__ = 1;   \
        jv_j__; jv_j__ = 0)                                                 \
        for (double x;                                                      \
        i < jv_len__ ?                                                      \
        (x = jv_number_value(jv_array_get(jv_copy(v), i)), 1) : 0;          \
        i++)

/* Distance measure between 2 vectors */
typedef enum {
    L1_DISTANCE,
    L2_DISTANCE,
    EUCLIDIAN_DISTANCE,
    SPHERICAL_DISTANCE
} DISTANCE_MEASURE;

typedef enum {
    EXACT,
    HNSW,
} KNN_ALGO;

jv f_vector_init(SPQL_FUNCTIONS_ARGS_0);

jv f_vector_ndims(SPQL_FUNCTIONS_ARGS_0);

jv f_vector_l2_norm(SPQL_FUNCTIONS_ARGS_0);

jv f_vector_l2_distance(SPQL_FUNCTIONS_ARGS_1);

jv f_vector_dot_product(SPQL_FUNCTIONS_ARGS_1);

jv f_vector_cosine_distance(SPQL_FUNCTIONS_ARGS_1);

jv f_vector_cosine_similarity(SPQL_FUNCTIONS_ARGS_1);

jv f_vector_knn_search_2(SPQL_FUNCTIONS_ARGS_2);

jv knn_search(jv candidates, vector v, int k, DISTANCE_MEASURE measure, KNN_ALGO algo);

/*
* #########################################################################################################
*                                       Generate
* #########################################################################################################
*/

/*
* #########################################################################################################
*                                       Embed
* #########################################################################################################
*/


jv f_prompt_1(SPQL_FUNCTIONS_ARGS_1);

jv f_prompt_0(SPQL_FUNCTIONS_ARGS_0);

#define HTTP_OPERATIONS \
    JQLLM_BUILTIN_FUNC(f_http_4,                    "http",                 FUNC_NARGS_4)

#define VECTOR_OPERATIONS \
    JQLLM_BUILTIN_FUNC(f_vector_init,               "vector",               FUNC_NARGS_0), \
    JQLLM_BUILTIN_FUNC(f_vector_ndims,              "ndims",                FUNC_NARGS_0), \
    JQLLM_BUILTIN_FUNC(f_vector_l2_norm,            "l2_norm",              FUNC_NARGS_0), \
    JQLLM_BUILTIN_FUNC(f_vector_l2_distance,        "l2_distance",          FUNC_NARGS_1), \
    JQLLM_BUILTIN_FUNC(f_vector_dot_product,        "dot_product",          FUNC_NARGS_1), \
    JQLLM_BUILTIN_FUNC(f_vector_cosine_distance,    "cosine_distance",      FUNC_NARGS_1), \
    JQLLM_BUILTIN_FUNC(f_vector_cosine_similarity,  "cosine_similarity",    FUNC_NARGS_1), \
    JQLLM_BUILTIN_FUNC(f_vector_knn_search_2,       "knn_search",           FUNC_NARGS_2)

#define JQLLM_BUILTINS \
    VECTOR_OPERATIONS, \
    HTTP_OPERATIONS

#endif
