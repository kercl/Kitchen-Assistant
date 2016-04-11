#ifndef FONT_H
#define FONT_H

#include <freetype2/ft2build.h>
#include <freetype2/freetype.h>
#include <freetype2/ftglyph.h>
#include <freetype2/ftoutln.h>
#include <freetype2/fttrigon.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <vector>
#include <string>
#include <sstream>

#define CHAR_COUNT_PRERENDER 200

class Font {
  float charwidths[CHAR_COUNT_PRERENDER];
public:
  float height;
  GLuint *textures, listbase;
  
  static FT_Library library;
  static bool initialized;

  Font(std::string path, unsigned int h);
  void enable() const;
  float get_height() const;
  void clear();
  
  float char_width(char c) const { return charwidths[c]; }
};

class Text: public Renderable {
  const Font &font;
  Vec2 p;
  std::stringstream stream;
  float character_spacing;
public:
  Text(const Font &ft, const Vec2 &p);
  void render();
  
  std::stringstream& get_stream();
  void set_position(const Vec2 &pos);
  
  float get_width();
};

#endif

