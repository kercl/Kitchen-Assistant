#include "animation.h"
//#include <GL/glut.h>
#include <SDL/SDL.h>

#include <iostream>

using namespace std;

RenderContext *Animator::context = NULL;

Animator::Animator(int d) {
  delay = d;
  last_invoke_time = -1;
  registered = false;
  disabled = true;
  frame = 0;
}

Animator::~Animator() {
  unregister();
}

void Animator::invoke() {
  if(disabled)
    return;

  if(SDL_GetTicks() - last_invoke_time < delay && last_invoke_time >= 0)
    return;
  
  //cout << "*" << flush;
  //cout << "passed " << hex << this << dec << endl;
  
  if(last_invoke_time >= 0)
    elapsed = SDL_GetTicks() - last_invoke_time;
  else
    elapsed = delay;
  last_invoke_time = SDL_GetTicks();
  frame++;
  step();
  
  Animator::context->refresh();
}

void Animator::unregister() {
  context->unregister_animator(this);
  registered = false;
  disabled = true;
}

int Animator::get_delay() {
  return delay;
}

void Animator::set_delay(int d) {
  delay = d;
}

void Animator::pause() {
  disabled = true;
}

void Animator::play() {
  disabled = false;
}

int Animator::get_last_invoke() {
  return last_invoke_time;
}

int Animator::get_frame() {
  return frame;
}

/*
void Animator::enable() {
  enabled = true;
}

void Animator::disable() {
  enabled = false;
}

bool Animator::is_enabled() {
  return enabled;
}
*/
