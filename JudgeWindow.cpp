#include "JudgeWindow.h"

JudgeWindow::JudgeWindow() {
	m_marv = 0;
	m_perfect = 0;
	m_great = 0;
	m_good = 0;
	m_boo  = 0;
}

bool JudgeWindow::operator==(const JudgeWindow& window) const {
	if (this->m_marv != window.m_marv)
		return false;
	if (this->m_perfect != window.m_perfect)
		return false;
	if (this->m_great != window.m_great)
		return false;
	if (this->m_good != window.m_good)
		return false;
	if (this->m_boo != window.m_good)
		return false;

	return true;
}

bool JudgeWindow::operator>(const JudgeWindow& window) const {
	if (this->m_marv < window.m_marv)
		return false;
	if (this->m_perfect < window.m_perfect)
		return false;
	if (this->m_great < window.m_great)
		return false;
	if (this->m_good < window.m_good)
		return false;
	if (this->m_boo < window.m_good)
		return false;

	return true;
}

bool JudgeWindow::operator<(const JudgeWindow& window) const {
	if (this->m_marv > window.m_marv)
		return false;
	if (this->m_perfect > window.m_perfect)
		return false;
	if (this->m_great > window.m_great)
		return false;
	if (this->m_good > window.m_good)
		return false;
	if (this->m_boo > window.m_good)
		return false;

	return true;
}

JudgeWindow& JudgeWindow::operator=(const JudgeWindow& window) {
	if (&window != this) {
		m_marv = window.m_marv;
		m_perfect = window.m_perfect;
		m_great = window.m_great;
		m_good = window.m_good;
		m_boo = window.m_boo;
	}
	return *this;
}

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
