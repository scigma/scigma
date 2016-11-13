#include "../mesh.hpp"
#include <catch.hpp>
#include <tinythread.h>
#include <iostream>

using scigma::dat::Mesh;

SCENARIO("Mesh: testing Mesh functionality in a single thread","[Mesh][single-thread]")
{
  GIVEN("A Mesh object initialized with a point in 2D-space")
    {
      std::vector<double> initial(2,0);
      initial.push_back(0.5);
      initial.push_back(-0.5);
      initial.push_back(0);
      initial.push_back(0.707);
      initial.push_back(-0.5);
      initial.push_back(-0.5);
      Mesh m(2,initial);

      std::vector<double> layer;
      layer.push_back(1);
      layer.push_back(-1);
      layer.push_back(1);
      layer.push_back(0.2);
      layer.push_back(1);
      layer.push_back(1);
      layer.push_back(0.1);
      layer.push_back(1.1);
      layer.push_back(-1);
      layer.push_back(1);
      layer.push_back(-0.8);
      layer.push_back(0);
      layer.push_back(-1);
      layer.push_back(-1);
      layer.push_back(0.1);
      layer.push_back(-0.9);

      m.add_strip(layer);

      std::vector<double> layer2;
      layer2.push_back(-2.5);
      layer2.push_back(0);
      layer2.push_back(0);
      layer2.push_back(-2.5);
      layer2.push_back(2.5);
      layer2.push_back(0);
      layer2.push_back(0);
      layer2.push_back(2.5);

      m.add_strip(layer2);
      
      std::cout<<"Triangle Data:"<<std::endl;
      for(size_t i(0);i<m.triangle_data().size();++i)
	{
	  std::cout<<m.triangle_data().data()[i]<<", ";
	  if(i%4==3)
	    std::cout<<std::endl;
	}
      std::cout<<std::endl<<"Triangle Indices:"<<std::endl;
      for(size_t i(0);i<m.triangle_indices().size();++i)
	std::cout<<m.triangle_indices().data()[i]<<", ";
      std::cout<<std::endl<<"Iso Indices:"<<std::endl;
      for(size_t i(0);i<m.iso_indices().size();++i)
	std::cout<<m.iso_indices().data()[i]<<", ";
      std::cout<<std::endl<<"Iso EndPoints Indices:"<<std::endl;
      for(size_t i(0);i<m.iso_end_points().size();++i)
	std::cout<<(int)m.iso_end_points().data()[i]<<", ";

      /*THEN("size() returns 1")
	{
	  REQUIRE(w.size()==1);
	  REQUIRE(m.size()==1);
	  }*/
      /*
      const double* d(m.triangle_data().data());
      for(size_t i(0);i<m.triangle_data().size();i+=8)
	{
	  std::cerr<<d[i]<<"\t"<<d[i+4]<<std::endl<<std::endl;
	  std::cerr<<d[i+1]<<"\t"<<d[i+5]<<std::endl;
	  std::cerr<<d[i+2]<<"\t"<<d[i+6]<<std::endl;
	  std::cerr<<d[i+3]<<"\t"<<d[i+7]<<std::endl;
	  std::cerr<<d[i+1]<<"\t"<<d[i+5]<<std::endl;
	  std::cerr<<std::endl;
	}
      std::cout<<std::endl;

      std::cout<<"Available iso index:"<<std::endl;
      for(size_t i(0);i<20;++i)
	std::cout<<i<<": "<<m.available_iso_layer(i,160)<<"\t";
      std::cout<<std::endl;
      std::cout<<"Available iso data:"<<std::endl;
      for(size_t i(0);i<20;++i)
	std::cout<<i*8<<": "<<m.available_iso_layer(20,i*8)<<"\t";
      std::cout<<std::endl;
      std::cout<<"Available triangle index:"<<std::endl;
      for(size_t i(0);i<50;++i)
	std::cout<<i<<": "<<m.available_triangle_layer(i,160)<<"\t";
      std::cout<<std::endl;
      std::cout<<"Available triangle data:"<<std::endl;
      for(size_t i(0);i<20;++i)
	std::cout<<i*8<<": "<<m.available_triangle_layer(50,i*8)<<"\t";
      std::cout<<std::endl;
      std::cout<<"Max iso"<<std::endl;
      for(size_t i(0);i<6;++i)
	std::cout<<i<<": "<<m.max_for_iso_layer(i)<<"\t";
      std::cout<<std::endl;
      std::cout<<"Max triangle"<<std::endl;
      for(size_t i(0);i<6;++i)
	std::cout<<i<<": "<<m.max_for_triangle_layer(i)<<"\t";
      std::cout<<std::endl;
      */     

    }
}
