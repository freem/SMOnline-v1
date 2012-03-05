#include "SQLfunctions.h"
#include "SMOnlineSQL.h"
#include <sstream>
#include <fstream>
#include "MercuryINI.h"

using namespace std;

MString sqlTextEnc(string text) {
	string tmp;
	unsigned int pos = text.find("'");
	while (pos != MString::npos) {
		tmp = text.substr(pos+1);	//Remember the remaining text
		text = text.substr(0, pos); //Remove text after '
		text.replace(pos,3,"\\'");  //Replace ' with \\'
		text += tmp;				//Tack on the rest of the text
		pos = text.find("'",pos+2); //Start looking right after previous '
	}
	return("'" + MString(text.c_str()) + "'");
}

/* 
 * (c) 2005-2006 Joshua Allen and Adam Lowman
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
