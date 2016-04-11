#include "render.h"
#include <iostream>

using namespace std;

FT_Library Font::library;
bool Font::initialized = false;

Font::Font(std::string path, unsigned int _height) {
	textures = new GLuint[CHAR_COUNT_PRERENDER];
	height = _height;
	
  if(!initialized) {
	  if(FT_Init_FreeType( &library )) 
		  throw std::runtime_error("FT_Init_FreeType failed");
		else
		  initialized = true;
	}
  FT_Face face;
	if (FT_New_Face(library, path.c_str(), 0, &face)) {
		throw std::runtime_error("FT_New_Face failed");
	}

	FT_Set_Char_Size(face, _height << 6, _height << 6, 96, 96);

	listbase = glGenLists(CHAR_COUNT_PRERENDER);
	glGenTextures(CHAR_COUNT_PRERENDER, textures);
  
	for(unsigned char ch=0; ch < CHAR_COUNT_PRERENDER; ch++) {
	  if(FT_Load_Glyph(face, FT_Get_Char_Index(face, ch), FT_LOAD_DEFAULT))
		  throw std::runtime_error("FT_Load_Glyph failed");

    FT_Glyph glyph;
    if(FT_Get_Glyph(face->glyph, &glyph))
		  throw std::runtime_error("FT_Get_Glyph failed");
  
	  FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

	  FT_Bitmap& bitmap=bitmap_glyph->bitmap;

	  int w = 1;
	  int h = 1;
	  
    while(w < bitmap.width) w<<=1;
    while(h < bitmap.rows) h<<=1;
    
	  GLubyte* expanded_data = new GLubyte[ 2 * w * h];
    
    for(int j = 0; j < h ; j++) {
      for(int i = 0; i < w; i++) {
        expanded_data[2 * (i + j * w)] = CHAR_COUNT_PRERENDER;
        expanded_data[2 * (i + j * w) + 1] = 
            (i >= bitmap.width || j >= bitmap.rows) ? 0 : bitmap.buffer[i + bitmap.width * j];
      }
    }

    glBindTexture( GL_TEXTURE_2D, textures[ch]);
	  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data);

    delete [] expanded_data;
    
	  glNewList(listbase + ch, GL_COMPILE);
	  glBindTexture(GL_TEXTURE_2D, textures[ch]);
	  glTranslatef(bitmap_glyph->left,0,0);
	  glPushMatrix();
	  if(ch == '\'' || ch == '"' || ch == 176)
	    glTranslatef(0, abs(int(bitmap_glyph->top-bitmap.rows)) - bitmap.rows * 1.5, 0);
	  else if(ch == '-')
	    glTranslatef(0, abs(int(bitmap_glyph->top-bitmap.rows)) + height * 0.5 - bitmap.rows * 1.5, 0);
	  else
	    glTranslatef(0, abs(int(bitmap_glyph->top-bitmap.rows)) + height - bitmap.rows, 0);
    
	  float	rx=(float)bitmap.width / (float)w,
			    ry=(float)bitmap.rows / (float)h;
    
	  glBegin(GL_QUADS);
	  glTexCoord2d( 0,ry); glVertex2f(0,bitmap.rows);
	  glTexCoord2d( 0, 0); glVertex2f(0,0);
	  glTexCoord2d(rx, 0); glVertex2f(bitmap.width,0);
	  glTexCoord2d(rx,ry); glVertex2f(bitmap.width,bitmap.rows);
	  glEnd();
	  glPopMatrix();
	  glTranslatef((face->glyph->advance.x >> 6) ,0,0);
	  glEndList();
	  
	  charwidths[ch] = (face->glyph->advance.x >> 6);
	  
	  FT_Done_Glyph(glyph);
	}
  
	FT_Done_Face(face);
}

void Font::enable() const {
  glListBase(listbase);
}

float Font::get_height() const {
  return height;
}

void Font::clear() {
  glDeleteLists(listbase, CHAR_COUNT_PRERENDER);
  glDeleteTextures(CHAR_COUNT_PRERENDER, textures);
  delete [] textures;
}

Text::Text(const Font &ft, const Vec2 &_p): font(ft), p(_p) {}

std::stringstream& Text::get_stream() {
  return stream;
}

void Text::render() {
	glEnable(GL_TEXTURE_2D);
	glColor4f(blendcol.red,blendcol.green,blendcol.blue,blendcol.alpha*opacity_factor);
	
	font.enable();
	
	float h = font.get_height() / 0.63f;
	std::istringstream ss(stream.str());
	
	std::string line;
	int i = 0;
	while(getline(ss, line, '\n')) {
		glPushMatrix();
		glTranslatef(p.x, p.y + h*i, 0);
		glCallLists(line.length(), GL_UNSIGNED_BYTE, line.c_str());
		glPopMatrix();
		i++;
	}
	glDisable(GL_TEXTURE_2D);
}

float Text::get_width() {
  float w = 0;
  
  std::istringstream ss(stream.str());
  std::string line;
  getline(ss, line, '\n');
  for(int i = 0; i < line.length(); ++i) {
    w += font.char_width(line[i]);
  }
  return w;
}

void Text::set_position(const Vec2 &pos) {
  p = pos;
}

