#include "render.h"
#include <iostream>
#include <GL/glu.h>

using namespace std;

FramebufferObject::FramebufferObject(int w, int h) {
  glGenTextures(1, &target);
  glBindTexture(GL_TEXTURE_2D, target);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glGenFramebuffers(1, &fb);
  glBindFramebuffer(GL_FRAMEBUFFER, fb);
  glFramebufferTexture2D(GL_FRAMEBUFFER,        // 1. fbo target: GL_FRAMEBUFFER 
                         GL_COLOR_ATTACHMENT0,  // 2. attachment point
                         GL_TEXTURE_2D,         // 3. tex target: GL_TEXTURE_2D
                         target,                // 4. tex ID
                         0);                    // 5. mipmap level: 0(base)
  
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status != GL_FRAMEBUFFER_COMPLETE)
    cout << "::FramebufferObject::error: Framebuffer init failed" << endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FramebufferObject::enable() {
  glBindFramebuffer(GL_FRAMEBUFFER, fb);
}

void FramebufferObject::bind_texture() {
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, target);
}

GLuint FramebufferObject::get_texture_id() {
  return target;
}

void FramebufferObject::disable() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FramebufferObject::~FramebufferObject() {
  glDeleteTextures(1, &target);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &fb);
}

FramebufferPass::FramebufferPass(int w, int h, bool _auto_rerender)
: fbo(w, h), 
  shader_obj(NULL),
  width(w), height(h),
  flag_rerender(true),
  auto_rerender(_auto_rerender)
{}

void FramebufferPass::use_shader(ShaderObject *sh) {
  shader_obj = sh;
}
 
void FramebufferPass::push(Renderable *obj) {
  if(obj == NULL)
    return;

  objects.push_back(obj);
/*
  std::sort(objects.begin(), objects.end(),
  	[](const Renderable *a, const Renderable *b) {
          return a->get_z_index() < b->get_z_index();
  	});*/
}

void FramebufferPass::render(bool once) {
  if(flag_rerender == false)
    return;

  //cout << "Rendering FBO " << fbo.get_texture_id() << endl;

  fbo.enable();
  
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glClearColor(0,0,0,0);

  GLfloat theta = 40.0,
          aspratio = float(width) / float(height),
          near = 1.0,
          far = 30.0,
          deltay = 10.0 * tan(theta * M_PI / 360.0),
          deltax = deltay * aspratio;

  gluPerspective(theta, aspratio, near, far);
  glTranslatef(-deltax, deltay, -10);
  glScalef (2 * deltax / width, -2 * deltay / height, 1.0);
  glMatrixMode(GL_MODELVIEW);
  
  glClear(GL_COLOR_BUFFER_BIT);

  if(shader_obj)
    shader_obj->enable();
  for(auto x: objects) {
    if(!x->hidden()) {
      glLoadIdentity();
      x->render();
    }
  }
  if(shader_obj)
    shader_obj->disable();
  fbo.disable();
  glViewport(0, 0, 1024, 600);
  
  if(once || auto_rerender == false)
    flag_rerender = false;
}

void FramebufferPass::bind_texture() {
  fbo.bind_texture();
}

void FramebufferPass::render_on() {
  flag_rerender = true;
}

GLuint FramebufferPass::get_texture_id() {
  return fbo.get_texture_id();
}

