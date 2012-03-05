//ConsoleCursor by Charles Lohr
//Easily use a cursor through the TTY or Windows Console
//
//Multiplatform (Linux/Windows/OSX)
//Please see note at bottom.

// some of the code used to clear the screen was found at:
// http://msdn.microsoft.com/library/en-us/dllproc/base/clearing_the_screen.asp

#include "concursor.h"
#include <stdio.h>

#if defined(WIN32)
#include <windows.h> 
#include <wincon.h>
#else
#include <unistd.h>	//For non-windows "READ" command
#endif

//*****************************************************************************
void GotoXY(const int x, const int y)
{
	if ( ( x < CON_UPLEFT_X ) || (y < CON_UPLEFT_Y ) )
		return;
#if !defined(USE_STD_OUTPUT)
	COORD c;

	c.X = x - CON_UPLEFT_X;
	c.Y = y - CON_UPLEFT_Y;

	SetConsoleCursorPosition( GetStdHandle( STD_OUTPUT_HANDLE ), c ) ;

#else
	printf( "%c[%d;%df", 27, x - CON_UPLEFT_X + 1, y - CON_UPLEFT_Y + 1 );
#endif
}

//*****************************************************************************
void ClearScr(const int type)
{
	if ( type == 4 )
	{
		ClearScr( 2 );
		gotoxy( CON_UPLEFT_X, CON_UPLEFT_Y );
		return ;
	}
#if !defined(USE_STD_OUTPUT)
    COORD coordScreen = { 0, 0 };
	DWORD cCW;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */ 
    DWORD dwConSize;                 /* number of character cells in */

    GetConsoleScreenBufferInfo( hConsole, &csbi );

	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
	switch (type)
	{
	case 0:
		dwConSize = dwConSize - csbi.dwCursorPosition.Y * csbi.dwSize.X + csbi.dwCursorPosition.X;
	    FillConsoleOutputCharacter( hConsole, (TCHAR) ' ', dwConSize, csbi.dwCursorPosition , &cCW );
		break;
	case 1:
		dwConSize = (csbi.dwCursorPosition.Y)* csbi.dwSize.X + csbi.dwCursorPosition.X;
	    FillConsoleOutputCharacter( hConsole, (TCHAR) ' ', dwConSize, coordScreen, &cCW );
		break;
	case 2:
	    FillConsoleOutputCharacter( hConsole, (TCHAR) ' ', dwConSize, coordScreen, &cCW );
	}

    GetConsoleScreenBufferInfo( hConsole, &csbi );
    FillConsoleOutputAttribute( hConsole, csbi.wAttributes,
       dwConSize, coordScreen, &cCW );
#else
	printf( "%c[%dJ", 27, type );
#endif
}

//*****************************************************************************
void ClearLine(const int type)
{
#if !defined(USE_STD_OUTPUT)
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordScreen = { 0, 0 };    /* here's where we'll home the
                                        cursor */ 
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */ 
    DWORD dwConSize;                 /* number of character cells in */

    GetConsoleScreenBufferInfo( hConsole, &csbi );

	dwConSize = csbi.dwSize.X;
	coordScreen.Y = csbi.dwCursorPosition.Y;

	switch (type)
	{
	case 0:
		dwConSize = dwConSize - csbi.dwCursorPosition.X;
	    FillConsoleOutputCharacter( hConsole, (TCHAR) ' ', dwConSize, csbi.dwCursorPosition , &cCharsWritten );
		break;
	case 1:
		dwConSize = csbi.dwCursorPosition.X;
	    FillConsoleOutputCharacter( hConsole, (TCHAR) ' ', dwConSize, coordScreen, &cCharsWritten );
		break;
	case 2:
	    FillConsoleOutputCharacter( hConsole, (TCHAR) ' ', dwConSize, coordScreen, &cCharsWritten );
	}

    GetConsoleScreenBufferInfo( hConsole, &csbi );
    FillConsoleOutputAttribute( hConsole, csbi.wAttributes,
       dwConSize, coordScreen, &cCharsWritten );

#else
	printf( "%c[%dK", 27, type );
#endif
}

//*****************************************************************************
void RenderCon(const int type)
{
#if !defined(USE_STD_OUTPUT)
	static int CurVal=0x07;

	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	switch ( type )
	{
	case 0:
		CurVal = 0x07;
		break;
	case 1:
		CurVal = CurVal & (0xFFFFFFF7) | 8;
		break;
	case 4:
		CurVal = CurVal | COMMON_LVB_UNDERSCORE;
		break;
	case 5:
		CurVal = CurVal & (0xFFFFFF7F) | 128;
		break;
	case 22:
		CurVal = CurVal & (0xFFFFFFF7) | 0;
		break;
	case 24:
		CurVal = CurVal & (!COMMON_LVB_UNDERSCORE);
		break;
	case 25:
		CurVal = CurVal & (0xFFFFFF7F);
		break;
	case 30:
		CurVal = CurVal & (0xFFFFFFF8) | 0;
		break;
	case 31:
		CurVal = CurVal & (0xFFFFFFF8) | 4;
		break;
	case 32:
		CurVal = CurVal & (0xFFFFFFF8) | 2;
		break;
	case 33:
		CurVal = CurVal & (0xFFFFFFF8) | 6;
		break;
	case 34:
		CurVal = CurVal & (0xFFFFFFF8) | 1;
		break;
	case 35:
		CurVal = CurVal & (0xFFFFFFF8) | 5;
		break;
	case 36:
		CurVal = CurVal & (0xFFFFFFF8) | 3;
		break;
	case 37:
	case 39:
		CurVal = CurVal & (0xFFFFFFF8) | 7;
		break;

	case 40:
		CurVal = CurVal & (0xFFFFFF8F) | 0;
		break;
	case 41:
		CurVal = CurVal & (0xFFFFFF8F) | 4*16;
		break;
	case 42:
		CurVal = CurVal & (0xFFFFFF8F) | 2*16;
		break;
	case 43:
		CurVal = CurVal & (0xFFFFFF8F) | 6*16;
		break;
	case 44:
		CurVal = CurVal & (0xFFFFFF8F) | 1*16;
		break;
	case 45:
		CurVal = CurVal & (0xFFFFFF8F) | 5*16;
		break;
	case 46:
		CurVal = CurVal & (0xFFFFFF8F) | 3*16;
		break;
	case 47:
		CurVal = CurVal & (0xFFFFFF8F) | 7*16;
		break;
	case 49:
		CurVal = CurVal & (0xFFFFFF8F) | 0;
		break;
	}
	SetConsoleTextAttribute(hStdOut, CurVal);
#else
	printf("%c[%dm",27,type);
#endif
}

//*****************************************************************************
void TextColor ( const int c ) 
{
	if ( ( c / COLOR_BRIGHT ) > 0 )
		RenderCon( 1 );
	else
		RenderCon( 22 );

	RenderCon( 30 + c % COLOR_BRIGHT );
}

//=============================================================================
//KEYBOARD FUNCTIONS
#ifdef USE_STD_KEYBOARD
#include <conio.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#endif

static bool KBblocking;

void SetKBBlocking( bool blocking )
{
	KBblocking = blocking;
#ifndef USE_STD_KEYBOARD
	termios termios_p;
	if (blocking)
		fcntl(0, F_SETFL, 0);
	else
		fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
	tcgetattr(0, &termios_p);
	termios_p.c_lflag &= ~(ICANON | ECHO);
	termios_p.c_lflag &= ~(ICANON);
	termios_p.c_cc[VMIN] = 1;
	termios_p.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &termios_p);
#endif

}

int GetCH()
{
#ifdef USE_STD_KEYBOARD
	if (KBblocking)
		return getch();
	else
		if (kbhit())
			return getch();
		else
			return (-1);
#else
	return getchar();
#endif
}

/*
 * (c) 2004 Charles Lohr (webmaster@cnlohr.net)
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
