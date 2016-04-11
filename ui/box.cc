#include "img.h"
#include "box.h"
#include <iostream>
#include <cmath>
#include <algorithm>

#define SHADOW_STEPS 20
#define KERNEL_SIZE 3

using namespace std;

void BoxShadow::apply_kernel(GLuint *dest, GLuint *src, int w, int h) {
  static float kernel[KERNEL_SIZE][KERNEL_SIZE] = {
    {0.077847, 0.123317, 0.077847},
    {0.123317, 0.195346, 0.123317},
    {0.077847, 0.123317, 0.077847}};
  
  float alpha_map[KERNEL_SIZE][KERNEL_SIZE];

  for(int x = 0; x < w; ++x){
    for(int y = 0; y < h; ++y) {
      alpha_map[0][0] = (src[max(0,x-1) +   max(0,y-1) * w] >> 24) & 0xFF;
      alpha_map[0][1] = (src[x +            max(0,y-1) * w] >> 24) & 0xFF;
      alpha_map[0][2] = (src[min(w-1,x+1) + max(0,y-1) * w] >> 24) & 0xFF;
      
      alpha_map[1][0] = (src[max(0,x-1) +   y * w] >> 24) & 0xFF;
      alpha_map[1][1] = (src[x +            y * w] >> 24) & 0xFF;
      alpha_map[1][2] = (src[min(w-1,x+1) + y * w] >> 24) & 0xFF;
      
      alpha_map[2][0] = (src[max(0,x-1) +   min(h-1,y+1) * w] >> 24) & 0xFF;
      alpha_map[2][1] = (src[x +            min(h-1,y+1) * w] >> 24) & 0xFF;
      alpha_map[2][2] = (src[min(w-1,x+1) + min(h-1,y+1) * w] >> 24) & 0xFF;
      
      GLuint sum = (GLuint)((alpha_map[0][0] * kernel[0][0]) + 
                   (alpha_map[0][1] * kernel[0][1]) +
                   (alpha_map[0][2] * kernel[0][2]) +
                   (alpha_map[1][0] * kernel[1][0]) +
                   (alpha_map[1][1] * kernel[1][1]) +
                   (alpha_map[1][2] * kernel[1][2]) +
                   (alpha_map[2][0] * kernel[2][0]) +
                   (alpha_map[2][1] * kernel[2][1]) +
                   (alpha_map[2][2] * kernel[2][2]));
                   
      dest[x + y * w] = (min(sum, (GLuint)0xFF) << 24) | (src[x + y * w] & 0x00FFFFFF);
    }
  }
}

BoxShadow::BoxShadow(const Rect &sh, int blur, const RGBA &color, float rad):
  shadow_blur(blur),
  shadow_color(color),
  shape(sh)
{
  int d = 2*(blur + 4);

  corner = new GLuint[d * d];
  edge = new GLuint[2 * d];
  
  GLuint *dummy_corner = new GLuint[d * d],
         *dummy_edge = new GLuint[2 * d];
  
  for(int y = 0; y < d; ++y) {
    for(int x = 0; x < d; ++x) {
      corner[x + d * y] = x <= blur + 1 && y <= blur + 1 ? color.to_int() : color.to_int() & 0x00FFFFFF;
    }
    edge[    2 * y] = y <= blur + 1 ? color.to_int() : color.to_int() & 0x00FFFFFF;
    edge[1 + 2 * y] = edge[2 * y];
  }

  for(int i = 0; i < blur; ++i) {
    std::swap(corner, dummy_corner);
    std::swap(edge, dummy_edge);
    
    apply_kernel(corner, dummy_corner, d, d);
    apply_kernel(edge, dummy_edge, 2, d);
  }
  
  delete [] dummy_corner;
  delete [] dummy_edge;

  glGenTextures(1, &edge_texid);
  glBindTexture(GL_TEXTURE_2D, edge_texid); 

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, d, 0, GL_RGBA, GL_UNSIGNED_BYTE, edge);
  
  glGenTextures(1, &corner_texid);
  glBindTexture(GL_TEXTURE_2D, corner_texid);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, d, d, 0, GL_RGBA, GL_UNSIGNED_BYTE, corner);
  
  shape.left += shadow_blur * 0.9;
  shape.right -= shadow_blur* 0.9;
  shape.top += shadow_blur * 0.9;
  shape.bottom -= shadow_blur* 0.9;
  
  if(shape.right < shape.left)
    shape.right = shape.left;
  if(shape.bottom < shape.top)
    shape.bottom = shape.top;
}

void BoxShadow::render() {
  int d = 2*(shadow_blur + 4);
  
  glColor4f(1.0, 1.0, 1.0, 1.0 * opacity_factor);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, corner_texid);
  glTranslatef(shape.left, shape.top, 0);
  glBegin(GL_QUADS);
      glTexCoord2f(1.0, 1.0); glVertex3f(-d, -d, 0);
      glTexCoord2f(1.0, 0.0); glVertex3f(0, -d, 0);
      glTexCoord2f(0.0, 0.0); glVertex3f(0, 0, 0);
      glTexCoord2f(0.0, 1.0); glVertex3f(-d, 0, 0);
  glEnd();
  glBindTexture(GL_TEXTURE_2D, edge_texid);
  glBegin(GL_QUADS);
      glTexCoord2f(1.0, 1.0); glVertex3f(0, -d, 0);
      glTexCoord2f(0.0, 1.0); glVertex3f(shape.right - shape.left, -d, 0);
      glTexCoord2f(0.0, 0.0); glVertex3f(shape.right - shape.left, 0, 0);
      glTexCoord2f(1.0, 0.0); glVertex3f(0, 0, 0);
      glTexCoord2f(0.0, 0.0); glVertex3f(0, 0, 0);
      glTexCoord2f(1.0, 0.0); glVertex3f(0, shape.bottom - shape.top, 0);
      glTexCoord2f(1.0, 1.0); glVertex3f(-d, shape.bottom - shape.top, 0);
      glTexCoord2f(0.0, 1.0); glVertex3f(-d, 0, 0);
  glEnd();
  glTranslatef(shape.right - shape.left, 0, 0);
  glBegin(GL_QUADS);
      glTexCoord2f(0.0, 0.0); glVertex3f(0, 0, 0);
      glTexCoord2f(1.0, 0.0); glVertex3f(0, shape.bottom - shape.top, 0);
      glTexCoord2f(1.0, 1.0); glVertex3f(d, shape.bottom - shape.top, 0);
      glTexCoord2f(0.0, 1.0); glVertex3f(d, 0, 0);
  glEnd();
  glBindTexture(GL_TEXTURE_2D, corner_texid);
  glBegin(GL_QUADS);
      glTexCoord2f(1.0, 1.0); glVertex3f(d, -d, 0);
      glTexCoord2f(1.0, 0.0); glVertex3f(0, -d, 0);
      glTexCoord2f(0.0, 0.0); glVertex3f(0, 0, 0);
      glTexCoord2f(0.0, 1.0); glVertex3f(d, 0, 0);
  glEnd();
  glTranslatef(0, shape.bottom - shape.top, 0);
  glBegin(GL_QUADS);
      glTexCoord2f(1.0, 1.0); glVertex3f(d, d, 0);
      glTexCoord2f(1.0, 0.0); glVertex3f(0, d, 0);
      glTexCoord2f(0.0, 0.0); glVertex3f(0, 0, 0);
      glTexCoord2f(0.0, 1.0); glVertex3f(d, 0, 0);
  glEnd();
  glBindTexture(GL_TEXTURE_2D, edge_texid);
  glBegin(GL_QUADS);
      glTexCoord2f(1.0, 1.0); glVertex3f(0, d, 0);
      glTexCoord2f(0.0, 1.0); glVertex3f(shape.left - shape.right, d, 0);
      glTexCoord2f(0.0, 0.0); glVertex3f(shape.left - shape.right, 0, 0);
      glTexCoord2f(1.0, 0.0); glVertex3f(0, 0, 0);
  glEnd();
  glTranslatef(shape.left - shape.right, 0, 0);
  glBindTexture(GL_TEXTURE_2D, corner_texid);
  glBegin(GL_QUADS);
      glTexCoord2f(1.0, 1.0); glVertex3f(-d, d, 0);
      glTexCoord2f(1.0, 0.0); glVertex3f(0, d, 0);
      glTexCoord2f(0.0, 0.0); glVertex3f(0, 0, 0);
      glTexCoord2f(0.0, 1.0); glVertex3f(-d, 0, 0);
  glEnd();
  glDisable(GL_TEXTURE_2D);
  glColor4f(shadow_color.red, shadow_color.green, shadow_color.blue, shadow_color.alpha * opacity_factor);
  glBegin(GL_QUADS);
      glTexCoord2f(1.0, 1.0); glVertex3f(0, 0, 0);
      glTexCoord2f(1.0, 0.0); glVertex3f(shape.right - shape.left, 0, 0);
      glTexCoord2f(0.0, 0.0); glVertex3f(shape.right - shape.left, shape.top - shape.bottom, 0);
      glTexCoord2f(0.0, 1.0); glVertex3f(0, shape.top - shape.bottom, 0);
  glEnd();
}

BoxShadow::~BoxShadow() {
  delete [] edge;
  delete [] corner;
}

Box::Box(const Rect &sh, float z):
  shape(sh),
  background_color(0.0,0.0,0.0,0.0),
  border_color(0.0,0.0,0.0,0.0),
  vertices(NULL),
  border_radius(0),
  border_width(0.0)
{
  z_index = z;

  set_shape(sh);
}

void Box::drop_shadow(const Vec2 &offset, float blur, const RGBA &color) {
  shadows.push_back(new BoxShadow(
    Rect(-(shape.right - shape.left) / 2 + offset.x,
         -(shape.bottom - shape.top) / 2 + offset.y,
         (shape.right - shape.left) / 2 + offset.x,
         (shape.bottom - shape.top) / 2 + offset.y), 
    blur, color));
  shadows[shadows.size() - 1]->set_opacity_factor(opacity_factor);
}

void Box::set_background_color(RGBA color) {
  background_color = color;
}

void Box::set_border_color(RGBA color) {
  border_color = color;
}

void Box::set_background_alpha(float alpha) {
  background_color.alpha = alpha;
}

void Box::set_border_style(Property prop) {
  border_style = prop;
}

void Box::set_opacity_factor(float f) {
  Renderable::set_opacity_factor(f);
  for(auto x: shadows)
    x->set_opacity_factor(f);
}

void Box::set_border_width(float w) {
  border_width = w;
}

Rect Box::get_shape() {
  return shape;
}

void Box::set_border_radius(int r, int sbits) {
  border_radius = r;

  if(vertices) {
    delete [] vertices;
    vertices = NULL;
  }
  if(r <= 0) {
    border_radius = 0;
    return;
  }
  
  int tr = int((sbits & TOP_RIGHT) > 0),
      tl = int((sbits & TOP_LEFT) > 0),
      bl = int((sbits & BOTTOM_LEFT) > 0),
      br = int((sbits & BOTTOM_RIGHT) > 0);
  
  vertex_count = r * (tr + tl + bl + br) + 4;
  
  vertices = new Vec2[vertex_count];
  
  for(int i = 0; i < r + 1; ++i) {
    float angle = M_PI * float(i) / float(2 * r),
          rc = r * cos(angle),
          rs = r * sin(angle);
    vertices[i*tr]                    = Vec2(w2,-h2) +  Vec2(-r+rc, r-rs)*int((sbits & TOP_RIGHT) > 0);
    vertices[i*tl + r*tr + 1]         = Vec2(-w2,-h2) + Vec2(r-rs, r-rc)*int((sbits & TOP_LEFT) > 0);
    vertices[i*bl + r*(tl+tr) + 2]    = Vec2(-w2,h2) +  Vec2(r-rc, -r+rs)*int((sbits & BOTTOM_LEFT) > 0);
    vertices[i*br + r*(bl+tl+tr) + 3] = Vec2(w2,h2) +   Vec2(-r+rs, -r+rc)*int((sbits & BOTTOM_RIGHT) > 0);
  }
}

void Box::append_texture(Texture *t, TexCoords tc) {
  textures.push_back({t,tc});
}

void Box::set_shape(const Rect &s) {
  shape = s;

  w2 = (shape.right - shape.left) / 2.0,
  h2 = (shape.bottom - shape.top) / 2.0;

  center.x = (shape.left + shape.right) / 2;
  center.y = (shape.top + shape.bottom) / 2;

  if(vertices)
    delete [] vertices;
  vertices = new Vec2[4];
  vertices[0] = Vec2(w2, -h2);
  vertices[1] = Vec2(-w2, -h2);
  vertices[2] = Vec2(-w2, h2);
  vertices[3] = Vec2(w2, h2);
}

void Box::render() {
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glTranslatef(center.x, center.y, 0);

  for(int i = 0; i < shadows.size(); ++i) {
    glPushMatrix();
    shadows[i]->render();
    glPopMatrix();
  }

  if(border_radius != 0) {
    if(opacity_factor > 0) {
      glBegin(GL_TRIANGLE_FAN);
        glColor4f(background_color.red, background_color.green, background_color.blue, background_color.alpha * opacity_factor);
        glVertex3f(0, 0, 0.0);
        for(int i = 0; i <= vertex_count; i++)
          glVertex3f(vertices[i % vertex_count].x, vertices[i % vertex_count].y, 0.0);
      glEnd();
    
      for(auto t: textures) {
        glColor4f(blendcol.red, blendcol.green, blendcol.blue, blendcol.alpha * opacity_factor);
        t.first->bind();
        
        glBegin(GL_TRIANGLE_FAN);
          Vec2 center_tex_coord = (t.second.lt + t.second.rt + t.second.rb + t.second.lb) * 0.25, tc;
          float ph, pv;
          
          glTexCoord2f(center_tex_coord.x, center_tex_coord.y);
          glVertex3f(0, 0, 0.0);
          
          for(int i = 0; i <= vertex_count; i++) {
            
            
            ph = (vertices[i % vertex_count].x + w2) / (2 * w2);
            pv = (vertices[i % vertex_count].y + h2) / (2 * h2);
            tc = (t.second.lt * (1 - ph) + t.second.rt * ph) * (1 - pv) +
                 (t.second.lb * (1 - ph) + t.second.rb * ph) * pv;
            glTexCoord2f(tc.x, tc.y);
            glVertex3f(vertices[i % vertex_count].x, vertices[i % vertex_count].y, 0.0);
          }
        glEnd();
      }
    }
    glDisable(GL_TEXTURE_2D);
    if(border_color.alpha * opacity_factor > 0.0) {
      glColor4f(border_color.red, border_color.green, border_color.blue, border_color.alpha * opacity_factor);
      glLineWidth(border_width); 
      glBegin(GL_LINE_STRIP);
        for(int i = 0; i <= vertex_count; i++) {
          glVertex3f(vertices[i % vertex_count].x, vertices[i % vertex_count].y, 0.0);
        }
      glEnd();
    }
  }else {
    if(background_color.alpha * opacity_factor > 0) {
      glColor4f(background_color.red, background_color.green, background_color.blue, background_color.alpha * opacity_factor);
      glBegin(GL_QUADS);
        glVertex3f(shape.left-center.x, shape.top-center.y, 0);
        glVertex3f(shape.right-center.x, shape.top-center.y, 0);
        glVertex3f(shape.right-center.x, shape.bottom-center.y, 0);
        glVertex3f(shape.left-center.x, shape.bottom-center.y, 0);
      glEnd();
    }
    
    int i = 0;
    while(i < textures.size()) {
      glColor4f(blendcol.red, blendcol.green, blendcol.blue, blendcol.alpha * opacity_factor);
      textures[i].first->bind();
      glBegin(GL_QUADS);
        glTexCoord2f(textures[i].second.lt.x, textures[i].second.lt.y);
        glVertex3f(shape.left-center.x, shape.top-center.y, 0);
        glTexCoord2f(textures[i].second.rt.x, textures[i].second.rt.y);
        glVertex3f(shape.right-center.x, shape.top-center.y, 0);
        glTexCoord2f(textures[i].second.rb.x, textures[i].second.rb.y);
        glVertex3f(shape.right-center.x, shape.bottom-center.y, 0);
        glTexCoord2f(textures[i].second.lb.x, textures[i].second.lb.y);
        glVertex3f(shape.left-center.x, shape.bottom-center.y, 0);
      glEnd();
      ++i;
    }
    glDisable(GL_TEXTURE_2D);
    if(border_color.alpha * opacity_factor > 0.0) {
      glColor4f(border_color.red, border_color.green, border_color.blue, border_color.alpha * opacity_factor);
      glLineWidth(border_width); 
      glBegin(GL_LINE_STRIP);
      glVertex3f(shape.left-center.x, shape.top-center.y, 0);
      glVertex3f(shape.right-center.x, shape.top-center.y, 0);
      glVertex3f(shape.right-center.x, shape.bottom-center.y, 0);
      glVertex3f(shape.left-center.x, shape.bottom-center.y, 0);
      glVertex3f(shape.left-center.x, shape.top-center.y, 0);
      glEnd();
    }
  }
}

Box::~Box() {
  for(int i = 0; i < shadows.size(); ++i)
    delete shadows[i];
}

