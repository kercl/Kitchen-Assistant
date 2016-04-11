#include "shader.h"

#include <fstream>
#include <string>
#include <cstring>
#include <iostream>

#include <GL/gl.h>
#include <GL/glext.h>

using namespace std;

ShaderObject::ShaderObject(GLuint _program): program(_program), vertex_shader(0), fragment_shader(0) {}

ShaderObject::ShaderObject(string vs, string fs): program(0) {
  ifstream ifs(vs);
  string vertexsrc( (istreambuf_iterator<char>(ifs) ),
                  (istreambuf_iterator<char>()) );
  ifs.close();
  ifs.clear();
  ifs.open(fs);
  string fragmentsrc( (istreambuf_iterator<char>(ifs) ),
                    (istreambuf_iterator<char>()) );
  
  int length = vertexsrc.length();
  const char *source = vertexsrc.c_str();
  
	GLint result;
	
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, (const char **)&source, &length);
	glCompileShader(vertex_shader);

	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &result);
	if(result == GL_FALSE) {
		char *log;
		glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &length);
		log = new char[length];
		glGetShaderInfoLog(vertex_shader, length, &result, log);
		cerr << "Compilation of vertex shader failed: " << log << endl;
		delete [] log;
		glDeleteShader(vertex_shader);
		return;
	}
	
  length = fragmentsrc.length();
  source = fragmentsrc.c_str();
  
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, (const char **)&source, &length);
	glCompileShader(fragment_shader);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &result);
	if(result == GL_FALSE) {
		char *log;
		glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &length);
		log = new char[length];
		glGetShaderInfoLog(fragment_shader, length, &result, log);
		cerr << "Compilation of fragment shader failed: " << log << endl;
		delete [] log;
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		return;
	}
	
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glLinkProgram(program);
	
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if(result == GL_FALSE) {
		char *log;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		log = new char[length];
		glGetProgramInfoLog(program, length, &result, log);
    cerr << "Creation of program failed: " << log << endl;
		glDeleteProgram(program);
		program = 0;
	}
}

void ShaderObject::enable() {
  glUseProgram(program);
  
  for(auto x: uniform1i) {
    GLint loc = glGetUniformLocation(program, x.first.c_str());
    glUniform1i(loc, x.second);
  }
  for(auto x: uniform2f) {
    GLint loc = glGetUniformLocation(program, x.first.c_str());
    glUniform2f(loc, x.second.x, x.second.y);
  }
  /*
  for(auto x: uniformtx) {
    GLint loc = glGetUniformLocation(program, x.first.c_str());
    glActiveTexture(GL_TEXTURE0 + x.second.second);
    glBindTexture(x.second.first);
    glUniform1i(loc, x.second.second);
    glActiveTexture(GL_TEXTURE0);
  } */
}

void ShaderObject::disable() {
  glUseProgram(0);
}

void ShaderObject::pass_sampler2D(std::string var, GLint val) {
  uniform1i[var] = val;
}

void ShaderObject::pass_vec2(std::string var, Vec2 val) {
  uniform2f[var] = val;
}

GLuint ShaderObject::get_program() {
  return program;
}

