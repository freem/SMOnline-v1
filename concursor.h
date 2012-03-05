//ConsoleCursor by Charles Lohr
//Easily use a cursor through the TTY or Windows Console
//Also acts as a replacement include for:
//clrscr(), gotoxy (x, y), and textcolor ( c )
//
//
//Multiplatform (Linux/Windows/OSX)
//Please see note at bottom.

#ifndef _CONCURSOR_H
#define _CONCURSOR_H

//If you wish to use STD output on windows, you can forcefully
//define it here.
#ifndef WIN32
#define USE_STD_OUTPUT
#else
#define USE_STD_KEYBOARD	//Means "kbhit()" is already included
							//If not, you must be running in a 
							//Linux/Irix (POSIX) environment
							//NOT Cygwin
#endif

//The upper left hand corner of the screen's coordinates
#define CON_UPLEFT_X		1
#define CON_UPLEFT_Y		1

//For use with TextColor()
#define COLOR_BRIGHT		8

#define COLOR_RED			1
#define COLOR_GREEN			2
#define COLOR_YELLOW		3
#define COLOR_BLUE			4
#define COLOR_MAGENTA		5
#define COLOR_CYAN			6
#define	COLOR_WHITE			7

//Macro'd commands
//So you can use concursor as a replacement for ncurses
//and the windows equivelant.
#ifndef gotoxy
#define gotoxy( x, y )		GotoXY( x, y )
#endif

#ifndef clrscr
#define clrscr()			ClearScr( 4 )
#endif

#ifndef textcolor
#define textcolor( c )		TextColor( c ) 
#endif


//=============================================================================
//Screen/Cursor functions

//This code contains the following functions:
//GotoXY
//ClearScr
//ClearLine
//RenderCon
//TextColor

//GotoXY
void GotoXY(const int x, const int y);

//Clear Screen
void ClearScr(const int type);
//0: Cursor to end of screen
//1: Cursor to beginning of screen
//2: Whole screen
//3: Whole screen & reset cursor

//Clear Line
void ClearLine(const int type);
//0: Cursor to end of line
//1: Cursor to beginning of line
//2: Whole line

//Change render type
void RenderCon(const int type);
//	(w) indicates Windows compatibility
/*
      type = None or 0      (w)	Default Rendition
           1			    (w)	Bold	(In windows, sets intensity)
           4					Underlined
           5				(w)	Bright BG  
				//NOTE: 5 on some systems will cause a blink
           7					Negative Image
           22           	(w)	Normal Intensity (In windows, unsets intensity)
           24           		Not Underlined
           25           	(w)	Standard BG
				//NOTE: On systems that consider '5' blink, 25 will turn it off
           27           		Positive Image
           30           	(w)	Foreground Black
           31           	(w)	Foreground Red
           32           	(w)	Foreground Green
           33           	(w)	Foreground Yellow
           34           	(w)	Foreground Blue
           35           	(w)	Foreground Magenta
           36           	(w)	Foreground Cyan
           37           	(w)	Foreground White
           39           	(w)	Foreground Default
           40           	(w)	Background Black
				...
		   49               (w) Background Default
*/

void TextColor ( const int c ); 


//=============================================================================
//KEYBOARD FUCNTIONS

//Sets input of chars to be blocking or non-blocking.
void SetKBBlocking( bool blocking );

//Get Char from Console
//NOTE: In windows, it simply runs a getch();
//In linux, it uses a getchar();
int GetCH();

#endif

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
