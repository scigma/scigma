#include "../definitions.hpp"
#include "../application.hpp"
#include "../glwindow.hpp"
#include "../cosy.hpp"
#include "../navigator.hpp"
#include "../bundle.hpp"
#include "../sheet.hpp"
#include "../../common/events.hpp"
#include "../../dat/wave.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <cmath>

using namespace scigma::gui;
using scigma::common::EventSource;
using scigma::common::connect;
typedef scigma::dat::AbstractWave<double> Wave;

int main()
{
  GLWindow w;
  Cosy c(&w,X_COORDINATE|Y_COORDINATE|Z_COORDINATE,false);
  Navigator n(X_COORDINATE|Y_COORDINATE|Z_COORDINATE);
  connect<MouseButtonEvent>(&w,&n);
  connect<ScrollEvent>(&w,&n);

  //#include "bundletest.hpp"
  #include "sheettest.hpp"
  
  return 0;
}
