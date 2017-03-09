/**
 * Copyright (c) since 2009 Simon Steele - http://untidy.net/
 * Based on the work of Simon Steele for Programmer's Notepad 2 (http://untidy.net)
 * Converted from boost::xpressive to boost::regex and performance improvements 
 * (principally caching the compiled regex), and support for UTF8 encoded text
 * (c) 2012 Dave Brotherstone - Changes for boost::regex
 * (c) 2013 Francois-R.Boyer@PolyMtl.ca - Empty match modes and best match backward search.
 * 
 */
#include <stdlib.h>
#include <iterator> 
#include <vector>
#include "scintilla.h"
#include "Platform.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "CellBuffer.h"
#include "CharClassify.h"
#include "Decoration.h"
#include "ILexer.h"
#include "CaseFolder.h"
#include "Document.h"
#include "UniConversion.h"
#include "UTF8DocumentIterator.h"
#include "AnsiDocumentIterator.h"
#include "BoostRegexSearch.h"
#include <boost/regex.hpp>
#define CP_UTF8 65001
#define SC_CP_UTF8 65001



#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

using namespace boost;

class BoostRegexSearch : public RegexSearchBase
{
public:
	BoostRegexSearch() : _substituted(NULL) {}
	
	virtual ~BoostRegexSearch()
	{
		delete[] _substituted;
		_substituted = NULL;
	}
	
	virtual long FindText(Document* doc, int startPosition, int endPosition, const char *regex,
                        bool caseSensitive, bool word, bool wordStart, int sciSearchFlags, int *lengthRet);
	
	virtual const char *SubstituteByPosition(Document* doc, const char *text, int *length);

private:
	class SearchParameters;

	class Match : private DocWatcher {
	public:
		Match() : _document(NULL), _documentModified(false), _position(-1), _endPosition(-1), _endPositionForContinuationCheck(-1)  {}
		~Match() { setDocument(NULL); }
		Match(Document* document, int position = -1, int endPosition = -1) : _document(NULL) { set(document, position, endPosition); }
		Match& operator=(Match& m) {
			set(m._document, m.position(), m.endPosition());
			return *this;
		}
		Match& operator=(int /*nullptr*/) {
			_position = -1;
			return *this;
		}
		
		void set(Document* document = NULL, int position = -1, int endPosition = -1) {
			setDocument(document);
			_position = position;
			_endPositionForContinuationCheck = _endPosition = endPosition;
			_documentModified = false;
		}
		
		bool isContinuationSearch(Document* document, int startPosition, int direction) {
			if (hasDocumentChanged(document))
				return false;
			if (direction > 0) 
				return startPosition == _endPositionForContinuationCheck;
			else
				return startPosition == _position;
		}
		bool isEmpty() {
			return _position == _endPosition;
		}
		int position() {
			return _position;
		}
		int endPosition() {
			return _endPosition;
		}
		int length() {
			return _endPosition - _position;
		}
		int found() {
			return _position >= 0;
		}
		
	private:
		bool hasDocumentChanged(Document* currentDocument) {
			return currentDocument != _document || _documentModified;
		}
		void setDocument(Document* newDocument) {
			if (newDocument != _document)
			{
				if (_document != NULL)
					_document->RemoveWatcher(this, NULL);
				_document = newDocument;
				if (_document != NULL)
					_document->AddWatcher(this, NULL);
			}
		}
		
		// DocWatcher, so we can track modifications to know if we should consider a search to be a continuation of last search:
		virtual void NotifyModified(Document* modifiedDocument, DocModification mh, void* /*userData*/)
		{
			if (modifiedDocument == _document)
			{
				if (mh.modificationType & (SC_PERFORMED_UNDO | SC_PERFORMED_REDO))
					_documentModified = true;
				// Replacing last found text should not make isContinuationSearch return false.
				else if (mh.modificationType & SC_MOD_DELETETEXT)
				{
					if (mh.position == position() && mh.length == length()) // Deleting what we last found.
						_endPositionForContinuationCheck = _position;
					else _documentModified = true;
				}
				else if (mh.modificationType & SC_MOD_INSERTTEXT)
				{
					if (mh.position == position() && position() == _endPositionForContinuationCheck) // Replace at last found position.
						_endPositionForContinuationCheck += mh.length;
					else _documentModified = true;
				}
			}
		}

		virtual void NotifyDeleted(Document* deletedDocument, void* /*userData*/)
		{
			if (deletedDocument == _document)
			{
				// We set the _document here, as we don't want to call the RemoveWatcher on this deleted document. 
				// Calling RemoveWatcher inside NotifyDeleted results in a crash, as NotifyDeleted is called whilst
				// iterating on the watchers list (since Scintilla 3.x).  Before 3.x, it was just a really bad idea.
				_document = NULL;
				set(NULL);
			}
		}
		virtual void NotifyModifyAttempt(Document* /*document*/, void* /*userData*/) {}
		virtual void NotifySavePoint(Document* /*document*/, void* /*userData*/, bool /*atSavePoint*/) {}
		virtual void NotifyStyleNeeded(Document* /*document*/, void* /*userData*/, int /*endPos*/) {}
		virtual void NotifyLexerChanged(Document* /*document*/, void* /*userData*/) {}
		virtual void NotifyErrorOccurred(Document* /*document*/, void* /*userData*/, int /*status*/) {}
		
		Document* _document;
		bool _documentModified;
		int _position, _endPosition;
		int _endPositionForContinuationCheck;
	};
	
	class CharTPtr { // Automatically translatable from utf8 to wchar_t*, if required, with allocation and deallocation on destruction; char* is not deallocated.
	public:
		CharTPtr(const char* ptr) : _charPtr(ptr), _wcharPtr(NULL) {}
		~CharTPtr() {
			delete[] _wcharPtr;
		}
		operator const char*() {
			return _charPtr;
		}
		operator const wchar_t*() {
			if (_wcharPtr == NULL)
				_wcharPtr = utf8ToWchar(_charPtr);
			return _wcharPtr;
		}
	private:
		const char* _charPtr;
		wchar_t* _wcharPtr;
	};

	template <class CharT, class CharacterIterator>
	class EncodingDependent {
	public:
		EncodingDependent() : _lastCompileFlags(-1) {}
		void compileRegex(const char *regex, const int compileFlags);
		Match FindText(SearchParameters& search);
		char *SubstituteByPosition(const char *text, int *length);
	private:
		Match FindTextImpl(SearchParameters& search);

	public:
		typedef CharT Char;
		typedef basic_regex<CharT> Regex;
		typedef match_results<CharacterIterator> MatchResults;
		
		MatchResults _match;
	private:
		Regex _regex;
		std::string _lastRegexString;
		int _lastCompileFlags;
	};
	
	class SearchParameters {
	public:
		bool isLineStart(int position);
		bool isLineEnd(int position);
		
		Document* _document;
		RESearchRange _resr;
		const char *_regexString;
		int _compileFlags;
		regex_constants::match_flag_type _boostRegexFlags;
		SearchParameters(Document* doc, int startPosition, int endPosition) : _document(doc), _resr(doc, startPosition, endPosition) {}
	};
	
	static wchar_t *utf8ToWchar(const char *utf8);
	static char    *wcharToUtf8(const wchar_t *w);
	static char    *stringToCharPtr(const std::string& str);
	static char    *stringToCharPtr(const std::wstring& str);
	
	EncodingDependent<char,    AnsiDocumentIterator> _ansi;
	EncodingDependent<wchar_t, UTF8DocumentIterator> _utf8;
	
	char *_substituted;
	
	Match _lastMatch;
	int _lastDirection;
};

#ifdef SCI_NAMESPACE
namespace Scintilla
{
#endif

#ifdef SCI_OWNREGEX
RegexSearchBase *CreateRegexSearch(CharClassify* /* charClassTable */)
{
	return new BoostRegexSearch();
}
#endif

#ifdef SCI_NAMESPACE
}
#endif

/**
 * Find text in document, supporting both forward and backward
 * searches (just pass startPosition > endPosition to do a backward search).
 */

long BoostRegexSearch::FindText(Document* doc, int startPosition, int endPosition, const char *regexString,
                        bool caseSensitive, bool /*word*/, bool /*wordStart*/, int sciSearchFlags, int *lengthRet) 
{
	try {
		SearchParameters search(doc, startPosition, endPosition);
	
		const bool isUtf8 = (doc->CodePage() == SC_CP_UTF8);
		search._compileFlags = 
			regex_constants::ECMAScript
			| (caseSensitive ? 0 : regex_constants::icase);
		search._regexString = regexString;
		
		const bool starts_at_line_start = search.isLineStart(startPosition);
		const bool ends_at_line_end     = search.isLineEnd(endPosition);
		search._boostRegexFlags = 
			  (starts_at_line_start ? regex_constants::match_default : regex_constants::match_not_bol)
			| (ends_at_line_end     ? regex_constants::match_default : regex_constants::match_not_eol)
			| ((sciSearchFlags & SCFIND_REGEXP_DOTMATCHESNL) ? regex_constants::match_default : regex_constants::match_not_dot_newline);
		
		Match match =
			isUtf8 ? _utf8.FindText(search)
			       : _ansi.FindText(search);
		
		if (match.found())
		{
			*lengthRet = match.length();
			_lastMatch = match;
			return match.position();
		}
		else
		{
			_lastMatch = NULL;
			return -1;
		}
	}

	catch(regex_error& /*ex*/)
	{
		// -1 is normally used for not found, -2 is used here for invalid regex
		return -2;
	}
}

template <class CharT, class CharacterIterator>
BoostRegexSearch::Match BoostRegexSearch::EncodingDependent<CharT, CharacterIterator>::FindText(SearchParameters& search)
{
	compileRegex(search._regexString, search._compileFlags);
	return FindTextImpl(search);
}

template <class CharT, class CharacterIterator>
BoostRegexSearch::Match BoostRegexSearch::EncodingDependent<CharT, CharacterIterator>::FindTextImpl(SearchParameters& search)
{
	bool found = false;
	// Line by line.
	for (int line = search._resr.lineRangeStart; line != search._resr.lineRangeBreak; line += search._resr.increment) {
		const Range lineRange = search._resr.LineRange(line);
		CharacterIterator itStart(search._document, lineRange.start, lineRange.end);
		CharacterIterator itEnd(search._document, lineRange.end, lineRange.end);
		search._boostRegexFlags = search.isLineStart(lineRange.start)
			? search._boostRegexFlags & ~regex_constants::match_not_bol
			: search._boostRegexFlags |  regex_constants::match_not_bol;
		found = boost::regex_search(itStart, itEnd, _match, _regex, search._boostRegexFlags);
		if (found) {
			const int  position = _match[0].first.pos();
			const int  length   = _match[0].second.pos() - position;
			if (length != 0)
				break;
		}
	}
	if (found)
		return Match(search._document, _match[0].first.pos(), _match[0].second.pos());
	else
		return Match();
}

template <class CharT, class CharacterIterator>
void BoostRegexSearch::EncodingDependent<CharT, CharacterIterator>::compileRegex(const char *regex, const int compileFlags)
{
	if (_lastCompileFlags != compileFlags || _lastRegexString != regex)
	{
		_regex = Regex(CharTPtr(regex), static_cast<regex_constants::syntax_option_type>(compileFlags));
		_lastRegexString = regex;
		_lastCompileFlags = compileFlags;
	}
}

bool BoostRegexSearch::SearchParameters::isLineStart(int position)
{
	return (position == 0)
		|| _document->CharAt(position-1) == '\n'
		|| _document->CharAt(position-1) == '\r' && _document->CharAt(position) != '\n';
}

bool BoostRegexSearch::SearchParameters::isLineEnd(int position)
{
	return (position == _document->Length())
		|| _document->CharAt(position) == '\r'
		|| _document->CharAt(position) == '\n' && (position == 0 || _document->CharAt(position-1) != '\n');
}

const char *BoostRegexSearch::SubstituteByPosition(Document* doc, const char *text, int *length) {
	delete[] _substituted;
	_substituted = (doc->CodePage() == SC_CP_UTF8)
		? _utf8.SubstituteByPosition(text, length)
		: _ansi.SubstituteByPosition(text, length);
	return _substituted;
}

template <class CharT, class CharacterIterator>
char *BoostRegexSearch::EncodingDependent<CharT, CharacterIterator>::SubstituteByPosition(const char *text, int *length) {
	char *substituted = stringToCharPtr(_match.format((const CharT*)CharTPtr(text), boost::format_all));
	*length = static_cast<int>(strlen(substituted));
	return substituted;
}

wchar_t *BoostRegexSearch::utf8ToWchar(const char *utf8)
{
	size_t utf8Size = strlen(utf8);
	size_t wcharSize = UTF16Length(utf8, utf8Size);
	wchar_t *w = new wchar_t[wcharSize + 1];
	UTF16FromUTF8(utf8, utf8Size, w, wcharSize + 1);
	w[wcharSize] = 0;
	return w;
}

char *BoostRegexSearch::wcharToUtf8(const wchar_t *w)
{
	int wcharSize = static_cast<int>(wcslen(w));
	int charSize = UTF8Length(w, wcharSize);
	char *c = new char[charSize + 1];
	UTF8FromUTF16(w, wcharSize, c, charSize);
	c[charSize] = 0;
	return c;
}

char *BoostRegexSearch::stringToCharPtr(const std::string& str)
{
	char *charPtr = new char[str.length() + 1];
	strcpy(charPtr, str.c_str());
	return charPtr;
}
char *BoostRegexSearch::stringToCharPtr(const std::wstring& str)
{
	return wcharToUtf8(str.c_str());
}