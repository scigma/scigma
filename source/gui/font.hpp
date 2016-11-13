#ifndef __SCIGMA_GUI_FONT_H__
#define __SCIGMA_GUI_FONT_H__

#include "definitions.hpp"
#include <string>
#include <vector>

namespace scigma
{
  namespace gui
  {
    
    class Font
    {
      
    public:
      
      enum Type
      {
	DEJAVU_SANS_MONO_14=0,
	DEJAVU_SANS_MONO_29=1,
	DEJAVU_SERIF_16=2,
	DEJAVU_SERIF_24=3,
	DEJAVU_SERIF_32=4,
	DEJAVU_SERIF_48=5
      };


      static void initialize();
      static bool is_initialized();
      static void terminate();
      
      static const Font& font(Type type);
      
      GLfloat string_width(const std::string& text) const;
      GLfloat char_width(char c) const;
      
      GLuint bitmapWidth;
      GLuint bitmapHeight;
      GLfloat charWidth[0x100];
      GLfloat charTextureWidth[0x100];
      GLfloat charHeight;
      GLfloat charTextureHeight;
      GLuint gridWidth;
      GLuint gridHeight;
      GLfloat charsPerRow;
      GLfloat charsPerColumn;
      GLuint textureID;
      unsigned char charOffset;
      
    private:
      Font();

      void set_metrics(const unsigned char* metricsData);
      void set_texture(unsigned char* pngData);
      
      static std::vector<Font> fonts;
      
      static bool isInitialized;

      static void add_font(GLfloat fontHeight, 
			   const unsigned char* metricsData, 
			   unsigned char* pngData);

      char padding_[3];
      
    };
    
  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_FONT_H__ */
