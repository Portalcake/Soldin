/*
 * Soldin - Lunia Server Emulator 
 * Copyright (c) 2010 Seipheroth
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE. 
 */
#ifndef __SOLDIN_CONSOLE_H__
#define __SOLDIN_CONSOLE_H__

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifdef WIN32
#	define FGC_BLACK		0
#	define FGC_DARKBLUE		FOREGROUND_BLUE
#	define FGC_DARKGREEN	FOREGROUND_GREEN
#	define FGC_DARKRED		FOREGROUND_RED
#	define FGC_BLUE			(FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#	define FGC_GREEN		(FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#	define FGC_RED			(FOREGROUND_RED | FOREGROUND_INTENSITY)
#	define FGC_GRAY			(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN)
#	define FGC_WHITE		(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#	define FGC_YELLOW		(FGC_RED | FGC_GREEN)
#	define FGC_PINK			(FGC_RED | FGC_BLUE)
#	define FGC_TEAL			(FGC_GREEN |FGC_BLUE)
#	define FGC_DARKYELLOW	(FOREGROUND_RED | FOREGROUND_GREEN)
#	define FGC_PURPLE		(FOREGROUND_RED | FOREGROUND_BLUE)
#	define FGC_DARKTEAL		(FOREGROUND_GREEN | FOREGROUND_BLUE)
#endif

#include <shared.h>

class Console {
public:
	/* Changes the console text color. */
	static void SetForeColor( uint16_t color )
	{
		if ( mHandle == NULL )
			mHandle = GetStdHandle( STD_OUTPUT_HANDLE );

		SetConsoleTextAttribute( mHandle, color );
	}

	/* Changes the title of the console window. */
	inline static void SetTitle( const char *title )
	{
		SetConsoleTitleA( title );
	}

private:
	static HANDLE mHandle;
};

#endif
