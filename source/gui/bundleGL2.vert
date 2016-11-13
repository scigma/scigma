/* number of points in one ray/layer that are drawn */
uniform int nDrawn_;
/* total number of points in one ray/layer */
uniform int nTotal_;
/* index of currently processed vertex, used for picking;
   could be replaced by gl_VertexID, which is not available
   on all platforms */
attribute float index_;
/* attributes for varying values go here; these attributes
   depend on the view and must be injected whenever the
   view changes; example:
   attribute float t;
   attribute float x;
   attribute float y;
   attribute float z;
   instead of _REPLACE_ATTRIBUTES_
*/
__REPLACE_ATTRIBUTES__

varying vec2 screenPos_;
varying float t_;
varying float indexF_;

/* if the colormap is not used, use comments to block the
   outgoing attribute, else just replace _REPLACE_COLOR_START_
   and _REPLACE_COLOR_END_ with empty strings
*/
__REPLACE_COLOR_START__
varying vec4 rgba_;

vec4 colormap_(in float p)
{
  vec4 color;
  if(p<-1.0||p>1.0)
    color= vec4(1,0.0,1,1.0);
  else if (p<0.0)
    color= mix(vec4(0,0,1.0,1.0),vec4(0.0,1.0,0,1.0),1-p);
  else
    color= mix(vec4(0,1.0,0.0,1.0),vec4(1.0,0.0,0.0,1.0),p);
  return color;
}
__REPLACE_COLOR_END__

void main()
{
  /* time coordinate goes here; example: t_=t; */
  t_=__REPLACE_TIME__;

  /* if the colormap is not used, use comments to block the
   outgoing attribute, else replace _REPLACE_COLOR_START_
   and _REPLACE_COLOR_END_ with empty strings and set
   _REPLACE_COLOR_ to the value used as color expression
  */
  __REPLACE_COLOR_START__
  float p_=__REPLACE_COLOR__;
  rgba_=colormap_(p_);
  __REPLACE_COLOR_END__
  
  /* coordinates in viewing space go here; example: vec4 xyz_=(x,y,z,1.0) */
  vec4 xyz_=vec4(__REPLACE_X__,__REPLACE_Y__,__REPLACE_Z__,1.0);
  vec4 devicePos_=projectionMatrix*rotationMatrix*scalingMatrix*translationMatrix*xyz_;
  gl_Position=devicePos_;devicePos_=devicePos_/devicePos_.w;
  screenPos_=vec2(devicePos_.x*deviceToScreenMatrix[0].x+deviceToScreenMatrix[3].x,devicePos_.y*deviceToScreenMatrix[1].y+deviceToScreenMatrix[3].y);
  indexF_=index_;
}
