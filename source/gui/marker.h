#ifndef __SCIGMA_GUI_MARKER_H__
#define __SCIGMA_GUI_MARKER_H__

#include "definitions.h"
#include <string>
#include <vector>

namespace scigma
{
  namespace gui
  {
    
    class Marker
    {
      
    public:
      
      enum Type
      {
	DOT=0,
	PLUS=1,
	CROSS=2,
	RING=3,
	RDOT=4,
	RPLUS=5,
	RCROSS=6,
	QUAD=7,
	QDOT=8,
	QPLUS=9,
	QCROSS=10,
	HASH=11,
	STAR=12,
	NONE=13
      };
      
      static const size_t N_MARKERS=13;

      static void initialize();
      static bool is_initialized();
      static void terminate();
      
      static GLuint cross_hair_texture_id();
      static GLfloat cross_hair_size();
      static GLuint texture_id(Type type);
      
    private:
      Marker();
      
      static std::vector<GLuint> textureIDs_;
      static GLuint crossHairTextureID_;
      static constexpr GLfloat crossHairSize_=25.0f;
      static bool isInitialized_;

    };
    
  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_MARKER_H__ */
