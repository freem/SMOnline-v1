#include "RoundInfo.h"

using std::vector;

const MString& SongInfo::GetTitle() const {
	return m_title;
}

void SongInfo::SetTitle(const MString& title) {
	m_title = title;
}

const MString& SongInfo::GetArtist() const {
	return m_artist;
}

void SongInfo::SetArtist(const MString& artist) {
	m_artist = artist;
}

const MString& SongInfo::GetSubtitle() const {
	return m_subtitle;
}

void SongInfo::SetSubtitle(const MString& subtitle) {
	m_subtitle = subtitle;
}

const vector<uint32_t>& SongInfo::GetHashes() const {
	return m_hashes;
}

vector<uint32_t>& SongInfo::GetHashes() {
	return m_hashes;
}

void SongInfo::SetHashes(const vector<uint32_t>& hashes) {
	m_hashes = hashes;
}

void SongInfo::Reset()
{
	m_title.assign("");
	m_artist.assign("");
	m_subtitle.assign("");
	m_hashes.clear();
}

bool hashListsAreSame(const vector<uint32_t>& x, const vector<uint32_t>& y)
{
	if(x.size() == y.size())
	{
		for(vector<uint32_t>::size_type xIndex = 0; xIndex < x.size(); xIndex++)
		{
			bool found = false;
			for(vector<uint32_t>::size_type yIndex = 0; yIndex < y.size(); yIndex++)
			{
				if(x[xIndex] == y[yIndex])
				{
					found = true;
					break;
				}
			}

			if(!found)
			{
				return false;
			}
		}

		return true;
	}
	else // different number of hashes, can't be the same
	{
		return false;
	}
}

bool SongInfo::operator==(const SongInfo& x) const
{
	if ((x.m_title == m_title) && (x.m_subtitle == m_subtitle) && (x.m_artist == m_artist) && hashListsAreSame(x.GetHashes(), m_hashes))
	{
		return true;
	}

	return false;
}

const MString& RoundInfo::GetCourse() const {
	return m_course;
}

void RoundInfo::SetCourse(const MString& course) {
	m_course = course;
}

const MString& RoundInfo::GetSongoptions() const {
	return m_songoptions;
}

void RoundInfo::SetSongoptions(const MString& songoptions) {
	m_songoptions = songoptions;
}

void RoundInfo::Reset()
{
	SongInfo::Reset();
	m_course.assign("");
	m_songoptions.assign("");
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
