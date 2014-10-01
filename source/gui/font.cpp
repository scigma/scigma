#include <string>
#include <cstdlib>
#include <cstdio>
#include "font.h"
#include "fontdata.h"
#include "glutil.h"
#include <iostream>



namespace scigma
{
  namespace gui
  {
    
    bool Font::isInitialized(false);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
    std::vector<Font> Font:: fonts;
#pragma clang diagnostic pop
    
    Font::Font()
    {}
    
    void Font::initialize()
    {
      if(is_initialized())
	return;
      add_font(14, DejaVuSansMono14_dat,DejaVuSansMono14_png);
      add_font(29, DejaVuSansMono29_dat,DejaVuSansMono29_png);
      add_font(16, DejaVuSerif16_dat,DejaVuSerif16_png);
      add_font(24, DejaVuSerif24_dat,DejaVuSerif24_png);
      add_font(32, DejaVuSerif32_dat,DejaVuSerif32_png);
      add_font(48, DejaVuSerif48_dat,DejaVuSerif48_png);
      isInitialized=true;
      GLERR;
    }
    
    void Font::terminate()
    {
      if(!is_initialized())
	return;
      for(size_t i(0);i<fonts.size();++i)
	glDeleteTextures(1,&fonts[i].textureID);
      fonts.clear();
      isInitialized=false;
      GLERR;
    }
    
    bool Font::is_initialized()
    {
      return isInitialized;
    }
    
    GLfloat Font::string_width(const std::string& text) const
    {
      GLfloat res(0.0);
      for(std::size_t i(0), l=text.length();i!=l;++i)
	res+=charWidth[size_t(text.at(i))];
      return res;
    }
    
    GLfloat Font::char_width(char c) const
    {
      return charWidth[size_t(c)];
    }
    
    const Font& Font::font(Type type){return fonts[type];}
    
    void Font::set_metrics(const unsigned char* metricsData)
    {
      bitmapWidth=GLuint(metricsData[0])+GLuint(metricsData[1])*0x100+GLuint(metricsData[2])*0x10000+GLuint(metricsData[3])*0x1000000;
      bitmapHeight=GLuint(metricsData[4])+GLuint(metricsData[5])*0x100+GLuint(metricsData[6])*0x10000+GLuint(metricsData[7])*0x1000000;
      gridWidth=GLuint(metricsData[8])+GLuint(metricsData[9])*0x100+GLuint(metricsData[10])*0x10000+GLuint(metricsData[11])*0x1000000;
      gridHeight=GLuint(metricsData[12])+GLuint(metricsData[13])*0x100+GLuint(metricsData[14])*0x10000+GLuint(metricsData[15])*0x1000000;
      charOffset=metricsData[16];
      for(int i=0;i<256;++i)
	{
	  charWidth[i]=GLfloat(metricsData[17+i]);
	  charTextureWidth[i]=charWidth[i]/GLfloat(bitmapWidth);
	}
      charTextureHeight=charHeight/GLfloat(bitmapHeight);
      
      charsPerRow=GLfloat(bitmapWidth)/GLfloat(gridWidth);
      charsPerColumn=GLfloat(bitmapHeight)/GLfloat(gridHeight);    
    }
    
    
    void Font::add_font(GLfloat fontHeight, const unsigned char* metricsData, unsigned char* pngData)
    {
      fonts.push_back(Font());
      Font& f=fonts.back();
      
      f.charHeight=fontHeight;
      f.set_metrics(metricsData);
      f.textureID=get_texture_from_png_data(pngData,NULL,NULL);
    }
    
  } /* end namespace gui */
} /* end namespace scigma */
