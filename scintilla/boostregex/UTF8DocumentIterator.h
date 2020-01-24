#pragma once

#include <iterator>
#include <memory>
#include <vector>
#include "Platform.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "Position.h"
#include "CellBuffer.h"
#include "CharClassify.h"
#include "Decoration.h"
#include <ILexer.h>
#include <ILoader.h>
#include "CaseFolder.h"
#include <Scintilla.h>
#include "../scintilla/lexlib/CharacterCategory.h"
#include <Document.h>

void ReadCharacterFromUTF8(Scintilla::Document* _doc, const Sci::Position _pos, const Sci::Position _end,
	wchar_t* _character, int& _characterIndex, int& _utf8Length, int& _utf16Length);

class UTF8DocumentIterator : public std::iterator<std::bidirectional_iterator_tag, wchar_t>
{
public:
        UTF8DocumentIterator() : 
                m_doc(0), 
                m_pos(0),
                m_end(0),
				m_characterIndex(0),
				m_utf8Length(0),
				m_utf16Length(0)
        {
        }

        UTF8DocumentIterator(Scintilla::Document* doc, Sci::Position pos, Sci::Position end) :
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

        UTF8DocumentIterator(const UTF8DocumentIterator& copy) :
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

        bool operator == (const UTF8DocumentIterator& other) const
        {
                return (ended() == other.ended()) && (m_doc == other.m_doc) && (m_pos == other.m_pos);
        }

        bool operator != (const UTF8DocumentIterator& other) const
        {
                return !(*this == other);
        }

        wchar_t operator * () const
        {
			return m_character[m_characterIndex];
        }

		UTF8DocumentIterator& operator = (int other)
		{
			m_pos = other;
			return *this;
		}

        UTF8DocumentIterator& operator ++ ()
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

        UTF8DocumentIterator& operator -- ()
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

		Sci::Position pos() const
        {
                return m_pos;
        }

private:
		void readCharacter()
		{
			ReadCharacterFromUTF8(m_doc, m_pos, m_end, m_character, m_characterIndex, m_utf8Length, m_utf16Length);
		}

        bool ended() const
        {
                return m_pos >= m_end;
        }

		Sci::Position m_pos;
		wchar_t m_character[2];
		int m_characterIndex;
		Sci::Position m_end;
		int m_utf8Length;
		int m_utf16Length;
		Scintilla::Document* m_doc;
};
