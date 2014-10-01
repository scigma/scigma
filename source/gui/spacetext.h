#ifndef __SCIGMA_GUI_SPACETEXT_H__
#define __SCIGMA_GUI_SPACETEXT_H__

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
        
    class SpaceText: public Drawable 
    {
    public:
      SpaceText(Font::Type type);
      ~SpaceText(); 
      
      void set_text(const std::vector<std::string>& text,
		    const std::vector<GLfloat>& x,
		    const std::vector<GLfloat>& y,
		    const std::vector<GLfloat>& z);
		   

      void set_text(const std::string* text,
		    const GLfloat* x,
		    const GLfloat* y,
		    const GLfloat* z,
		    size_t n);

      void set_color(GLfloat* fColor, GLfloat* bColor);
      
      void set_offset(GLfloat xOffset, GLfloat yOffset);

      static void on_gl_context_creation(GLContext* glContext);
      static void on_gl_context_destruction(GLContext* glContext);

      static void before_batch_draw(GLContext* glContext);

      void draw(GLContext* glContext);

    private:
      SpaceText(const SpaceText&);
      SpaceText& operator=(const SpaceText&);

      void prepare_attributes();

      const Font& font_;
      
      size_t bufferSize_;
      size_t usedSize_;
   
      GLuint glBuffer_;

      GLfloat fColor_[4];
      GLfloat bColor_[4];

      GLfloat globalOffset_[2];

      char padding_[4];

      static const char* vSource_;
      static const char* fSource_;    

      static const GLuint positionLocation_=0;
      static const GLuint localOffsetLocation_=1;
      static const GLuint uvValueLocation_=2;
      static const GLuint widthLocation_=3;

      static std::map<GLContext*,GLint> fColorLocationMap_;
      static std::map<GLContext*,GLint> bColorLocationMap_;
      static std::map<GLContext*,GLint> globalOffsetLocationMap_;
      static std::map<GLContext*,GLint> heightLocationMap_;

      static GLint fColorLocation_;
      static GLint bColorLocation_;
      static GLint globalOffsetLocation_;
      static GLint heightLocation_;

    };

  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_SPACETEXT_H__ */
