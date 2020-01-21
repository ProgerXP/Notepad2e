#pragma once

#ifdef ICU_BUILD

#include "UTF8DocumentIterator.h"

class UTF32DocumentIterator : public std::iterator<std::bidirectional_iterator_tag, UChar32>
{
public:
	UTF32DocumentIterator() :
		m_doc(0),
		m_pos(0),
		m_end(0),
		m_characterIndex(0),
		m_utf8Length(0),
		m_utf16Length(0)
	{
	}

	UTF32DocumentIterator(Scintilla::Document* doc, int pos, int end) :
		m_doc(doc),
		m_pos(pos),
		m_end(end),
		m_characterIndex(0)
	{
		// Check for debug builds
		PLATFORM_ASSERT(m_pos <= m_end);

		// Ensure for release.
		if (m_pos > m_end)
		{
			m_pos = m_end;
		}
		readCharacter();
	}

	UTF32DocumentIterator(const UTF32DocumentIterator& copy) :
		m_doc(copy.m_doc),
		m_pos(copy.m_pos),
		m_end(copy.m_end),
		m_characterIndex(copy.m_characterIndex),
		m_utf8Length(copy.m_utf8Length),
		m_utf16Length(copy.m_utf16Length)
	{
		// Check for debug builds
		PLATFORM_ASSERT(m_pos <= m_end);
		m_character[0] = copy.m_character[0];
		m_character[1] = copy.m_character[1];

		// Ensure for release.
		if (m_pos > m_end)
		{
			m_pos = m_end;
		}
	}

	bool operator == (const UTF32DocumentIterator& other) const
	{
		return (ended() == other.ended()) && (m_doc == other.m_doc) && (m_pos == other.m_pos);
	}

	bool operator != (const UTF32DocumentIterator& other) const
	{
		return !(*this == other);
	}

	wchar_t operator * () const
	{
		return m_character[m_characterIndex];
	}

	UTF32DocumentIterator& operator = (int other)
	{
		m_pos = other;
		return *this;
	}

	UTF32DocumentIterator& operator ++ ()
	{
		PLATFORM_ASSERT(m_pos < m_end);
		if (2 == m_utf16Length && 0 == m_characterIndex)
		{
			m_characterIndex = 1;
		}
		else
		{
			m_pos += m_utf8Length;

			if (m_pos > m_end)
			{
				m_pos = m_end;
			}
			m_characterIndex = 0;
			readCharacter();
		}
		return *this;
	}

	UTF32DocumentIterator& operator -- ()
	{
		if (m_utf16Length == 2 && m_characterIndex == 1)
		{
			m_characterIndex = 0;
		}
		else
		{
			--m_pos;
			// Skip past the UTF-8 extension bytes
			while (0x80 == (m_doc->CharAt(m_pos) & 0xC0) && m_pos > 0)
				--m_pos;

			readCharacter();
			if (m_utf16Length == 2)
			{
				m_characterIndex = 1;
			}
		}
		return *this;
	}

	int pos() const
	{
		return m_pos;
	}

private:
	void readCharacter()
	{
		ReadCharacterFromUTF8(m_doc, m_pos, m_end, &m_character[0], m_characterIndex, m_utf8Length, m_utf16Length);
	}

	bool ended() const
	{
		return m_pos >= m_end;
	}

	int m_pos;
	wchar_t m_character[2];
	int m_characterIndex;
	int m_end;
	int m_utf8Length;
	int m_utf16Length;
	Scintilla::Document* m_doc;
};

#endif	// #ifdef ICU_BUILD
