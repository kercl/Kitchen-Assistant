#include "img.h"
#include <iostream>
//#include <GL/glut.h>

using namespace std;

ImgStorage Img::imgdump;

ImgStorage::~ImgStorage() {
  //cout << "[Message] deconstructing image storage." << endl;
  for(auto x: image_dump)
    FreeImage_Unload(x.second);
  image_dump.clear();
}

FIBITMAP* ImgStorage::get(std::string id) {
  if(image_dump.find(id) == image_dump.end())
    return NULL;
  return image_dump[id];
}

float Img::get_width() {
  return shape.right - shape.left;
}
  
float Img::get_height() {
  return shape.bottom - shape.top;
}

void Img::set_shape(const Rect &sh) {
  shape = sh;
}

const std::string& Img::get_source() const {
  return source;
}

Img::Img(std::string src, const Rect &sh, float z, bool persist)
: loaded(false), 
  shape(sh),
  source(src)
{
  FIBITMAP *data = imgdump.get(src);
  
  if(data == NULL) {
#ifdef DEBUG
    //cout << "[Message] Loading image from file." << endl;
#endif
    const char *cstrfn = src.c_str();
    
    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(cstrfn , 0);

    data = FreeImage_Load(format, cstrfn, PNG_DEFAULT);
    if(!data) {
      cerr << "[Error] Image " << src << " not loaded: failed." << endl;
      return;
    }
  }else {
#ifdef DEBUG
    //cout << "[Message] Image still in memory." << endl;
#endif
  }

  RGBQUAD rgbquad;
  FREE_IMAGE_TYPE type;
  BITMAPINFOHEADER *header;

  type = FreeImage_GetImageType(data);
  height = FreeImage_GetHeight(data);
  width = FreeImage_GetWidth(data);

  if((FreeImage_GetBits(data) == 0) || (width == 0) || (height == 0))
    return;

  header = FreeImage_GetInfoHeader(data);

  glGenTextures(1, &texid);
  glBindTexture(GL_TEXTURE_2D, texid);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  if(header->biBitCount == 24)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, FreeImage_GetBits(data));
  else if(header->biBitCount == 32) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, FreeImage_GetBits(data));
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, FreeImage_GetBits(data));
  }

  if(persist) {
    imgdump.image_dump[src] = data;
  }else {
    FreeImage_Unload(data);
  }
  loaded = true;

  z_index = z;
  
#ifdef DEBUG
  //cout << "[Message] Loaded image '" << src << "'\n";
#endif
}

void Img::render() {
  if(loaded) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texid);
    glBegin(GL_QUADS);
      glColor4f(blendcol.red, blendcol.green, blendcol.blue, blendcol.alpha * opacity_factor);
      glTexCoord2f(0.0, 0.0); glVertex3f(shape.left, shape.bottom, z_index);
      glTexCoord2f(1.0, 0.0); glVertex3f(shape.right, shape.bottom, z_index);
      glTexCoord2f(1.0, 1.0); glVertex3f(shape.right, shape.top, z_index);
      glTexCoord2f(0.0, 1.0); glVertex3f(shape.left, shape.top, z_index);
    glEnd();
    glDisable(GL_TEXTURE_2D);
  }else {
      glColor4f(blendcol.red, blendcol.green, blendcol.blue, blendcol.alpha * opacity_factor);
      glVertex3f(shape.left, shape.top, z_index);
      glVertex3f(shape.right, shape.top, z_index);
      glVertex3f(shape.right, shape.bottom, z_index);
      glVertex3f(shape.left, shape.bottom, z_index);
    glEnd();
  }
}

Img::~Img() {
  glDeleteTextures(1, &texid);
}

WallpaperFader::WallpaperFader()
: Animator(35) {
  fade_state = 0.0;
}

void WallpaperFader::step() {
  fade_state += 0.05;
  if(fade_state > 1.0) {
    fade_state = 1.0;
    unregister();
  }
}

void WallpaperFader::reset() {
  fade_state = 0.0;
}

float WallpaperFader::get_fade_state() {
  return fade_state;
}

Wallpaper::Wallpaper(const std::vector<std::string> &p)
: Img(p[rand() % p.size()], Rect(0,0, 1024,600)), Animator(300000)
{
  z_index = -1000;
  next_image = NULL;
  pool = p;
  next_image = new Img(pool[rand() % pool.size()], Rect(0,0, 1024,600), 0, true);
  Renderable::context->register_animator(this);
}

void Wallpaper::step() {
  if(get_frame() == 1) // skip first call
    return;
  
  fader.reset();
  Renderable::context->register_animator(&fader);
}

void Wallpaper::render() {
  z_index = 0;
  
  glLoadIdentity();
  
  Img::render();
  
  if(pool.size() > 0) {
    if(fader.get_fade_state() > 0.01) {
      next_image->set_blendcol(RGBA(1.0, 1.0, 1.0, fader.get_fade_state()));
      next_image->render();
    }
    if(fader.get_fade_state() > 0.99) {
      GLuint tmp = next_image->texid;
      next_image->texid = texid;
      texid = tmp;
      
      delete next_image;
      next_image = new Img(pool[rand() % pool.size()], Rect(0,0, 1024,600), 0, true);
      
      fader.reset();
    }
  }
}

