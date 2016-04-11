#ifndef UTILS_H
#define UTILS_H
#include <GL/gl.h>
#include <cmath>
#include <algorithm>
#include <string>
#include <iostream>
#include <cstring>

enum Directon {
  Up,
  Down,
  Left,
  Right
};

struct RGBA {
  float red, green, blue, alpha;

  RGBA(float r, float g, float b, float a): red(r),green(g),blue(b),alpha(a) {}
  RGBA(const char *str, float a) {
    if(strlen(str) == 7 && str[0] == '#') {
       int col = std::strtol(str + 1, nullptr, 16);
       red =   (float)((col & 0xFF0000) >> 16) / 255.0;
       green = (float)((col & 0x00FF00) >>  8) / 255.0;
       blue =  (float)((col & 0x0000FF)      ) / 255.0;
       alpha = a;
    }else
      RGBA(0,0,0,a);
  }
  
  GLuint to_int() const { return (std::min(int(255*alpha), 255) << 24) | 
                             (std::min(int(255*blue), 255) << 16) |
                             (std::min(int(255*green), 255) << 8) |
                             (std::min(int(255*red), 255) << 0); }
  
  RGBA mix(const RGBA &b, float lambda) const {
    return RGBA(red * lambda + b.red * (1 - lambda),
                green * lambda + b.green * (1 - lambda),
                blue * lambda + b.blue * (1 - lambda),
                alpha * lambda + b.alpha * (1 - lambda));
  }
};


struct Vec2 {
  float x, y;

  Vec2(float _x, float _y): x(_x), y(_y) {}
  Vec2(): x(0),y(0) {}

  Vec2 operator + (const Vec2 &b) {
    return Vec2(x+b.x, y+b.y);
  }
  
  Vec2 operator - (const Vec2 &b) {
    return Vec2(x-b.x, y-b.y);
  }
  
  Vec2 operator += (const Vec2 &b) {
    x += b.x;
    y += b.y;
    return *this;
  }
  
  Vec2 operator * (float b) {
    return Vec2(x*b, y*b);
  }
  
  Vec2 operator / (float b) {
    return Vec2(x/b, y/b);
  }
  
  float length() {
    return sqrt(x*x+y*y);
  }
};

struct Rect {
  float left, right, top, bottom;

  Rect(float l, float t, float r, float b): left(l),right(r),top(t),bottom(b) {}
  bool contains(const Vec2 &p) { return (left <= p.x && p.x <= right) && (top <= p.y && p.y <= bottom); }
  Vec2 center() { return Vec2( (left+right)*0.5f, (top+bottom)*0.5f ); }
};

struct TexCoords {
  Vec2 lt, rt, rb, lb; // right-top ... left bottom
  
  TexCoords(Vec2 a, Vec2 b, Vec2 c, Vec2 d) {
    lt = a;
    rt = b;
    rb = c;
    lb = d;
  }
  
  TexCoords()
  : lt(0,1),
    rt(1,1),
    rb(1,0),
    lb(0,0) {}
};

std::string urlencode(const std::string &s);

/*
class SystemCommands
std::string exec(const char* cmd) {
  FILE* pipe = popen(cmd, "r");
  if (!pipe) return "ERROR";
  char buffer[128];
  std::string result = "";
  while (!feof(pipe)) {
    if (fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  pclose(pipe);
  return result;
  
  
  find "data/wallpapers" -iname "*.jpg" -type f | xargs -I{} identify -format '%w %h %i\n' {} | awk '$1==1024 && $2==600{print $3}'
}*/

#endif

