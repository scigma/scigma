#include <algorithm>
#include "../common/util.hpp"
#include "atwpanel.hpp"


using namespace scigma;
using namespace scigma::gui;

void TW_CALL ScigmaGuiATWPanelGetCallbackInt(void *value, void *callbackData)
{*(reinterpret_cast<int*>(value))=*(reinterpret_cast<ATWPanel::Int*>(callbackData)->storage);}
void TW_CALL ScigmaGuiATWPanelGetCallbackFloat(void *value, void *callbackData)
{*(reinterpret_cast<GLfloat*>(value))=*(reinterpret_cast<ATWPanel::Float*>(callbackData)->storage);}
void TW_CALL ScigmaGuiATWPanelGetCallbackDouble(void *value, void *callbackData)
{*(reinterpret_cast<double*>(value))=*(reinterpret_cast<ATWPanel::Double*>(callbackData)->storage);}
void TW_CALL ScigmaGuiATWPanelGetCallbackString(void *value, void *callbackData)
{TwCopyStdStringToLibrary(*(reinterpret_cast<std::string*>(value)), *(reinterpret_cast<ATWPanel::String*>(callbackData)->storage));}
void TW_CALL ScigmaGuiATWPanelGetCallbackColor(void *value, void *callbackData)
{
  GLfloat* source(reinterpret_cast<ATWPanel::Float*>(callbackData)->storage);
  GLfloat* destination(reinterpret_cast<GLfloat*>(value));
  for(int i(0);i<4;++i)
    destination[i]=source[i];
}
void TW_CALL ScigmaGuiATWPanelGetCallbackDirection(void *value, void *callbackData)
{
  GLfloat* source(reinterpret_cast<ATWPanel::Float*>(callbackData)->storage);
  GLfloat* destination(reinterpret_cast<GLfloat*>(value));
  for(int i(0);i<3;++i)
    destination[i]=source[i];
}

void TW_CALL ScigmaGuiATWPanelSetCallbackInt(const void *value, void *callbackData)
{
  ATWPanel::Int* cbData(reinterpret_cast<ATWPanel::Int*>(callbackData));
  if(cbData->forward)
    *(cbData->storage)=*reinterpret_cast<const int*>(value);
  cbData->panel->EventSource<EntryChangeEvent>::Type::emit(cbData->identifier.c_str(),value);
}
void TW_CALL ScigmaGuiATWPanelSetCallbackFloat(const void *value, void *callbackData)
{
  ATWPanel::Float* cbData(reinterpret_cast<ATWPanel::Float*>(callbackData));
  if(cbData->forward)
    *(cbData->storage)=*reinterpret_cast<const GLfloat*>(value);
  cbData->panel->EventSource<EntryChangeEvent>::Type::emit(cbData->identifier.c_str(),value);
}
void TW_CALL ScigmaGuiATWPanelSetCallbackDouble(const void *value, void *callbackData)
{
  ATWPanel::Double* cbData(reinterpret_cast<ATWPanel::Double*>(callbackData));
  if(cbData->forward)
    *(cbData->storage)=*reinterpret_cast<const double*>(value);
  cbData->panel->EventSource<EntryChangeEvent>::Type::emit(cbData->identifier.c_str(),value);
}
void TW_CALL ScigmaGuiATWPanelSetCallbackString(const void *value, void *callbackData)
{
  ATWPanel::String* cbData(reinterpret_cast<ATWPanel::String*>(callbackData));
  const std::string* s=reinterpret_cast<const std::string*>(value);
  if(cbData->forward)
    *(cbData->storage)=*s;
  cbData->panel->EventSource<EntryChangeEvent>::Type::emit(cbData->identifier.c_str(),reinterpret_cast<const void*>(s->c_str()));
}
void TW_CALL ScigmaGuiATWPanelSetCallbackColor(const void *value, void *callbackData)
{
  ATWPanel::Float* cbData(reinterpret_cast<ATWPanel::Float*>(callbackData));
  if(cbData->forward) 
    {
      const GLfloat* source(reinterpret_cast<const GLfloat*>(value));
      GLfloat* destination(cbData->storage);
      for(int i(0);i<4;++i)
	destination[i]=source[i];
    }
  cbData->panel->EventSource<EntryChangeEvent>::Type::emit(cbData->identifier.c_str(),value);
}
void TW_CALL ScigmaGuiATWPanelSetCallbackDirection(const void *value, void *callbackData)
{
  ATWPanel::Float* cbData(reinterpret_cast<ATWPanel::Float*>(callbackData));
  if(cbData->forward)
    {
      const GLfloat* source(reinterpret_cast<const GLfloat*>(value));
      GLfloat* destination(cbData->storage);
      for(int i(0);i<3;++i)
	destination[i]=source[i];
    }
  cbData->panel->EventSource<EntryChangeEvent>::Type::emit(cbData->identifier.c_str(),value);
}

void TW_CALL ScigmaGuiATWPanelCallbackButton(void *callbackData)
{
  ATWPanel::Int* cbData(reinterpret_cast<ATWPanel::Int*>(callbackData));
  cbData->panel->EventSource<ButtonEvent>::Type::emit(cbData->identifier.c_str());
}

namespace scigma
{
  namespace gui
  {

    ATWPanel::ATWPanel(GLContext* glContext,const char* name):PythonObject<ATWPanel>(this),glContext_(glContext),name_(name),wWindowOld_(0),hWindowOld_(0)
    {
      TwSetCurrentWindow(size_t(glContext_));
      bar_=TwNewBar(name);
      TwGetParam(bar_,NULL,"size",TW_PARAM_INT32,2,size_);
      TwGetParam(bar_,NULL,"position",TW_PARAM_INT32,2,pos_);
      name_=std::string("`")+name+"`";
      TwDefine((name_ +" label = "+name_).c_str());
    }

    ATWPanel::~ATWPanel()
    {
      TwSetCurrentWindow(size_t(glContext_));
      TwDeleteBar(bar_);

      std::map<std::string,Int*>::iterator i1(intData_.begin()),i2(intData_.end());
      while(i1!=i2){delete[] i1->second->storage;delete i1->second;++i1;}

      std::map<std::string,Float*>::iterator f1(floatData_.begin()),f2(floatData_.end());
      while(f1!=f2){delete[] f1->second->storage;delete f1->second;++f1;}

      std::map<std::string,Double*>::iterator d1(doubleData_.begin()),d2(doubleData_.end());
      while(d1!=d2){delete[] d1->second->storage;delete d1->second;++d1;}

      std::map<std::string,String*>::iterator s1(stringData_.begin()),s2(stringData_.end());
      while(s1!=s2){delete[] s1->second->storage;delete s1->second;++s1;}	

      for(size_t i(0), end(enumLabels_.size());i<end;++i)
	delete enumLabels_[i];
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

    bool ATWPanel::process(ResizeEvent event, int wWindow, int hWindow)
    {
      if(wWindowOld_!=0&&hWindowOld_!=0)
	{
	  TwSetCurrentWindow(size_t(glContext_));
	  TwGetParam(bar_,NULL,"size",TW_PARAM_INT32,2,size_);
	  TwGetParam(bar_,NULL,"position",TW_PARAM_INT32,2,pos_);
	  if(pos_[0]>=wWindowOld_-(pos_[0]+size_[0]))
	    pos_[0]+=(wWindow-wWindowOld_);
	  if(pos_[1]>=hWindowOld_-(pos_[1]+size_[1]))
	    pos_[1]+=(hWindow-hWindowOld_);
	  TwSetParam(bar_,NULL,"size",TW_PARAM_INT32,2,size_);
	  TwSetParam(bar_,NULL,"position",TW_PARAM_INT32,2,pos_);
	}
      wWindowOld_=wWindow;
      hWindowOld_=hWindow;
      return false;
    }
#pragma GCC diagnostic pop

    void ATWPanel::add_bool(const char* identifier, bool forward, const char* defs)
    {
      int* storage = new int[1];
      storage[0]=0;
      add_external_bool(identifier,storage,forward,defs);
    }
    void ATWPanel::add_external_bool(const char* identifier, int* storage, bool forward, const char* defs)
    {
      std::map<std::string,Int*>::iterator entry(intData_.find(identifier));
      if(entry!=intData_.end())
	return;
      else
	{
	  TwSetCurrentWindow(size_t(glContext_));
	  Int* cbData(new Int(this,identifier,storage,forward));
	  intData_.insert(std::pair<std::string,Int*>(identifier,cbData));
	  TwAddVarCB(bar_,identifier,TW_TYPE_BOOL32,ScigmaGuiATWPanelSetCallbackInt,ScigmaGuiATWPanelGetCallbackInt,cbData,defs);
	  sort_into_group(identifier);
	  glContext_->request_redraw();
	}
    }

    void ATWPanel::add_int(const char* identifier, bool forward, const char* defs)
    {
      int* storage = new int[1];
      storage[0]=0;
      add_external_int(identifier,storage,forward,defs);
    }
    void ATWPanel::add_external_int(const char* identifier, int* storage, bool forward, const char* defs)
    {
      std::map<std::string,Int*>::iterator entry(intData_.find(identifier));
      if(entry!=intData_.end())
	return;
      else
	{
	  TwSetCurrentWindow(size_t(glContext_));
	  Int* cbData(new Int(this,identifier,storage,forward));
	  intData_.insert(std::pair<std::string,Int*>(identifier,cbData));
	  TwAddVarCB(bar_,identifier,TW_TYPE_INT32,ScigmaGuiATWPanelSetCallbackInt,ScigmaGuiATWPanelGetCallbackInt,cbData,defs);
	  sort_into_group(identifier);
	  glContext_->request_redraw();
	}
    }

    void ATWPanel::add_float(const char* identifier, bool forward, const char* defs)
    {
      GLfloat* storage = new GLfloat[1];
      storage[0]=0.0;
      add_external_float(identifier,storage,forward,defs);
    }
    void ATWPanel::add_external_float(const char* identifier, GLfloat* storage, bool forward, const char* defs)
    {
      std::map<std::string,Float*>::iterator entry(floatData_.find(identifier));
      if(entry!=floatData_.end())
	return;
      else
	{
	  TwSetCurrentWindow(size_t(glContext_));
	  Float* cbData(new Float(this,identifier,storage,forward));
	  floatData_.insert(std::pair<std::string,Float*>(identifier,cbData));
	  TwAddVarCB(bar_,identifier,TW_TYPE_FLOAT,ScigmaGuiATWPanelSetCallbackFloat,ScigmaGuiATWPanelGetCallbackFloat,cbData,defs);
	  sort_into_group(identifier);
	  glContext_->request_redraw();
	}
    }
    
    void ATWPanel::add_double(const char* identifier, bool forward, const char* defs)
    {
      double* storage = new double[1];
      storage[0]=0.0;
      add_external_double(identifier,storage,forward,defs);
    }
    void ATWPanel::add_external_double(const char* identifier, double* storage, bool forward, const char* defs)
    {
      std::map<std::string,Double*>::iterator entry(doubleData_.find(identifier));
      if(entry!=doubleData_.end())
	return;
      else
	{
	  TwSetCurrentWindow(size_t(glContext_));
	  Double* cbData(new Double(this,identifier,storage,forward));
	  doubleData_.insert(std::pair<std::string,Double*>(identifier,cbData));
	  TwAddVarCB(bar_,identifier,TW_TYPE_DOUBLE,ScigmaGuiATWPanelSetCallbackDouble,ScigmaGuiATWPanelGetCallbackDouble,cbData,defs);
	  sort_into_group(identifier);
	  glContext_->request_redraw();
	}
    }
    
    void ATWPanel::add_string(const char* identifier, bool forward, const char* defs)
    {
      std::string* storage = new std::string[1];
      storage[0]="";
      add_external_string(identifier,storage,forward,defs);
    }
    void ATWPanel::add_external_string(const char* identifier, std::string* storage, bool forward, const char* defs)
    {
      std::map<std::string,String*>::iterator entry(stringData_.find(identifier));
      if(entry!=stringData_.end())
	return;
      else
	{
	  TwSetCurrentWindow(size_t(glContext_));
	  String* cbData(new String(this,identifier,storage,forward));
	  stringData_.insert(std::pair<std::string,String*>(identifier,cbData));
	  TwAddVarCB(bar_,identifier,TW_TYPE_STDSTRING,ScigmaGuiATWPanelSetCallbackString,ScigmaGuiATWPanelGetCallbackString,cbData,defs);
	  sort_into_group(identifier);
	  glContext_->request_redraw();
	}
    }
    
    void ATWPanel::add_color(const char* identifier, bool forward, const char* defs)
    {
      GLfloat* storage = new GLfloat[4];
      storage[0]=storage[1]=storage[2]=storage[3]=1.0;
      add_external_color(identifier,storage,forward,defs);
    }
    void ATWPanel::add_external_color(const char* identifier, GLfloat* storage, bool forward, const char* defs)
    {
      std::map<std::string,Float*>::iterator entry(floatData_.find(identifier));
      if(entry!=floatData_.end())
	return;
      else
	{
	  TwSetCurrentWindow(size_t(glContext_));
	  Float* cbData(new Float(this,identifier,storage,forward));
	  floatData_.insert(std::pair<std::string,Float*>(identifier,cbData));
	  TwAddVarCB(bar_,identifier,TW_TYPE_COLOR4F,ScigmaGuiATWPanelSetCallbackColor,ScigmaGuiATWPanelGetCallbackColor,cbData,defs);
	  sort_into_group(identifier);
	  glContext_->request_redraw();
	}
    }
    
    void ATWPanel::add_direction(const char* identifier, bool forward, const char* defs)
    {
      GLfloat* storage = new GLfloat[3];
      storage[0]=storage[1]=storage[2]=1.0;
      add_external_direction(identifier,storage,forward,defs);
    }
    void ATWPanel::add_external_direction(const char* identifier, GLfloat* storage, bool forward, const char* defs)
    {
      std::map<std::string,Float*>::iterator entry(floatData_.find(identifier));
      if(entry!=floatData_.end())
	return;
      else
	{
	  TwSetCurrentWindow(size_t(glContext_));
	  Float* cbData(new Float(this,identifier,storage,forward));
	  floatData_.insert(std::pair<std::string,Float*>(identifier,cbData));
	  TwAddVarCB(bar_,identifier,TW_TYPE_DIR3F,ScigmaGuiATWPanelSetCallbackDirection,ScigmaGuiATWPanelGetCallbackDirection,cbData,defs);
	  sort_into_group(identifier);
	  glContext_->request_redraw();
	}
    }
    
    void ATWPanel::add_enum(const char* identifier, std::string labels, int* values, bool forward, const char* defs)
    {
      int* storage = new int[1];
      storage[0]=values[0];
      add_external_enum(identifier,storage,labels,values,forward,defs);
    }

    using scigma::common::append_tokens;
    
    void ATWPanel::add_external_enum(const char* identifier, int* storage,std::string labels, int* values, bool forward, const char* defs)
    {
      std::map<std::string,Int*>::iterator entry(intData_.find(identifier));
      if(entry!=intData_.end())
	return;
      else
	{
	  std::vector<std::string> tokens;
	  append_tokens(labels,tokens,'|');

	  TwSetCurrentWindow(size_t(glContext_));
	  unsigned int n(GLuint(tokens.size()));
	  TwEnumVal* EV = new TwEnumVal[n];
	  for(unsigned int i(0);i<n;++i)
	    {
	      EV[i].Value=values[i];
	      std::string* label = new std::string(tokens[i]);
	      EV[i].Label = label->c_str();
	      enumLabels_.push_back(label);
	    }
	  TwType enumType(TwDefineEnum(identifier,EV,n));
	  Int* cbData(new Int(this,identifier,storage,forward));
	  intData_.insert(std::pair<std::string,Int*>(identifier,cbData));
	  TwAddVarCB(bar_,identifier,enumType,ScigmaGuiATWPanelSetCallbackInt,ScigmaGuiATWPanelGetCallbackInt,cbData,defs);
	  sort_into_group(identifier);
	  glContext_->request_redraw();
	}
    }
    
    void ATWPanel::add_button(const char* identifier, const char* defs)
    {
      std::map<std::string,Int*>::iterator entry(intData_.find(identifier));
      if(entry!=intData_.end())
	return;
      else
	{
	  TwSetCurrentWindow(size_t(glContext_));
	  Int* cbData(new Int(this,identifier,NULL,true));
	  intData_.insert(std::pair<std::string,Int*>(identifier,cbData));
	  TwAddButton(bar_,identifier,ScigmaGuiATWPanelCallbackButton,cbData,defs);
	  sort_into_group(identifier);
	  glContext_->request_redraw();
	}
    }

    void ATWPanel::set_bool(const char* identifier, int value)
    {
      std::map<std::string,Int*>::iterator entry(intData_.find(identifier));
      if(entry==intData_.end())
	return;
      *(entry->second->storage)=value;
      TwRefreshBar(bar_);
      glContext_->request_redraw();
    }

    void ATWPanel::set_int(const char* identifier, int value)
    {
      int min,max;
      TwGetParam(bar_, identifier, "min", TW_PARAM_INT32, 1, &min); 
      TwGetParam(bar_, identifier, "max", TW_PARAM_INT32, 1, &max); 
      if(value<min)
	value=min;
      if(value>max)
	value=max;

      std::map<std::string,Int*>::iterator entry(intData_.find(identifier));
      if(entry==intData_.end())
	return;
      *(entry->second->storage)=value;
      TwRefreshBar(bar_);
      glContext_->request_redraw();
    }
    
    void ATWPanel::set_float(const char* identifier, GLfloat value)
    {
      GLfloat min,max;
      TwGetParam(bar_, identifier, "min", TW_PARAM_FLOAT, 1, &min); 
      TwGetParam(bar_, identifier, "max", TW_PARAM_FLOAT, 1, &max); 
      if(value<min)
	value=min;
      if(value>max)
	value=max;

      std::map<std::string,Float*>::iterator entry(floatData_.find(identifier));
      if(entry==floatData_.end())
	return;
      *(entry->second->storage)=value;
      TwRefreshBar(bar_);
      glContext_->request_redraw();
    }

    void ATWPanel::set_double(const char* identifier, double value)
    {
      double min,max;
      TwGetParam(bar_, identifier, "min", TW_PARAM_DOUBLE, 1, &min); 
      TwGetParam(bar_, identifier, "max", TW_PARAM_DOUBLE, 1, &max); 
      if(value<min)
	value=min;
      if(value>max)
	value=max;
      
      std::map<std::string,Double*>::iterator entry(doubleData_.find(identifier));
      if(entry==doubleData_.end())
	return;
      *(entry->second->storage)=value;     
      TwRefreshBar(bar_);
      glContext_->request_redraw();
    }
    
    void ATWPanel::set_string(const char* identifier, std::string value)
    {
      std::map<std::string,String*>::iterator entry(stringData_.find(identifier));
      if(entry==stringData_.end())
	return;
      *(entry->second->storage)=value;
      TwRefreshBar(bar_);
      glContext_->request_redraw();
    }
    
    void ATWPanel::set_color(const char* identifier, GLfloat* values)
    {
      //      values=NULL;
      std::map<std::string,Float*>::iterator entry(floatData_.find(identifier));
      if(entry==floatData_.end())
	return;
      entry->second->storage[0]=values[0];      
      entry->second->storage[1]=values[1];      
      entry->second->storage[2]=values[2];      
      entry->second->storage[3]=values[3];      
      TwRefreshBar(bar_);
      glContext_->request_redraw();
    }

    void ATWPanel::set_direction(const char* identifier, GLfloat* values)
    {
      //      values =NULL;
      std::map<std::string,Float*>::iterator entry(floatData_.find(identifier));
      if(entry==floatData_.end())
	return;
      entry->second->storage[0]=values[0];      
      entry->second->storage[1]=values[1];      
      entry->second->storage[2]=values[2];      
      TwRefreshBar(bar_);
      glContext_->request_redraw();
    }

    void ATWPanel::set_enum(const char* identifier, int value)
    {
      std::map<std::string,Int*>::iterator entry(intData_.find(identifier));
      if(entry==intData_.end())
	return;
      *(entry->second->storage)=value;
      TwRefreshBar(bar_);
      glContext_->request_redraw();
    }
    
    void ATWPanel::remove(const char* identifier)
    {
      remove_external(identifier);
      std::map<std::string,Int*>::iterator i(intData_.find(identifier));
      if(intData_.end()!=i)
	{delete [] i->second->storage;delete i->second;intData_.erase(i);return;}
      std::map<std::string,Float*>::iterator f(floatData_.find(identifier));
      if(floatData_.end()!=f)
	{delete [] f->second->storage;delete f->second;floatData_.erase(f);return;}
      std::map<std::string,Double*>::iterator d(doubleData_.find(identifier));
      if(doubleData_.end()!=d)
	{delete [] d->second->storage;delete d->second;doubleData_.erase(d);return;}
      std::map<std::string,String*>::iterator s(stringData_.find(identifier));
      if(stringData_.end()!=s)
	{delete [] s->second->storage;delete s->second;stringData_.erase(s);}
    }

    void ATWPanel::remove_external(const char* identifier)
    {
      TwSetCurrentWindow(size_t(glContext_));
      TwRemoveVar(bar_, identifier);
      glContext_->request_redraw();
    }

    void ATWPanel::define( const char* defs)
    {
      TwSetCurrentWindow(size_t(glContext_));
      TwDefine((name_+" "+defs).c_str());
      glContext_->request_redraw();
    }

    void ATWPanel::define(const char* identifier, const char* defs)
    {
      TwSetCurrentWindow(size_t(glContext_));
      TwDefine((name_+"\\`"+identifier+"` "+defs).c_str());
      glContext_->request_redraw();
    }

    void ATWPanel::add_separator(const char* identifier, const char* defs)
    {
      TwSetCurrentWindow(size_t(glContext_));
      TwAddSeparator(bar_,identifier,defs);
      glContext_->request_redraw();
    }

    void ATWPanel::set_param(const char* identifier, const char* parameter, TwParamValueType type, unsigned int count, const void* values)
    {
      TwSetCurrentWindow(size_t(glContext_));
      TwSetParam(bar_,identifier,parameter,type,count,values);
      glContext_->request_redraw();
    }
    
    void ATWPanel::get_param(const char* identifier, const char* parameter, TwParamValueType type, unsigned int count, void* values)
    {
      TwSetCurrentWindow(size_t(glContext_));
      TwGetParam(bar_,identifier,parameter,type,count,values);
      glContext_->request_redraw();
    }
    
    void ATWPanel::sort_into_group(const char* identifier)
    {
      std::vector<std::string> nameTokens;	
      append_tokens(identifier,nameTokens,'.');
      std::vector<std::string> labelTokens(nameTokens);	 
      size_t n(nameTokens.size());
      for(unsigned int i(1);i<n;++i)
	nameTokens[i]=nameTokens[i-1]+"."+nameTokens[i];
      while(--n>0)
	{
	  TwDefine((name_+"/`"+nameTokens[n]+"`  group=`"+nameTokens[n-1]+"`").c_str());
	  TwDefine((name_+"/`"+nameTokens[n]+"`  label=`"+labelTokens[n]+"`").c_str());
	}
      TwDefine((name_+"/`"+nameTokens[0]+"`  label=`"+labelTokens[0]+"`").c_str());
    }
   
  } /* end namespace gui */
} /* end namespace scigma */
