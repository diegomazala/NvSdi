#ifndef __WIN_APP_H__
#define __WIN_APP_H__

#include "WinCore.h"

class BasicWindow;



class WINAPP_API WinApp
{

public:

	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	WinApp();

	virtual ~WinApp();

	virtual void InitSetup();

	virtual void Run();

	virtual bool ProcessMainLoop();

	virtual void Update(){};

	virtual bool ActivateJoysticks();

	virtual KeyboardDevice* Keyboard();

	virtual MouseDevice*	Mouse();

	virtual JoystickDevice* Joystick(int index=0);
	
	std::size_t JoystickCount()const 
	{
		return mJoysticks.size();
	}


	virtual std::size_t GetWindowsCount() const
	{
		return Windows.size();
	}

	
	static std::list<BasicWindow*> Windows;

protected:

	KeyboardDevice				mKeyboard;	///< Keyboard device
	MouseDevice					mMouse;		///< Mouse device
	std::vector<JoystickDevice> mJoysticks;	///< Joystick devices


	//! Catch the joystick events
	void PollJoysticks(); 

};


#endif	// __WIN_APP_H__