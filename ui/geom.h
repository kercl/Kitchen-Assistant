#ifndef GEOM_H
#define GEOM_H

#include "render.h"

class Circle: public Renderable {
  float radius;
  Vec2 center;
  int res;
  RGBA border_color;
  float border_width;
  
  float section_s, section_l;
public:
  Circle(Vec2 center, float radius, int res = 360);
  void render();
  
  void set_border_color(const RGBA &c);
  void set_border_width(float w);
  void set_center(Vec2 c);
  void set_center_y(float y);
  void set_center_x(float x);
  void set_radius(float r);
  
  void section_start(float alpha);
  void section_length(float alpha);
  
  Vec2 get_center() const;
  float get_radius() const;
};

#endif
