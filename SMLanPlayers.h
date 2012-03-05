#ifndef SMLANPLAYER_H
#define SMLANPLAYER_H

#include "MercuryString.h"
#include "JudgeWindow.h"
#include "ConstructCount.h"

enum StepType
{
	UNUSED = 0,
	HITMINE, //1
	AVOIDMINE,  //2
	MISS,	//Miss
	W5,		//Boo
	W4,		//Good
	W3,		//Great
	W2,		//Perfect
	W1,		//Marvelous
	LETGO,	//Let Go
	HELD	//OK
};

class PlayerStats
{
public:
	virtual ~PlayerStats() {};
	inline long GetScore() const { return m_score; }
	inline void SetScore(long score) { m_score = score; }
	inline int GetHealth() const { return m_health; }
	inline void SetHealth(int health) { m_health = health; }
	inline int GetFeet() const { return m_feet; }
	inline void SetFeet(int feet) { m_feet = feet; }
	inline int GetProjgrade() const { return m_projgrade; }
	inline void SetProjgrade(int projgrade) { m_projgrade = (projgrade<8)?projgrade:7; }
	inline int GetCombo() const { return m_combo; }
	inline void SetCombo(int combo) { m_combo = combo; if (m_combo > m_maxCombo) m_maxCombo = m_combo; }
	inline StepType GetCurrstep() const { return m_currstep; }
	inline int GetSteps(int index) const {	if (index <= HELD) return m_steps[index]; return 0; }
	inline int GetMaxCombo() const { return m_maxCombo; }
	inline const MString& GetOptions() const { return m_options; }
	inline void SetOptions(const MString &options) { m_options = options; }
	inline int GetDiff() const { return m_diff; }
	inline void SetDiff(int diff) { m_diff = diff; }
	inline int GetToasty() const { return m_toasty; }
	inline int GetPlayerID() const { return m_playerID; }
	inline void SetPlayerID(int ID) { m_playerID = ID; }
	inline void SetTiming(bool x) { m_timing = x; }
	inline const bool GetTiming() const { return m_timing; }
	inline void SetRank(int rank) { m_rank = rank; }
	inline int GetRank() const { return m_rank; }
	inline void SetVersion(int version) { m_version = version; }
	inline int GetVersion() { return m_version; }

	void SetCurrstep(StepType currstep);
	void IncrementSteps();
	virtual void ClearStats();

	bool operator<(const PlayerStats& x) const;
	bool operator>(const PlayerStats& x) const;
	bool operator==(const PlayerStats& x) const;
private:
	void CheckToastyStream();
	long m_score;
	int m_health;
	int m_feet;
	int m_projgrade;
	int m_combo;
	StepType m_currstep;
	int m_steps[HELD+1];
	int m_maxCombo;
	int m_Grade;
	int m_toasty;
	int m_toaststream;
	int m_diff;
	MString m_options;
	int m_playerID;
	bool m_timing;
	int m_rank;
	int m_version;
};

class LanPlayer	: public PlayerStats {
public:
  virtual ~LanPlayer() {};
	LanPlayer(const MString &Name, const int PlayerID);
	inline const MString& GetName() const { return m_name; }
	inline void SetName(const MString& name) { m_name = name; }
	inline void SetScreenPosition(int x) { m_screenpos = x; }
	inline int ScreenPosition() const { return m_screenpos; }
	virtual void ClearStats();
	inline double GetOffset() const { return m_offset; }
	inline void SetOffset(double offset) { m_offset = offset; }
private:
	MString m_name;
	double m_offset;


	int m_screenpos;
	ConstructCount m_counter;
};

#endif

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
