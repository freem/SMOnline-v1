#include "ezMySQLConstants.h"

const char * MySQLCharsetNames[] = {
	"",
	"big5",
	"czech",
	"dec8",	
	"dos",
	"german1",
	"hp8",
	"koi8_ru",
	"latin1",
	"latin2",
	"swe7",
	"usa7",
	"ujis",
	"sjis",
	"cp1251",
	"danish",
	"hebrew",
	"win1251",
	"tis620",
	"euc_kr",
	"estonia",
	"hungarian",
	"koi8_ukr",
	"win1251ukr",
	"gb2312",
	"greek",
	"win1250",
	"croat",
	"gbk",
	"cp1257",
	"latin5"
};

const char * GetCharsetName( int cNum )
{
	return MySQLCharsetNames[cNum];
}

/* 
 * (c) 2005 Charles Lohr
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
