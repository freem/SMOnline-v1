#ifndef JUDGEWINDOW_H
#define JUDGEWINDOW_H

#include "ConstructCount.h"

//using namespace std;

class JudgeWindow {
public:
	JudgeWindow();
	inline void SetMarv(double sec) { m_marv = sec; }
	inline void SetPerfect(double sec) { m_perfect = sec; }
	inline void SetGreat(double sec) { m_great = sec; }
	inline void SetGood(double sec) { m_good = sec; }
	inline void SetBoo(double sec) { m_boo = sec; }
	inline double GetMarv() const { return m_marv; }
	inline double GetPerfect() const { return m_perfect; }
	inline double GetGreat() const { return m_great; }
	inline double GetGood() const { return m_good; }
	inline double GetBoo() const { return m_boo; }
	bool operator==(const JudgeWindow& window) const;
	bool operator>(const JudgeWindow& window) const;
	bool operator<(const JudgeWindow& window) const;
	JudgeWindow& operator=(const JudgeWindow& window);
private:
	double m_marv;
	double m_perfect;
	double m_great;
	double m_good;
	double m_boo;
	ConstructCount m_counter;
};

#endif

/*
 * (c) 2004-2005 Joshua Allen
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
