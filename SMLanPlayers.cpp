#include "SMLanPlayers.h"
#include "SimpleLog.h"

LanPlayer::LanPlayer(const MString &Name, const int PlayerID)
{
	m_name = Name;
	SetPlayerID(PlayerID);
	ClearStats();
	m_screenpos = 1; //we know stuff
}

void PlayerStats::SetCurrstep(StepType currstep) {
	//Does not work with version 2 and under
	//so add 2 to the step ID to account for version differences
	if (m_version <= 2)
	{
		int t = currstep + 2;
		m_currstep = (StepType)t;
	}
	else
	{
		m_currstep = (StepType)currstep;
	}
}

void PlayerStats::IncrementSteps() {
	/*	This new code needs to be tested with mines
		and all the steps including the holds and the OK
		and the NG.
	*/

	if (m_currstep <= HELD) {
		m_steps[m_currstep]++;
	}
	else
	{
		LOG->Write( "StepID is not valid!  Aborting count" );
		return;
	}

	if (m_currstep >= W2) {
		//A hold counts as two steps.
		if (m_currstep == HELD)
			m_toaststream += 2;
		else
			m_toaststream++;

		//Letting go should end the combo
		if (m_currstep == LETGO)
			m_toaststream = 0;
	} else {
		//If we miss a mine it does not affect the stream
		if (m_currstep != AVOIDMINE)
			m_toaststream = 0;
	}
	CheckToastyStream();
}

void PlayerStats::ClearStats() {
	m_diff = 0;
	m_score = 0;
	m_health = 0;
	m_feet = 0;
	m_projgrade = 7;
	m_combo = 0;
	m_currstep = UNUSED;
	m_maxCombo = 0;
	m_Grade = 0;
	m_options = "";
	memset(m_steps, 0, sizeof(int)*(HELD+1));
	m_toasty = 0;
	m_toaststream = 0;
	m_timing = false;
}

void LanPlayer::ClearStats() {
	PlayerStats::ClearStats();
	m_offset = 0;
}

void PlayerStats::CheckToastyStream() {
	if ((m_toaststream % 250) == 0) {
		m_toasty += m_toaststream/250;
		m_toaststream = 0;
	}
}

bool PlayerStats::operator<(const PlayerStats& x) const
{
	if (m_score < x.m_score)
		return true;

	return false;
}

bool PlayerStats::operator>(const PlayerStats& x) const
{
	if (m_score > x.m_score)
		return true;

	return false;
}

bool PlayerStats::operator==(const PlayerStats& x) const
{
	if (m_score == x.m_score)
		return true;

	return false;
}

/*
 * (c) 2003-2004 Joshua Allen
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

