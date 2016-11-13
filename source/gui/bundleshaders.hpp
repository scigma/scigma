#include <string>

namespace scigma
{
  namespace gui
  {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"

    const std::string Bundle::vertexShaderGL2_=
      "/* number of points in one ray/layer that are drawn */\n"
      "uniform int nDrawn_;\n"
      "/* total number of points in one ray/layer */\n"
      "uniform int nTotal_;\n"
      "/* index of currently processed vertex, used for picking;\n"
      "   could be replaced by gl_VertexID, which is not available\n"
      "   on all platforms */\n"
      "attribute float index_;\n"
      "/* attributes for varying values go here; these attributes\n"
      "   depend on the view and must be injected whenever the\n"
      "   view changes; example:\n"
      "   attribute float t;\n"
      "   attribute float x;\n"
      "   attribute float y;\n"
      "   attribute float z;\n"
      "   instead of _REPLACE_ATTRIBUTES_\n"
      "*/\n"
      "__REPLACE_ATTRIBUTES__\n"
      "\n"
      "varying vec2 screenPos_;\n"
      "varying float indexF_;\n"
      "varying float t_;\n"
      "\n"
      "/* if the colormap is not used, use comments to block the\n"
      "   outgoing attribute, else just replace _REPLACE_COLOR_START_\n"
      "   and _REPLACE_COLOR_END_ with empty strings\n"
      "*/\n"
      "__REPLACE_COLOR_START__\n"
      "varying vec4 rgba_;\n"
      "\n"
      "vec4 colormap_(in float p)\n"
      "{\n"
      "  vec4 color;\n"
      "  if(p<-1.0||p>1.0)\n"
      "    color= vec4(1,0.0,1,1.0);\n"
      "  else if (p<0.0)\n"
      "    color= mix(vec4(0,0,1.0,1.0),vec4(0.0,1.0,0,1.0),1-p);\n"
      "  else\n"
      "    color= mix(vec4(0,1.0,0.0,1.0),vec4(1.0,0.0,0.0,1.0),p);\n"
      "  return color;\n"
      "}\n"
      "__REPLACE_COLOR_END__\n"
      "\n"
      "void main()\n"
      "{\n"
      "  /* time coordinate goes here; example: t_=t; */\n"
      "  t_=__REPLACE_TIME__;\n"
      "\n"
      "  /* if the colormap is not used, use comments to block the\n"
      "   outgoing attribute, else replace _REPLACE_COLOR_START_\n"
      "   and _REPLACE_COLOR_END_ with empty strings and set\n"
      "   _REPLACE_COLOR_ to the value used as color expression\n"
      "  */\n"
      "  __REPLACE_COLOR_START__\n"
      "  float p_=__REPLACE_COLOR__;\n"
      "  rgba_=colormap_(p_);\n"
      "  __REPLACE_COLOR_END__\n"
      "  \n"
      "  /* coordinates in viewing space go here; example: vec4 xyz_=(x,y,z,1.0) */\n"
      "  vec4 xyz_=vec4(__REPLACE_X__,__REPLACE_Y__,__REPLACE_Z__,1.0);\n"
      "  vec4 devicePos_=projectionMatrix*rotationMatrix*scalingMatrix*translationMatrix*xyz_;\n"
      "  gl_Position=devicePos_;devicePos_=devicePos_/devicePos_.w;\n"
      "  screenPos_=vec2(devicePos_.x*deviceToScreenMatrix[0].x+deviceToScreenMatrix[3].x,devicePos_.y*deviceToScreenMatrix[1].y+deviceToScreenMatrix[3].y);\n"
      "  indexF_=index_;\n"
      "}\n";
    const std::string Bundle::vertexShaderGL3_=
      "/* number of points in one ray/layer that are drawn */\n"
      "uniform int nDrawn_;\n"
      "/* total number of points in one ray/layer */\n"
      "uniform int nTotal_;\n"
      "/* index of currently processed vertex, used for picking;\n"
      "   could be replaced by gl_VertexID, which is not available\n"
      "   on all platforms */\n"
      "in float index_;\n"
      "/* attributes for varying values go here; these attributes\n"
      "   depend on the view and must be injected whenever the \n"
      "   view changes; example:\n"
      "   in float t;\n"
      "   in float x;\n"
      "   in float y;\n"
      "   in float z;\n"
      "   instead of _REPLACE_ATTRIBUTES_\n"
      "*/\n"
      "__REPLACE_ATTRIBUTES__\n"
      "\n"
      "out vec2 screenPos_;\n"
      "out float indexF_;\n"
      "out float t_;\n"
      "\n"
      "/* if the colormap is not used, use comments to block the\n"
      "   outgoing attribute, else just replace _REPLACE_COLOR_START_\n"
      "   and _REPLACE_COLOR_END_ with empty strings\n"
      "*/\n"
      "__REPLACE_COLOR_START__\n"
      "out vec4 rgba_;\n"
      "\n"
      "vec4 colormap_(in float p)\n"
      "{\n"
      "  vec4 color;\n"
      "  if(p<-1.0||p>1.0)\n"
      "    color= vec4(1,0.0,1,1.0);\n"
      "  else if (p<0.0)\n"
      "    color= mix(vec4(0,0,1.0,1.0),vec4(0.0,1.0,0,1.0),1-p);\n"
      "  else\n"
      "    color= mix(vec4(0,1.0,0.0,1.0),vec4(1.0,0.0,0.0,1.0),p);\n"
      "  return color;\n"
      "}\n"
      "__REPLACE_COLOR_END__\n"
      "\n"
      "void main()\n"
      "{\n"
      "  /* time coordinate goes here; example: t_=t; */\n"
      "  t_=__REPLACE_TIME__;\n"
      "\n"
      "  /* if the colormap is not used, use comments to block the\n"
      "   outgoing attribute, else replace _REPLACE_COLOR_START_\n"
      "   and _REPLACE_COLOR_END_ with empty strings and set\n"
      "   _REPLACE_COLOR_ to the value used as color expression \n"
      "  */\n"
      "  __REPLACE_COLOR_START__\n"
      "  float p_=__REPLACE_COLOR__;\n"
      "  rgba_=colormap_(p_);\n"
      "  __REPLACE_COLOR_END__\n"
      "  \n"
      "  /* coordinates in viewing space go here; example: vec4 xyz_=(x,y,z,1.0) */\n"
      "  vec4 xyz_=vec4(__REPLACE_X__,__REPLACE_Y__,__REPLACE_Z__,1.0);\n"
      "  vec4 devicePos_=projectionMatrix*rotationMatrix*scalingMatrix*translationMatrix*xyz_;\n"
      "  gl_Position=devicePos_;devicePos_=devicePos_/devicePos_.w;\n"
      "  screenPos_=vec2(devicePos_.x*deviceToScreenMatrix[0].x+deviceToScreenMatrix[3].x,devicePos_.y*deviceToScreenMatrix[1].y+deviceToScreenMatrix[3].y);\n"
      "  indexF_=index_;\n"
      "}\n";
    const std::string Bundle::fragmentShaderGL2_=
      "/* number of points in one ray/layer that are drawn */\n"
      "uniform int nDrawn_;\n"
      "/* total number of points in one ray/layer */\n"
      "uniform int nTotal_;\n"
      "/* sprite_==0: no sprite; sprite_==1: regular marker; sprite==2: hovering */\n"
      "uniform int sprite_;\n"
      "/* size of marker */\n"
      "uniform float size_;\n"
      "/* display bundle lighter if hovering over it*/\n"
      "uniform int lighter_;\n"
      "/* texture of the currently used sprite */\n"
      "uniform sampler2D sampler_;\n"
      "\n"
      "/* if the colormap is used, use comments to block the\n"
      "   uniform, else replace _REPLACE_COLOR_UNIFORM_START_\n"
      "   and _REPLACE_COLOR_UNIFORM_END_ with empty strings \n"
      "*/\n"
      "__REPLACE_COLOR_UNIFORM_START__\n"
      "uniform vec4 rgba_;\n"
      "__REPLACE_COLOR_UNIFORM_END__\n"
      "\n"
      "\n"
      "varying vec2 screenPos_;\n"
      "varying float indexF_;\n"
      "\n"
      "/* if the colormap is not used, use comments to block the\n"
      "   incoming attribute, else replace _REPLACE_COLOR_IN_START_\n"
      "   and _REPLACE_COLOR_IN_END_ with empty strings \n"
      "*/\n"
      "__REPLACE_COLOR_IN_START__\n"
      "varying vec4 rgba_;\n"
      "__REPLACE_COLOR_IN_END__\n"
      "\n"
      "varying float t_;\n"
      "\n"
      "void main()\n"
      "{\n"
      "  if(t_<-1.0||t_>1.0)\n"
      "    discard;\n"
      "  /* do not draw connection between different rays/layers when drawing lines*/\n" 
      "  if((sprite_==0)&&(mod(indexF_,nTotal_)>=nDrawn_-1))\n"
      "    discard;\n"
      "  vec4 rgba2_=rgba_;\n"
      "  if(sprite_==2) // do not draw hover sprite in hover picking mode (uniqueID.z==2)\n"
      "    {\n"
      "      if(1.5<uniqueID.z)\n"
      "	discard;\n"
      "      else\n"
      "	rgba2_=foregroundColor;\n"
      "    }\n"
      "  if(sprite_!=0) // draw sprites\n"
      "    {\n"
      "      if(size_>2)\n"
      "	{\n"
      "	  vec2 pointCoord_=(screenPos_-gl_FragCoord.xy)/size_+vec2(0.5,0.5);\n"
      "	  rgba2_=texture2D(sampler_,pointCoord_).r*rgba2_;\n"
      "	  if(rgba2_.a==0)discard;\n"
      "	}\n"
      "    }\n"
      "  if(lighter_==1)\n"
      "    rgba2_=rgba2_+(1-rgba2_)*0.5;\n"
      "  if(0.5>uniqueID.z) // normal coloring (uniqueID.z==0)\n"
      "    {\n"
      "      gl_FragColor=rgba2_;\n"
      "    }\n"
      "  else if(1.5>uniqueID.z) // coloring for hover check (uniqueID.z==1)\n"
      "    {\n"
      "      gl_FragColor=vec4(uniqueID.xy,gl_FragCoord.z,1);\n"
      "    }\n"
      "  else // color if we are actually hovering (picking of individual points (uniqueID.z==2)\n"
      "    {\n"
      "      gl_FragColor=vec4(mod(int(indexF_/(256*256)),256)/256.0,mod(int(indexF_/256),256)/256.0,(mod(indexF_,256)+0.5)/256.0,1.0);\n"
      "    }\n"
      "}\n";
    const std::string Bundle::fragmentShaderGL3_=
      "/* number of points in one ray/layer that are drawn */\n"
      "uniform int nDrawn_;\n"
      "/* total number of points in one ray/layer */\n"
      "uniform int nTotal_;\n"
      "/* sprite_==0: no sprite; sprite_==1: regular marker; sprite==2: hovering */\n"
      "uniform int sprite_;\n"
      "/* size of marker */\n"
      "uniform float size_;\n"
      "/* display bundle lighter if hovering over it*/\n"
      "uniform int lighter_;\n"
      "/* texture of the currently used sprite */\n"
      "uniform sampler2D sampler_;\n"
      "\n"
      "/* if the colormap is used, use comments to block the\n"
      "   uniform, else replace _REPLACE_COLOR_UNIFORM_START_\n"
      "   and _REPLACE_COLOR_UNIFORM_END_ with empty strings \n"
      "*/\n"
      "__REPLACE_COLOR_UNIFORM_START__\n"
      "uniform vec4 rgba_;\n"
      "__REPLACE_COLOR_UNIFORM_END__\n"
      "\n"
      "\n"
      "in vec2 screenPos_;\n"
      "in float indexF_;\n"
      "\n"
      "/* if the colormap is not used, use comments to block the\n"
      "   incoming attribute, else replace _REPLACE_COLOR_IN_START_\n"
      "   and _REPLACE_COLOR_IN_END_ with empty strings \n"
      "*/\n"
      "__REPLACE_COLOR_IN_START__\n"
      "in vec4 rgba_;\n"
      "__REPLACE_COLOR_IN_END__\n"
      "\n"
      "in float t_;\n"
      "\n"
      "out vec4 color_;\n"
      "\n"
      "void main()\n"
      "{\n"
      "  if(t_<-1.0||t_>1.0)\n"
      "    discard;\n"
      "  /* do not draw connection between different rays/layers when drawing lines*/\n" 
      "  if((sprite_==0)&&(mod(indexF_,nTotal_)>=nDrawn_-1))\n"
      "    discard;\n"
      "  vec4 rgba2_=rgba_;\n"
      "  if(sprite_==2) // do not draw hover sprite in hover picking mode (uniqueID.z==2)\n"
      "    {\n"
      "      if(1.5<uniqueID.z)\n"
      "	discard;\n"
      "      else\n"
      "	rgba2_=foregroundColor;\n"
      "    }\n"
      "  if(sprite_!=0) // draw sprites\n"
      "    {\n"
      "      if(size_>2)\n"
      "	{\n"
      "	  vec2 pointCoord_=(screenPos_-gl_FragCoord.xy)/size_+vec2(0.5,0.5);\n"
      "	  rgba2_=texture(sampler_,pointCoord_).r*rgba2_;\n"
      "	  if(rgba2_.a==0)discard;\n"
      "	}\n"
      "    }\n"
      "  if(lighter_==1)\n"
      "    rgba2_=rgba2_+(1-rgba2_)*0.5;\n"
      "  if(0.5>uniqueID.z) // normal coloring (uniqueID.z==0)\n"
      "    {\n"
      "      color_=rgba2_;\n"
      "    }\n"
      "  else if(1.5>uniqueID.z) // coloring for picking of whole bundles (uniqueID.z==1)\n"
      "    {\n"
      "      color_=vec4(uniqueID.xy,gl_FragCoord.z,1);\n"
      "    }\n"
      "  else\n"
      "    {\n"
      "      color_=vec4(mod(int(indexF_/(256*256)),256)/256.0,mod(int(indexF_/256),256)/256.0,(mod(indexF_,256)+0.5)/256.0,1.0);\n"
      "    }\n"
      "}\n";

#pragma clang diagnostic pop
    
  } /* closing namespace gui */
}/* closing namespace scigma */