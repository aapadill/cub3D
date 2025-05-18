#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cjson/cJSON.h>

#define API_KEY ""
// #define OUTPUT_PATH    "./textures/hand/generated.png"

// Base64 decoding tables and functions
static const char encoding_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";
static unsigned char *decoding_table = NULL;
// static int mod_table[] = {0, 2, 1};

void build_decoding_table() {
    decoding_table = malloc(256);
    for (int i = 0; i < 64; i++) {
        decoding_table[(unsigned char)encoding_table[i]] = i;
    }
}

unsigned char *base64_decode(const char *data, size_t *out_len) {
    if (!decoding_table) build_decoding_table();
    size_t input_length = strlen(data);
    *out_len = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*out_len)--;
    if (data[input_length - 2] == '=') (*out_len)--;

    unsigned char *decoded_data = malloc(*out_len);
    if (!decoded_data) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];

        uint32_t triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;

        if (j < *out_len) decoded_data[j++] = (triple >> 16) & 0xFF;
        if (j < *out_len) decoded_data[j++] = (triple >> 8) & 0xFF;
        if (j < *out_len) decoded_data[j++] = triple & 0xFF;
    }
    return decoded_data;
}

// A dynamically resizing buffer for HTTP responses
struct memory {
    char *response;
    size_t size;
    size_t capacity;
};

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct memory *mem = (struct memory *)userp;
    if (mem->size + realsize + 1 > mem->capacity) {
        size_t new_capacity = mem->capacity * 2;
        while (new_capacity < mem->size + realsize + 1) new_capacity *= 2;
        char *ptr = realloc(mem->response, new_capacity);
        if (!ptr) realsize = mem->capacity - mem->size - 1;
        else { mem->response = ptr; mem->capacity = new_capacity; }
    }
    memcpy(mem->response + mem->size, contents, realsize);
    mem->size += realsize;
    mem->response[mem->size] = '\0';
    return size * nmemb;
}

int download_image(const char *url, const char *filename) {
    CURL *curl = curl_easy_init(); if (!curl) return 1;
    FILE *fp = fopen(filename, "wb"); if (!fp) { curl_easy_cleanup(curl); return 2; }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    CURLcode res = curl_easy_perform(curl);
    fclose(fp);
    curl_easy_cleanup(curl);
    return (res == CURLE_OK) ? 0 : 3;
}

int generate_with_gpt_image(const char *prompt, const char *save_path) {
    CURL *curl = curl_easy_init(); if (!curl) return 1;
    struct curl_slist *headers = NULL;
    char auth_hdr[1024];
    snprintf(auth_hdr, sizeof(auth_hdr), "Authorization: Bearer %s", API_KEY);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth_hdr);

    // Build request body (default returns `b64_json`)
    char body[1024];
    snprintf(body, sizeof(body),
        "{"
          "\"model\": \"gpt-image-1\","  
          "\"prompt\": \"%s\","     
          "\"n\": 1,"                   
          "\"size\": \"1024x1024\""  //1024x1024
        "}", prompt
    );

    struct memory chunk = { .capacity = 4096, .size = 0 };
    chunk.response = malloc(chunk.capacity);
    if (!chunk.response) { curl_slist_free_all(headers); curl_easy_cleanup(curl); return 2; }
    chunk.response[0] = '\0';

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/images/generations");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0; curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    printf("HTTP status: %ld\nRaw JSON response:\n%s\n", http_code, chunk.response);

    if (res != CURLE_OK || http_code != 200) {
        fprintf(stderr, "❌ Request failed: %s\n",
                res != CURLE_OK ? curl_easy_strerror(res) : "non-OK HTTP status");
        goto cleanup;
    }

    cJSON *json = cJSON_Parse(chunk.response);
    if (!json) { fprintf(stderr, "❌ JSON parse error\n"); goto cleanup; }

    cJSON *err = cJSON_GetObjectItem(json, "error");
    if (err) {
        cJSON *msg = cJSON_GetObjectItem(err, "message");
        fprintf(stderr, "❌ API error: %s\n",
                msg && cJSON_IsString(msg) ? msg->valuestring : "unknown");
        cJSON_Delete(json); goto cleanup;
    }

    cJSON *data = cJSON_GetObjectItem(json, "data");
    cJSON *first = cJSON_GetArrayItem(data, 0);
    // Try URL first
    cJSON *url = cJSON_GetObjectItem(first, "url");
    if (url && cJSON_IsString(url)) {
        printf("🎨 URL: %s\n", url->valuestring);
        download_image(url->valuestring, save_path);
    } else {
        // Fallback to base64
        cJSON *b64 = cJSON_GetObjectItem(first, "b64_json");
        if (b64 && cJSON_IsString(b64)) {
            size_t out_len;
            unsigned char *img = base64_decode(b64->valuestring, &out_len);
            FILE *fp = fopen(save_path, "wb");
            fwrite(img, 1, out_len, fp);
            fclose(fp);
            free(img);
            printf("✅ Image written from base64 to %s\n", save_path);
        } else {
            fprintf(stderr, "❌ No usable image field in response\n");
        }
    }
    cJSON_Delete(json);

cleanup:
    free(chunk.response);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(decoding_table);
    decoding_table = NULL;
    return 0;
}