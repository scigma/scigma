#include "marker.h"
#include "spritedata.h"
#include "glutil.h"

namespace scigma
{
  namespace gui
  {
    
    bool Marker::isInitialized_(false);
    GLuint Marker::crossHairTextureID_(0);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
    std::vector<GLuint> Marker:: textureIDs_(N_MARKERS);
#pragma clang diagnostic pop
    
    Marker::Marker()
    {}
    
    void Marker::initialize()
    {
      if(isInitialized_)
      return;

      unsigned char* spriteData[]={dot_png,plus_png,cross_png,
				   ring_png,rdot_png,rplus_png,crosshair_png,
				   quad_png,qdot_png,qplus_png,qcross_png,
				   hash_png,star_png};

      glActiveTexture(GL_TEXTURE0);
      for(size_t i(0);i<N_MARKERS;++i)
	textureIDs_[i]=get_texture_from_png_data(spriteData[i]);
      crossHairTextureID_=get_texture_from_png_data(crosshair_png);
      isInitialized_=true;
      GLERR;

    }
    
    void Marker::terminate()
    {
      if(!isInitialized_)
      return;
      for(size_t i(0);i<N_MARKERS;++i)
	glDeleteTextures(1,&textureIDs_[i]);
      isInitialized_=false;
      GLERR;
    }
    
    bool Marker::is_initialized()
    {
      return isInitialized_;
    }

    GLuint Marker::cross_hair_texture_id(){return crossHairTextureID_;}
    GLfloat Marker::cross_hair_size(){return crossHairSize_;}
    GLuint Marker::texture_id(Type type){return textureIDs_[type];}
    
  } /* end namespace gui */
} /* end namespace scigma */
