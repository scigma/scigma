#ifndef SCIGMA_GUI_GLUTIL_HPP
#define SCIGMA_GUI_GLUTIL_HPP

#include <sstream>
#include <iostream>
#include "application.hpp"

#define GLERR push_opengl_error(__FILE__,__LINE__)

namespace scigma
{
  namespace gui
  {

    class GLContext;
    class GLContextStaller
    {
    public:
      GLContextStaller(GLContext* glContext);
      ~GLContextStaller();
    private:
      GLContext* glContext_;
    };

    //!generate OpenGL error messages 
    /*!@ingroup gui
      This function pushes the last OpenGL error to the end of the error queue, from where it can be retrieved with the pop_error() function (after all previous errors have been retrieved).
      @param file is the name of the source file of the function where the 
      error occurred (optional).
      @param line is the line number where the error occurred (optional). 
    */
    inline void push_opengl_error(const char* file, int line)
    {
      unsigned int code;
      while ((code=glGetError())!= GL_NO_ERROR)
	{
	  std::ostringstream combine;
	  combine<<"OpenGL error code "<<std::hex<<code<<std::dec;
	  Application::get_instance()->push_error(combine.str(),file,line);
	}
    }

    //!generate GLSL compile error messages
    /*!@ingroup gui
      This function pushes the compile output of a GLSL shader to the end of the error queue, from where it can be retrieved with the pop_error() function (after all previous errors have been retrieved).
      @param shader is a previously compiled shader object
      @param file is the name of the source file of the function where the 
      error occurred (optional).
      @param line is the line number where the error occurred (optional). 
    */
    inline void push_shader_compile_error(GLuint shader, const char* file, int line)
    {
      char log[0x1000]; GLsizei length;
      std::ostringstream combine;
      glGetShaderInfoLog(shader,0x1000,&length,log);
      if(length)
	{
	  log[length]=0;
	  combine<<"shader compilation output: "<<log; 
	  Application::get_instance()->push_error(combine.str(),file,line);
	  //	  throw(combine.str());
	}
    }
    
    //!generate GLSL link error messages
    /*!@ingroup gui
      This function pushes the compile output of a GLSL program to the end of the error queue, from where it can be retrieved with the pop_error() function (after all previous errors have been retrieved).
      @param program is a previously linked program object
      @param file is the name of the source file of the function where the 
      error occurred (optional).
      @param line is the line number where the error occurred (optional). 
    */
    inline void push_program_link_error(GLuint program, const char* file, int line)
    {
      char log[0x1000]; GLsizei length;
      std::ostringstream combine;
      glGetProgramInfoLog(program,0x1000,&length,log);
      if(length)
	{
	  log[length]=0;
	  combine<<"program linking output: "<<log; 
	  Application::get_instance()->push_error(combine.str(),file,line);
	  //throw(combine.str());
	}
    }

    inline void enable_float_attribute(GLuint attributePtr, GLfloat value)
    {
      glDisableVertexAttribArray(attributePtr);
      glVertexAttrib1f(attributePtr,value);
    }

    inline void find_and_enable_float_attribute(GLuint program, const char* name, GLfloat value)
    {
      GLuint attributePtr = GLuint(glGetAttribLocation(program,name));
      glDisableVertexAttribArray(attributePtr);
      glVertexAttrib1f(attributePtr,value);
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"

    template <size_t size, GLenum Type> void enable_attribute_array(GLuint attributePtr, size_t pointer)
    {
      glEnableVertexAttribArray(attributePtr);
      glVertexAttribPointer(attributePtr, size, Type, GL_FALSE, 0, (const GLvoid*)(pointer));
    }
    
    template <int size, GLenum Type> void find_and_enable_attribute_array(GLuint program, const char* name, size_t pointer)
    {
      GLuint attributePtr = GLuint(glGetAttribLocation(program,name));
      glEnableVertexAttribArray(attributePtr);
      glVertexAttribPointer(attributePtr, size, Type, GL_FALSE, 0, (const GLvoid*)(pointer));
    }
    
    template <int size, GLenum Type> void enable_attribute_array_with_buffer(GLuint attributePtr, GLuint buffer, size_t pointer)
    {
      glBindBuffer(GL_ARRAY_BUFFER,buffer);
      glEnableVertexAttribArray(attributePtr);
      glVertexAttribPointer(attributePtr, size, Type, GL_FALSE, 0, (const GLvoid*)(pointer));
      glBindBuffer(GL_ARRAY_BUFFER,0);
    }
    
    template <int size, GLenum Type> void find_and_enable_attribute_array_with_buffer(GLuint program, const char* name, GLuint buffer, size_t pointer)
    {
      glBindBuffer(GL_ARRAY_BUFFER,buffer);
      GLuint attributePtr = GLuint(glGetAttribLocation(program,name));
      glEnableVertexAttribArray(attributePtr);
      glVertexAttribPointer(attributePtr, size, Type, GL_FALSE, 0, (const GLvoid*)(pointer));
      glBindBuffer(GL_ARRAY_BUFFER,0);
    }

#pragma clang diagnostic pop

    GLuint get_texture_from_png_data(unsigned char* data, unsigned int* width=NULL, unsigned int* height=NULL);
       
  } /* end namespace gui */
} /* end namespace scigma */


#endif /* SCIGMA_GUI_GLUTIL_HPP */
