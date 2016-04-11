#include "clock.h"
#include "../geom.h"
#include "../box.h"
#include "../fbo.h"

#include <ctime>
#include <iostream>
#include <cmath>

using namespace std;

void ClockWidget::step() {
  fbp.render_on();
  
  time_t now = time(0);
  ltm = localtime(&now);
  
  if(init_flag) {
    init_flag = false;
    return;
  }
  
  float minsecs = ltm->tm_min * 60.0 + ltm->tm_sec,
        hoursecs = ltm->tm_hour * 3600.0 + minsecs;
  
  sec_grp->set_angle(int(6 * (ltm->tm_sec - 1)) % 360);
  min_grp->set_angle(int(6 * (minsecs / 60.0)) % 360);
  hour_grp->set_angle(int(30 * (hoursecs / 3600.0)) % 360);
  
  sha.reference_time = sec_grp->get_angle();
  Renderable::context->register_animator(&sha);
}

int ClockWidget::get_year() { return ltm->tm_year; }
int ClockWidget::get_month() { return ltm->tm_mon; }
int ClockWidget::get_day() { return ltm->tm_mday; }
int ClockWidget::get_day_of_week() { return ltm->tm_wday; }

ClockWidget::SecondHandAnimator::SecondHandAnimator(ClockWidget *_parent): parent(_parent), Animator(20), cum_elapsed(0) {}

void ClockWidget::SecondHandAnimator::step() {
  parent->fbp.render_on();
  parent->sec_grp->set_angle(reference_time + 6 * (1 - exp(-0.35*cum_elapsed)*cos(0.5 * cum_elapsed)));
  cum_elapsed += 1;
  
  if(cum_elapsed > 15) {
    unregister();
    cum_elapsed = 0;
  }
}

ClockWidget::ClockWidget(Vec2 position, float radius)
: fbp(4*radius, 4*(radius), false), // render larger for sampling (twice the radius), render only if told
  r(position.x-radius,
    position.y-radius,
    position.x+radius,
    position.y+radius),
  Animator(1000),
  init_flag(true), // don't do any animation on initialization
  sha(this),
  pos(position),
  rad(radius)
{
  radius = radius * 2;
  Vec2 p0 = Vec2(radius,radius);
  
  Circle *c = new Circle(p0, radius, 70);
  c->set_blendcol(RGBA(0,0,0,0.5));
  fbp.push(c);
  
  c = new Circle(p0, radius - 4, 70);
  c->set_blendcol(RGBA(1,1,1,0.8));
  fbp.push(c);
  c = new Circle(p0, radius - 14, 70);
  c->set_blendcol(RGBA(1,1,1,0.8));
  fbp.push(c);
  
  Vec2 tv;
  for(int i = 0; i < 12; ++i) {
    tv = Vec2(sin((M_PI / 6.0) * i), -cos((M_PI / 6.0) * i));
    c = new Circle(p0 + tv*radius*0.85, radius * (i % 3 == 0 ? 0.07: 0.05), 25);
    c->set_blendcol(RGBA(0,0,0,1));
    fbp.push(c);
  }
  
  c = new Circle(p0, radius * 0.1, 25);
  c->set_blendcol(RGBA(0,0,0,1));
  fbp.push(c);
  
  sec_grp = new RenderableGroup();
  Box *hand = new Box(Rect(-radius*0.02, -0.7*radius,radius*0.02, 0.3*radius));
  hand->set_border_radius(radius*0.02);
  hand->set_background_color(RGBA(0,0,0,1));
  sec_grp->attach(hand);
  sec_grp->set_origin(p0);
  fbp.push(sec_grp);
  
  min_grp = new RenderableGroup();
  hand = new Box(Rect(-radius*0.035, -0.65*radius,radius*0.035, 0.0));
  hand->set_border_radius(radius*0.035);
  hand->set_background_color(RGBA(0,0,0,1));
  min_grp->attach(hand);
  min_grp->set_origin(p0);
  fbp.push(min_grp);
  
  hour_grp = new RenderableGroup();
  hand = new Box(Rect(-radius*0.035, -0.5*radius,radius*0.035, 0.0));
  hand->set_border_radius(radius*0.035);
  hand->set_background_color(RGBA(0,0,0,1));
  hour_grp->attach(hand);
  hour_grp->set_origin(p0);
  fbp.push(hour_grp);
  
  sec_grp->set_angle(45);
  min_grp->set_angle(90);
  hour_grp->set_angle(210);
  
  
  Box* clock_face = new Box(r);
  clock_face->set_background_color(RGBA(0,0,0,0));
  clock_face->set_blendcol(RGBA(1,1,1,0.65));
  clock_face->append_texture(new Texture(fbp.get_texture_id()));
  clock_face->append_texture(new RadialGradientTexture({
    {0,RGBA(1,1,1,0.5)},
    {50,RGBA(1,1,1,0.0)}}), TexCoords(Vec2(-0.25,-0.25), Vec2(-0.25,1.8), Vec2(1.8,1.8), Vec2(1.8,-0.25)));
  RenderableGroup::attach(clock_face);
  
  Renderable::context->push(&fbp);
  Renderable::context->register_animator(this);
  
  step();
}

void ClockWidget::on_mouse_click(const MouseEvent &ev) {

}

EventReceiver* ClockWidget::catches_event(const Event &ev) {
  if(ev.get_event_type() <= EventTypeMouse)
    if((dynamic_cast<const MouseEvent&>(ev).get_cursor() - pos).length() < rad)
      return this;
  return NULL;
}

void ClockWidget::issue_event(const Event &ev) {

}

ClockWidget::ClockWidget(Vec2 position, float radius, bool empty)
: fbp(4*radius, 4*(radius), false), // render larger for sampling (twice the radius), render only if told
  Animator(1000),
  init_flag(true), // don't do any animation on initialization
  sha(this),
  r(position.x-radius,
    position.y-radius,
    position.x+radius,
    position.y+radius),
  pos(position),
  rad(radius) {}

MinimalisticClockWidget::MinimalisticClockWidget(Vec2 position, float radius)
: ClockWidget(position, radius, true)
{
  radius = radius * 2;
  Vec2 p0 = Vec2(radius,radius);
  
  Circle *c = new Circle(p0, 0.90 * radius, 70);
  c->set_blendcol(RGBA(0,0,0,0.62));
  c->set_border_color(RGBA(0,0,0,1));
  c->set_border_width(0.040*radius);
  fbp.push(c);
  /*
  c = new Circle(p0, radius - 4, 70);
  c->set_blendcol(RGBA(0,0,0,0.8));
  fbp.push(c);*/
  /*
  Vec2 tv;
  for(int i = 0; i < 12; ++i) {
    tv = Vec2(sin((M_PI / 6.0) * i), -cos((M_PI / 6.0) * i));
    c = new Circle(p0 + tv*radius*0.85, radius * (i % 3 == 0 ? 0.07: 0.05), 25);
    c->set_blendcol(RGBA(0,0,0,1));
    fbp.push(c);
  }*/
  
  c = new Circle(p0, radius * 0.1, 25);
  c->set_blendcol(RGBA(0,0,0,1));
  fbp.push(c);
  
  sec_grp = new RenderableGroup();
  Box *hand = new Box(Rect(-radius*0.01, -0.85*radius,radius*0.01, 0.3*radius));
  //hand->set_border_radius(radius*0.02);
  hand->set_border_radius(radius*0.004);
  hand->set_background_color(RGBA(0,0,0,1));
  sec_grp->attach(hand);
  sec_grp->set_origin(p0);
  fbp.push(sec_grp);
  
  min_grp = new RenderableGroup();
  hand = new Box(Rect(-radius*0.020, -0.75*radius,radius*0.020, 0.0));
  //hand->set_border_radius(radius*0.035);
  hand->set_border_radius(radius*0.008);
  hand->set_background_color(RGBA(0,0,0,1));
  min_grp->attach(hand);
  min_grp->set_origin(p0);
  fbp.push(min_grp);
  
  hour_grp = new RenderableGroup();
  hand = new Box(Rect(-radius*0.020, -0.60*radius,radius*0.020, 0.0));
  //hand->set_border_radius(radius*0.035);
  hand->set_border_radius(radius*0.008);
  hand->set_background_color(RGBA(0,0,0,1));
  hour_grp->attach(hand);
  hour_grp->set_origin(p0);
  fbp.push(hour_grp);
  
  sec_grp->set_angle(45);
  min_grp->set_angle(90);
  hour_grp->set_angle(210);
  
  
  Box* clock_face = new Box(r);
  clock_face->set_background_color(RGBA(0,0,0,0));
  clock_face->set_blendcol(RGBA(1,1,1,0.65));
  clock_face->append_texture(new Texture(fbp.get_texture_id()));
  /*clock_face->append_texture(new RadialGradientTexture({
    {0,RGBA(1,1,1,0.5)},
    {50,RGBA(1,1,1,0.0)}}), TexCoords(Vec2(-0.25,-0.25), Vec2(-0.25,1.8), Vec2(1.8,1.8), Vec2(1.8,-0.25)));*/
  RenderableGroup::attach(clock_face);
  
  Renderable::context->push(&fbp);
  Renderable::context->register_animator(this);
  
  step();
}

