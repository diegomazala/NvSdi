#include "timecode.h"
#include "digits.h"

CTimeCode::CTimeCode(void)
{
	m_dwHours = 0;
	m_dwHours = 0;
	m_dwSeconds = 0;
	m_dwFrame = 0;
	m_uiBase = 0;
}

CTimeCode::~CTimeCode(void)
{
}

void
CTimeCode::init(float _uiBase)
{
	if (_uiBase <= 24)
		m_uiBase = 24;
	else if ((_uiBase > 24) && (_uiBase <= 25))
		m_uiBase = 25;
	else if ((_uiBase > 25) && (_uiBase <= 30))
		m_uiBase = 30;
	else if ((_uiBase > 30) && (_uiBase <= 48))
		m_uiBase = 24;
	else if ((_uiBase > 48) && (_uiBase <= 50))
		m_uiBase = 25;	
	else m_uiBase = 30;

	//	m_uiBase = (unsigned int)_uiBase;
}

void
CTimeCode::reset()
{
	m_dwHours = 0;
	m_dwMinutes = 0;
	m_dwSeconds = 0;
	m_dwFrame = 0;
}

void
CTimeCode::increment()
{
	if (m_dwFrame == m_uiBase-1) {
		if (m_dwSeconds == 59) {
			if (m_dwMinutes == 59) {
				if (m_dwHours == 59) {
					m_dwHours = 0;
				} else {
					m_dwHours++;
				}
				m_dwMinutes = 0;
			} else {
				m_dwMinutes++;
			}
			m_dwSeconds = 0;
		} else {
			m_dwSeconds++;
		}
		m_dwFrame = 0;
	} else {
		m_dwFrame++;
	}
}

void
CTimeCode::decrement()
{
	if (m_dwFrame == 0) {
		if (m_dwSeconds == 0) {
			if (m_dwMinutes == 0) {
				if (m_dwHours == 0) {
					m_dwHours = 59;
				} else {
					m_dwHours--;
				}
				m_dwMinutes = 59;
			} else {
				m_dwMinutes--;
			}
			m_dwSeconds = 59;
		} else {
			m_dwSeconds--;
		}
		m_dwFrame = m_uiBase - 1;
	} else {
		m_dwFrame--;
	}
}

void
CTimeCode::render(unsigned int _x, unsigned int _y)
{
	glPushMatrix();
	
	glScalef(4.0f, 4.0f, 4.0f);

	glColor4f(1.0, 1.0, 0.0, 0.0);

	draw2Digits(m_dwHours, _x, _y);
	_x+=16;
	drawSemicolon(_x, _y);
	_x+=4;
	draw2Digits(m_dwMinutes, _x, _y);
	_x+=16;
	drawSemicolon(_x, _y);
	_x+=4;
	draw2Digits(m_dwSeconds, _x, _y);
	_x+=16;
	drawSemicolon(_x, _y);
	_x+=4;
	draw2Digits(m_dwFrame, _x, _y);		

	glPopMatrix();
}

void
CTimeCode::print()
{
	fprintf(stderr, "%d:%d:%d:%d\n", m_dwHours, m_dwMinutes, m_dwSeconds, m_dwFrame);
}
