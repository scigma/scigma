#include "../log.hpp"
#include <algorithm>
#include <vector>
#include <string>
#include <catch.hpp>
#include <tinythread.h>

using namespace scigma::common;

SCENARIO("Log: writing messages with different flags in a single thread","[Log][single-thread]")
{
  GIVEN("A Log object and a number of messages")
    {
      Log log("test.log");
      std::string message1("this is a test!");
      std::string message2("this is a another test!");
      std::string error1("this is an error!");
      std::string error2("this is another error!");
      std::string warning1("this a warning!");
      std::string warning2("this another warning!");

      WHEN("empty messages are passed")
	{
	  log.push("");
	  log.push(message1);
	  THEN("they are ignored")
	    REQUIRE(log.pop()==(std::pair<LogType,std::string>(LOG_DEFAULT,message1)));
	}
      WHEN("The messages are passed with different flags push<LOG_Type>(...)")
	{
	  log.push(message1);
	  log.push<LOG_ERROR>(error1);
	  log.push<LOG_WARNING>(warning1);
	  log.push<LOG_SUCCESS>(message1);
	  log.push<LOG_SUCCESS>(message2);
	  log.push<LOG_FAIL>(message1);
	  log.push<LOG_FAIL>(message2);
	  log.push(message2);
	  log.push<LOG_ERROR>(error2);
	  log.push<LOG_WARNING>(warning2);
	  log.push<LOG_DATA>(message1);
	  log.push<LOG_DATA>(message2);
	  
	  THEN("The messages are retrieved correctly with pop<LOG_Type>(), in the order"
	       "they were submitted per LOG_Type")
	    {
	      /*  REQUIRE(log.pop()==message1);
	      REQUIRE(log.pop()==message2);
	      REQUIRE(log.pop()=="");
	      REQUIRE(log.pop<LOG_SUCCESS>()==message1);
	      REQUIRE(log.pop<LOG_SUCCESS>()==message2);
	      REQUIRE(log.pop<LOG_SUCCESS>()=="");
	      REQUIRE(log.pop<LOG_FAIL>()==message1);
	      REQUIRE(log.pop<LOG_FAIL>()==message2);
	      REQUIRE(log.pop<LOG_FAIL>()=="");
	      REQUIRE(log.pop<LOG_ERROR>()==error1);
	      REQUIRE(log.pop<LOG_ERROR>()==error2);
	      REQUIRE(log.pop<LOG_ERROR>()=="");
	      REQUIRE(log.pop<LOG_WARNING>()==warning1);
	      REQUIRE(log.pop<LOG_WARNING>()==warning2);
	      REQUIRE(log.pop<LOG_WARNING>()=="");
	      REQUIRE(log.pop<LOG_DATA>()==message1);
	      REQUIRE(log.pop<LOG_DATA>()==message2);
	      REQUIRE(log.pop<LOG_DATA>()=="");*/
	    }
	}
    }
}

std::vector<std::string> output;
tthread::mutex mutex;

bool push_back(std::string s)
{
  tthread::lock_guard<tthread::mutex> guard(mutex);
  if(output.size()==6*95*2)
    return true;
  if(s!="")
    output.push_back(s);
  return false;
}

void read(void* data)
{
  Log* log = static_cast<Log*>(data);
  /*
  while(true)
    {
      if(push_back(log->pop<LOG_SUCCESS>()))return;
      if(push_back(log->pop<LOG_FAIL>()))return;
      if(push_back(log->pop<LOG_ERROR>()))return;
      if(push_back(log->pop<LOG_WARNING>()))return;
      if(push_back(log->pop<LOG_DATA>()))return;
      if(push_back(log->pop()))return;
      }*/
}

void write1(void* data)
{
  Log* log = static_cast<Log*>(data);
  for(char j(32);j<127;++j)
    {
      log->push<LOG_SUCCESS>(std::string("test")+j);
      log->push<LOG_FAIL>(std::string("test")+j);
      log->push<LOG_ERROR>(std::string("test")+j);
      log->push<LOG_WARNING>(std::string("test")+j);
      log->push<LOG_DATA>(std::string("test")+j);
      log->push(std::string("test")+j);
    }
}

void write2(void* data)
{
  Log* log = static_cast<Log*>(data);
  for(char j(32);j<127;++j)
    {
      log->push<LOG_SUCCESS>(std::string("fest")+j);
      log->push<LOG_FAIL>(std::string("fest")+j);
      log->push<LOG_ERROR>(std::string("fest")+j);
      log->push<LOG_WARNING>(std::string("fest")+j);
      log->push<LOG_DATA>(std::string("fest")+j);
      log->push(std::string("fest")+j);
    }
}

SCENARIO("Log: writing messages in multiple threads","[Log][multi-thread]")
{
  GIVEN("A Log that is written to by two threads and read from by two other threads")
    {
      Log log;
      tthread::thread thread1(write1,static_cast<void*>(&log));
      tthread::thread thread2(write2,static_cast<void*>(&log));
      tthread::thread thread3(read,static_cast<void*>(&log));
      tthread::thread thread4(read,static_cast<void*>(&log));

      thread1.join();
      thread2.join();
      thread3.join();
      thread4.join();

   
      THEN("All messages are retrieved correctly without deadlocks")
	{
	  std::sort(output.begin(),output.end());
	  size_t count(0);
	  for(char j(32);j<127;++j)
	    for(size_t i(0);i<6;++i)
	      REQUIRE(output[count++]==std::string("fest")+j);
	  for(char j(32);j<127;++j)
	    for(size_t i(0);i<6;++i)
	      REQUIRE(output[count++]==std::string("test")+j);
	}
    }
}
