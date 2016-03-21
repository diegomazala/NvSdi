#ifndef __GL_FONT_H__
#define __GL_FONT_H__



struct GLFont
{
	GLuint base;
	
	GLFont():base(-1){}

	void Create(GLuint size = -16, char* font_name = "Courier New")
	{
		if (base > -1)	// it's already initialized
			return;

		HFONT	font;										// Windows Font ID
		HFONT	oldfont;									// Used For Good House Keeping

		base = glGenLists(96);								// Storage For 96 Characters

		font = CreateFont(	size,			// Height Of Font
			0,								// Width Of Font
			0,								// Angle Of Escapement
			0,								// Orientation Angle
			FW_BOLD,						// Font Weight
			FALSE,							// Italic
			FALSE,							// Underline
			FALSE,							// Strikeout
			ANSI_CHARSET,					// Character Set Identifier
			OUT_TT_PRECIS,					// Output Precision
			CLIP_DEFAULT_PRECIS,			// Clipping Precision
			ANTIALIASED_QUALITY,			// Output Quality
			FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
			font_name);					// Font Name

		HDC hDC = wglGetCurrentDC();
		oldfont = (HFONT)SelectObject(hDC, font);           // Selects The Font We Want
		wglUseFontBitmaps(hDC, 32, 96, base);				// Builds 96 Characters Starting At Character 32
		SelectObject(hDC, oldfont);							// Selects The Font We Want
		DeleteObject(font);									// Delete The Font
	}


	void Destroy()										// Delete The Font List
	{
		glDeleteLists(base, 96);						// Delete All 96 Characters
	}
	

	GLvoid Print(float raster_pos_x, float raster_pos_y, const char* fmt, ...)	// Custom GL "Print" Routine
	{
		char		text[256];								// Holds Our String
		va_list		ap;										// Pointer To List Of Arguments

		if (fmt == NULL)									// If There's No Text
			return;											// Do Nothing

		va_start(ap, fmt);									// Parses The String For Variables
		vsprintf(text, fmt, ap);							// And Converts Symbols To Actual Numbers
		va_end(ap);											// Results Are Stored In Text

		glRasterPos2f(raster_pos_x, raster_pos_y);

		glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
		glListBase(base - 32);								// Sets The Base Character to 32
		glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
		glPopAttrib();										// Pops The Display List Bits
	}



	GLvoid Plot(float raster_pos_x, float raster_pos_y, const char* fmt, ...)
	{
		// 2D Projection
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();	// projection
		glLoadIdentity();
		gluOrtho2D( -1.0, 1.0, -1.0, 1.0 ); 
			
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();	// modelview
		glLoadIdentity();
		
		/////////////////////////////////////////////////////
		// begin: the same code for print method
		//
		{
			char		text[256];								// Holds Our String
			va_list		ap;										// Pointer To List Of Arguments

			if (fmt == NULL)									// If There's No Text
				return;											// Do Nothing

			va_start(ap, fmt);									// Parses The String For Variables
			vsprintf(text, fmt, ap);							// And Converts Symbols To Actual Numbers
			va_end(ap);											// Results Are Stored In Text

			glRasterPos2f(raster_pos_x, raster_pos_y);

			glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
			glListBase(base - 32);								// Sets The Base Character to 32
			glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
			glPopAttrib();										// Pops The Display List Bits
		}
		//
		// end: the same code for print method
		/////////////////////////////////////////////////////

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();	// projection

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();	// modelview
	}
};


#endif // __GL_FONT_H__