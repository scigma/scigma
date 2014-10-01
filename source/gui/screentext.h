#ifndef __SCIGMA_GUI_SCREENTEXT_H__
#define __SCIGMA_GUI_SCREENTEXT_H__

#include <string>
#include <vector>
#include <map>
#include "definitions.h"
#include "drawable.h"
#include "font.h"

namespace scigma
{
  namespace gui
  {
    class GLContext;
        
    class ScreenText: public Drawable 
    {
    public:
      ScreenText(Font::Type type);
      ~ScreenText(); 
      
      void set_text(const std::vector<std::string>& text,
		    const std::vector<GLfloat>& x,
		    const std::vector<GLfloat>& y,
		    GLfloat xAnchor,
		    GLfloat yAnchor);

      void set_text(const std::string* text,
		    const GLfloat* x,
		    const GLfloat* y,
		    GLfloat xAnchor,
		    GLfloat yAnchor,
		    size_t n);

      void set_color(GLfloat* fColor, GLfloat* bColor);

      static void on_gl_context_creation(GLContext* glContext);
      static void on_gl_context_destruction(GLContext* glContext);

      static void before_batch_draw(GLContext* glContext);

      void draw(GLContext* glContext);

      void set_offset(GLfloat xOffset, GLfloat yOffset);

    private:
      ScreenText(const ScreenText&);
      ScreenText& operator=(const ScreenText&);

      void prepare_attributes();

      const Font& font_;
      
      size_t bufferSize_;
      size_t usedSize_;
   
      GLuint glBuffer_;

      GLfloat fColor_[4];
      GLfloat bColor_[4];

      GLfloat offset_[2];

      enum ScreenAnchor
      {
	SCREEN_ANCHOR_BOTTOM_LEFT= 1,
	SCREEN_ANCHOR_BOTTOM_RIGHT= 2,
	SCREEN_ANCHOR_TOP_RIGHT= 3,
	SCREEN_ANCHOR_TOP_LEFT= 4,
      };
      
      ScreenAnchor screenAnchor_;

      static const char* vSource_;
      static const char* fSource_;    

      static const GLuint positionLocation_=0;
      static const GLuint uvValueLocation_=1;

      static std::map<GLContext*,GLint> screenAnchorLocationMap_;
      static std::map<GLContext*,GLint> offsetLocationMap_;
      static std::map<GLContext*,GLint> fColorLocationMap_;
      static std::map<GLContext*,GLint> bColorLocationMap_;

      static GLint screenAnchorLocation_;
      static GLint offsetLocation_;
      static GLint fColorLocation_;
      static GLint bColorLocation_;


    };

  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_SCREENTEXT_H__ */
