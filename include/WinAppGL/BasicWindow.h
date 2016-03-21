#ifndef __BASIC_WINDOW_H__
#define __BASIC_WINDOW_H__

#include "WinCore.h"
#include "WinApp.h"



class WINAPP_API BasicWindow : public EventHandler
{

public:

	enum eWINDOW_TYPE
	{
		PERFORM_WINDOW,
		SETUP_WINDOW
	};

	BasicWindow();

	virtual ~BasicWindow();

	virtual bool Create(int w, int h, const char* title, WinApp* _app);

 	virtual bool Create(const CreationParameters& params, WinApp* _app);

	virtual void SetApplication(WinApp* _app);
 
 	virtual void Destroy();
 
	virtual void Update(){};

	virtual void Render() = 0;

	//! Called before all work be done. Before update
	virtual void PreProcess();

	//! Called after all work be done. After swap_buffers
	virtual void PostProcess();

	//! Process all work related to send or receive data from network
	virtual void ProcessComm();

	virtual void Close();

 	virtual int  Width() const;
 
	virtual int  Height() const;

	virtual float Aspect() const;
 
	virtual HDC  GetDC() const { return hDC; }

	virtual void SetDC(HDC _hdc){hDC = _hdc;}
 
 	virtual HWND GetWnd() const { return hWnd; }

	virtual void SetWnd(HWND _wnd) {hWnd = _wnd;}

	virtual HINSTANCE GetInstance() {return hInstance;}

	virtual void SetInstance(HINSTANCE _instance) {hInstance = _instance;}

	virtual void SwapBuffers() const;

	virtual void ShowConsoleWindow(bool enable);
 
	virtual void OnActive(bool flag);
 		
	virtual void OnExit();

	virtual void OnResize(int w, int h);

	virtual void OnKeyEvent(const KeyEvent* pEvent);

	virtual void OnMouseEvent(const MouseEvent* pEvent);

	virtual void OnJoystickEvent(const JoystickEvent* pEvent);

	virtual std::string GetTitle() const {return mParams.Title;}

	virtual eWINDOW_TYPE GetWindowType(){return (eWINDOW_TYPE)0;};

	//! Process the events (mouse, keyboard, joystick, etc..)
	virtual void ProcessEvents();

	//! Add a new event to be processed
	virtual void AddEvent(const IEvent* pEvent);

	//! Erase all events
	virtual void ClearEvents();

	//! Enable events listening
	void EnableEvents(bool enable){mEnableEvents=enable;}

	friend class WinApp;

protected:

	virtual bool SetPixelFormat(HDC hdc);

	virtual void Show() const;

	virtual void RunSetup(){}
	

	WinApp*						mpApp;		///< Application that holds this window

	bool						mRunning;	///< Flag to check when this window is active

	bool						mActive;	///< Window Active Flag Set To TRUE By Default

	HDC							hDC;		///< Private GDI Device Context

	HWND						hWnd;		///< Holds Our Window Handle

	HINSTANCE					hInstance;	///< Holds The Instance Of The Application

	CreationParameters			mParams;	///< The initial parameters used for window creation

	std::vector<const IEvent*>	mEvents;	///< Events to be processed (mouse, keyboards, etc..)

	bool						mEnableEvents;	///< Enable events listening

	bool						mConsoleWindowEnabled;	///< Control 'h' key : show or hide console window
};


#endif	// __BASIC_WINDOW_H__