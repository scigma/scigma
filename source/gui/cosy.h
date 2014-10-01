#ifndef __SCIGMA_GUI_COSY_H__
#define __SCIGMA_GUI_COSY_H__

#include <string>
#include "../pythonobject.h"
#include "../events.h"
#include "definitions.h"
#include "viewingvolumeevents.h"
#include "font.h"
#include "ruler.h"

namespace scigma
{
  namespace gui
  {
    class GLWindow;
    class GLContext;
    class ViewingVolume;
    class Cosy:public Drawable,public PythonObject<Cosy>,
      public EventSink<ScaleEvent>::Type,  
      public EventSink<ShiftEvent>::Type,
      public EventSink<RotateEvent>::Type,
      public EventSink<RangeEvent>::Type,  
      public EventSink<ResetEvent>::Type,
      public EventSink<AdjustEvent>::Type
      {
       public:
	Cosy(GLWindow* glWindow,int view,bool largeFontsFlag=false);
	~Cosy();
	virtual bool process(ScaleEvent event, const ViewingVolume* volume, GLfloat xFactor, GLfloat yFactor, GLfloat zFactor);
	virtual bool process(ShiftEvent event, const ViewingVolume* volume, GLfloat xShift, GLfloat yShift, GLfloat zShift);
	virtual bool process(RotateEvent event, const ViewingVolume* volume, GLfloat x, GLfloat y, GLfloat z,GLfloat angle);
	virtual bool process(RangeEvent event, const ViewingVolume* volume);
	virtual bool process(ResetEvent event, const ViewingVolume* volume);
	virtual bool process(AdjustEvent event, const Ruler* ruler, GLfloat min, GLfloat max);
     
	void set_view(int view);
	void set_label(int coordinate,std::string label);
	
	void draw(GLContext* glContext);

      private:
	Cosy(const Cosy&);
	Cosy& operator=(const Cosy&);

	void update_rulers(const ViewingVolume* volume);

	Ruler xRuler_,yRuler_,zRuler_;
        GLWindow* glWindow_;
	
	int view_;
	char padding[4];
      };

  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_COSY__H__ */
