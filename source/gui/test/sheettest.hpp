std::vector<double> initial(3,0);
initial[2]=1;
for(size_t i(0);i<10;++i)
  {
    initial.push_back(std::cos(i/5.*3.1415)*std::sin(0.1));
    initial.push_back(std::sin(i/5.*3.1415)*std::sin(0.1));
    initial.push_back(std::cos(0.1));
  }
Mesh m(3,initial);

for(double theta(0.2);theta <3.1415;theta+=0.05)
  {
    std::vector<double> layer;
    for(double phi(0);phi<6.283;phi+=0.02/sin(theta))
      {
	layer.push_back(std::cos(phi)*sin(theta));
	layer.push_back(std::sin(phi)*std::sin(theta));
	layer.push_back(std::cos(theta));
      }
    m.add_strip(layer);
  }

Wave constWave(1);
constWave.push_back(3.1415);

Sheet s(&w,&m,3,&constWave);

std::vector<int> indices;
indices.push_back(1);
indices.push_back(2);
indices.push_back(3);
indices.push_back(-1);

s.set_attributes_for_view(indices);

std::vector<std::string> ind;
std::vector<std::string> exp;
  
ind.push_back("x");
ind.push_back("y");
ind.push_back("z");
ind.push_back("pi");

/*exp.push_back("x");
exp.push_back("y");
exp.push_back("z");*/

/*exp.push_back("sqrt(sqrt(abs(x)))*sign(x)");
exp.push_back("sqrt(sqrt(abs(y)))*sign(y)");
exp.push_back("sqrt(sqrt(abs(z)))*sign(z)");*/

exp.push_back("x*x*x");
exp.push_back("y*y*y");
exp.push_back("z*z*z");

/*  exp.push_back("cos(2*pi*v)*sqrt(1-pow(u-1,2))");
    exp.push_back("sin(2*pi*v)*sqrt(1-pow(u-1,2))");*/

//exp.push_back("pow(u,2)+pow(v,2)");
exp.push_back("x*x+y*y+z*z");
exp.push_back("0");

s.adjust_shaders_for_view(ind,exp,0);

w.gl_context()->add_drawable(&s);

/*for(size_t i(0);i<m.iso_indices().size();++i)
  std::cout<<m.iso_indices().data()[i]<<",";
std::cout<<std::endl;
for(size_t i(0);i<m.iso_end_points().size();++i)
  std::cout<<(int)m.iso_end_points().data()[i]<<",";
std::cout<<std::endl;
GLfloat color[]={1,0,0,0.5};
s.set_color(color);
s.set_style(Graph::ISOLINES);
w.gl_context()->request_redraw();;
Application::get_instance()->loop(10);
s.set_style(Graph::POINTS);
w.gl_context()->request_redraw();
Application::get_instance()->loop(10);
s.set_style(Graph::WIREFRAME);
w.gl_context()->request_redraw();
Application::get_instance()->loop(10);
s.set_style(Graph::SOLID);
w.gl_context()->request_redraw();
Application::get_instance()->loop(10);
*/
  //  b.set_style(Graph::ISOLINES);
/*  b.set_style(Graph::POINTS);
  w.gl_context()->request_redraw();
  Application::get_instance()->loop(5);
  b.set_style(Graph::ISOLINES);
  w.gl_context()->request_redraw();
  Application::get_instance()->loop(5);
  b.set_style(Graph::LINES);
  w.gl_context()->request_redraw();
  Application::get_instance()->loop(50);*/

GLfloat color[]={1,0,0,0.5};
s.set_color(color);
s.set_style(Graph::SOLID);

s.set_delay(0.25);
s.replay();

for(double e(0.001);e<5;e*=1.02)
  {
    std::vector<std::string> ex;
    std::stringstream ss;
    ss<<e;
    ex.push_back("pow(abs(x),"+ss.str()+")*sign(x)");
    ex.push_back("pow(abs(y),"+ss.str()+")*sign(y)");
    ex.push_back("pow(abs(z),"+ss.str()+")*sign(z)");
    ex.push_back("z");
    ex.push_back("0");
    s.adjust_shaders_for_view(ind,ex,0);
    w.gl_context()->request_redraw();
    Application::get_instance()->loop(0.05);
  }
 Application::get_instance()->loop(10);
