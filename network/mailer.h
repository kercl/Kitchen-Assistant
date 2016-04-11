#ifndef MAILER_H
#define MAILER_H

#include <curl/curl.h>
#include <string>
#include <vector>

class Mailer {
  int lines_read;
  std::vector<std::string> payload;
  
  static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp);
public:
  Mailer(std::string to, std::string from, std::string subj, std::string data);
};

#endif

