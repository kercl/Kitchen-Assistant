#ifndef STREAM_H
#define STREAM_H

#include <string>
#include <fstream>
#include <boost/network/protocol/http/client.hpp>

using namespace boost::network;
using namespace boost::network::http;

typedef basic_client<http::tags::http_keepalive_8bit_tcp_resolve, 1, 1> http_stream_client;

class SimpleStream {
public:
  virtual std::string read(int bytes) = 0;
};

class HTTPStream: public SimpleStream {
  http_stream_client::request req;
  http_stream_client client;
public:
  HTTPStream(std::string source);
  std::string read(int bytes);
};

class FileStream: public SimpleStream {
  std::ifstream stream;
  char *buffer;
  int size;
public:
  FileStream(int min_buffersize, std::string source);
  std::string read(int bytes);
  
  ~FileStream();
};

class DataStream: public SimpleStream {
  int adaptive_buffersize;
  SimpleStream *srcstream;
public:
  DataStream(int min_buffersize, std::string source);
  ~DataStream();
  
  std::string read(int bytes);
};

#endif

