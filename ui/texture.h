#ifndef TEXTURE_H
#define TEXTURE_H

#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <GL/gl.h>

#include "utils.h"

class Texture {
protected:
  GLuint texid;
  GLint slot;
  static std::map<std::string,GLuint> texture_mem;
public:
  Texture(GLint slot = -1);
  Texture(GLuint preexist, GLint slot = -1);
  virtual void bind(GLint slot = -1) const;
  GLuint get_texture_id() const;
  ~Texture();
};

class MemoryTexture: public Texture {
  int width, height, bpp;
public:
  MemoryTexture(int w, int h, int bpp);
  void update_from_buffer(GLuint *buf, int w, int h);
  ~MemoryTexture();
};

class ImageTexture: public Texture {
public:
  ImageTexture(std::string fn, bool persist=true);
};

class EmptyTexture: public Texture {
  RGBA color;
public:
  EmptyTexture(RGBA col);
  void bind() const;
};

class LinearGradientTexture: public Texture {
public:
  LinearGradientTexture(const std::vector<std::pair<int, RGBA>> &pts);
};

class RadialGradientTexture: public Texture {
public:
  RadialGradientTexture(const std::vector<std::pair<int, RGBA>> &pts);
  void fill_circle(GLuint *grad, int maxrad, int rad, GLuint col);
};

#endif

