#include "geom.h"
#include <iostream>

using namespace std;

Circle::Circle(Vec2 _center, float _radius, int _res)
: radius(_radius), 
  center(_center), 
  res(_res), 
  border_color(0,0,0,0), 
  border_width(0.0f),
  section_s(0),
  section_l(2*M_PI),
  Renderable() {}

void Circle::section_start(float alpha) {
  section_s = alpha;
}

void Circle::section_length(float alpha) {
  section_l = alpha;
}

void Circle::render() {
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glTranslatef(center.x, center.y, 0);
  
  int loop_end = (section_l / (2.0f*M_PI)) * res;
  
  if(blendcol.alpha * opacity_factor > 0.0001) {
    //cout << "sld: " << blendcol.red << ", "<< blendcol.green << blendcol.alpha << endl;
    glColor4f(blendcol.red, blendcol.green, blendcol.blue, blendcol.alpha * opacity_factor);
    //glColor4f(0,0,0,opacity_factor);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0,0,0);
    for(int i = 0; i <= loop_end; ++i) {
      float alpha = 2 * M_PI * ((float)i) / ((float)res) + section_s;
      
      glVertex2f(radius * cos(alpha),
                 radius * sin(alpha));
    }
    glEnd();
  }  
  if(border_color.alpha * opacity_factor > 0.0001 && border_width > 0.0001) {
    glColor4f(border_color.red, border_color.green, border_color.blue, border_color.alpha * opacity_factor);
    /*glLineWidth(border_width);
    glBegin(GL_LINE_STRIP);
    for(int i = 0; i <= res; ++i)
      glVertex2f(radius * cos(2 * M_PI * ((float)i) / ((float)res)),
                 radius * sin(2 * M_PI * ((float)i) / ((float)res)));
    glEnd();*/
    glBegin(GL_TRIANGLE_STRIP);
    for(int i = 0; i <= loop_end; ++i) {
      float alpha = 2 * M_PI * ((float)i) / ((float)res) + section_s;
    
      glVertex2f(radius * cos(alpha),
                 radius * sin(alpha));
      glVertex2f((radius + border_width) * cos(alpha),
                 (radius + border_width) * sin(alpha));
    }
    glEnd();
  }
}

void Circle::set_border_color(const RGBA &c) {
  border_color = c;
}

void Circle::set_border_width(float w) {
  border_width = w;
}

void Circle::set_center(Vec2 c) {
  center = c;
}

void Circle::set_radius(float r) {
  radius = r;
}

Vec2 Circle::get_center() const { return center; }
float Circle::get_radius() const { return radius; }

void Circle::set_center_y(float y) { center.y = y; }
void Circle::set_center_x(float x) { center.x = x; }

