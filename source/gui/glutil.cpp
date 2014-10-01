#include <png.h>
#include "glutil.h"
#include "glcontext.h"

namespace scigma
{
  namespace gui
  {
    
    GLContextStaller::GLContextStaller(GLContext* glContext):glContext_(glContext)
    {
      glContext_->stall();
    }
    
    GLContextStaller::~GLContextStaller()
    {
      glContext_->flush();
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"    
#pragma clang diagnostic ignored "-Wold-style-cast"    
    
    void my_read_function(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead)
    {
      unsigned char** ptr = (unsigned char**)(png_get_io_ptr(png_ptr));
      for(png_size_t i(0);i<byteCountToRead;++i)
	((unsigned char*)outBytes)[i]=(*ptr)[i];
      (*ptr)+=byteCountToRead;
    }
    
    GLuint get_texture_from_png_data(unsigned char* data, unsigned int* width, unsigned int* height)
    {
      unsigned char* ptr=data;
      
      if (png_sig_cmp(data, 0, 8))
	return 0;
      ptr+=8;
      
      png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
      if (!png_ptr)
	return 0;
      
      // create png info struct
      png_infop info_ptr = png_create_info_struct(png_ptr);
      if (!info_ptr)
	{
	  png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
	  return 0;
	}
      
      // create png info struct
      png_infop end_info = png_create_info_struct(png_ptr);
      if (!end_info)
	{
	  png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
	  return 0;
	}
      
      // the code in this if statement gets called if libpng encounters an error
      if (setjmp(png_jmpbuf(png_ptr))) {
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	return 0;
      }
      
      png_set_read_fn(png_ptr, (void*)(&(ptr)), my_read_function);
      
      // let libpng know you already read the first 8 bytes
      png_set_sig_bytes(png_ptr, 8);
      
      // read all the info up to the image data
      png_read_info(png_ptr, info_ptr);
      
      // variables to pass to get info
      int bit_depth, color_type;
      png_uint_32 temp_width, temp_height;
      
      // get info about png
      png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
		   NULL, NULL, NULL);
      
      if (width){ *width = temp_width; }
      if (height){ *height = temp_height; }
      
      //printf("%s: %lux%lu %d\n", file_name, temp_width, temp_height, color_type);
      
      if (bit_depth != 8)
	return 0;
      
      GLint format;
      switch(color_type)
	{
	case PNG_COLOR_TYPE_RGB:
	  format = GL_RGB;
	  break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
	  format = GL_RGBA;
	  break;
	default:
	  return 0;
	}
      
      // Update the png info struct.
      png_read_update_info(png_ptr, info_ptr);
      
      // Row size in bytes.
      png_size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);
      
      // glTexImage2d requires rows to be 4-byte aligned
      rowbytes += 3 - ((rowbytes-1) % 4);
      
      // Allocate the image_data as a big block, to be given to opengl
      png_byte * image_data = (png_byte *)malloc(rowbytes * temp_height * sizeof(png_byte)+15);
      if (image_data == NULL)
	{
	  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	  return 0;
	}
      
      // row_pointers is for pointing to image_data for reading the png with libpng
      png_byte ** row_pointers = (png_byte **)malloc(temp_height * sizeof(png_byte *));
      if (row_pointers == NULL)
	{
	  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	  free(image_data);
	  return 0;
	}
      
      // set the individual row_pointers to point at the correct offsets of image_data
      for (unsigned int i = 0; i < temp_height; i++)
	{
	  row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
	}
      
      // read the png into image_data through row_pointers
      png_read_image(png_ptr, row_pointers);
      
      // Generate the OpenGL texture object
      GLuint texture;
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexImage2D(GL_TEXTURE_2D, 0, format, GLsizei(temp_width), GLsizei(temp_height), 0,GLenum(format), GL_UNSIGNED_BYTE, image_data);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      
      push_opengl_error(__FILE__,__LINE__);
      
      // clean up
      png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
      free(image_data);
      free(row_pointers);
      return texture;
    }

#pragma clang diagnostic pop

  } /* end namespace gui */
} /* end namespace scigma */
