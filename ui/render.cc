#include "render.h"
#include <algorithm>
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>

RenderContext *Renderable::context = NULL;

Renderable::Renderable()
:z_index(0), 
 h(false), 
 blendcol(1.0,1.0,1.0,1.0),
 opacity_factor(1.0)
{}

Renderable::~Renderable() {}

std::string Renderable::to_string() {
  return "Renderable";
}

bool Renderable::hidden() {
  return h;
}

float Renderable::get_z_index() const {
  return z_index;
}

void Renderable::set_z_index(float z) {
  z_index = z;
}

void Renderable::set_blendcol(RGBA c) {
  blendcol = c;
}

RGBA Renderable::get_blendcol() {
  return blendcol;
}

void Renderable::goto_z_layer() {
  glLoadIdentity ();
  float z = (-z_index + 10.0) / 10.0;
}

void Renderable::hide() {
  h = true;
}

void Renderable::show() {
  h = false;
}

void Renderable::toggle_visibility() {
  if(hidden())
    show();
  else
    hide();
}

void Renderable::registered_callback() {}

void Renderable::set_opacity_factor(float f) {
  opacity_factor = f;
}

void RenderableGroup::translate(Vec2 x) {
  translation += x;
}

void RenderableGroup::set_origin(Vec2 x) {
  translation = x;
}

void RenderableGroup::rotate(float r) {
  rotation += r;
}

void RenderableGroup::set_angle(float r) {
  rotation = r;
}

float RenderableGroup::get_angle() {
  return rotation;
}

Vec2 RenderableGroup::get_origin() {
  return translation;
}

void RenderableGroup::attach(Renderable *r) {
  if(r == NULL)
    return;
  for(int i = 0; i < children.size(); ++i)
    if(children[i] == NULL) {
      children[i] = r;
      return;
    }
  children.push_back(r);
}

void RenderableGroup::detach(Renderable *r) {
  for(int i = 0; i < children.size(); ++i)
    if(children[i] == r)
      children[i] = NULL;
}

RenderableGroup::RenderableGroup()
:translation(0,0), rotation(0) {}

RenderableGroup::~RenderableGroup() {
  for(auto x: children) {
    if(x != NULL)
      delete x;
  }
}

void RenderableGroup::render() {
  glTranslatef(translation.x, translation.y, 0);
  glRotatef(rotation, 0, 0, 1);
  for(auto x: children) {
    if(x == NULL)
      continue;
    if(!x->hidden()) {
      glPushMatrix();
      x->render();
      glPopMatrix();
    }
  }
}

std::vector<Renderable*>& RenderableGroup::get_children() {
  return children;
}

void RenderableGroup::set_opacity_factor(float f) {
  opacity_factor = f;
  for(auto x: children) {
    if(x == NULL)
      continue;
    x->set_opacity_factor(f);
  }
}

void RenderableGroup::registered_callback() {
  for(auto x: children) {
    if(x == NULL)
      continue;
    x->registered_callback();
  }
}

RenderContext::RenderContext() {
  Renderable::context = this;
  Animator::context = this;
  update = true;
  term = false;
}

void RenderContext::clear() {

}

void RenderContext::refresh() {
  update = true;
}

void RenderContext::register_animator(Animator *anim) {
  if(anim == NULL)
    return;
  
  if(anim->registered == false) {
    anim->last_invoke_time = -1;
    anim->frame = 0;
    anim->disabled = false;
    //anim->id = 
    animators.insert(animators.end(), anim);
    anim->registered = true;
  }
}

void RenderContext::unregister_animator(Animator* anim) {
  for(std::deque<Animator*>::iterator it = animators.begin(); it != animators.end(); ++it) {
    if((*it) == anim) {
      animators.erase(it);
      break;
    }
  }
}

std::deque<Animator*> RenderContext::get_animators() {
  return animators;
}

void RenderContext::render() {
  if(update == false)
    return;

  for(auto x: framebuffers) 
    x->render();

  glViewport(0, 0, 1024, 600);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  GLfloat theta = 40.0,
          aspratio = 1024.0 / 600.0,
          near = 1.0,
          far = 30.0,
          deltay = 10.0 * tan(theta * M_PI / 360.0),
          deltax = deltay * aspratio;

  gluPerspective(theta, aspratio, near, far);
  glTranslatef(-deltax, deltay, -10);
  glScalef (deltax / 512.0, -deltay / 300.0, 1.0);
  glMatrixMode(GL_MODELVIEW);

  for(auto x: objects) {
    if(!x->hidden()) {
      glLoadIdentity();
      x->render();
    }
  }
  
  update = false;
  SDL_GL_SwapBuffers();
}

void RenderContext::push(FramebufferPass *fbo) {
  if(fbo == NULL)
    return;

  framebuffers.push_back(fbo);
}

void RenderContext::push(Renderable *obj) {
  if(obj == NULL)
    return;

  objects.push_back(obj);
  obj->registered_callback();
}

const Font& RenderContext::load_font(std::string fn, unsigned int height) {
  std::string key = fn + std::to_string(height);
  if(fonts.find(key) != fonts.end())
    return fonts.at(key);
  
  fonts.insert(std::pair<std::string, Font>(key, Font(fn, height)));
  
  return fonts.at(key);
}

void RenderContext::register_receiver(EventReceiver *rcv) {
  mainhandler.register_receiver(rcv);
}

void RenderContext::cast_event(const Event &ev) {
  mainhandler.evoke(ev);
}

void RenderContext::mark_for_termination() {
  term = true;
}

bool RenderContext::terminate() {
  return term;
}

RenderContext::~RenderContext() {
  for(auto x: fonts)
    x.second.clear();
  for(auto x: objects) {
    std::cout << "::RenderContext::deleting object " << x << std::endl;
    delete x;
  }
}

