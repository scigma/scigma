#include "../pythonobject.hpp"
#include "../log.hpp"
#include "../blob.hpp"
#include <catch.hpp>

using scigma::common::PythonObject;
using scigma::common::Log;
using scigma::common::Blob;

SCENARIO("PythonObject","[PythonObject]")
{

  GIVEN("Three PythonObject instances of Type Blob and three Python instances of Type Log")
    {
      Blob *b1p(new Blob),*b2p(new Blob),*b3p(new Blob);
      Log l1,l2,l3;

      THEN("The objects are assigned IDs 0,1 and 2 respectively, both for Blob and Log")
	{
	  REQUIRE(l1.get_python_id()==0);
	  REQUIRE(l2.get_python_id()==1);
	  REQUIRE(l3.get_python_id()==2);
	  REQUIRE(b1p->get_python_id()==0);
	  REQUIRE(b2p->get_python_id()==1);
	  REQUIRE(b3p->get_python_id()==2);
	}
      THEN("Pointers to the original objects can be retrieved with their ID")
	{
	  REQUIRE(PythonObject<Log>::get_pointer(0)==&l1);
	  REQUIRE(PythonObject<Log>::get_pointer(1)==&l2);
	  REQUIRE(PythonObject<Log>::get_pointer(2)==&l3);
	  REQUIRE(PythonObject<Blob>::get_pointer(2)==b1p);
	  REQUIRE(PythonObject<Blob>::get_pointer(1)==b2p);
	  REQUIRE(PythonObject<Blob>::get_pointer(0)==b3p);
	}
   
      WHEN("A Blob is deleted")
	{
	  delete b2p;
	  THEN("an old ID is recycled, while a new Log gets the new ID 3")
	    {
	      Log l4;
	      b2p = new Blob;
	      REQUIRE(b2p->get_python_id()==1);
	      REQUIRE(l4.get_python_id()==3);
	    }
	}
      THEN("Trying to retrieve pointers for non-existing IDs returns a null pointer")
	{
	  REQUIRE(!PythonObject<Log>::get_pointer(-1));
	  REQUIRE(!PythonObject<Log>::get_pointer(4));
	  REQUIRE(!PythonObject<Blob>::get_pointer(-1));
	  REQUIRE(!PythonObject<Blob>::get_pointer(3));
	}
      
      delete b1p;
      delete b2p;
      delete b3p;
    }
}
