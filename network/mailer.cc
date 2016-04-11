#include "mailer.h"
#include "../storage/storage.h"

#include <stdio.h>
#include <string.h>
#include <cstring>
#include <iostream>

using namespace std;

size_t Mailer::payload_source(void *ptr, size_t size, size_t nmemb, void *userp) {
  Mailer *upload_ctx = (Mailer*)userp;
  const char *data;

  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }
    
  if(upload_ctx->lines_read < upload_ctx->payload.size()) {
    size_t len = upload_ctx->payload[upload_ctx->lines_read].length();
    memcpy(ptr, 
           upload_ctx->payload[upload_ctx->lines_read].c_str(), 
           len);
    upload_ctx->lines_read++;
    return len;
  }
    
  return 0;
}
  
Mailer::Mailer(std::string to, std::string from, std::string subj, std::string data) {
  payload.push_back(string("To: ") + to + "\r\n");
  payload.push_back(string("From: ") + from + "\r\n");
  payload.push_back(string("Subject: ") + subj + "\r\n");
  payload.push_back("\r\n");
  payload.push_back(data + "\r\n");
  
  lines_read = 0;
  
  CURL *curl = curl_easy_init();
  CURLcode res = CURLE_OK;
  struct curl_slist *recipients = NULL;
  if(curl) {
    std::string user, passwd;
    Storage::instance().get_setting("google-user-name", user);
    curl_easy_setopt(curl, CURLOPT_USERNAME, user.c_str());
    
    Storage::instance().get_setting("google-user-password", passwd);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, passwd.c_str());
    
    curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from.c_str());
    recipients = curl_slist_append(recipients, to.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, Mailer::payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, this);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    res = curl_easy_perform(curl);

    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
  }
}

