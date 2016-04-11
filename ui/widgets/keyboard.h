#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "widget.h"
#include "button.h"
#include "panel.h"
#include "../text.h"

#include <string>
#include <vector>
#include <sstream>

struct KeyLayoutData {
  KeyLayoutData(std::string normal, 
                std::string caps, 
                std::string alt);
  std::string normal, caps, alternative;
};

class Keyboard;

class SpecialKey: public Button {
public:
  enum key_type_t {
    SwitchLayout=0x0100,
    CapsLock=0x0200,
    BackSpace=0x0400,
    Return=0x0500,
    SpaceBar=0x0600
  };
private:
  Img *get_image(key_type_t t);
  
  key_type_t type;
  Keyboard *keyboard;
public:
  SpecialKey(Keyboard *kb, int x, int y, int w, int h, key_type_t t);
  void callback();
};

class Key: public Button {
  Text *main_label, *alt_label;
  Keyboard *kb;
public:
  Key(Keyboard *kb, int x, int y, int w, int h, KeyLayoutData kld);
  void render();
  void on_mouse_down(const MouseEvent &ev);
  void load_layout(int i, bool caps);
  void callback();
  
  KeyLayoutData key_data;
};

class Keyboard: public AppPanel {
  int current_layout;
  std::vector<Key*> keys;
  bool single_line, caps;
  
  std::string data;
  Text *result;
  int caret;
public:
  Keyboard(bool single_line = true);
  
  bool is_single_line();
  
  void cancel();
  void launch(std::string init_str = "");
  void confirm();
  void load_layout(int i, bool caps);
  virtual void on_confirm(std::string s) = 0;
  
  void keypress(const KeyLayoutData &kld);
  void keypress(SpecialKey::key_type_t kld);
  
  virtual void on_keypress(int keycode);
};

#endif

