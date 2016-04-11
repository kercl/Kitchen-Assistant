#include "list.h"
#include "../box.h"

#include <iostream>

using namespace std;

ListItem::ListItem(float height): h(height), Animator(10) {
  Renderable::context->register_animator(this);
  pause();
}

void ListItem::resize(float width) {}

void ListItem::step() {
  if(std::abs(parent->get_origin().x - init_org.x) < 0.1f) {
    parent->set_origin(init_org);
    pause();
  }else {
    parent->set_origin((init_org + parent->get_origin()) * 0.5f);
  }
  adapt_opacity();
  lst->rearrange_items();
  Renderable::context->refresh();
}

void ListItem::on_drag_end(const MouseEvent &ev) {
  play();
  if(opacity_factor == 0.0f) {
    lst->remove_item(this);
    on_removed();
  }
}

void ListItem::on_mouse_down(const MouseEvent &ev) {
  mouse_init_down = ev.get_cursor();
  drag_mode = -1;
  scrollstate = lst->get_origin().y;
}

void ListItem::on_mouse_drag_motion(const MouseEvent &ev) {
  float diffx = ev.get_x() - mouse_init_down.x,
        diffy = ev.get_y() - mouse_init_down.y;
  
  if(drag_mode == -1) {
    if(abs(diffy) >= abs(diffx))
      drag_mode = DRAG_MODE_SCROLL;
    else
      drag_mode = DRAG_MODE_REMOVE;
  }
  
  if(drag_mode == DRAG_MODE_SCROLL) {
    Vec2 org;
    org.y = scrollstate + diffy;
    
    Vec2 bounds(0.0f, min(0.0f, 600 - lst->list_height()));
    //org.y = max(0.0f, max(min(0.0f, 600 - lst->list_height()), diffy));
    if(bounds.x > org.y && org.y > bounds.y)
      lst->set_origin(org);
    Renderable::context->refresh();
  }else {
    parent->set_origin(init_org + Vec2(diffx / (1 + std::abs(diffx) * 0.01f), 0));
    adapt_opacity();
    lst->rearrange_items();
    Renderable::context->refresh();
  }
}

float ListItem::get_height() {
  return h * opacity_factor;
}
  
EventReceiver* ListItem::catches_event(const Event &ev) {
  return Widget::catches_event(ev);
}

ListItemText::ListItemText(std::string _text, float height)
: ListItem(height),
  Button(200, height, _text)
{
  //Button::translate(Vec2(72,0));
}

void ListItemText::callback() {}

void ListItemText::render() {
  Button::render();
}

void ListItemText::resize(float width) {
  set_dimensions(width, get_height());
}

EventReceiver* ListItemText::catches_event(const Event &ev) {
  return Button::catches_event(ev);
}

List::List(const Rect &layout) {
  bg = new Box(layout);
  bg->set_background_color(RGBA("#ffffff",0.6));
  RenderableGroup::attach(bg);
  
  org.x = layout.left;
  org.y = layout.top;
}

float List::list_height() {
  float h = 0;
  for(auto x: items) {
    h += x->get_height() + 1;
  }
  return h;
}

void List::synchronize(std::string _column, std::string _table) {
  column = _column;
  table = _table;
}

void List::update() {
  clear();
}


void List::resize(float w) {
  if(w < 0.f)
    return;

  for(auto x: items)
    x->resize(w);
  Rect r(bg->get_shape());
  r.right = r.left + w;
  bg->set_shape(r);
}
  
void List::append_item(ListItem *li) {
  float h = list_height();
  Rect r(bg->get_shape());

  RenderableGroup *item_container = new Widget();
  item_container->attach(li);
  item_container->translate(Vec2(r.left,h + 1));
  
  li->set_parent_group(item_container);
  li->set_list_container(this);
  
  RenderableGroup::attach(item_container);
  items.push_back(li);
  
  li->resize(r.right - r.left);
  
  if((r.bottom - r.top) < li->get_height()) {
    bg->hide();
  }else {
    bg->show();
    Rect r(bg->get_shape());
    r.top = h + li->get_height() + 2;
    bg->set_shape(r);
  }
  
  for(auto x: deleted) {
    if(x)
      delete x->get_container();
  }
  deleted.clear();
}

void List::remove_item(ListItem* li) {
  RenderableGroup::detach(li->get_container());
  li->unregister();
  deleted.push_back(li);
  for(std::deque<ListItem*>::iterator it=items.begin(); it != items.end(); ++it) {
    if(*it == li) {
      items.erase(it);
      break;
    }
  }
  rearrange_items();
  Renderable::context->refresh();
}

void List::clear() {
  set_origin(Vec2());
  for(deque<ListItem*>::iterator it = items.begin(); it != items.end(); ++it) {
    RenderableGroup::detach((*it)->get_container());
    (*it)->unregister();
    delete (*it)->get_container();
  }
  items.clear();
  rearrange_items();
}
  
EventReceiver* List::catches_event(const Event &ev) {
  Vec2 p0(org);
  p0.y += get_origin().y;
  for(auto x: items) {
    if(!x)
      continue;
  
    if(ev.get_event_type() <= EventTypeMouse) {
      MouseEvent mev(dynamic_cast<const MouseEvent&>(ev));
      mev.set_cursor(mev.get_cursor() - p0);
      EventReceiver *w = x->catches_event(mev);
      if(w != NULL)
        return w;
    }
    p0.y += x->get_height();
  }
  return NULL;
}

void List::rearrange_items() {
  float h = org.y + 1;
  for(auto x: items) {
    Vec2 org(x->get_container()->get_origin());
    x->get_container()->set_origin(Vec2(org.x, h));
    h += x->get_height() + 1;
  }
  
  if(h > bg->get_shape().bottom) {
    bg->hide();
  }else {
    bg->show();
    Rect r(bg->get_shape());
    r.top = h;
    bg->set_shape(r);
  }
}

