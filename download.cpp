#include <stdio.h>
#include <curl/curl.h>

int main(void) {
    CURL *curl;
    FILE *file;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {
        file = fopen("database2.db", "wb");
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8002/database.db");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        fclose(file);
    }

    return 0;
}
