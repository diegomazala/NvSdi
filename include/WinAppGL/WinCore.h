#ifndef __WIN_CORE_h__
#define __WIN_CORE_h__


#ifdef WINAPP_EXPORTS
#define WINAPP_API __declspec(dllexport)
#else
#define WINAPP_API __declspec(dllimport)
#endif



#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#include <windows.h>
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <mmsystem.h>

#include "EventHandler.h"

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif



class WinApp;
struct EnvMapper
{
	HWND		hWnd;
	WinApp*	WinApp;
};


struct Dim2d
{
	Dim2d(int w, int h): Width(w), Height(h){}
	Dim2d(): Width(600), Height(480){}
	Dim2d& operator=(const Dim2d& other){Width=other.Width; Height=other.Height; return *this;} 
	int Width, Height;
};

struct Pos2d
{
	Pos2d(int x, int y): X(x), Y(y){}
	Pos2d(): X(10), Y(10){}
	Pos2d& operator=(const Pos2d& other){X=other.X; Y=other.Y; return *this;} 
	int X, Y;
};

struct RECTx
{
  RECTx(RECT r): right(r.right), bottom(r.bottom),left(r.left),top(r.top){}
	RECTx(int l, int t, int r, int b): right(r), bottom(b),left(l),top(t){}
	RECTx(int w, int h): right(w), bottom(h),left(0),top(0){}
	RECTx(): right(640), bottom(480),left(0),top(0){}
	RECTx& operator=(const RECTx& other)
  {left=other.left; top=other.top,
  right=other.right; bottom=other.bottom;return *this;}
  int Width() const {return right-left;}
  int Height() const {return bottom-top;}
  void Width(LONG w){right=left+w;}
  void Height(LONG h){bottom=top+h;}
  bool isIn(RECTx pr);//{if( (pr.left < left || pr.right  > right  ) &&
                       //   (pr.top  < top  || pr.bottom > bottom ) ) return false;
                       //    return true;}
  LONG    left;
  LONG    top;
  LONG    right;
  LONG    bottom;
};

enum WINAPP_API EKEY_CODE
{
	KEY_LBUTTON          = 0x01,  // Left mouse button
	KEY_RBUTTON          = 0x02,  // Right mouse button
	KEY_CANCEL           = 0x03,  // Control-break processing
	KEY_MBUTTON          = 0x04,  // Middle mouse button (three-button mouse)
	KEY_XBUTTON1         = 0x05,  // Windows 2000/XP: X1 mouse button
	KEY_XBUTTON2         = 0x06,  // Windows 2000/XP: X2 mouse button
	KEY_BACK             = 0x08,  // BACKSPACE key
	KEY_TAB              = 0x09,  // TAB key
	KEY_CLEAR            = 0x0C,  // CLEAR key
	KEY_RETURN           = 0x0D,  // ENTER key
	KEY_SHIFT            = 0x10,  // SHIFT key
	KEY_CONTROL          = 0x11,  // CTRL key
	KEY_MENU             = 0x12,  // ALT key
	KEY_PAUSE            = 0x13,  // PAUSE key
	KEY_CAPITAL          = 0x14,  // CAPS LOCK key
	KEY_KANA             = 0x15,  // IME Kana mode
	KEY_HANGUEL          = 0x15,  // IME Hanguel mode (maintained for compatibility use KEY_HANGUL)
	KEY_HANGUL           = 0x15,  // IME Hangul mode
	KEY_JUNJA            = 0x17,  // IME Junja mode
	KEY_FINAL            = 0x18,  // IME final mode
	KEY_HANJA            = 0x19,  // IME Hanja mode
	KEY_KANJI            = 0x19,  // IME Kanji mode
	KEY_ESCAPE           = 0x1B,  // ESC key
	KEY_CONVERT          = 0x1C,  // IME convert
	KEY_NONCONVERT       = 0x1D,  // IME nonconvert
	KEY_ACCEPT           = 0x1E,  // IME accept
	KEY_MODECHANGE       = 0x1F,  // IME mode change request
	KEY_SPACE            = 0x20,  // SPACEBAR
	KEY_PRIOR            = 0x21,  // PAGE UP key
	KEY_NEXT             = 0x22,  // PAGE DOWN key
	KEY_END              = 0x23,  // END key
	KEY_HOME             = 0x24,  // HOME key
	KEY_LEFT             = 0x25,  // LEFT ARROW key
	KEY_UP               = 0x26,  // UP ARROW key
	KEY_RIGHT            = 0x27,  // RIGHT ARROW key
	KEY_DOWN             = 0x28,  // DOWN ARROW key
	KEY_SELECT           = 0x29,  // SELECT key
	KEY_PRINT            = 0x2A,  // PRINT key
	KEY_EXECUT           = 0x2B,  // EXECUTE key
	KEY_SNAPSHOT         = 0x2C,  // PRINT SCREEN key
	KEY_INSERT           = 0x2D,  // INS key
	KEY_DELETE           = 0x2E,  // DEL key
	KEY_HELP             = 0x2F,  // HELP key
	KEY_KEY_0            = 0x30,  // 0 key
	KEY_KEY_1            = 0x31,  // 1 key
	KEY_KEY_2            = 0x32,  // 2 key
	KEY_KEY_3            = 0x33,  // 3 key
	KEY_KEY_4            = 0x34,  // 4 key
	KEY_KEY_5            = 0x35,  // 5 key
	KEY_KEY_6            = 0x36,  // 6 key
	KEY_KEY_7            = 0x37,  // 7 key
	KEY_KEY_8            = 0x38,  // 8 key
	KEY_KEY_9            = 0x39,  // 9 key
	KEY_KEY_A            = 0x41,  // A key
	KEY_KEY_B            = 0x42,  // B key
	KEY_KEY_C            = 0x43,  // C key
	KEY_KEY_D            = 0x44,  // D key
	KEY_KEY_E            = 0x45,  // E key
	KEY_KEY_F            = 0x46,  // F key
	KEY_KEY_G            = 0x47,  // G key
	KEY_KEY_H            = 0x48,  // H key
	KEY_KEY_I            = 0x49,  // I key
	KEY_KEY_J            = 0x4A,  // J key
	KEY_KEY_K            = 0x4B,  // K key
	KEY_KEY_L            = 0x4C,  // L key
	KEY_KEY_M            = 0x4D,  // M key
	KEY_KEY_N            = 0x4E,  // N key
	KEY_KEY_O            = 0x4F,  // O key
	KEY_KEY_P            = 0x50,  // P key
	KEY_KEY_Q            = 0x51,  // Q key
	KEY_KEY_R            = 0x52,  // R key
	KEY_KEY_S            = 0x53,  // S key
	KEY_KEY_T            = 0x54,  // T key
	KEY_KEY_U            = 0x55,  // U key
	KEY_KEY_V            = 0x56,  // V key
	KEY_KEY_W            = 0x57,  // W key
	KEY_KEY_X            = 0x58,  // X key
	KEY_KEY_Y            = 0x59,  // Y key
	KEY_KEY_Z            = 0x5A,  // Z key
	KEY_LWIN             = 0x5B,  // Left Windows key (Microsoft® Natural® keyboard)
	KEY_RWIN             = 0x5C,  // Right Windows key (Natural keyboard)
	KEY_APPS             = 0x5D,  //Applications key (Natural keyboard)
	KEY_SLEEP            = 0x5F,  // Computer Sleep key
	KEY_NUMPAD0          = 0x60,  // Numeric keypad 0 key
	KEY_NUMPAD1          = 0x61,  // Numeric keypad 1 key
	KEY_NUMPAD2          = 0x62,  // Numeric keypad 2 key
	KEY_NUMPAD3          = 0x63,  // Numeric keypad 3 key
	KEY_NUMPAD4          = 0x64,  // Numeric keypad 4 key
	KEY_NUMPAD5          = 0x65,  // Numeric keypad 5 key
	KEY_NUMPAD6          = 0x66,  // Numeric keypad 6 key
	KEY_NUMPAD7          = 0x67,  // Numeric keypad 7 key
	KEY_NUMPAD8          = 0x68,  // Numeric keypad 8 key
	KEY_NUMPAD9          = 0x69,  // Numeric keypad 9 key
	KEY_MULTIPLY         = 0x6A,  // Multiply key
	KEY_ADD              = 0x6B,  // Add key
	KEY_SEPARATOR        = 0x6C,  // Separator key
	KEY_SUBTRACT         = 0x6D,  // Subtract key
	KEY_DECIMAL          = 0x6E,  // Decimal key
	KEY_DIVIDE           = 0x6F,  // Divide key
	KEY_F1               = 0x70,  // F1 key
	KEY_F2               = 0x71,  // F2 key
	KEY_F3               = 0x72,  // F3 key
	KEY_F4               = 0x73,  // F4 key
	KEY_F5               = 0x74,  // F5 key
	KEY_F6               = 0x75,  // F6 key
	KEY_F7               = 0x76,  // F7 key
	KEY_F8               = 0x77,  // F8 key
	KEY_F9               = 0x78,  // F9 key
	KEY_F10              = 0x79,  // F10 key
	KEY_F11              = 0x7A,  // F11 key
	KEY_F12              = 0x7B,  // F12 key
	KEY_F13              = 0x7C,  // F13 key
	KEY_F14              = 0x7D,  // F14 key
	KEY_F15              = 0x7E,  // F15 key
	KEY_F16              = 0x7F,  // F16 key
	KEY_F17              = 0x80,  // F17 key
	KEY_F18              = 0x81,  // F18 key
	KEY_F19              = 0x82,  // F19 key
	KEY_F20              = 0x83,  // F20 key
	KEY_F21              = 0x84,  // F21 key
	KEY_F22              = 0x85,  // F22 key
	KEY_F23              = 0x86,  // F23 key
	KEY_F24              = 0x87,  // F24 key
	KEY_NUMLOCK          = 0x90,  // NUM LOCK key
	KEY_SCROLL           = 0x91,  // SCROLL LOCK key
	KEY_LSHIFT           = 0xA0,  // Left SHIFT key
	KEY_RSHIFT           = 0xA1,  // Right SHIFT key
	KEY_LCONTROL         = 0xA2,  // Left CONTROL key
	KEY_RCONTROL         = 0xA3,  // Right CONTROL key
	KEY_LMENU            = 0xA4,  // Left MENU key
	KEY_RMENU            = 0xA5,  // Right MENU key
	KEY_PLUS             = 0xBB,  // Plus Key   (+)
	KEY_COMMA            = 0xBC,  // Comma Key  (,)
	KEY_MINUS            = 0xBD,  // Minus Key  (-)
	KEY_PERIOD           = 0xBE,  // Period Key (.)
	KEY_ATTN             = 0xF6,  // Attn key
	KEY_CRSEL            = 0xF7,  // CrSel key
	KEY_EXSEL            = 0xF8,  // ExSel key
	KEY_EREOF            = 0xF9,  // Erase EOF key
	KEY_PLAY             = 0xFA,  // Play key
	KEY_ZOOM             = 0xFB,  // Zoom key
	KEY_PA1              = 0xFD,  // PA1 key
	KEY_OEM_CLEAR        = 0xFE,   // Clear key

	KEY_KEY_CODES_COUNT  = 0xFF // this is not a key, but the amount of keycodes there are.
};



//! Enumeration for all event types there are.
enum WINAPP_API EEVENT_TYPE
{
	EET_MOUSE_INPUT_EVENT,
	EET_KEY_INPUT_EVENT,
	EET_JOYSTICK_INPUT_EVENT, 
};



//! Enumeration for all mouse input events
enum WINAPP_API EMOUSE_INPUT_EVENT
{
	//! Left mouse button was pressed down.
	LBUTTON_DOWN = 0,

	//! Right mouse button was pressed down.
	RBUTTON_DOWN,

	//! Middle mouse button was pressed down.
	MBUTTON_DOWN,

	//! Left mouse button was left up.
	LBUTTON_UP,

	//! Right mouse button was left up.
	RBUTTON_UP,

	//! Middle mouse button was left up.
	MBUTTON_UP,

	//! The mouse cursor changed its position.
	MOUSE_MOVE,

	//! The mouse wheel was moved. Use Wheel value in event data to find out
	//! in what direction and how fast.
	MOUSE_WHEEL,

	//! No real event. Just for convenience to get number of events
	MOUSE_EVENT_COUNT
};


enum WINAPP_API EMOUSE_BUTTON
{
	LEFT_BUTTON = 0,
	MIDDLE_BUTTON,
	RIGHT_BUTTON,
	MOUSE_BUTTON_COUNT
};


struct JoystickInfo : public JOYINFOEX
{
	JoystickInfo(){}

	JoystickInfo(const JOYINFOEX& joy)
	{
		dwSize			= joy.dwSize;
		dwFlags			= joy.dwFlags; 
		dwXpos			= joy.dwXpos;
		dwYpos			= joy.dwYpos;
		dwZpos			= joy.dwZpos;
		dwUpos			= joy.dwUpos;
		dwVpos			= joy.dwVpos;
		dwButtons		= joy.dwButtons;
		dwButtonNumber	= joy.dwButtonNumber;
		dwPOV			= joy.dwPOV;
		dwReserved1		= joy.dwReserved1;
		dwReserved2		= joy.dwReserved2;
	}

	//! Overload the output operator. It's used to output the data stream
	friend std::ostream& operator<<(std::ostream& os, const JoystickInfo& joy)
	{
 		os	<< joy.dwSize << '\t' << joy.dwFlags << '\t' << joy.dwXpos << '\t'
 			<< joy.dwYpos << '\t' << joy.dwZpos << '\t' << joy.dwRpos << '\t'
 			<< joy.dwUpos << '\t' << joy.dwVpos << '\t' << joy.dwButtons << '\t'
 			<< joy.dwButtonNumber << '\t' << joy.dwPOV << '\t' 
 			<< joy.dwReserved1 << '\t' << joy.dwReserved2 << '\t';
		return os;
	}

	//! Overload the input operator. It's used to input the data stream
	friend std::istream& operator>>(std::istream& is, JoystickInfo& joy)
	{
 		is	>> joy.dwSize >> joy.dwFlags >> joy.dwXpos >> joy.dwYpos 
 			>> joy.dwZpos >> joy.dwRpos >> joy.dwUpos >> joy.dwVpos 
 			>> joy.dwButtons >> joy.dwButtonNumber >> joy.dwPOV 
 			>> joy.dwReserved1 >> joy.dwReserved2;
		return is;
	}
};



//////////////////////////////////////////////////////////////////////////
//                             System Devices							//
//////////////////////////////////////////////////////////////////////////		
struct WINAPP_API IDevice
{
protected:
	IDevice(){}
	IDevice(const IDevice&){}
};

struct WINAPP_API KeyboardDevice : public IDevice
{
	//! True if shift was also pressed
	bool Shift;

	//! True if ctrl was also pressed
	bool Control;

	KeyboardDevice():Shift(false),Control(false){}
};


struct WINAPP_API MouseDevice : public IDevice
{

	struct MouseButton
	{
		float	Down[2];
		float	Up[2];
		bool	Pressed;
		float	Sensitivity;
		MouseButton():Pressed(false),Sensitivity(1.f){};
	};

	MouseDevice():Button(new MouseButton[MOUSE_BUTTON_COUNT])
	{
	}

	~MouseDevice()
	{
		if(Button) delete [] Button;
	}

	MouseButton* Button;
};


struct WINAPP_API JoystickDevice : public IDevice
{
	enum
	{
		NUMBER_OF_BUTTONS = 32,

		AXIS_X = 0, // e.g. analog stick 1 left to right
		AXIS_Y,		// e.g. analog stick 1 top to bottom
		AXIS_Z,		// e.g. throttle, or analog 2 stick 2 left to right
		AXIS_R,		// e.g. rudder, or analog 2 stick 2 top to bottom
		AXIS_U,
		AXIS_V,
		NUMBER_OF_AXES
	};



	JOYCAPS		Caps;
	
	JOYINFOEX	Info;


	//! The ID of the joystick
	unsigned char		Id;

	//! A bitmap of button states.  
	DWORD&	ButtonStates(){return Info.dwButtons;}

	DWORD&	Pov(){return Info.dwPOV;}

	DWORD&	Axis(unsigned int axis)
	{
		switch(axis)
		{
			case AXIS_X :	return Info.dwXpos;
			case AXIS_Y :	return Info.dwYpos;
			case AXIS_Z :	return Info.dwZpos;
			case AXIS_R :	return Info.dwRpos;
			case AXIS_U :	return Info.dwUpos;
			case AXIS_V :	return Info.dwVpos;
			default		:	return Info.dwVpos;
		}
	}
	

};

//////////////////////////////////////////////////////////////////////////
//                             System Devices							//
//////////////////////////////////////////////////////////////////////////	

class WINAPP_API IEvent
{
public:
	enum EType
	{
		UNKNOWN,
		KEYBOARD,
		MOUSE,
		JOYSTICK,
		RESIZE,
		ACTIVATE,
		EXIT
	};

	virtual ~IEvent() {};
	
	EType GetType()const {return mType;}


	//! Overload the output operator. It's used to output the data stream
	virtual std::ostream& ToStream(std::ostream& os) const
	{
		return os;
	}

	//! Overload the input operator. It's used to input the data stream
	virtual std::istream& FromStream(std::istream& is)
	{
		return is;
	}


protected:

	IEvent(EType evType):mType(evType){}

	EType mType;
};



//! Any kind of mouse event.
struct WINAPP_API MouseEvent : public IEvent
{
	MouseEvent():IEvent(MOUSE){}

	MouseEvent(EMOUSE_INPUT_EVENT _event, int x, int y, float wheel=0.f):
				IEvent(MOUSE), 
				Event(_event), X(x), Y(y), Wheel(wheel){}

	//! X position of mouse cursor
	int X;

	//! Y position of mouse cursor
	int Y;

	//! mouse wheel delta, usually 1.0 or -1.0.
	/** Only valid if event was EMIE_MOUSE_WHEEL */
	float Wheel;

	//! Type of mouse event
	EMOUSE_INPUT_EVENT Event;


	//! Overload the output operator. It's used to output the data stream
	virtual std::ostream& ToStream(std::ostream& os) const
	{
		os	<< this->X << '\t' << this->Y 
			<< '\t' << this->Wheel << '\t' << this->Event << '\t';
		return os;
	}

	//! Overload the input operator. It's used to input the data stream
	virtual std::istream& FromStream(std::istream& is)
	{
		unsigned int mouseEv;
		is >> this->X >> this->Y >> this->Wheel >> mouseEv;
		this->Event = EMOUSE_INPUT_EVENT(mouseEv);
		return is;
	}
};



//! Any kind of keyboard event.
struct WINAPP_API KeyEvent : public IEvent
{
	//! Constructor 
	KeyEvent():IEvent(KEYBOARD){}

	//! Constructor with parameters
	KeyEvent(EKEY_CODE key, wchar_t _char, bool pressed, bool shift, bool control):
		IEvent(KEYBOARD), 
		Key(key), Char(_char), PressedDown(pressed), Shift(shift), Control(control)
		{}

	//! Character corresponding to the key (0, if not a character)
	wchar_t Char;

	//! Key which has been pressed or released
	EKEY_CODE Key;

	//! If not true, then the key was left up
	bool PressedDown;

	//! True if shift was also pressed
	bool Shift;

	//! True if ctrl was also pressed
	bool Control;



	//! It's used to output the data stream
	virtual std::ostream& ToStream(std::ostream& os) const
	{
		os	<< this->Key << '\t' << this->Char << '\t' 
			<< this->PressedDown << '\t' << this->Shift << '\t' << this->Control << '\t';
		return os;
	}

	//!It's used to input the data stream
	virtual std::istream& FromStream(std::istream& is)
	{
		unsigned int keycode;
		unsigned char keychar;
		is >> keycode >> keychar >> this->PressedDown >> this->Shift >> this->Control;
		this->Key = EKEY_CODE(keycode);
		this->Char = keychar;
		return is;
	}
};



//! A joystick event.
struct WINAPP_API JoystickEvent : public IEvent
{
	//! Constructor 
	JoystickEvent():IEvent(JOYSTICK){}

	//! Constructor a joystick event using the joystick info
	JoystickEvent(unsigned char joy, JoystickInfo info): IEvent(JOYSTICK), Joystick(joy), Info(info){}

	//! The ID of the joystick which generated this event.
	unsigned int Joystick;

	//! Current joystick status
	JoystickInfo Info;

	//! A helper function to check if a button is pressed.
	bool IsButtonPressed(unsigned int button) const
	{
		if(button >= JoystickDevice::NUMBER_OF_BUTTONS)
			return false;
		return (Info.dwButtons & (1 << button)) ? true : false;
	}


	//! It's used to output the data stream
// 	virtual std::ostream& ToStream(std::ostream& os) const
// 	{
// 		os	<< this->Joystick << '\t';
// 		return os;
// 	}
// 
// 	//! It's used to input the data stream
// 	virtual std::istream& FromStream(std::istream& is)
// 	{
// 		is >> this->Joystick;
// 		return is;
// 	}
};



//! Events hold information about an event. See irr::IEventReceiver for details on event handling.
struct WINAPP_API SEvent
{
	//! Any kind of mouse event.
	struct SMouseEvent
	{
		//! X position of mouse cursor
		int X;

		//! Y position of mouse cursor
		int Y;

		//! mouse wheel delta, usually 1.0 or -1.0.
		/** Only valid if event was EMIE_MOUSE_WHEEL */
		float Wheel;

		//! Type of mouse event
		EMOUSE_INPUT_EVENT Event;
	};

	EEVENT_TYPE EventType;
	union
	{
		struct SMouseEvent		MouseEvent;
	};
};


struct WINAPP_API CreationParameters
{
	//! Constructs a structure with default values.
	CreationParameters() :
		Title("------ Window ------"),
		WindowSize(Dim2d(800, 600)),
		WinPos(Pos2d(10, 10)),
		ColorDepthBits(24),
		AlphaDepthBits(1),
		DepthBufferBits(24),
		StencilBufferBits(8),
		Fullscreen(false),
		WindowId(0)
	{
	}

	
	CreationParameters& operator=(const CreationParameters& other)
	{
		Title				= other.Title;
		WindowSize			= other.WindowSize;
		WinPos				= other.WinPos;
		ColorDepthBits		= other.ColorDepthBits;
		AlphaDepthBits		= other.AlphaDepthBits;
		DepthBufferBits		= other.DepthBufferBits;
		StencilBufferBits	= other.StencilBufferBits;
		Fullscreen			= other.Fullscreen;
		WindowId			= other.WindowId;
		return *this;
	}

	std::string		Title;
	Dim2d			WindowSize;
	Pos2d			WinPos;
	unsigned char	ColorDepthBits;
	unsigned char	AlphaDepthBits;
	unsigned char	DepthBufferBits;
	unsigned char	StencilBufferBits;
	bool			Fullscreen;
	void*			WindowId;
};


#endif	// __WIN_CORE_H__