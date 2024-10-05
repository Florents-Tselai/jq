#include "jqllm.h"

typedef struct memory {
    char *ptr;
    size_t size;
} memory;

jv f_http_header_2(SPQL_FUNCTIONS_ARGS_2) {
    jv_free(input);
    assert(jv_get_kind(a) == JV_KIND_STRING && jv_get_kind(b) == JV_KIND_STRING);

    return JV_ARRAY_2(a, b);
}

static size_t
write_cb(void *contents, size_t size, size_t nmemb, void *userp) {
    struct memory *mem = userp;
    size_t realsize = size * nmemb;
    char *ptr = realloc(mem->ptr, mem->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "not enough memory\n");
        return 0;
    }

    mem->ptr = ptr;
    memcpy(&mem->ptr[mem->size], contents, realsize);
    mem->size += realsize;
    mem->ptr[mem->size] = 0;

    return realsize;
}

jv __jq_http_get(const char *url) {
    jv res = jv_null();

    CURL *easy;
    CURLcode curl_code;
    struct memory chunk;

    chunk.ptr = malloc(1); /* grown as needed */
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);

    easy = curl_easy_init();

    curl_easy_setopt(easy, CURLOPT_URL, url);
    curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(easy, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(easy, CURLOPT_USERAGENT, "jqllm");

    curl_code = curl_easy_perform(easy);

    if (curl_code == CURLE_OK)
        res = jv_parse(chunk.ptr);

    curl_easy_cleanup(easy);

    curl_global_cleanup();

    return res;
}

jv __jq_http_post(const char *url) {
    jv res = jv_null();


    CURL *easy;
    CURLcode curl_code;
    struct memory chunk;

    chunk.ptr = malloc(1); /* grown as needed */
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);

    easy = curl_easy_init();

    curl_easy_setopt(easy, CURLOPT_URL, url);
    curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(easy, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(easy, CURLOPT_USERAGENT, "jqllm");

    curl_code = curl_easy_perform(easy);

    if (curl_code == CURLE_OK)
        res = jv_parse(chunk.ptr);

    curl_easy_cleanup(easy);

    curl_global_cleanup();

    return res;
}


// Callback function to write received data
size_t _post_write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t realsize = size * nmemb;
    char **response_ptr = (char **) userdata;

    // Allocate memory for response data
    *response_ptr = (char *) realloc(*response_ptr, realsize + 1);
    if (*response_ptr == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 0; // Returning 0 indicates failure
    }

    // Copy received data to the response buffer
    memcpy(*response_ptr, ptr, realsize);
    (*response_ptr)[realsize] = '\0'; // Null-terminate the string

    return realsize;
}

jv http_post(jv url, jv body, jv headers) {
    jv res = jv_null();

    const char *body_str = jv_string_value(jv_dump_string(body, 0));
    size_t payload_size = strlen(body_str);

    CURLcode curl_code;
    CURL *hnd;
    struct curl_slist *headers_list;
    char *response = NULL;

    headers_list = NULL;

    headers_list = curl_slist_append(headers_list, "Content-Type: aplication/json");


    hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(hnd, CURLOPT_URL, jv_string_value(url));
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, body_str);
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)payload_size);
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers_list);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, DEFAULT_USERAGENT);
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, _post_write_callback);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &response);

    curl_code = curl_easy_perform(hnd);

    if (curl_code == CURLE_OK) {
        res = jv_parse(response);
    } else {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(curl_code));
    }

    free(response);
    curl_easy_cleanup(hnd);
    curl_slist_free_all(headers_list);

    return res;
}

jv f_http_get(SPQL_FUNCTIONS_ARGS_0) {
    return __jq_http_get(jv_string_value(input));
}

jv f_http_post_2(SPQL_FUNCTIONS_ARGS_2) {
    return http_post(a, input, b);
}

#include "math.h"

int is_valid_vector(jv i) {
    /* TODO: also validate it's an array of numbers*/
    return jv_get_kind(i) == JV_KIND_ARRAY;
}

vector init_vector(jv i) {
    assert(is_valid_vector(i));

    return i;
}

jv l2_norm(vector a) {
    double norm = 0.0;

    vector_foreach(a, i, ax) {
        norm += ax * ax;
    }

    return jv_number(sqrt(norm));
}

jv cos_distance(vector a, vector b) {
    check_dims(a, b);

    float distance = 0.0;
    float norma = 0.0;
    float normb = 0.0;
    double similarity;

    vector_foreach(a, i, ax) {
        double bx = vector_get(b, i);

        distance += ax * bx;
        norma += ax * ax;
        normb += bx * bx;
    }

    /* Use sqrt(a * b) over sqrt(a) * sqrt(b) */
    similarity = (double) distance / sqrt((double) norma * (double) normb);

    /* Keep in range */
    if (similarity > 1)
        similarity = 1.0;
    else if (similarity < -1)
        similarity = -1.0;

    return jv_number(1 - similarity);
}

jv cos_similarity(vector a, vector b) {
    return jv_number(1 - jv_number_value(cos_distance(a, b)));
}

jv l2_distance(vector a, vector b) {
    check_dims(a, b);

    float distance = 0.0;
    float diff;

    vector_foreach(a, i, x) {
        diff = x - vector_get(b, i);
        distance += diff * diff;
    }

    return jv_number(sqrt(distance));
}

jv dot_product(vector a, vector b) {
    check_dims(a, b);

    float distance = 0.0;

    vector_foreach(a, i, x) {
        distance += x * vector_get(b, i);
    }
    return jv_number(distance);
}

jv knn_search(jv candidates, vector v, int k, DISTANCE_MEASURE measure, KNN_ALGO algo) {
    jv res = JV_ARRAY(JV_ARRAY(jv_number(1), jv_number(2), jv_number(3), jv_number(4)));
    double *distance_matrix;
    int n = jv_array_length(candidates);

    /* stores the distances between each candidate and v */
    distance_matrix = (double *) malloc(n * sizeof(double));

    if (distance_matrix == NULL) {
        printf("Memory allocation failed\n");
        return jv_invalid();
    }

    for (int i = 0; i < n; i++) {
        distance_matrix[i] = 0.0;
    }

    //    printf("candidates length=%d\n", jv_array_length(candidates));

    //    jv_array_foreach(jv_copy(candidates), i, c) {
    //            switch (measure) {
    //                case L2_DISTANCE: {
    //                    printf("i=%d", i);
    //                    distance_matrix[i] = jv_number_value(l2_distance(v, c));
    //                }
    //            }
    //        }

    free(distance_matrix);
    return res;
}


/* ------------------------------- Vectors ------------------------------------ */

jv f_vector_init(SPQL_FUNCTIONS_ARGS_0) {
    return init_vector(input);
}

jv f_vector_ndims(SPQL_FUNCTIONS_ARGS_0) {
    return jv_number(ndims(input));
}

jv f_vector_l2_norm(SPQL_FUNCTIONS_ARGS_0) {
    return l2_norm(input);
}

jv f_vector_l2_distance(SPQL_FUNCTIONS_ARGS_1) {
    jv ret = l2_distance(jv_copy(input), jv_copy(a));
    jv_free(a);
    return ret;
}

jv f_vector_cosine_distance(SPQL_FUNCTIONS_ARGS_1) {
    jv ret = cos_distance(jv_copy(input), jv_copy(a));
    jv_free(a);
    return ret;
}

jv f_vector_cosine_similarity(SPQL_FUNCTIONS_ARGS_1) {
    jv ret = cos_similarity(jv_copy(input), jv_copy(a));
    jv_free(a);
    return ret;
}

jv f_vector_dot_product(SPQL_FUNCTIONS_ARGS_1) {
    jv ret = dot_product(jv_copy(input), jv_copy(a));
    jv_free(a);
    return ret;
}

jv f_vector_knn_search_2(SPQL_FUNCTIONS_ARGS_2) {
    jv candidates = input;
    jv v = a;
    int k = jv_number_value(b);
    return knn_search(candidates, v, 1, L2_DISTANCE, EXACT);
}

#define JQLLM_DEFAULT_GENERATE_MODEL "llama3.2"

jv f_prompt_1(SPQL_FUNCTIONS_ARGS_1) {
    const char *prompt = jv_string_value(input);
    const char *model = jv_string_value(a);

    // jv request = jv_object();

    /* See https://github.com/ollama/ollama/blob/main/docs/api.md#generate-a-completion*/

    jv request = JV_OBJECT(jv_string("prompt"), input,
                           jv_string("model"), jv_string("string"),
                           jv_string("stream"), jv_false()
    );
    // request = jv_object_set(request, jv_string("prompt"), input);
    // request = jv_object_set(request, jv_string("model"), a);
    // request = jv_object_set(request, jv_string("stream"), jv_false());

    // return jv_dump_string(request, 0);
    return request;
}

jv f_prompt_0(SPQL_FUNCTIONS_ARGS_0) {
    return f_prompt_1(jq, input, jv_string(JQLLM_DEFAULT_GENERATE_MODEL));
}

jv f_http_4(SPQL_FUNCTIONS_ARGS_4) {
    assert(jv_get_kind(a)== JV_KIND_STRING);
    assert(jv_get_kind(b) == JV_KIND_STRING);
    assert(jv_get_kind(c) == JV_KIND_ARRAY);
    assert(jv_get_kind(d) == JV_KIND_STRING);

    jv body = input;
    const char *body_str = jv_string_value(jv_dump_string(body, 0));
    size_t payload_size = strlen(body_str);

    const char *url = jv_string_value(b);

    jv res = jv_null();

    char *response = NULL;

    struct curl_slist *headers_list = NULL;

    headers_list = curl_slist_append(headers_list, "Content-Type: application/json");

    CURL *easy = curl_easy_init();
    curl_easy_setopt(easy, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(easy, CURLOPT_URL, url);
    curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 1L);


    curl_easy_setopt(easy, CURLOPT_HTTPHEADER, headers_list);
    curl_easy_setopt(easy, CURLOPT_USERAGENT, DEFAULT_USERAGENT);
    curl_easy_setopt(easy, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(easy, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(easy, CURLOPT_FTP_SKIP_PASV_IP, 1L);
    curl_easy_setopt(easy, CURLOPT_TCP_KEEPALIVE, 1L);

    if (strcmp(jv_string_value(a), "POST") == 0) {
        curl_easy_setopt(easy, CURLOPT_POSTFIELDS, body_str);
        curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)payload_size);
    }

    // Set callback function to receive data
    curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, _post_write_callback);
    curl_easy_setopt(easy, CURLOPT_WRITEDATA, &response);

    CURLcode curl_code = curl_easy_perform(easy);

    if (curl_code == CURLE_OK) {
        res = jv_parse(response);
    } else {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(curl_code));
    }

    free(response);
    curl_easy_cleanup(easy);
    curl_slist_free_all(headers_list);
    jv_free(a);
    jv_free(b);
    jv_free(c);
    jv_free(d);

    return res;
}
