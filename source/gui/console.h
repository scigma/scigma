#ifndef __SCIGMA_GUI_CONSOLE_H__
#define __SCIGMA_GUI_CONSOLE_H__

#include <vector>
#include <string>
#include "../events.h"
#include "../pythonobject.h"
#include "glwindowevents.h"
#include "inputevent.h"
#include "screentext.h"

namespace scigma
{
  namespace gui
  {
    class GLWindow;
    class Console:public PythonObject<Console>,
      public EventSource<InputEvent>::Type,
      public EventSink<KeyEvent>::Type,
      public EventSink<CharEvent>::Type
      {
      public:
	Console(GLContext* context,bool largeFontsFlag=false);
	virtual ~Console();
	virtual bool process(CharEvent event, GLWindow* window, unsigned int unicode);
	virtual bool process(KeyEvent event, GLWindow* window, int key,int scancode,int action,int mods);
	void set_displayed_screen_lines(unsigned int number);
	void set_history_fadeout(bool yesNo);
	void set_foreground_color(const float* rgba);
	void set_error_color(const float* rgba);
	void set_data_color(const float* rgba);
	void set_background_color(const float* rgba);
      
	void write(std::string text);
	void write_data(std::string data);
	void write_error(std::string error);
	
      private:
	class ColoredText
	{
	public:
	  std::string text;
	  std::string displayedText; // tab stops replaced by spaces
	  float* color;
	};
	typedef std::vector<ColoredText> SingleScreenLine;

	Console(const Console& c);
	Console& operator=(const Console&);
	
	void setup_previous_text();
	void setup_current_text();
	void setup_cursor();

	size_t remove_backspaces(std::string& text);
	size_t expand_tabulators(ColoredText& cText, size_t offset);
	void append_to_line(ColoredText& cText); 
	void append_text(const std::string& text, float* color);
	void feed_line();
	
	std::vector<SingleScreenLine> screenLines;
	std::vector<std::string> history;
	std::vector<std::string>::iterator historyPoint; 
	
	
	std::string currentLine;
	std::string currentLineBackup;
	std::string::iterator cursor;
	ScreenText* currentLineBitmap;
	ScreenText* cursorBitmap;
	std::vector<ScreenText*> screenLineBitmaps;
	unsigned int nScreenLines;
	unsigned int tabWidth;
	
	float fColor[4];
	float dColor[4];
	float eColor[4];
	float bColor[4];   
      
	GLContext* glContext;
	Font::Type fontType;

	bool fadeout;
	bool delayedLineFeed;

	char padding_[2];
      };

  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_CONSOLE__H__ */
