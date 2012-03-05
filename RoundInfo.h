#ifndef ROUNDINFO_H
#define ROUNDINFO_H

#include "MercuryString.h"
#include "ConstructCount.h"
#include "arch_setup.h"
#include <vector>

class SongInfo
{
public:
  virtual ~SongInfo() {};
	const MString& GetTitle() const;
	void SetTitle(const MString& title);
	const MString& GetArtist() const;
	void SetArtist(const MString& artist);
	const MString& GetSubtitle() const;
	void SetSubtitle(const MString& subtitle);
	const std::vector<uint32_t>& GetHashes() const; // A list of hashes of the charts of the stepfile selected by the room leader
	std::vector<uint32_t>& GetHashes();
	void SetHashes(const std::vector<uint32_t>& hashes);
	virtual void Reset();
	bool operator==(const SongInfo& x) const;
private:
	MString m_title;
	MString m_artist;
	MString m_subtitle;
	std::vector<uint32_t> m_hashes; // using a vector and not, say, a set because the server has to send the hashes out in the same order they were received. This should be a class, but meh.
};

bool hashListsAreSame(const std::vector<uint32_t>& x, const std::vector<uint32_t>& y);

class RoundInfo : public SongInfo
{
public:
  virtual ~RoundInfo() {};
	const MString& GetCourse() const;
	void SetCourse(const MString& course);
	const MString& GetSongoptions() const;
	void SetSongoptions(const MString& songoptions);
	virtual void Reset();
private:
	MString m_course;
	MString m_songoptions;
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
