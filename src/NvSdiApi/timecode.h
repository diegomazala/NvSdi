
#include <stdio.h>

class CTimeCode
{
	int m_dwHours;
	int m_dwMinutes;
	int m_dwSeconds;
	int m_dwFrame;

	unsigned int m_uiBase;

public:
	CTimeCode(void);
	~CTimeCode(void);

	void init(float base);

	void increment();

	void decrement();

	void reset();

	void render(unsigned int x, unsigned int y);

	void print();

	int frame() { return m_dwFrame; };

	int second() { return m_dwSeconds; };

	int minute() { return m_dwMinutes; };

	int hour() { return m_dwHours; };
};
