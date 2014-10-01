#ifndef __SCIGMA_GUI_ATWPANEL_H__
#define __SCIGMA_GUI_ATWPANEL_H__

#include <map>
#include <string>
#include <AntTweakBar.h>
#include "../pythonobject.h"
#include "../events.h"
#include "glcontext.h"
#include "viewingareaevents.h"

void ScigmaGuiATWPanelGetCallbackInt(void *value, void *callbackData);
void ScigmaGuiATWPanelSetCallbackInt(const void *value, void *callbackData);
void ScigmaGuiATWPanelGetCallbackDouble(void *value, void *callbackData);
void ScigmaGuiATWPanelSetCallbackDouble(const void *value, void *callbackData);
void ScigmaGuiATWPanelGetCallbackFloat(void *value, void *callbackData);
void ScigmaGuiATWPanelSetCallbackFloat(const void *value, void *callbackData);
void ScigmaGuiATWPanelGetCallbackString(void *value, void *callbackData);
void ScigmaGuiATWPanelSetCallbackString(const void *value, void *callbackData);
void ScigmaGuiATWPanelGetCallbackColor(void *value, void *callbackData);
void ScigmaGuiATWPanelSetCallbackColor(const void *value, void *callbackData);
void ScigmaGuiATWPanelGetCallbackDirection(void *value, void *callbackData);
void ScigmaGuiATWPanelSetCallbackDirection(const void *value, void *callbackData);
void ScigmaGuiATWPanelCallbackButton(void *callbackData);

namespace scigma
{
  namespace gui
  {
    struct EntryChangeEvent
    { 
      typedef LOKI_TYPELIST_2(const char*, const void*) Arguments;};
    
    struct ButtonEvent
    { 
      typedef LOKI_TYPELIST_1(const char*) Arguments;};

    class ATWPanel;
    
    template <class T> class ATWPanelCallbackData
    {
    public:
    ATWPanelCallbackData(ATWPanel* p,std::string id,T* ptr, bool f):panel(p),identifier(id),storage(ptr),forward(f)
      {}
      ATWPanel* panel;
      std::string identifier;
      T* storage;
      bool forward;
      char padding[7];
    };
    
    class ATWPanel:public PythonObject<ATWPanel>,
      public EventSource<EntryChangeEvent>::Type,
      public EventSource<ButtonEvent>::Type,
      public EventSink<ResizeEvent>::Type
	{
	  friend void ::ScigmaGuiATWPanelSetCallbackInt(const void *value, void *callbackData);
	  friend void ::ScigmaGuiATWPanelSetCallbackFloat(const void *value, void *callbackData);
	  friend void ::ScigmaGuiATWPanelSetCallbackDouble(const void *value, void *callbackData);
	  friend void ::ScigmaGuiATWPanelSetCallbackString(const void *value, void *callbackData);
	  friend void ::ScigmaGuiATWPanelSetCallbackColor(const void *value, void *callbackData);
	  friend void ::ScigmaGuiATWPanelSetCallbackDirection(const void *value, void *callbackData);

	  friend void ::ScigmaGuiATWPanelGetCallbackInt(void *value, void *callbackData);
	  friend void ::ScigmaGuiATWPanelGetCallbackFloat(void *value, void *callbackData);
	  friend void ::ScigmaGuiATWPanelGetCallbackDouble(void *value, void *callbackData);
	  friend void ::ScigmaGuiATWPanelGetCallbackString(void *value, void *callbackData);
	  friend void ::ScigmaGuiATWPanelGetCallbackColor(void *value, void *callbackData);
	  friend void ::ScigmaGuiATWPanelGetCallbackDirection(void *value, void *callbackData);

	  friend void ::ScigmaGuiATWPanelCallbackButton(void *callbackData);
	
      public:
	
	ATWPanel(GLContext* context, const char* name);
	virtual ~ATWPanel();
	/* int the following, forward=true means that if a new value is entered in a panel, it is
	   immediately written to the storage of the displayed value, before invoking the callback
	   routine. Use forward=false if the callback routine might revoke the change for some reason
	   (sanity checks, etc.). In this case, it is the callback's responsibility to adjust the
	   displayed value.
	*/
	void add_external_bool(const char* identifier, int* storage, bool forward=true, const char* defs =NULL);
	void add_external_int(const char* identifier, int* storage, bool forward=true, const char* defs =NULL);
	void add_external_float(const char* identifier, GLfloat* storage, bool forward=true, const char* defs =NULL);
	void add_external_double(const char* identifier, double* storage, bool forward=true, const char* defs =NULL);
	void add_external_string(const char* identifier, std::string* storage, bool forward=true, const char* defs =NULL);
	void add_external_color(const char* identifier, GLfloat* storage, bool forward=true, const char* defs =NULL);
	void add_external_direction(const char* identifier, GLfloat* storage, bool forward=true, const char* defs =NULL);
	void add_external_enum(const char* identifier, int* storage, std::string labels, int* values, bool forward=true, const char* defs =NULL);

	void add_bool(const char* identifier, bool forward=true, const char* defs =NULL);
	void add_int(const char* identifier, bool forward=true, const char* defs =NULL);
	void add_float(const char* identifier, bool forward=true, const char* defs =NULL);
	void add_double(const char* identifier, bool forward=true, const char* defs =NULL);
	void add_string(const char* identifier, bool forward=true, const char* defs =NULL);
	void add_color(const char* identifier, bool forward=true, const char* defs =NULL);
	void add_direction(const char* identifier, bool forward=true, const char* defs =NULL);
	void add_enum(const char* identifier, std::string labels, int* values, bool forward=true, const char* defs =NULL);
	void add_button(const char* identifier, const char* defs=NULL);

	void set_bool(const char* identifier, int value);
        void set_int(const char* identifier, int value);
	void set_float(const char* identifier, GLfloat value);
	void set_double(const char* identifier, double value);
	void set_string(const char* identifier, std::string value);
	void set_color(const char* identifier, GLfloat* values);
	void set_direction(const char* identifier, GLfloat* values);
	void set_enum(const char* identifier, int value);

	void remove_external(const char* identifier);
	void remove(const char* identifier);
	
	void define(const char* defs);
	void define(const char* identifier, const char* defs);
	void add_separator(const char* identifier, const char* defs);
 
	void set_param(const char* identifier, const char* parameter, TwParamValueType type, unsigned int count, const void* values);
	void get_param(const char* identifier, const char* parameter, TwParamValueType type, unsigned int count, void* values);
	
	virtual bool process(ResizeEvent event, int x, int y);

      private:
	TwBar* bar_;
	GLContext* glContext_;
	
	std::string name_;
	int wWindowOld_,hWindowOld_;
	int size_[2];
	int pos_[2];

	void sort_into_group(const char* identifier);

	typedef ATWPanelCallbackData<int> Int;
	typedef ATWPanelCallbackData<GLfloat> Float;
	typedef ATWPanelCallbackData<double> Double;
	typedef ATWPanelCallbackData<std::string> String;

	std::map<std::string,Int*> intData_;
	std::map<std::string,Float*> floatData_;
	std::map<std::string,Double*> doubleData_;
	std::map<std::string,String*> stringData_;

	std::vector<std::string*> enumLabels_;
      };
    
  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_ATWPANEL_H__ */
