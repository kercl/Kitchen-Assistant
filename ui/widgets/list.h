#ifndef LIST_H
#define LIST_H

#include "widget.h"
#include "button.h"
#include "../utils.h"

#include <string>
#include <deque>
#include <algorithm>
#include <iostream>

#define LIST_ITEM_TEXT_HEIGHT 41
#define DRAG_MODE_REMOVE 0
#define DRAG_MODE_SCROLL 1

class List;

class ListItem: public Widget, public Animator {
protected:
  float h, scrollstate;
  RenderableGroup *parent;
  List *lst;
  Vec2 init_org, mouse_init_down;
  int drag_mode;
public:
  ListItem(float height);
  
  virtual EventReceiver* catches_event(const Event &ev);
  float get_height();
  virtual void resize(float width);
  void rearrange(float top) {
    parent->set_origin(Vec2(parent->get_origin().x, top));
  }
  
  virtual void on_mouse_down(const MouseEvent &ev);
  virtual void on_drag_end(const MouseEvent &ev);
  virtual void on_mouse_drag_motion(const MouseEvent &ev);
  virtual void on_removed() {}
  
  void step();
  
  virtual void adapt_opacity() {
    float diffx = parent->get_origin().x - init_org.x;
    opacity_factor = std::max(0.0f, (1.0f - std::max(0.0f, std::abs(diffx)) / 70.f));
    for(auto x: parent->get_children())
      x->set_opacity_factor(opacity_factor);
  }
  
  void set_parent_group(RenderableGroup *pg) {
    parent = pg;
    init_org = parent->get_origin();
  }
  
  void set_list_container(List *_lst) { lst = _lst; }
  List* get_list_container() { return lst; }
  RenderableGroup* get_container() { return parent; }
};

class ListItemText: public Button, public ListItem {
public:
  ListItemText(std::string _text, float height=LIST_ITEM_TEXT_HEIGHT);
  virtual void callback();
  virtual void resize(float width);
  virtual void render();
  
  virtual EventReceiver* catches_event(const Event &ev);
  
  virtual void on_mouse_down(const MouseEvent &ev) {
    Button::on_mouse_down(ev);
    ListItem::on_mouse_down(ev);
  }
  
  virtual void on_mouse_up(const MouseEvent &ev) {
    Button::on_mouse_up(ev);
  }
  
  virtual void on_drag_end(const MouseEvent &ev) {
    ListItem::on_drag_end(ev);
  }
  
  virtual void adapt_opacity() {
    ListItem::adapt_opacity();
    Button::set_opacity_factor(ListItem::opacity_factor);
  }
  
  virtual void on_mouse_drag_motion(const MouseEvent &ev) { 
    ListItem::on_mouse_drag_motion(ev);
  }
};

class List: public Widget {
  std::deque<ListItem*> items, deleted;
  Box *bg;
  
  Vec2 org;
  
  std::string column, table;
public:
  List(const Rect &layout);
  void synchronize(std::string _column, std::string _table);
  
  void append_item(ListItem *li);
  void remove_item(ListItem *li);
  
  void resize(float w);
  void update();
  void clear();
  
  std::deque<ListItem*>& get_items() { return items; }
  
  virtual EventReceiver* catches_event(const Event &ev);
  
  void rearrange_items();
  float list_height();
};

#endif

