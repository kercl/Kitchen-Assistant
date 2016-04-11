#include "texture.h"
#include "img.h"

#include <iostream>
#include <cmath>
#include <sstream>

using namespace std;

std::map<std::string,GLuint> Texture::texture_mem;

Texture::Texture(GLint _slot): texid(0), slot(_slot) {}

Texture::Texture(GLuint preexist, GLint _slot): texid(preexist), slot(_slot) {}

void Texture::bind(GLint _slot) const {
  if(_slot > 0)
    glActiveTexture(GL_TEXTURE0 + _slot);
  else if(slot > 0)
    glActiveTexture(GL_TEXTURE0 + slot);
  else
    glActiveTexture(GL_TEXTURE0);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texid);
}

GLuint Texture::get_texture_id() const {
  return texid;
}

Texture::~Texture() {
  glDeleteTextures(1, &texid);
}

EmptyTexture::EmptyTexture(RGBA col): color(col) {}

void EmptyTexture::bind() const {
  glDisable(GL_TEXTURE_2D);
  glColor4f(color.red, color.green, color.blue, color.alpha);
}

ImageTexture::ImageTexture(std::string src, bool persist) {
  FIBITMAP *data = Img::imgdump.get(src);
  
  if(data == NULL) {
    cout << "::ImageTexture::Loading image from file." << endl;
    const char *cstrfn = src.c_str();
    
    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(cstrfn , 0);

    data = FreeImage_Load(format, cstrfn, PNG_DEFAULT);
    if(!data) {
      cerr << "::ImageTexture::error: Image " << src << " not loaded: failed." << endl;
      return;
    }
  }else {
    cout << "::ImageTexture::Image still in memory." << endl;
  }

  RGBQUAD rgbquad;
  FREE_IMAGE_TYPE type;
  BITMAPINFOHEADER *header;

  type = FreeImage_GetImageType(data);
  int height = FreeImage_GetHeight(data);
  int width = FreeImage_GetWidth(data);

  if((FreeImage_GetBits(data) == 0) || (width == 0) || (height == 0))
    return;

  header = FreeImage_GetInfoHeader(data);

  glGenTextures(1, &texid);
  glBindTexture(GL_TEXTURE_2D, texid);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  if(header->biBitCount == 24)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, FreeImage_GetBits(data));
  else if(header->biBitCount == 32) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, FreeImage_GetBits(data));
  }
  
  if(persist) {
    Img::imgdump.image_dump[src] = data;
  }else {
    FreeImage_Unload(data);
  }
}

LinearGradientTexture::LinearGradientTexture(const std::vector<std::pair<int, RGBA>> &pts) {
  stringstream ss;
  ss << "LinGrad:";
  for(auto p: pts) {
    ss << p.first << "," << hex << p.second.to_int() << dec << ";";
  }
  
  if(Texture::texture_mem.find(ss.str()) != Texture::texture_mem.end()) {
    texid = Texture::texture_mem[ss.str()];
    return;
  }
  
  int current_pos = 0,
      max_pos = pts.back().first;
  RGBA current_col(0,0,0,0);
  
  if(max_pos <= 0)
    return;
  
  GLuint *grad = new GLuint[max_pos + 1];
  for(int i = 0; i <= max_pos; ++i)
    grad[i] = 0;
  
  for(auto p: pts) {
    if(current_pos == p.first) {
      grad[current_pos] = p.second.to_int();
    }else {
      for(int x = current_pos; x <= p.first; ++x) {
        grad[x] = current_col.mix(p.second, 1 - ((float)(x - current_pos)) / ((float)(p.first - current_pos)) ).to_int();
      }
    }
    current_pos = p.first;
    current_col = p.second;
  }
  
  glEnable(GL_TEXTURE_2D);
  
  glGenTextures(1, &texid);
  glBindTexture(GL_TEXTURE_2D, texid);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, max_pos + 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, grad);
  
  Texture::texture_mem[ss.str()] = texid;
  
  delete [] grad;
}

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

void RadialGradientTexture::fill_circle(GLuint *grad, int maxrad, int rad, GLuint col) {
  rad = min(rad, maxrad);
  
  int oldy = 0;
  
  for(int x = -rad; x <= rad; ++x) {
    int y = min((int)maxrad, (int)sqrt(rad*rad - x*x));
    grad[(maxrad + y) * (2 * maxrad) + maxrad + x] = col;
    grad[(maxrad + y) * (2 * maxrad) + maxrad + x - 1 * sgn(x)] = col;
    grad[(maxrad + y) * (2 * maxrad) + maxrad + x - 2 * sgn(x)] = col;
    grad[(maxrad - y) * (2 * maxrad) + maxrad + x] = col;
    grad[(maxrad - y) * (2 * maxrad) + maxrad + x - 1 * sgn(x)] = col;
    grad[(maxrad - y) * (2 * maxrad) + maxrad + x - 2 * sgn(x)] = col;
    
    for(int i = 1; i < abs(oldy - y); ++i) {
      grad[(maxrad + min(oldy,y) + i) * (2 * maxrad) + maxrad + x * sgn(x)] = col;
      grad[(maxrad - min(oldy,y) - i) * (2 * maxrad) + maxrad + x * sgn(x)] = col;
      grad[(maxrad + min(oldy,y) + i) * (2 * maxrad) + maxrad + x - 1 * sgn(x)] = col;
      grad[(maxrad - min(oldy,y) - i) * (2 * maxrad) + maxrad + x - 1 * sgn(x)] = col;
      grad[(maxrad + min(oldy,y) + i) * (2 * maxrad) + maxrad + x - 2 * sgn(x)] = col;
      grad[(maxrad - min(oldy,y) - i) * (2 * maxrad) + maxrad + x - 2 * sgn(x)] = col;
    }
    
    oldy = y;
  }
}

RadialGradientTexture::RadialGradientTexture(const std::vector<std::pair<int, RGBA>> &pts){
  stringstream ss;
  ss << "RadGrad:";
  for(auto p: pts) {
    ss << p.first << "," << hex << p.second.to_int() << dec << ";";
  }
  
  if(Texture::texture_mem.find(ss.str()) != Texture::texture_mem.end()) {
    texid = Texture::texture_mem[ss.str()];
    return;
  }
  
  int current_pos = pts.back().first,
      max_pos = pts.back().first;
  RGBA current_col(0,0,0,0);
  
  if(max_pos <= 0)
    return;
  
  GLuint *grad = new GLuint[4 * (max_pos + 1) * (max_pos + 1)];
  for(int i = 0; i < 4 * (max_pos + 1) * (max_pos + 1); ++i)
    grad[i] = 0;
  
  for(int i = pts.size() - 1; i >= 0; --i) {
    std::pair<int, RGBA> p = pts[i];
    
    if(current_pos == p.first) {
      fill_circle(grad, max_pos + 1, current_pos, p.second.to_int());
    }else {
      for(int x = current_pos; x >= p.first; --x) {
        fill_circle(grad, max_pos + 1, x, current_col.mix(p.second, 1 - ((float)(x - current_pos)) / ((float)(p.first - current_pos)) ).to_int());
      }
    }
    current_pos = p.first;
    current_col = p.second;
  }
  
  glEnable(GL_TEXTURE_2D);
  
  glGenTextures(1, &texid);
  glBindTexture(GL_TEXTURE_2D, texid);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2 * (max_pos + 1), 2 * (max_pos + 1), 0, GL_RGBA, GL_UNSIGNED_BYTE, grad);
  
  Texture::texture_mem[ss.str()] = texid;
  
  delete [] grad;
}

MemoryTexture::MemoryTexture(int w, int h, int _bpp) 
: width(w),
  height(h),
  bpp(_bpp)
{
  glEnable(GL_TEXTURE_2D);
  
  glGenTextures(1, &texid);
  glBindTexture(GL_TEXTURE_2D, texid);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void MemoryTexture::update_from_buffer(GLuint *buf, int w, int h) {
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texid);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA,  GL_UNSIGNED_BYTE, buf);
  /*GLuint *testdata = new GLuint[100 * 100];
  for(int i = 0; i < 100*100; ++i)
    testdata[i] = rand();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 100, 100, 0, GL_RGB, GL_UNSIGNED_BYTE, testdata);
  delete [] testdata;*/
}

MemoryTexture::~MemoryTexture() {}

