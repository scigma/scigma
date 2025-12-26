#ifndef SCIGMA_GUI_RULER_HPP
#define SCIGMA_GUI_RULER_HPP

#include <string>
#include "definitions.hpp"
#include "drawable.hpp"
#include "spacetext.hpp"
#include "font.hpp"
#include "../common/events.hpp"
#include "glwindowevents.hpp"
#include "viewingareaevents.hpp"
#include "viewingvolumeevents.hpp"

using scigma::common::EventSource;
using scigma::common::EventSink;

namespace scigma
{
  namespace gui
  {
    class GLWindow;
    class GLContext;
    class Ruler;
    
    struct AdjustEvent
    { //!Argument list for AdjustEvent 
      typedef LOKI_TYPELIST_3(const Ruler*,GLfloat,GLfloat) Arguments;};

    class Ruler:public Drawable,
      public EventSource<AdjustEvent>::Type,
      public EventSink<RotateEvent>::Type,
      public EventSink<ResetEvent>::Type,
      public EventSink<MouseButtonEvent>::Type,
      public EventSink<MouseMotionEvent>::Type,
      public EventSink<ScrollEvent>::Type
    {
    public:
      Ruler(GLWindow* glWindow,Font::Type labelFont, Font::Type tickFont);
      ~Ruler();

      bool process(RotateEvent event, const ViewingVolume* volume, GLfloat ax,GLfloat ay, GLfloat az, GLfloat angle) override;
      bool process(ResetEvent event, const ViewingVolume* volume) override;
      bool process(MouseButtonEvent event, GLWindow* w, int button , int action, int mods) override;
      bool process(MouseMotionEvent event, GLWindow* w, GLfloat x, GLfloat y, GLfloat xOld, GLfloat yOld) override;
      bool process(ScrollEvent event, GLWindow* w, GLfloat xScroll, GLfloat yScroll) override;

      void set_label(std::string label);
      void set_end_points(const GLfloat* p1, const GLfloat* p2);
      void set_bounds(GLfloat min, GLfloat max);

      static void on_gl_context_creation(GLContext* context);
      static void on_gl_context_destruction(GLContext* context);
      
      void on_addition(GLContext* context);
      void on_removal(GLContext* context);

      void on_hover_begin(GLContext* context);
      void on_hover_end(GLContext* context);
                  
      static void before_batch_draw(GLContext* context);

      void draw(GLContext* context);

      void update_geometry(const ViewingVolume* volume);
      
    private:
      Ruler(const Ruler&);
      Ruler& operator=(const Ruler&);

      void prepare_attributes();      
      void rebuild();

      void tick_values(std::vector<GLfloat>& major, std::vector<GLfloat>& minor) const;

      GLfloat p1_[3];
      GLfloat p2_[3];

      GLfloat q1_[3];
      GLfloat q2_[3];

      GLfloat c_[2];
      GLfloat d_[2];

      GLfloat length_;

      GLfloat min_;
      GLfloat max_;

      GLfloat thickness_;
      
      GLWindow* glWindow_;

      std::string label_;

      Font::Type labelFont_;
      Font::Type tickFont_;
      
      SpaceText labelSpaceText_;
      SpaceText tickSpaceText_;
   
      size_t bufferSize_;
      size_t usedSize_;

      GLuint glBuffer_;

      bool attributesInvalid_;
      bool sliding_;
      bool hovering_;
      char padding_[1];

      static const char* vSource_;
      static const char* fSource_;    

      static const GLuint positionLocation_=0;
      static const GLuint offsetLocation_=1;

      static constexpr GLfloat minMajorDist_=80.0f;
      static constexpr GLfloat minMinorDist_=20.0f;
    };
    
  } /* end namespace gui */
} /* end namespace scigma */

#endif /* SCIGMA_GUI_RULER_HPP */
