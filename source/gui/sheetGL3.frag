/* number of points in one ray/layer that are drawn */
uniform int nDrawn_;
/* total number of points in one ray/layer */
uniform int nTotal_;
/* sprite_==0: no sprite; sprite_==1: regular marker; sprite==2: hovering */
uniform int sprite_;
/* size of marker */
uniform float size_;
/* display bundle lighter if hovering over it*/
uniform int lighter_;
/* texture of the currently used sprite */
uniform sampler2D sampler_;

/* if the colormap is used, use comments to block the
   uniform, else replace _REPLACE_COLOR_UNIFORM_START_
   and _REPLACE_COLOR_UNIFORM_END_ with empty strings 
*/
__REPLACE_COLOR_UNIFORM_START__
uniform vec4 rgba_;
__REPLACE_COLOR_UNIFORM_END__


in vec2 screenPos_;
in float indexF_;

/* if the colormap is not used, use comments to block the
   incoming attribute, else replace _REPLACE_COLOR_IN_START_
   and _REPLACE_COLOR_IN_END_ with empty strings 
*/
__REPLACE_COLOR_IN_START__
in vec4 rgba_;
__REPLACE_COLOR_IN_END__

in float t_;

out vec4 color_;

void main()
{
  if(t_<-1.0||t_>1.0)
    discard;

  /* do not draw connection between different rays/layers */ 
  if(mod(indexF_,nTotal_)>=nDrawn_-1)
    discard;

  vec4 rgba2_=rgba_;
  if(sprite_==2) // do not draw hover sprite in hover picking mode (uniqueID.z==2)
    {
      if(1.5<uniqueID.z)
	discard;
      else
	rgba2_=foregroundColor;
    }
  if(sprite_!=0) // draw sprites
    {
      if(size_>2)
	{
	  vec2 pointCoord_=(screenPos_-gl_FragCoord.xy)/size_+vec2(0.5,0.5);
	  rgba2_=texture(sampler_,pointCoord_).r*rgba2_;
	  if(rgba2_.a==0)discard;
	}
    }
  if(lighter_==1)
    rgba2_=rgba2_+(1-rgba2_)*0.5;
  if(0.5>uniqueID.z) // normal coloring (uniqueID.z==0)
    {
      color_=rgba2_;
    }
  else if(1.5>uniqueID.z) // coloring for picking of whole bundles (uniqueID.z==1)
    {
      color_=vec4(uniqueID.xy,gl_FragCoord.z,1);
    }
  else
    {
      // coloring for picking of individual points (uniqueID.z==2)
      color_=vec4(mod(indexF_/(256*256),256)/256.0,mod(indexF_/256,256)/256.0,(mod(indexF_,256)+0.5)/256.0,1.0);
    }
}

