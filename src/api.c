
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cub3D.h" // 這裡包含 t_data 定義
#include <cjson/cJSON.h>
#define IMAGE_SAVE_PATH "./textures/hand/gun.png"

#define API_KEY ""
struct memory {
    char *response;
    size_t size;
};



static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t total = size * nmemb;
	strncat((char *)userp, contents, total);
	return total;
}

int download_image(const char *url, const char *filename)
{
	CURL *curl = curl_easy_init();
	if (!curl) return 1;

	FILE *fp = fopen(filename, "wb");
	if (!fp) return 2;

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	CURLcode res = curl_easy_perform(curl);

	fclose(fp);
	curl_easy_cleanup(curl);
	return res != CURLE_OK;
}

void call_dalle_with_reference(const char *prompt_text, const char *image_path)
{
	CURL *curl = curl_easy_init();
	if (!curl) return;

	struct curl_slist *headers = NULL;
	char auth[512];
	snprintf(auth, sizeof(auth), "Authorization: Bearer %s", API_KEY);
	headers = curl_slist_append(headers, auth);

	curl_mime *form = curl_mime_init(curl);
	curl_mimepart *part;

	// image
	part = curl_mime_addpart(form);
	curl_mime_name(part, "image");
	curl_mime_filedata(part, image_path);

	// prompt
	part = curl_mime_addpart(form);
	curl_mime_name(part, "prompt");
	curl_mime_data(part, prompt_text, CURL_ZERO_TERMINATED);

	// size
	part = curl_mime_addpart(form);
	curl_mime_name(part, "size");
	curl_mime_data(part, "256x256", CURL_ZERO_TERMINATED);

	// n (how many images)
	part = curl_mime_addpart(form);
	curl_mime_name(part, "n");
	curl_mime_data(part, "1", CURL_ZERO_TERMINATED);

	char response[8192] = {0};
	curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/images/edits");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		fprintf(stderr, "❌ CURL error: %s\n", curl_easy_strerror(res));
		goto cleanup;
	}

	printf("🧪 Raw response: %s\n", response);

	// parse & save image
	cJSON *json = cJSON_Parse(response);
	if (!json) goto cleanup;
	cJSON *data = cJSON_GetObjectItem(json, "data");
	cJSON *first = cJSON_GetArrayItem(data, 0);
	cJSON *url = cJSON_GetObjectItem(first, "url");
	if (url && url->valuestring)
	{
		printf("🎨 DALL·E generated image URL: %s\n", url->valuestring);
		download_image(url->valuestring, "./textures/hand/gun.png");
	}
	cJSON_Delete(json);

cleanup:
	curl_slist_free_all(headers);
	curl_mime_free(form);
	curl_easy_cleanup(curl);
}


/*
void call_dalle_with_base64(const char *prompt, const char *base64_image) {
    // Combine prefix with base64
    char *image_data = malloc(strlen(base64_image) + 32);
    if (!image_data) return;

    sprintf(image_data, "data:image/png;base64,%s", base64_image);

    // Compose JSON
    const char *json_template = "{ \"prompt\": \"%s\", \"image\": \"%s\" }";
    size_t json_size = strlen(prompt) + strlen(image_data) + strlen(json_template) + 1;
    char *json = malloc(json_size);
    if (!json) {
        free(image_data);
        return;
    }
    snprintf(json, json_size, json_template, prompt, image_data);

    // Send POST with curl
    CURL *curl = curl_easy_init();
    if (curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        // Add your OpenAI key securely
        char auth_header[512];
        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", API_KEY);
        headers = curl_slist_append(headers, auth_header);

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/images/edits"); // or /variations
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Curl error: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    free(json);
    free(image_data);
}*/

/*
void call_dalle(char *image_prompt, t_data *data)
{
	
	char response[8192] = {0};
	CURL *curl = curl_easy_init();
	if (!curl) {
		fprintf(stderr, "❌ curl init failed\n");
		return;
	}

	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");
	char auth[512];
	snprintf(auth, sizeof(auth), "Authorization: Bearer %s", API_KEY);
	headers = curl_slist_append(headers, auth);

	char body[2048];
	snprintf(body, sizeof(body),
		"{"
		"\"model\": \"dall-e-3\","
		"\"prompt\": \"%s\","
		"\"n\": 1,"
		"\"size\": \"1024x1024\""
		"}", image_prompt);

	curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/images/generations");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		fprintf(stderr, "❌ DALL·E API call failed: %s\n", curl_easy_strerror(res));
		goto cleanup;
	}

	// Parse the JSON
	cJSON *json = cJSON_Parse(response);
	if (!json) {
		fprintf(stderr, "❌ Failed to parse DALL·E JSON.\n");
		goto cleanup;
	}

	cJSON *data_array = cJSON_GetObjectItem(json, "data");
	cJSON *first = cJSON_GetArrayItem(data_array, 0);
	cJSON *url = cJSON_GetObjectItem(first, "url");

	if (url && url->valuestring) {
		printf("🖼️ DALL·E image URL: %s\n", url->valuestring);
		if (download_image(url->valuestring, IMAGE_SAVE_PATH) == 0) {
			printf("✅ Image downloaded to %s\n", IMAGE_SAVE_PATH);
		} else {
			printf("❌ Failed to download image.\n");
		}
	} else {
		printf("❌ No image URL found in DALL·E response.\n");
	}

	cJSON_Delete(json);

cleanup:
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
}

*/

void call_chatgpt( char *prompt, t_data *data)
{
	CURL *curl;
	CURLcode res;
	char response[4096] = {0};

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if (curl)
	{
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Content-Type: application/json");
		char auth_header[512];
		snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", API_KEY);
		headers = curl_slist_append(headers, auth_header);

		char body[2048];
		snprintf(body, sizeof(body),
			"{"
            "\"model\": \"gpt-4o\","
            "\"messages\": [{\"role\": \"user\", \"content\": \"%s\"}],"
            "\"temperature\": 0.7,"
            "\"max_tokens\": 50"
            "}", prompt);

		curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

		res = curl_easy_perform(curl);
		if (res == CURLE_OK)
		{
			printf("✅ GPT raw JSON response：\n%s\n", response);

			// JSON 解析
			cJSON *json = cJSON_Parse(response);
			if (!json) {
				printf("❌ Failed to parse JSON.\n");
				curl_easy_cleanup(curl);
				curl_slist_free_all(headers);
				curl_global_cleanup();
				return;
			}

			cJSON *choices = cJSON_GetObjectItem(json, "choices");
			cJSON *first_choice = cJSON_GetArrayItem(choices, 0);
			cJSON *message = cJSON_GetObjectItem(first_choice, "message");
			cJSON *content = cJSON_GetObjectItem(message, "content");

			if (content && cJSON_IsString(content)) {
				if (data->gun_description)
					free(data->gun_description);
				data->gun_description = strdup(content->valuestring);
				printf("🔧 GPT reply：%s\n", data->gun_description);
				// call DALL·E API
				//call_dalle(data->gun_description, data);
			} else {
				printf("❌ Failed to extract content from response.\n");
			}

			cJSON_Delete(json);
		}
		else
		{
			fprintf(stderr, "❌ failed to call GPT：%s\n", curl_easy_strerror(res));
		}

		curl_easy_cleanup(curl);
		curl_slist_free_all(headers);
	}
	curl_global_cleanup();
}
