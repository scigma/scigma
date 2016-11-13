  Wave varWave(2000);
  Wave constWave(1);
  
  for(size_t i=0;i<1000;++i)
    {
      varWave.push_back(double(i/10)/99.);
      varWave.push_back(double(i%10)/9.);
    }

constWave.push_back(3.1415);

Wave varWave2(0);
Wave constWave2(3);

constWave2.push_back(0.1);
constWave2.push_back(0.2);
constWave2.push_back(0.3);

Bundle b(&w,"bundle1",100,10,2,&varWave,&constWave);
Bundle bb(&w, "bundle2",1,1,0,&varWave2,&constWave2);

b.set_point_size(7);

GLfloat col[]={1,0,0,1};
GLfloat col2[]={0,1,0,1}; 
b.set_color(col);
bb.set_color(col2);

std::vector<int> indices;
indices.push_back(1);
indices.push_back(2);
indices.push_back(-1);

b.set_attributes_for_view(indices);

std::vector<int> indices2;
indices2.push_back(-1);
indices2.push_back(-2);
indices2.push_back(-3);

bb.set_attributes_for_view(indices2);

  
std::vector<std::string> ind;
std::vector<std::string> exp;
  
ind.push_back("u");
ind.push_back("v");
ind.push_back("pi");

/*exp.push_back("0.5");
    exp.push_back("0.8");
    exp.push_back("0");*/

exp.push_back("cos(2*pi*v)*sqrt(1-pow(u-1,2))");
exp.push_back("sin(2*pi*v)*sqrt(1-pow(u-1,2))");

exp.push_back("u-1");
exp.push_back("");
exp.push_back("0");


b.adjust_shaders_for_view(ind,exp,0);

w.gl_context()->add_drawable(&b);

std::vector<std::string> exp2;
exp2.push_back("u");
exp2.push_back("v");
exp2.push_back("pi");
exp2.push_back("");
exp2.push_back("0");

bb.adjust_shaders_for_view(ind,exp2,0);
w.gl_context()->add_drawable(&bb);

b.set_style(Graph::POINTS);
Application::get_instance()->loop(5);
b.set_delay(0.15);
b.replay();
Application::get_instance()->loop(5);
b.set_style(Graph::ISOLINES);
Application::get_instance()->loop(5);
b.set_style(Graph::LINES);
Application::get_instance()->loop(150);
