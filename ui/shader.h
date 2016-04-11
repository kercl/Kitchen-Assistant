#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <cstring>
#include <map>

#include <GL/gl.h>
#include <GL/glext.h>

#include "texture.h"

class ShaderObject {
private:
  GLuint vertex_shader, fragment_shader, program;
  
  std::map<std::string, GLint> uniform1i;
  std::map<std::string, Vec2> uniform2f;
public:
  ShaderObject(std::string vs, std::string fs);
  ShaderObject(GLuint program);
  
  void pass_sampler2D(std::string var, GLint val);
  void pass_vec2(std::string var, Vec2 val);
  
  void enable();
  void disable();
  
  GLuint get_program();
};

#endif

