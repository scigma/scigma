#include <stdlib.h>
#include "console.h"
#include "font.h"
#include "glutil.h"
#include "glwindow.h"
#include "spacetext.h"

namespace scigma
{
  namespace gui
  {
    
    Console::Console(GLContext* context,bool largeFontsFlag):PythonObject<Console>(this),
							     currentLineBitmap(NULL),cursorBitmap(NULL),
							     nScreenLines(25),tabWidth(8),
							     glContext(context),
							     fontType(largeFontsFlag?Font::DEJAVU_SANS_MONO_29:Font::DEJAVU_SANS_MONO_14),
							     fadeout(true),delayedLineFeed(false)

    {
      float fg[]={1.0f,1.0f,1.0f,1.0f};
      float da[]={0.0f,0.6f,0.0f,1.0f};
      float err[]={1.0f,0.0f,0.0f,1.0f};
      float bg[]={0.0f,0.0f,0.0f,0.0f};
      set_foreground_color(fg);
      set_data_color(da);
      set_error_color(err);
      set_background_color(bg);
      cursor=currentLine.end();
      historyPoint=history.begin();
      screenLines.push_back(SingleScreenLine(0));
    }
    
    Console::~Console()
    {
      // clean up remaining bitmapstrings
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
        
    bool Console::process(CharEvent event, GLWindow* window, unsigned int unicode)
    {
      if(unicode<256)
	{
	  char c((char(unicode)));
	  cursor=++currentLine.insert(cursor,c);
	  glContext->stall();
	  setup_current_text();
	  setup_cursor();
	  glContext->flush();
	  return false;
	}
      return true;
    }
    
    bool Console::process(KeyEvent event, GLWindow* window, int key,int scancode, int action, int mods)
    {
      std::string input;
      /* this brace delimits the scope of staller below,
	 so the window will be redrawn before the input
	 event is propagated
      */
      {
	GLContextStaller staller(window->gl_context());
	if(GLFW_RELEASE==action)
	  return false;
	if(key==GLFW_KEY_BACKSPACE&&cursor!=currentLine.begin())
	  cursor=currentLine.erase(--cursor);
	else if(key==GLFW_KEY_DELETE&&cursor!=currentLine.end())
	  cursor=currentLine.erase(cursor);
	else if(key==GLFW_KEY_ENTER)
	  {
	    delayedLineFeed=true;
	    ColoredText ctl;ctl.text=ctl.displayedText=currentLine;ctl.color=fColor;ctl.text+="\n";
	    screenLines.push_back(SingleScreenLine(1,ctl));
	    history.push_back(currentLine);
	    historyPoint=history.end();
	    input=currentLine;
	    currentLine="";
	    cursor=currentLine.end();
	    setup_previous_text();
	}
	else if(key==GLFW_KEY_LEFT&&cursor!=currentLine.begin())
	  --cursor;
	else if(key==GLFW_KEY_RIGHT&&cursor!=currentLine.end())
	  ++cursor;
	else if(key==GLFW_KEY_UP)
	  {
	    if(historyPoint==history.end())
	      currentLineBackup=currentLine;
	    if(historyPoint!=history.begin())
	      {
		currentLine=*(--historyPoint);
		cursor=currentLine.end();
	      }
	  }
	else if(key==GLFW_KEY_DOWN)
	  {
	    if(historyPoint!=history.end())
	      {
		++historyPoint;
		if(historyPoint!=history.end())
		  currentLine=*historyPoint;
		else
		  currentLine=currentLineBackup;
		cursor=currentLine.end();
	      }
	    else
	      return false;
	  }
	else
	  return false;
	glContext->stall();
	setup_current_text();
	setup_cursor();
	glContext->flush();
      }
      if(!input.empty())
	EventSource<InputEvent>::Type::emit(input);
      return true;
    }
    
#pragma GCC diagnostic pop
    
    void Console::set_displayed_screen_lines(unsigned int number)
    {
      nScreenLines=number;
    }
    
    void Console::set_history_fadeout(bool yesNo)
    {
      fadeout=yesNo;
    }
    
    void Console::set_foreground_color(const float* rgba)
    {
      for(unsigned int i =0;i<4;++i)
	fColor[i]=rgba[i];
      if(currentLineBitmap)
	currentLineBitmap->set_color(fColor,bColor);
      if(cursorBitmap)
	cursorBitmap->set_color(bColor,fColor);
    }
    
    void Console::set_data_color(const float* rgba)
    {
      for(unsigned int i =0;i<4;++i)
	dColor[i]=rgba[i];
    }
    
    void Console::set_error_color(const float* rgba)
    {
      for(unsigned int i =0;i<4;++i)
	eColor[i]=rgba[i];
    }
    
    void Console::set_background_color(const float* rgba)
    {
      for(unsigned int i =0;i<4;++i)
	bColor[i]=rgba[i];
    }
    
    void Console::feed_line()
    {
      screenLines.push_back(SingleScreenLine(0));
    }
    
    size_t Console::remove_backspaces(std::string& text)
    {
      size_t lastBackspace=text.rfind('\b');
      if(lastBackspace==std::string::npos)
	return 0;
      char charToDelete;
      size_t deletionPosition(lastBackspace);
      while(deletionPosition!=0)
	{
	  charToDelete=text[deletionPosition];
	  if(charToDelete!='\b')
	    {
	      text.erase(lastBackspace,1);
	      text.erase(deletionPosition,1);
	      return remove_backspaces(text);
	    }
	  --deletionPosition;
	}
      text.erase(0,lastBackspace);
      return lastBackspace;
    }
    
    size_t Console::expand_tabulators(ColoredText& cText, size_t offset)
    {
      cText.displayedText=cText.text;
      size_t firstTabulator;
      while((firstTabulator=cText.displayedText.find('\t'))!=std::string::npos)
	cText.displayedText.replace(firstTabulator,1,tabWidth-((firstTabulator+offset)%tabWidth),' ');
      return cText.displayedText.length();
    }
    
    void Console::append_to_line(ColoredText& cText)
    {
      size_t remainingBackspaces=remove_backspaces(cText.text);
      SingleScreenLine::iterator previousOutput;
      size_t offset(0);
      if(!screenLines.back().empty())
	{
	  previousOutput=screenLines.back().end();
	  do
	    {
	      if(remainingBackspaces==0)
		break;
	      --previousOutput;
	      previousOutput->text.insert(previousOutput->text.end(),remainingBackspaces,'\b');
	      remainingBackspaces=remove_backspaces(previousOutput->text);	  
	    }
	  while(previousOutput!=screenLines.back().begin());
	  previousOutput=screenLines.back().begin();
	  while(previousOutput!=screenLines.back().end())
	    offset=expand_tabulators(*(previousOutput++),offset);
	}
      expand_tabulators(cText,offset);
      screenLines.back().push_back(cText);
    }
    
    void Console::append_text(const std::string& lines, float* color)
    {
      if(delayedLineFeed)
	{
	  delayedLineFeed=false;
	  feed_line();
	}

      size_t l;
      if((l=lines.length())==0)
	return;
      
      ColoredText ctl;ctl.color=color;
      char* c = new char[l+1];
      size_t first(0),last(lines.find('\n'));
      while(last!=std::string::npos)
	{
	  lines.copy(c,last-first,first);
	  c[last-first]=0;
	  ctl.text=c;
	  append_to_line(ctl);
	  if(l-1==last)
	    delayedLineFeed=true;
	  else
	    feed_line();
	  
	  first=last+1;
	  last=lines.find('\n',first);
	}
      if(first!=l)
	{
	  lines.copy(c,l-first,first);
	  c[l-first]=0;
	  ctl.text=c;
	  append_to_line(ctl);
	}
      delete []c;
      
      glContext->stall();
      setup_previous_text();
      glContext->flush();
    }
    
    void Console::write(std::string text)
    {
      append_text(text,fColor);
    }
    
    void Console::write_data(std::string data)
    {
      append_text(data,dColor);
    }
    
    void Console::write_error(std::string error)
    {
      append_text(error,eColor);
    }
    
    void Console::setup_previous_text()
    {
      std::vector<ScreenText*>::iterator i = screenLineBitmaps.begin();
      while(i!=screenLineBitmaps.end())
	{
	  glContext->remove_drawable(*i);
	  delete *(i++);
	}
      screenLineBitmaps.clear();
      
      std::vector<SingleScreenLine> displayedLines;
      std::vector<SingleScreenLine>::iterator j = screenLines.end();
      while(j!=screenLines.begin()&&displayedLines.size()<nScreenLines)
	displayedLines.push_back(*(--j));
      
      const Font& font = Font::font(fontType);
      GLfloat lineHeight= font.charHeight;
      size_t lines = displayedLines.size();
      
      GLfloat* coords=new GLfloat[lines*2];
      for(size_t k=0;k<lines;++k)
	{
	  coords[k]=5;
	  coords[lines+k]=(k+1)*lineHeight;
	}
      
      float fAlpha=fColor[3];
      float dAlpha=dColor[3];
      float eAlpha=eColor[3];
      float bAlpha=bColor[3];

      for(size_t k=0;k<lines;++k)
	{
	  if(fadeout)
	    {
	      float factor=float(nScreenLines-k)/float(nScreenLines);
	      fColor[3]=fAlpha*factor;
	      dColor[3]=dAlpha*factor;
	      eColor[3]=eAlpha*factor;
	      fColor[3]=fAlpha*factor;
	    }
	  for(unsigned int l=0;l<displayedLines[k].size();++l)
	    {
	      ScreenText* line = new ScreenText(fontType);
	      line->set_color(displayedLines[k][l].color,bColor);
	      line->set_text(&displayedLines[k][l].displayedText,&coords[k],&coords[lines+k],-0.5,-0.5,1);
	      coords[k]+=font.string_width(displayedLines[k][l].displayedText);
	      push_opengl_error(__FILE__,__LINE__);
	      glContext->add_drawable(line);
	      push_opengl_error(__FILE__,__LINE__);
	      screenLineBitmaps.push_back(line);
	    }
	}
      fColor[3]=fAlpha;
      dColor[3]=dAlpha;
      eColor[3]=eAlpha;
      bColor[3]=bAlpha;
      
      delete [] coords;
    }
    
    void Console::setup_current_text()
    {
      if(!currentLineBitmap)
	{
	  currentLineBitmap=new ScreenText(fontType);
	  currentLineBitmap->set_color(fColor,bColor);
	  glContext->add_drawable(currentLineBitmap);
	}
      GLfloat x(5),y(0),xA(-0.5),yA(-0.5);      
      currentLineBitmap->set_text(&currentLine,&x,&y,xA,yA,1);
      glContext->request_redraw();
      GLERR;
    }
    
    void Console::setup_cursor()
    {
      const Font& font = Font::font(fontType);
      const GLfloat* charWidth = &font.charWidth[0];

      if(!cursorBitmap)
	{
	  cursorBitmap=new ScreenText(fontType);
	  cursorBitmap->set_color(bColor,fColor);
	  glContext->add_drawable(cursorBitmap);
	}
      
      std::string c("");  
      GLfloat position(0);
      if(cursor==currentLine.end())
	{
	  c.append(1,' ');
	  position=font.string_width(currentLine);  
	}
      else
	{
	  c.append(1,*cursor);
	  std::string::iterator i=currentLine.begin();
	  while(cursor!=i)
	    {
	      position+=charWidth[GLubyte(*i)];
	      ++i;
	    };
	}
      GLfloat x(5+position),y(0),xA(-0.5),yA(-0.5);
      cursorBitmap->set_text(&c,&x,&y,xA,yA,1);
      glContext->request_redraw();
      GLERR;
    }
    
  } /* end namespace gui */
} /* end namespace scigma */
