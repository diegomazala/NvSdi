#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <sstream>
#include <fstream>
#include <iostream>
#include <ctime>
#include <cstdio>
#include <iostream>
#include <windows.h>

////////////////////////////////////////////
// usage:
//	Log::ToFile();
//	std::cerr << "1 logger cout" << std::endl;
//	std::cout << "1 logger cerr" << std::endl;
//	Log::ToScreen();
//	std::cerr << "2 logger cerr" << std::endl;
//	std::cout << "2 logger cout" << std::endl;
//	Log::ToFile("LogName");
//	std::cerr << "3 logger cerr" << std::endl;
//	std::cout << "3 logger cout" << std::endl;
//	Log::ToScreen();
//	std::cerr << "4 logger cerr" << std::endl;
//	std::cout << "4 logger cout" << std::endl;
////////////////////////////////////////////

static std::string getTimeStr();


// This is defined in "GLNvSdiCore.h"
//#ifdef GLNVSDI_EXPORTS
//#define GLNVSDI_API __declspec(dllexport)
//#else
//#define GLNVSDI_API __declspec(dllimport)
//#endif


class Logger
{
public:
	
				Logger();
	virtual		~Logger();
	void		ToScreen();
	void		ToFile(const char* logPrefix=NULL);

protected:
		
	std::ofstream m_out;
	std::ofstream m_err;
	std::ifstream m_in;

	std::streambuf* m_old_cout;
	std::streambuf* m_old_cerr;
	std::streambuf* m_old_cin;

	bool m_console;
};



#endif	// __LOGGER_H__
