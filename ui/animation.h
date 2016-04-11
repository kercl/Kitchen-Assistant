#ifndef ANIMATION_H
#define ANIMATION_H

#include "render.h"
#include <deque>

class RenderContext;

class Animator {
  static RenderContext *context;
  //std::deque<Animator*>::iterator id;
  
  friend class RenderContext;
  int delay, last_invoke_time;
  bool registered, disabled;
  int frame;
protected:
  int elapsed;
public:
  Animator(int d);
  ~Animator();
  virtual void step() = 0;
  
  void invoke();
  
  void unregister();
  int get_delay();
  void set_delay(int d);
  
  int get_last_invoke();
  int get_frame();
  
  void pause();
  void play();
};

#endif

