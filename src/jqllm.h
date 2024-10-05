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

/* ------------------------------- Functions ------------------------------------ */

/*
 * The following is inspired by Postgres' fmgrh.h.
 *
 * When the programmer runs `jq '... | llm_xxx(a, b,...)',
 * behind the scenes there's a function that does the real work
 * and looks like this:
 *
 * jv f_llm_xxxx(jq_state *jq, jv input, jv a, jv b, ...) {
 *      .....
 * }
 *
 */

/* Macros for functions */
#define SPQL_FUNCTIONS_ARGS_0 jq_state *jq, jv input
#define SPQL_FUNCTIONS_ARGS_1 SPQL_FUNCTIONS_ARGS_0, jv a
#define SPQL_FUNCTIONS_ARGS_2 SPQL_FUNCTIONS_ARGS_1, jv b
#define SPQL_FUNCTIONS_ARGS_3 SPQL_FUNCTIONS_ARGS_2, jv c
#define SPQL_FUNCTIONS_ARGS_4 SPQL_FUNCTIONS_ARGS_3, jv d
#define SPQL_FUNCTIONS_ARGS_5 SPQL_FUNCTIONS_ARGS_4, jv e

/* Number of args for function */
#define FUNC_NARGS_0 1
#define FUNC_NARGS_1 2
#define FUNC_NARGS_2 3
#define FUNC_NARGS_3 4
#define FUNC_NARGS_4 5


#define JQLLM_BUILTIN_FUNC(func, name, nargs) \
{(func), name, (nargs)}

#define DEFAULT_USERAGENT "spql/v0.1.0a1"
//
/* HTTP request methods we support */
typedef enum {
    HTTP_GET,
    HTTP_POST,
    HTTP_DELETE,
    HTTP_PUT,
    HTTP_HEAD,
    HTTP_PATCH,
    HTTP_UNKNOWN
} http_method;

//
///* Components (and postitions) of the http_request tuple type */
//enum {
//    REQ_METHOD = 0,
//    REQ_URI = 1,
//    REQ_HEADERS = 2,
//    REQ_CONTENT_TYPE = 3,
//    REQ_CONTENT = 4
//} http_request_type;
//
///* Components (and postitions) of the http_response tuple type */
//enum {
//    RESP_STATUS = 0,
//    RESP_CONTENT_TYPE = 1,
//    RESP_HEADERS = 2,
//    RESP_CONTENT = 3
//} http_response_type;
//
///* Components (and postitions) of the http_header tuple type */
//enum {
//    HEADER_FIELD = 0,
//    HEADER_VALUE = 1
//} http_header_type;
//
typedef enum {
    CURLOPT_STRING,
    CURLOPT_LONG
} http_curlopt_type;

/* CURLOPT string/enum value mapping */
typedef struct {
    char *curlopt_str;
    char *curlopt_val;
    CURLoption curlopt;
    http_curlopt_type curlopt_type;
    bool superuser_only;
} http_curlopt;

//
//
///* CURLOPT values we allow user to set at run-time */
///* Be careful adding these, as they can be a security risk */
//static http_curlopt settable_curlopts[] = {
//        { "CURLOPT_CAINFO", NULL, CURLOPT_CAINFO, CURLOPT_STRING, false },
//        { "CURLOPT_TIMEOUT", NULL, CURLOPT_TIMEOUT, CURLOPT_LONG, false },
//        { "CURLOPT_TIMEOUT_MS", NULL, CURLOPT_TIMEOUT_MS, CURLOPT_LONG, false },
//        { "CURLOPT_CONNECTTIMEOUT", NULL, CURLOPT_CONNECTTIMEOUT, CURLOPT_LONG, false },
//        { "CURLOPT_USERAGENT", NULL, CURLOPT_USERAGENT, CURLOPT_STRING, false },
//        { "CURLOPT_USERPWD", NULL, CURLOPT_USERPWD, CURLOPT_STRING, false },
//        { "CURLOPT_IPRESOLVE", NULL, CURLOPT_IPRESOLVE, CURLOPT_LONG, false },
//#if LIBCURL_VERSION_NUM >= 0x070903 /* 7.9.3 */
//        { "CURLOPT_SSLCERTTYPE", NULL, CURLOPT_SSLCERTTYPE, CURLOPT_STRING, false },
//#endif
//#if LIBCURL_VERSION_NUM >= 0x070e01 /* 7.14.1 */
//        { "CURLOPT_PROXY", NULL, CURLOPT_PROXY, CURLOPT_STRING, false },
//        { "CURLOPT_PROXYPORT", NULL, CURLOPT_PROXYPORT, CURLOPT_LONG, false },
//#endif
//#if LIBCURL_VERSION_NUM >= 0x071301 /* 7.19.1 */
//        { "CURLOPT_PROXYUSERNAME", NULL, CURLOPT_PROXYUSERNAME, CURLOPT_STRING, false },
//        { "CURLOPT_PROXYPASSWORD", NULL, CURLOPT_PROXYPASSWORD, CURLOPT_STRING, false },
//#endif
//#if LIBCURL_VERSION_NUM >= 0x071504 /* 7.21.4 */
//        { "CURLOPT_TLSAUTH_USERNAME", NULL, CURLOPT_TLSAUTH_USERNAME, CURLOPT_STRING, false },
//        { "CURLOPT_TLSAUTH_PASSWORD", NULL, CURLOPT_TLSAUTH_PASSWORD, CURLOPT_STRING, false },
//        { "CURLOPT_TLSAUTH_TYPE", NULL, CURLOPT_TLSAUTH_TYPE, CURLOPT_STRING, false },
//#endif
//#if LIBCURL_VERSION_NUM >= 0x071800 /* 7.24.0 */
//        { "CURLOPT_DNS_SERVERS", NULL, CURLOPT_DNS_SERVERS, CURLOPT_STRING, false },
//#endif
//#if LIBCURL_VERSION_NUM >= 0x071900 /* 7.25.0 */
//        { "CURLOPT_TCP_KEEPALIVE", NULL, CURLOPT_TCP_KEEPALIVE, CURLOPT_LONG, false },
//        { "CURLOPT_TCP_KEEPIDLE", NULL, CURLOPT_TCP_KEEPIDLE, CURLOPT_LONG, false },
//#endif
//#if LIBCURL_VERSION_NUM >= 0x072500 /* 7.37.0 */
//        { "CURLOPT_SSL_VERIFYHOST", NULL, CURLOPT_SSL_VERIFYHOST, CURLOPT_LONG, false },
//        { "CURLOPT_SSL_VERIFYPEER", NULL, CURLOPT_SSL_VERIFYPEER, CURLOPT_LONG, false },
//#endif
//        { "CURLOPT_SSLCERT", NULL, CURLOPT_SSLCERT, CURLOPT_STRING, false },
//        { "CURLOPT_SSLKEY", NULL, CURLOPT_SSLKEY, CURLOPT_STRING, false },
//#if LIBCURL_VERSION_NUM >= 0x073400  /* 7.52.0 */
//        { "CURLOPT_PRE_PROXY", NULL, CURLOPT_PRE_PROXY, CURLOPT_STRING, false },
//        { "CURLOPT_PROXY_CAINFO", NULL, CURLOPT_PROXY_TLSAUTH_USERNAME, CURLOPT_STRING, false },
//        { "CURLOPT_PROXY_TLSAUTH_USERNAME", NULL, CURLOPT_PROXY_TLSAUTH_USERNAME, CURLOPT_STRING, false },
//        { "CURLOPT_PROXY_TLSAUTH_PASSWORD", NULL, CURLOPT_PROXY_TLSAUTH_PASSWORD, CURLOPT_STRING, false },
//        { "CURLOPT_PROXY_TLSAUTH_TYPE", NULL, CURLOPT_PROXY_TLSAUTH_TYPE, CURLOPT_STRING, false },
//#endif
//        { NULL, NULL, 0, 0, false } /* Array null terminator */
//};


#define VECTOR_MAX_DIM 16000

typedef jv vector;

#define ndims(v) jv_array_length(v)
#define vector_free(v) jv_free(v)
#define vector_get(v, i) jv_number_value(jv_array_get(jv_copy(v), (i)))

static inline void check_dims(vector a, vector b) {
    assert(ndims(a) == ndims(b));
}

/* Macro to iterate over a vector. inspired by jv_array_foreach */
#define vector_foreach(v, i, x) \
for (int jv_len__ = jv_array_length(jv_copy(v)), i=0, jv_j__ = 1;     \
jv_j__; jv_j__ = 0)                                              \
for (double x;                                                      \
i < jv_len__ ?                                                 \
(x = jv_number_value(jv_array_get(jv_copy(v), i)), 1) : 0;                    \
i++)

/* Distance measure between 2 vectors */
typedef enum {
    L1_DISTANCE,
    L2_DISTANCE,
    EUCLIDIAN_DISTANCE,
    SPHERICAL_DISTANCE
} DISTANCE_MEASURE;

/* ------------------------------- API ---------------------------- */

int is_valid_vector(jv);

vector init_vector(jv);

/* vector math */
jv l2_norm(vector);

jv cos_distance(vector, vector);

jv cos_similarity(vector, vector);

jv l2_distance(vector, vector);

jv dot_product(vector, vector);

typedef enum {
    EXACT,
    HNSW,
} KNN_ALGO;


jv f_http_header_2(SPQL_FUNCTIONS_ARGS_2);

jv f_http_get(SPQL_FUNCTIONS_ARGS_0);

jv f_http_post_2(SPQL_FUNCTIONS_ARGS_2);

jv f_http_4(SPQL_FUNCTIONS_ARGS_4);

/* knn */
jv knn_search(jv candidates, vector v, int k, DISTANCE_MEASURE measure, KNN_ALGO algo);

jv f_vector_init(SPQL_FUNCTIONS_ARGS_0);

jv f_vector_ndims(SPQL_FUNCTIONS_ARGS_0);

jv f_vector_l2_norm(SPQL_FUNCTIONS_ARGS_0);

jv f_vector_l2_distance(SPQL_FUNCTIONS_ARGS_1);

jv f_vector_dot_product(SPQL_FUNCTIONS_ARGS_1);

jv f_vector_cosine_distance(SPQL_FUNCTIONS_ARGS_1);

jv f_vector_cosine_similarity(SPQL_FUNCTIONS_ARGS_1);

jv f_vector_knn_search_2(SPQL_FUNCTIONS_ARGS_2);


jv f_prompt_1(SPQL_FUNCTIONS_ARGS_1);

jv f_prompt_0(SPQL_FUNCTIONS_ARGS_0);

#define VECTOR_OPERATIONS \
JQLLM_BUILTIN_FUNC(f_vector_init,               "vector",               FUNC_NARGS_0), \
JQLLM_BUILTIN_FUNC(f_vector_ndims,              "ndims",                FUNC_NARGS_0), \
JQLLM_BUILTIN_FUNC(f_vector_l2_norm,            "l2_norm",              FUNC_NARGS_0), \
JQLLM_BUILTIN_FUNC(f_vector_l2_distance,        "l2_distance",          FUNC_NARGS_1), \
JQLLM_BUILTIN_FUNC(f_vector_dot_product,        "dot_product",          FUNC_NARGS_1), \
JQLLM_BUILTIN_FUNC(f_vector_cosine_distance,    "cosine_distance",      FUNC_NARGS_1), \
JQLLM_BUILTIN_FUNC(f_vector_cosine_similarity,  "cosine_similarity",    FUNC_NARGS_1), \
JQLLM_BUILTIN_FUNC(f_vector_knn_search_2,       "knn_search",           FUNC_NARGS_2)

#define HTTP_OPERATIONS \
    JQLLM_BUILTIN_FUNC(f_http_4,                    "http",                 FUNC_NARGS_4)


    #define PROMPT_FUNCTIONS \
    JQLLM_BUILTIN_FUNC(f_prompt_1,                  "generate",               FUNC_NARGS_1), \
    JQLLM_BUILTIN_FUNC(f_prompt_0,                  "generate",               FUNC_NARGS_0)

#define JQLLM_BUILTINS \
VECTOR_OPERATIONS, \
HTTP_OPERATIONS, \
PROMPT_FUNCTIONS

jv __jq_http_get(const char *url);

jv __jq_http_post(const char *url);

jv http_post(jv url, jv body, jv headers);


#endif
