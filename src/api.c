
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cub3D.h" // 這裡包含 t_data 定義
#include <cjson/cJSON.h>

#define API_KEY "put api key here"
struct memory {
    char *response;
    size_t size;
};


static size_t write_callback(void *data, size_t size, size_t nmemb, void *userp)
{
	strncat((char *)userp, data, size * nmemb);
	return size * nmemb;
}

// 
void call_chatgpt(const char *prompt, t_data *data)
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
