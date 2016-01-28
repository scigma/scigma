#ifndef __SCIGMAEVENTS_H__
#define __SCIGMAEVENTS_H__

#include <vector>
#include <algorithm>
#include <Typelist.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"

namespace scigma
{
    
  template <class T> struct Event
		     {
		       typedef LOKI_TYPELIST_0 ArgumentList;
		       //or, for one argument:
		       //typedef LOKI_TYPELIST_1(ArgumentType1) ArgumentList;
		       //or, for two arguments:
		       //typedef LOKI_TYPELIST_0(ArgumentType1, ArgumentType2) ArgumentList;
		       //...
  };
  
  template <class Event, unsigned int nArguments> class EventSinkWithNArguments;
  
  template <class Event> class EventSinkWithNArguments<Event,0>
    {
    public:
      virtual bool process(Event event)=0;
    };
  
  template <class Event> class EventSinkWithNArguments<Event,1>
    {
    public:
      virtual bool process(Event event,
			   typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1)=0;
    };
  
  template <class Event> class EventSinkWithNArguments<Event,2>
    {
    public:
      virtual bool process(Event event,
			   typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1,
			   typename Loki::TL::TypeAt<typename Event::Arguments,1>::Result arg2)=0;
    };
  
  template <class Event> class EventSinkWithNArguments<Event,3>
    {
    public:
      virtual bool process(Event event,
			   typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1,
			   typename Loki::TL::TypeAt<typename Event::Arguments,1>::Result arg2,
			   typename Loki::TL::TypeAt<typename Event::Arguments,2>::Result arg3)=0;
    };
  
  template <class Event> class EventSinkWithNArguments<Event,4>
    {
    public:
      virtual bool process(Event event,
			   typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1,
			   typename Loki::TL::TypeAt<typename Event::Arguments,1>::Result arg2,
			   typename Loki::TL::TypeAt<typename Event::Arguments,2>::Result arg3,
			   typename Loki::TL::TypeAt<typename Event::Arguments,3>::Result arg4)=0;
    };
  
  template <class Event> class EventSinkWithNArguments<Event,5>
    {
    public:
      virtual bool process(Event event,
			   typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1,
			   typename Loki::TL::TypeAt<typename Event::Arguments,1>::Result arg2,
			   typename Loki::TL::TypeAt<typename Event::Arguments,2>::Result arg3,
			   typename Loki::TL::TypeAt<typename Event::Arguments,3>::Result arg4,
			   typename Loki::TL::TypeAt<typename Event::Arguments,4>::Result arg5)=0;
    };
  
  //!Base class wrapper EventSink
  /*!@ingroup gui
    This struct consists only of a typedef (namely Type), which is the specialized base class
    for EventSink objects that monitor events of type Event.
    @tparam Event 
   */
  template <class Event> struct EventSink
			 {
			   /*! sdgdg */
			   typedef EventSinkWithNArguments<Event,Loki::TL::Length<typename Event::Arguments>::value> Type;
  };
  
  template <class Event, unsigned int nArguments> class EventSourceWithNArguments; 
  
  template <class Event> class EventSourceWithNArguments<Event,0>
    {
    protected:
      std::vector<typename EventSink<Event>::Type*> sinks;
    public:
      void connect_before(typename EventSink<Event>::Type* sink){sinks.insert(sinks.begin(),sink);}
      void connect(typename EventSink<Event>::Type* sink){sinks.push_back(sink);}
      bool disconnect(typename EventSink<Event>::Type* sink){
	typename std::vector<typename EventSink<Event>::Type*>::iterator  position = std::find(sinks.begin(),sinks.end(),sink);
    if(sinks.end()!=position){sinks.erase(position);return true;}else{return false;}}
    protected:
      void emit()
      {
	for(typename std::vector<typename EventSink<Event>::Type *>::iterator i=sinks.begin(),end=sinks.end();i!=end;++i)
	  if((*i)->process(Event()))
	    return;
      }
    };
  
  template <class Event> class EventSourceWithNArguments<Event,1>
    {
    protected:
      std::vector<typename EventSink<Event>::Type*> sinks;
    public:
      void connect_before(typename EventSink<Event>::Type* sink){sinks.insert(sinks.begin(),sink);}
      void connect(typename EventSink<Event>::Type* sink){sinks.push_back(sink);}
      bool disconnect(typename EventSink<Event>::Type* sink){
	typename std::vector<typename EventSink<Event>::Type*>::iterator  position = std::find(sinks.begin(),sinks.end(),sink);
    if(sinks.end()!=position){sinks.erase(position);return true;}else{return false;}}
    protected: 
      void emit(typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1)
      {
	for(typename std::vector<typename EventSink<Event>::Type *>::iterator i=sinks.begin(),end=sinks.end();i!=end;++i)
	  if((*i)->process(Event(), arg1))
	    return;
      }
    };
  
  template <class Event> class EventSourceWithNArguments<Event,2>
    {
    protected:
      std::vector<typename EventSink<Event>::Type*> sinks;
    public:
      void connect_before(typename EventSink<Event>::Type* sink){sinks.insert(sinks.begin(),sink);}
      void connect(typename EventSink<Event>::Type* sink){sinks.push_back(sink);}
      bool disconnect(typename EventSink<Event>::Type* sink){
	typename std::vector<typename EventSink<Event>::Type*>::iterator  position = std::find(sinks.begin(),sinks.end(),sink);
    if(sinks.end()!=position){sinks.erase(position);return true;}else{return false;}}
    protected:
      void emit(typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1,
		typename Loki::TL::TypeAt<typename Event::Arguments,1>::Result arg2)
      {
	for(typename std::vector<typename EventSink<Event>::Type *>::iterator i=sinks.begin(),end=sinks.end();i!=end;++i)
	  if((*i)->process(Event(), arg1, arg2))
	    return;
      }
    };
  
  template <class Event> class EventSourceWithNArguments<Event,3>
    {
    protected:
      std::vector<typename EventSink<Event>::Type*> sinks;
    public:
      void connect_before(typename EventSink<Event>::Type* sink){sinks.insert(sinks.begin(),sink);}
      void connect(typename EventSink<Event>::Type* sink){sinks.push_back(sink);}
      bool disconnect(typename EventSink<Event>::Type* sink){
	typename std::vector<typename EventSink<Event>::Type*>::iterator  position = std::find(sinks.begin(),sinks.end(),sink);
    if(sinks.end()!=position){sinks.erase(position);return true;}else{return false;}}
    protected:
      void emit(typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1,
		typename Loki::TL::TypeAt<typename Event::Arguments,1>::Result arg2,
		typename Loki::TL::TypeAt<typename Event::Arguments,2>::Result arg3)
      {
	for(typename std::vector<typename EventSink<Event>::Type *>::iterator i=sinks.begin(),end=sinks.end();i!=end;++i)
	  if((*i)->process(Event(), arg1, arg2, arg3))
	    return;
      }
    };
  
  template <class Event> class EventSourceWithNArguments<Event,4>
    {
    protected:
      std::vector<typename EventSink<Event>::Type*> sinks;
    public:
      void connect_before(typename EventSink<Event>::Type* sink){sinks.insert(sinks.begin(),sink);}
      void connect(typename EventSink<Event>::Type* sink){sinks.push_back(sink);}
      bool disconnect(typename EventSink<Event>::Type* sink){
	typename std::vector<typename EventSink<Event>::Type*>::iterator  position = std::find(sinks.begin(),sinks.end(),sink);
    if(sinks.end()!=position){sinks.erase(position);return true;}else{return false;}}
    protected:
      void emit(typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1,
		typename Loki::TL::TypeAt<typename Event::Arguments,1>::Result arg2,
		typename Loki::TL::TypeAt<typename Event::Arguments,2>::Result arg3,
		typename Loki::TL::TypeAt<typename Event::Arguments,3>::Result arg4)
      {
	for(typename std::vector<typename EventSink<Event>::Type *>::iterator i=sinks.begin(),end=sinks.end();i!=end;++i)
	  if((*i)->process(Event(), arg1, arg2, arg3, arg4))
	    return;
      }
    };
  
  template <class Event> class EventSourceWithNArguments<Event,5>
    {
    protected:
      std::vector<typename EventSink<Event>::Type*> sinks;
    public:
      void connect_before(typename EventSink<Event>::Type* sink){sinks.insert(sinks.begin(),sink);}
      void connect(typename EventSink<Event>::Type* sink){sinks.push_back(sink);}
      bool disconnect(typename EventSink<Event>::Type* sink){
	typename std::vector<typename EventSink<Event>::Type*>::iterator  position = std::find(sinks.begin(),sinks.end(),sink);
    if(sinks.end()!=position){sinks.erase(position);return true;}else{return false;}}
    protected:
      void emit(typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1,
		typename Loki::TL::TypeAt<typename Event::Arguments,1>::Result arg2,
		typename Loki::TL::TypeAt<typename Event::Arguments,2>::Result arg3,
		typename Loki::TL::TypeAt<typename Event::Arguments,3>::Result arg4,
		typename Loki::TL::TypeAt<typename Event::Arguments,4>::Result arg5)
      {
	for(typename std::vector<typename EventSink<Event>::Type *>::iterator i=sinks.begin(),end=sinks.end();i!=end;++i)
	  if((*i)->process(Event(), arg1, arg2, arg3, arg4, arg5))
	    return;
      }
    };
  
  template <class Event> struct EventSource
			 {
			   typedef EventSourceWithNArguments<Event,Loki::TL::Length<typename Event::Arguments>::value> Type;
  };
  
} /* end namespace scigma */ 

#pragma clang diagnostic pop
  
#endif /* __SCIGMAEVENTS_H__ */
