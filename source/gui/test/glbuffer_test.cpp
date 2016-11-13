#include "../definitions.hpp"
#include "../application.hpp"
#include "../glbuffer.hpp"
#include "../../dat/wave.hpp"
#include <catch.hpp>

using scigma::gui::Application;
using scigma::gui::IdleEvent;
using scigma::common::EventSource;
using scigma::common::EventSink;

typedef scigma::dat::AbstractWave<double> Wave;
typedef scigma::dat::AbstractWave<GLuint> Mesh;

typedef scigma::gui::AbstractGLBuffer<GLfloat,double> GLfloatBuffer;
typedef scigma::gui::AbstractGLBuffer<GLuint,GLuint> GLuintBuffer;

SCENARIO("GLBuffer: single thread usage","[GLBuffer][single-thread]")
{
  Application* app(Application::get_instance());
  
  GIVEN("An instance of GLfloatBuffer/GLintBuffer initialized with an empty Wave/Mesh")
    {
      Wave w;
      Mesh m;
      GLfloatBuffer fBuffer(&w,100);
      GLuintBuffer iBuffer(&m,100);

    }
}

