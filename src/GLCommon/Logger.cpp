#include "Logger.h"

static std::string getTimeStr()
{
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
		
	char timestr [32];
	strftime(timestr,32,"%Y-%m-%d.%H-%M-%S",timeinfo);
		
	return std::string(timestr);
}




Logger::Logger() : m_console(false)
{
	// redirect std::cout to our console window
	m_old_cout = std::cout.rdbuf();

	// redirect std::cerr to our console window
	m_old_cerr = std::cerr.rdbuf();

	// redirect std::cin to our console window
	m_old_cin = std::cin.rdbuf();
}
	
Logger::~Logger()
{
	// reset the standard streams
	std::cin.rdbuf(m_old_cin);
	std::cerr.rdbuf(m_old_cerr);
	std::cout.rdbuf(m_old_cout);

	// remove the console window
	if(m_console)
		FreeConsole();
}



void Logger::ToScreen()
{
	// create a console window
	if(AllocConsole())
		m_console = true;
	else
		m_console = false;


	// redirect std::cout to our console window
	if(m_out.is_open())
		m_out.close();
	m_out.open("CONOUT$", std::ios::app);
	std::cout.rdbuf(m_out.rdbuf());

	// redirect std::cerr to our console window
	if(m_err.is_open())
		m_err.close();
	m_err.open("CONOUT$", std::ios::app);
	std::cerr.rdbuf(m_err.rdbuf());

	// redirect std::cin to our console window
	if(!m_in.is_open())
		m_in.close();
	m_in.open("CONIN$", std::ios::app);
	std::cin.rdbuf(m_in.rdbuf());
}

void Logger::ToFile(const char* logPrefix)
{
	if(m_out.is_open())
		m_out.close();

	std::string prefix = "Output";
	if(logPrefix!=NULL)
		prefix = logPrefix;

	std::string filename(prefix + '.' + getTimeStr() + ".log");
	
	m_out.open(filename.c_str(), std::ios::app);

	// redirect std::cout to our log file
	std::cout.rdbuf(m_out.rdbuf());

	// redirect std::cerr to our log file
	std::cerr.rdbuf(m_out.rdbuf());

	// redirect std::cin to our log file
	std::cin.rdbuf(m_out.rdbuf());
}
