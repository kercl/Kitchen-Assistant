#ifndef BOX_H
#define BOX_H

#include "render.h"
#include "utils.h"
#include "texture.h"

#include <cstring>
#include <string>
#include <vector>
#include <GL/gl.h>

#define TOP_LEFT 1
#define TOP_RIGHT 2
#define BOTTOM_RIGHT 4
#define BOTTOM_LEFT 8

#define PIXEL_UNIT 0.0121323
#define SHADOW_DELTA_ALPHA 0.01

enum Property {
  None,
  Solid
};

class BoxShadow: public Renderable {
private:
	GLuint *edge, *corner;
	int shadow_blur;
	Vec2 shadow_offset;
	RGBA shadow_color;
	Rect shape;
	
	GLuint edge_texid, corner_texid;
	
	void apply_kernel(GLuint *dest, GLuint *src, int w, int h);
public:
  BoxShadow(const Rect &sh, int blur, const RGBA &color, float rad=0); /* TODO: radius blur */
  void render();
  ~BoxShadow();
};

class Box: public Renderable {
protected:
  Rect shape;
  RGBA background_color;
  RGBA border_color;
  float border_width, border_radius;
  Property border_style;
  std::vector<BoxShadow*> shadows;
  std::vector<std::pair<Texture*,TexCoords>> textures;
  float w2, h2;
  Vec2 *vertices, center;
  int vertex_count;
public:
  Box(const Rect &sh, float z=0.0);

  void drop_shadow(const Vec2 &offset, float blur, const RGBA &color);
  void set_background_color(RGBA color);
  void set_background_alpha(float alpha);
  void set_border_color(RGBA color);
  void set_border_style(Property pop);
  void set_border_width(float w);
  void set_border_radius(int r, int selection_bitmask = 0xF);
  
  virtual void set_opacity_factor(float f);
  
  Rect get_shape();
  void set_shape(const Rect &s);
  
  void append_texture(Texture *t, TexCoords tc = TexCoords());

  virtual void render();

  ~Box();
};

#endif

