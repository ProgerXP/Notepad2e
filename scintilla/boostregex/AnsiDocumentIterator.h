#ifndef ANSIDOCUMENTITERATOR_H_12481491281240
#define ANSIDOCUMENTITERATOR_H_12481491281240

class AnsiDocumentIterator : public std::iterator<std::bidirectional_iterator_tag, char>
{
public:
	AnsiDocumentIterator() : 
		m_doc(0), 
		m_pos(0),
		m_end(0)
	{
	}

	AnsiDocumentIterator(Scintilla::Document* doc, Sci::Position pos, Sci::Position end) :
		m_doc(doc),
		m_pos(pos),
		m_end(end)
	{
		// Check for debug builds
		PLATFORM_ASSERT(m_pos <= m_end);

		// Ensure for release.
		if (m_pos > m_end)
		{
			m_pos = m_end;
		}
	}

	AnsiDocumentIterator(const AnsiDocumentIterator& copy) :
		m_doc(copy.m_doc),
		m_pos(copy.m_pos),
		m_end(copy.m_end)
	{
		// Check for debug builds
		PLATFORM_ASSERT(m_pos <= m_end);

		// Ensure for release.
		if (m_pos > m_end)
		{
			m_pos = m_end;
		}
	}

	bool operator == (const AnsiDocumentIterator& other) const
	{
		return (ended() == other.ended()) && (m_doc == other.m_doc) && (m_pos == other.m_pos);
	}

	bool operator != (const AnsiDocumentIterator& other) const
	{
		return !(*this == other);
	}

	char operator * () const
	{
		return charAt(m_pos);
	}

	AnsiDocumentIterator& operator ++ ()
	{
		PLATFORM_ASSERT(m_pos < m_end);

		m_pos++;
		return *this;
	}

	AnsiDocumentIterator& operator -- ()
	{
		m_pos--;
		return *this;
	}

	Sci::Position pos() const
	{
		return m_pos;
	}

private:
	char charAt(Sci::Position position) const
	{
		return m_doc->CharAt(position);
	}

	bool ended() const
	{
		return m_pos == m_end;
	}

	Sci::Position m_pos;
	Sci::Position m_end;
	Scintilla::Document* m_doc;
};

#endif