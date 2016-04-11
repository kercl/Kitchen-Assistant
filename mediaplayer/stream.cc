#include <boost/network/protocol/http/client.hpp>
#include <iostream>
#include <cstring>

#include "stream.h"

using namespace boost::network;
using namespace boost::network::http;

DataStream::DataStream(int min_buffersize, std::string source): srcstream(NULL) {
  if(!strncmp(source.c_str(), "http://", 7)) {
  
  }else if(!strncmp(source.c_str(), "https://", 7)) {
  
  }else if(!strncmp(source.c_str(), "file://", 7)) {
    srcstream = new FileStream(min_buffersize, source.substr(7));
  }
}

std::string DataStream::read(int bytes) {
  if(srcstream)
    return srcstream->read(bytes);
  return std::string();
}

DataStream::~DataStream() {
  if(srcstream)
    delete srcstream;
}

HTTPStream::HTTPStream(std::string source): req(source.c_str()) {
  req << header("Connection", "keep-alive");
}

std::string HTTPStream::read(int bytes) {
  http_stream_client::response resp = client.get(req);
  //while(!ready(resp));
  return body(resp); 
}

FileStream::FileStream(int min_buffersize, std::string source) {
  stream.open(source.c_str(), std::ifstream::in | std::ifstream::binary);
  buffer = new char[min_buffersize + 1];
  size = min_buffersize;
}

FileStream::~FileStream() {
  delete [] buffer;
}

std::string FileStream::read(int bytes) {
  if(stream.is_open()) {
    if(stream.good()) {
      stream.read(buffer, size);
      buffer[stream.gcount()] = 0;
      return std::string(buffer);
    }else
      stream.close();
  }  
}

