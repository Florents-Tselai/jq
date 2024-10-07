#include "jqllm.h"




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



typedef struct {
    char *buf;
    size_t size;
} memory;

size_t grow_buffer(void *contents, size_t sz, size_t nmemb, void *ctx) {
    size_t realsize = sz * nmemb;
    memory *mem = (memory *) ctx;

    // Attempt to realloc with extra space to reduce frequent realloc calls
    char *ptr = realloc(mem->buf, mem->size + realsize + 4096);  // Adding extra space for larger responses
    if (!ptr) {
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    mem->buf = ptr;
    memcpy(&(mem->buf[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->buf[mem->size] = 0; // Ensure null-termination of the buffer
    return realsize;
}


jv f_http_4(SPQL_FUNCTIONS_ARGS_4) {
    assert(jv_get_kind(a)== JV_KIND_STRING);
    assert(jv_get_kind(b) == JV_KIND_STRING);
    assert(jv_get_kind(c) == JV_KIND_ARRAY);
    assert(jv_get_kind(d) == JV_KIND_STRING);

    jv body = input;
    const char *body_str = jv_string_value(jv_dump_string(body, 0));

    const char *url = jv_string_value(b);

    jv res = jv_null();


    struct curl_slist *headers_list = NULL;

    headers_list = curl_slist_append(headers_list, "Content-Type: application/json");

    CURL *easy = curl_easy_init();
    curl_easy_setopt(easy, CURLOPT_URL, url);

    curl_easy_setopt(easy, CURLOPT_HTTPHEADER, headers_list);
    curl_easy_setopt(easy, CURLOPT_TIMEOUT, 1200L);   // Increase timeout
    curl_easy_setopt(easy, CURLOPT_BUFFERSIZE, 102400L);  // Increase buffer size


    if (strcmp(jv_string_value(a), "POST") == 0)
        curl_easy_setopt(easy, CURLOPT_POSTFIELDS, body_str);

    memory *mem = malloc(sizeof(memory));
    mem->size = 0;
    mem->buf = malloc(1);
    curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, grow_buffer);
    curl_easy_setopt(easy, CURLOPT_WRITEDATA, mem);

    CURLcode curl_code = curl_easy_perform(easy);

    if (curl_code == CURLE_OK) {
        res = jv_parse_sized(mem->buf, mem->size);
    } else {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(curl_code));
    }

    curl_easy_cleanup(easy);
    curl_slist_free_all(headers_list);
    free(mem->buf);
    free(mem);
    return res;
}
