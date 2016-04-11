#ifndef RENDER_H
#define RENDER_H

#include <deque>
#include <vector>
#include <map>

#include "utils.h"
#include "animation.h"
#include "widgets/event.h"

class RenderContext;
class Animator;
class FramebufferPass;

class Renderable {
protected:
  int unique_id;
  bool h;
  float z_index;
  float opacity_factor;
  RGBA blendcol;

  void goto_z_layer();
  
  friend class RenderContext;
public:
  Renderable();
  virtual ~Renderable();
  
  virtual void render() = 0;
  bool hidden();
  void hide();
  void show();
  void toggle_visibility();
  float get_z_index() const;
  void set_z_index(float z);
  
  virtual void set_opacity_factor(float f);
  float get_opacity_factor() { return opacity_factor; }
  
  void set_blendcol(RGBA c);
  RGBA get_blendcol();
  
  int get_unique_id();
  
  virtual std::string to_string();
  virtual void registered_callback();
  
  static RenderContext *context;
};

#include "fbo.h"
#include "text.h"
#include "widgets/event.h"

class RenderableGroup: public Renderable {
  float rotation;
  Vec2 translation;
protected:
  std::vector<Renderable*> children;
public:
  RenderableGroup();
  virtual ~RenderableGroup();

  void translate(Vec2 x);
  void set_origin(Vec2 x);
  void rotate(float r);
  void set_angle(float r);
  float get_angle();
  Vec2 get_origin();
  
  virtual void set_opacity_factor(float f);
  
  virtual void attach(Renderable *r);
  virtual void detach(Renderable *r);
  
  virtual void render();
  virtual void registered_callback();
  
  std::vector<Renderable*>& get_children();
};

class RenderContext {
private:
  std::deque<FramebufferPass*> framebuffers;
  std::deque<Renderable*> objects;
  std::deque<Animator*> animators;
  
  std::map<std::string, Font> fonts;
  
  EventHandler mainhandler;
  
  int passes;
  bool update, term;
public:
  RenderContext();
  ~RenderContext();

  void clear();
  void pre_render();
  void render();
  
  void push(Renderable *obj);
  void push(FramebufferPass *fbo);
  
  const Font& load_font(std::string fn, unsigned int height);
  
  void register_animator(Animator *anim);
  void unregister_animator(std::deque<Animator*>::iterator it);
  void unregister_animator(Animator* anim);
  std::deque<Animator*> get_animators();
  
  void cast_event(const Event &ev);
  void register_receiver(EventReceiver *rcv);
  
  void refresh();
  
  void mark_for_termination();
  bool terminate();
};

#endif

