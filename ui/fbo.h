#ifndef FBO_H
#define FBO_H

#include <GL/gl.h>
#include <GL/glext.h>

#include "shader.h"

class FramebufferObject {
private:
  GLuint target, fb;
  std::vector<Renderable*> children;
public:
  FramebufferObject(int w, int h);
  void enable();
  void bind_texture();
  void disable();
  GLuint get_texture_id();
  
  ~FramebufferObject();
};

class FramebufferPass {
private:
  std::vector<Renderable*> objects;
  FramebufferObject fbo;
  ShaderObject *shader_obj;
  int width, height;
  bool flag_rerender, auto_rerender;
public:
  FramebufferPass(int w, int h, bool auto_rerender = true);
  
  void use_shader(ShaderObject *sh);
  
  void render_on();
  
  void push(Renderable *obj);
  void render(bool once = false);
  void bind_texture();
  GLuint get_texture_id();
};

#endif

