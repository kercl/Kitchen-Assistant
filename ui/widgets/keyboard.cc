#include "keyboard.h"
#include "../img.h"
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <algorithm>

#define KB_Y_0 (600-68*4-70)

using namespace std;

KeyLayoutData::KeyLayoutData(std::string _normal, 
                std::string _caps, 
                std::string _sym)
: normal(_normal), caps(_caps), alternative(_sym) {}


Key::Key(Keyboard *_kb, int x, int y, int w, int h, KeyLayoutData kld)
: Button(w, h, NULL),
  key_data(kld),
  kb(_kb)
{
  translate(Vec2(x + 72, y));
  //set_color_scheme(RGBA("#34495e",1), RGBA("#34495e", 1));
  
  main_label = new Text(Renderable::context->load_font("data/fonts/OpenSans-Bold.ttf", 17), Vec2(15, h-30));
  main_label->set_blendcol(RGBA(1,1,1,0.9));
  main_label->get_stream() << kld.normal;
  RenderableGroup::attach(main_label);
  
  alt_label = new Text(Renderable::context->load_font("data/fonts/OpenSans-Regular.ttf", 10), Vec2(w-15, 5));
  alt_label->set_blendcol(RGBA("#f1c40f",0.9));
  alt_label->get_stream() << kld.alternative;
  RenderableGroup::attach(alt_label);
}
  
void Key::render() {
  Button::render();
}
  
void Key::callback() {}

void Key::on_mouse_down(const MouseEvent &ev) {
  kb->keypress(key_data);
}
  
void Key::load_layout(int i, bool caps) {
  if(i == 1) {
    main_label->get_stream().str(string());
    string s(key_data.normal);
    if(caps)
      transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return toupper(c); });
    main_label->get_stream() << s;
    main_label->set_blendcol(RGBA(1,1,1,0.9));
    
    alt_label->get_stream().str(string());
    alt_label->get_stream() << key_data.alternative;
    alt_label->set_blendcol(RGBA("#f1c40f",0.9));
  }else if(i == 2) {
    alt_label->get_stream().str(string());
    alt_label->get_stream() << key_data.normal;
    alt_label->set_blendcol(RGBA(1,1,1,0.9));
    
    main_label->get_stream().str(string());
    main_label->get_stream() << key_data.alternative;
    main_label->set_blendcol(RGBA("#f1c40f",0.9));
  }
}


Img* SpecialKey::get_image(key_type_t t) {
  Img *ret = NULL;
  if(t == SwitchLayout)
    ret = new Img("data/keyboard/switchlayout.png", Rect(0,0,32,32));
  else if(t == CapsLock)
    ret = new Img("data/keyboard/caps.png", Rect(0,0,32,32));
  else if(t == BackSpace)
    ret = new Img("data/keyboard/backspace.png", Rect(0,0,32,32));
  else if(t == Return)
    ret = new Img("data/keyboard/return.png", Rect(0,0,32,32));
  else if(t == SpaceBar)
    ret = new Img("data/keyboard/spacebar.png", Rect(0,0,64,32));
  
  ret->set_blendcol(RGBA(1,1,1,0.9));
  return ret;
}

SpecialKey::SpecialKey(Keyboard *kb, int x, int y, int w, int h, key_type_t t)
: Button(w, h, get_image(t)), 
  type(t),
  keyboard(kb)
{
  translate(Vec2(72 + x, y));
}
  
void SpecialKey::callback() {
  keyboard->keypress(type);
}

void Keyboard::load_layout(int i, bool caps) {
  current_layout = i; //current_layout ^ 3;
  for(auto x: keys)
    x->load_layout(i, caps);
  Renderable::context->refresh();
}

bool Keyboard::is_single_line() {
  return single_line;
}
  
Keyboard::Keyboard(bool _single_line)
: AppPanel(RGBA("#2c3e50",1), RGBA("#34495e",1)),
  current_layout(1),
  single_line(_single_line),
  caret(0),
  caps(false)
{
  hide_panel();
  
  Box *bg = new Box(Rect(72,0,1024,250));
  bg->set_background_color(RGBA("#ecf0f1",1));
  RenderableGroup::attach(bg);
  
  get_background()->set_background_color(RGBA("#2c3e50",1));
  /*
  char l0[] = "1234567890?",
       l1[] = "qwertzuiop/",
       l2[] = "asdfghjkl+",
       l3[] = "yxcvbnm,.-";*/
  vector<KeyLayoutData> l0 = {
    KeyLayoutData("1","1","1"),
    KeyLayoutData("2","2","2"),
    KeyLayoutData("3","3","3"),
    KeyLayoutData("4","4","4"),
    KeyLayoutData("5","5","5"),
    KeyLayoutData("6","6","6"),
    KeyLayoutData("7","7","7"),
    KeyLayoutData("8","8","8"),
    KeyLayoutData("9","9","9"),
    KeyLayoutData("0","0","0")};
  vector<KeyLayoutData> l1 = {
    KeyLayoutData("q","Q","@"),
    KeyLayoutData("w","W","!"),
    KeyLayoutData("e","E","\""),
    KeyLayoutData("r","R","$"),
    KeyLayoutData("t","T","%"),
    KeyLayoutData("y","Y","&"),
    KeyLayoutData("u","U","("),
    KeyLayoutData("i","I",")"),
    KeyLayoutData("o","O","="),
    KeyLayoutData("p","P","?"),
    KeyLayoutData("/","/","\\")};
  vector<KeyLayoutData> l2 = {
    KeyLayoutData("a","A","{"),
    KeyLayoutData("s","S","["),
    KeyLayoutData("d","D","]"),
    KeyLayoutData("f","F","}"),
    KeyLayoutData("g","G","#"),
    KeyLayoutData("h","H","'"),
    KeyLayoutData("j","J","~"),
    KeyLayoutData("k","K","<"),
    KeyLayoutData("l","L",">"),
    KeyLayoutData("+","+","*")};
  vector<KeyLayoutData> l3 = {
    KeyLayoutData("z","Z","z"),
    KeyLayoutData("x","X","x"),
    KeyLayoutData("c","C","c"),
    KeyLayoutData("v","V","v"),
    KeyLayoutData("b","B","b"),
    KeyLayoutData("n","N","n"),
    KeyLayoutData("m","M","m"),
    KeyLayoutData(",",",",";"),
    KeyLayoutData(".",".",":"),
    KeyLayoutData("-","-","_")};
       
  Key *tmp;
  for(int i = 0; i < l1.size(); ++i) {
    if(i < l0.size()) {
      tmp = new Key(this, 20 + 68 * i, KB_Y_0+68*0+10, 64, 48, l0[i]);
      keys.push_back(tmp);
      attach(tmp);
    }
    tmp = new Key(this, 20 + 34*1 + 68 * i, KB_Y_0+68*1, 64, 64, l1[i]);
    keys.push_back(tmp);
    attach(tmp);
    if(i < l2.size()) {
      tmp = new Key(this, 20 + 34*2 + 68 * i, KB_Y_0+68*2, 64, 64, l2[i]);
      keys.push_back(tmp);
      attach(tmp);
    }
    if(i < l3.size()) {
      tmp = new Key(this, 20 + 34*3 + 68 * i, KB_Y_0+68*3, 64, 64, l3[i]);
      keys.push_back(tmp);
      attach(tmp);
    }
  }
  attach(new SpecialKey(this, 20, KB_Y_0+68*3, 64+34, 64, SpecialKey::SwitchLayout));
  attach(new SpecialKey(this, 20, KB_Y_0+68*2, 64, 64, SpecialKey::CapsLock));
  attach(new SpecialKey(this, 20 + 34*1 + 68 *11, KB_Y_0+68*1, 64+62, 64, SpecialKey::BackSpace));
  attach(new SpecialKey(this, 20 + 68 *11, KB_Y_0+68*2, 64+62+32+2, 64, SpecialKey::Return));
  attach(new SpecialKey(this, 20 + 68 * 3, KB_Y_0+68*4, 20+64*6, 52, SpecialKey::SpaceBar));
  
  //attach(new Key(20, KB_Y_0+68*3, 64+34, 64, "Caps", this));
  
  //attach(new Key(10 + 69, 100, "w", this));
  result = new Text(Renderable::context->load_font("data/fonts/OpenSans-Regular.ttf", 15), Vec2(15+72, 15));
  result->set_blendcol(RGBA(0,0,0,0.9));
  RenderableGroup::attach(result);
}

void Keyboard::confirm() {
  hide_panel();
  on_confirm(data);
}

void Keyboard::cancel() {
  hide_panel();
}

void Keyboard::launch(std::string init_str) {
  show_panel();
  data = init_str;
  result->get_stream().str(data);
  caret = init_str.length();
}

void Keyboard::keypress(const KeyLayoutData &kld) {
  if(current_layout == 1) {
    string s(kld.normal);
    if(caps)
      transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return toupper(c); });
    data += s;
    if(s.length() == 1) {
      on_keypress(s[0]);
    }
  }else if(current_layout == 2) {
    data += kld.alternative;
    if(kld.alternative.length() == 1)
      on_keypress(kld.alternative[0]);
  }
  
  result->get_stream().str(data);
  Renderable::context->refresh();
  
  caret++;
}

void Keyboard::keypress(SpecialKey::key_type_t kld) {
  if(kld == SpecialKey::Return && is_single_line()) {
    confirm();
    on_keypress(SpecialKey::Return);
  }else if(kld == SpecialKey::BackSpace) {
    data = data.substr(0,max(0,caret-1)) + data.substr(caret);
    result->get_stream().str(data);
    caret = max(0,min(caret - 1, (int)data.length()));
    on_keypress(SpecialKey::BackSpace);
  }else if(kld == SpecialKey::SwitchLayout) {
    load_layout(current_layout ^ 3, caps);
  }else if(kld == SpecialKey::CapsLock) {
    caps = !caps;
    load_layout(current_layout, caps);
  }else if(kld == SpecialKey::SpaceBar) {
    data += ' ';
    caret++;
    on_keypress(SpecialKey::SpaceBar);
  }
  
  result->get_stream().str(data);
  Renderable::context->refresh();
}

void Keyboard::on_keypress(int keycode) {}

