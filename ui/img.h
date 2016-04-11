#ifndef IMG_H
#define IMG_H

#include "render.h"
#include "utils.h"

#include <cstring>
#include <string>
#include <map>
#include <GL/gl.h>
#include <FreeImage.h>

struct ImgStorage {
  std::map<std::string, FIBITMAP*> image_dump;
  ~ImgStorage();
  FIBITMAP* get(std::string id);
};

class Wallpaper;
class ImageTexture;

class Img: public Renderable {
protected:
  static ImgStorage imgdump;
  GLuint texid;

  bool loaded;
  int width, height;

  Rect shape;
  std::string source;
  
  friend class Wallpaper;
  friend class ImageTexture;
public:
  Img(std::string src, const Rect &sh, float z=0.0, bool persist=true);
  ~Img();
  virtual void render();
  
  float get_width();
  float get_height();
  
  void set_shape(const Rect &sh);
  
  const std::string& get_source() const;
};

class WallpaperFader: public Animator {
  float fade_state;
public:
  WallpaperFader();
  void step();
  void reset();
  float get_fade_state();
};

class Wallpaper: public Img, public Animator {
  WallpaperFader fader;
  std::vector<std::string> pool;
  
  Img *next_image;
public:
  Wallpaper(const std::vector<std::string> &p);
  
  void step();
  void render();
};

#endif

