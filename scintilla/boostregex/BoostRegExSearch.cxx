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
#include <memory>
#include <vector>
#include "scintilla.h"
#include "Platform.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "Position.h"
#include "CellBuffer.h"
#include "CharClassify.h"
#include "Decoration.h"
#include "ILexer.h"
#include "ILoader.h"
#include "CaseFolder.h"
#include "../scintilla/lexlib/CharacterCategory.h"
#include "Document.h"
#include "UniConversion.h"
#include "BoostRegexSearch.h"

#ifdef ICU_BUILD
#include <boost/regex/icu.hpp>
#include "UTF32DocumentIterator.h"
#else
#include <boost/regex.hpp>
#include "UTF8DocumentIterator.h"
#include "AnsiDocumentIterator.h"
#endif

#define CP_UTF8 65001
#define SC_CP_UTF8 65001

using namespace Scintilla;
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
	
	virtual Sci::Position FindText(Document* doc, Sci::Position startPosition, Sci::Position endPosition, const char *regex,
                        bool caseSensitive, bool word, bool wordStart, int sciSearchFlags, Sci::Position *lengthRet);
	
	virtual const char *SubstituteByPosition(Document* doc, const char *text, Sci::Position *length);

private:
	class SearchParameters;

	class Match : private DocWatcher {
	public:
		Match() : _document(NULL), _documentModified(false), _position(-1), _endPosition(-1), _endPositionForContinuationCheck(-1)  {}
		~Match() { setDocument(NULL); }
		Match(Document* document, Sci::Position position = -1, Sci::Position endPosition = -1) : _document(NULL) { set(document, position, endPosition); }
		Match& operator=(const Match& m) {
			set(m._document, m.position(), m.endPosition());
			return *this;
		}
		Match& operator=(int /*nullptr*/) {
			_position = -1;
			return *this;
		}
		
		void set(Document* document = NULL, Sci::Position position = -1, Sci::Position endPosition = -1) {
			setDocument(document);
			_position = position;
			_endPositionForContinuationCheck = _endPosition = endPosition;
			_documentModified = false;
		}
		
		bool isContinuationSearch(Document* document, Sci::Position startPosition, Sci::Position direction) {
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
		Sci::Position position() const {
			return _position;
		}
		Sci::Position endPosition() const {
			return _endPosition;
		}
		Sci::Position length() {
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

		virtual void NotifyDeleted(Document* deletedDocument, void* /*userData*/) noexcept
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
		virtual void NotifyStyleNeeded(Document* /*document*/, void* /*userData*/, Sci::Position /*endPos*/) {}
		virtual void NotifyLexerChanged(Document* /*document*/, void* /*userData*/) {}
		virtual void NotifyErrorOccurred(Document* /*document*/, void* /*userData*/, int /*status*/) {}
		
		Document* _document;
		bool _documentModified;
		Sci::Position _position, _endPosition;
		Sci::Position _endPositionForContinuationCheck;
	};
	
	class CharTPtr { // Automatically translatable from utf8 to wchar_t*, if required, with allocation and deallocation on destruction; char* is not deallocated.
	public:
		CharTPtr(const char* ptr) : _charPtr(ptr),
#ifdef ICU_BUILD
			_wchar32Ptr(NULL)
#else
			_wcharPtr(NULL)
#endif
		{
		}
		~CharTPtr() {
#ifdef ICU_BUILD
			delete[] _wchar32Ptr;
#else
			delete[] _wcharPtr;
#endif
		}

#ifdef ICU_BUILD
		operator const UChar32*()
		{
			if (_wchar32Ptr == NULL)
				_wchar32Ptr = utf8ToUchar32(_charPtr);
			return _wchar32Ptr;
		}
#else
		operator const char*() {
			return _charPtr;
		}
		operator const wchar_t*() {
			if (_wcharPtr == NULL)
				_wcharPtr = utf8ToWchar(_charPtr);
			return _wcharPtr;
		}
#endif
	private:
		const char* _charPtr;
#ifdef ICU_BUILD
		UChar32* _wchar32Ptr;
#else
		wchar_t* _wcharPtr;
#endif
	};

	template <class CharT, class CharacterIterator>
	class EncodingDependent {
	public:
		EncodingDependent() : _lastCompileFlags(-1) {}
		void compileRegex(const char *regex, const int compileFlags);
		Match FindText(SearchParameters& search);
		char *SubstituteByPosition(const char *text, Sci::Position *length);
	private:
		Match FindTextImpl(SearchParameters& search);

	public:
#ifdef ICU_BUILD
		typedef CharT Char;
		typedef boost::u32regex Regex;
#else
		typedef CharT Char;
		typedef basic_regex<CharT> Regex;
#endif
		typedef match_results<CharacterIterator> MatchResults;
		
		MatchResults _match;
	private:
		Regex _regex;
		std::string _lastRegexString;
		int _lastCompileFlags;
	};
	
	class SearchParameters {
	public:
		bool isLineStart(Sci::Position position);
		bool isLineEnd(Sci::Position position);
		
		Document* _document;
		RESearchRange _resr;
		const char *_regexString;
		int _compileFlags;
		regex_constants::match_flag_type _boostRegexFlags;
		bool _reverseSearchFlag;
		SearchParameters(Document* doc, Sci::Position startPosition, Sci::Position endPosition) : _document(doc), _resr(doc, startPosition, endPosition), _reverseSearchFlag(false) {}
	};

#ifdef ICU_BUILD
	static UChar32 *utf8ToUchar32(const char *utf8);
	static char    *stringToCharPtr(const std::basic_string<UChar32, std::char_traits<UChar32>, std::allocator<UChar32>>& str, Sci::Position *lengthRet);

	EncodingDependent<UChar32, UTF32DocumentIterator> _utf32;
#else
	static wchar_t *utf8ToWchar(const char *utf8);
	static char    *stringToCharPtr(const std::string& str, Sci::Position *lengthRet);
	static char    *stringToCharPtr(const std::wstring& str, Sci::Position *lengthRet);

	EncodingDependent<char, AnsiDocumentIterator> _ansi;
	EncodingDependent<wchar_t, UTF8DocumentIterator> _utf8;
#endif
	
	char *_substituted;
	
	Match _lastMatch;
	int _lastDirection;
};

#ifdef SCI_NAMESPACE
namespace Scintilla
{
#endif

#ifdef SCI_OWNREGEX
namespace Scintilla
{
	RegexSearchBase *CreateRegexSearch(CharClassify* /* charClassTable */)
	{
		return new BoostRegexSearch();
	}
}
#endif

#ifdef SCI_NAMESPACE
}
#endif

/**
 * Find text in document, supporting both forward and backward
 * searches (just pass startPosition > endPosition to do a backward search).
 */

Sci::Position BoostRegexSearch::FindText(Document* doc, Sci::Position startPosition, Sci::Position endPosition, const char *regexString,
                        bool caseSensitive, bool /*word*/, bool /*wordStart*/, int sciSearchFlags, Sci::Position *lengthRet)
{
	try {
		SearchParameters search(doc, startPosition, endPosition);
	
		const bool isUtf8 = (doc->CodePage() == SC_CP_UTF8);
		search._compileFlags = 
			regex_constants::ECMAScript
			| (caseSensitive ? 0 : regex_constants::icase)
			| regex_constants::no_except;
		search._regexString = regexString;
		search._boostRegexFlags = regex_constants::match_default;
		
		Match match =
#ifdef ICU_BUILD
			_utf32.FindText(search);
#else
			isUtf8 ? _utf8.FindText(search)
			       : _ansi.FindText(search);
#endif
		
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
	Range lineRangeCurrent(0);
	for (Sci::Line line = search._resr.lineRangeStart; line != search._resr.lineRangeBreak; line += search._resr.increment) {
		const Range lineRange = search._resr.LineRange(line);
		lineRangeCurrent = lineRange;

		CharacterIterator itStart(search._document, lineRange.start, lineRange.end);
		CharacterIterator itEnd(search._document, lineRange.end, lineRange.end);
		search._boostRegexFlags = search.isLineStart(lineRange.start)
			? search._boostRegexFlags & ~regex_constants::match_not_bol
			: search._boostRegexFlags |  regex_constants::match_not_bol;
		try
		{
			found = boost::regex_search(itStart, itEnd, _match, _regex, search._boostRegexFlags);
		}
		catch (...)
		{
			found = false;
		}
		if (found) {
			const Sci::Position position = _match[0].first.pos();
			const Sci::Position length   = _match[0].second.pos() - position;
			break;
		}
	}
	if (found)
	{
		const Match defaultResult = Match(search._document, _match[0].first.pos(), _match[0].second.pos());
		if (search._resr.increment < 0)
		{
			if (!search._reverseSearchFlag)
			{
				SearchParameters searchNew = search;
				searchNew._reverseSearchFlag = true;
				searchNew._resr.endPos = _match[0].second.pos();
				searchNew._resr.startPos = lineRangeCurrent.end;
				searchNew._resr.lineRangeEnd = searchNew._resr.lineRangeStart;
				searchNew._resr.lineRangeBreak = searchNew._resr.lineRangeEnd + search._resr.increment;

				Match res = defaultResult;
				Match resPrev = res;
				while (!res.isEmpty())
				{
					resPrev = res;

					if (searchNew._resr.endPos >= searchNew._resr.startPos)
						break;

					res = FindTextImpl(searchNew);

					if (res.isEmpty())
						break;

					searchNew._resr.endPos = res.endPosition();
				}
				return resPrev;
			}
		}
		return defaultResult;
	}
	else
		return Match();
}

template <class CharT, class CharacterIterator>
void BoostRegexSearch::EncodingDependent<CharT, CharacterIterator>::compileRegex(const char *regex, const int compileFlags)
{
	if (_lastCompileFlags != compileFlags || _lastRegexString != regex)
	{
#ifdef ICU_BUILD
		_regex = make_u32regex(regex, static_cast<regex_constants::syntax_option_type>(compileFlags));
#else
		_regex = Regex(CharTPtr(regex), static_cast<regex_constants::syntax_option_type>(compileFlags));
#endif
		_lastRegexString = regex;
		_lastCompileFlags = compileFlags;
	}
}

bool BoostRegexSearch::SearchParameters::isLineStart(Sci::Position position)
{
	return (position == 0)
		|| _document->CharAt(position-1) == '\n'
		|| _document->CharAt(position-1) == '\r' && _document->CharAt(position) != '\n';
}

bool BoostRegexSearch::SearchParameters::isLineEnd(Sci::Position position)
{
	return (position == _document->Length())
		|| _document->CharAt(position) == '\r'
		|| _document->CharAt(position) == '\n' && (position == 0 || _document->CharAt(position-1) != '\n');
}

const char *BoostRegexSearch::SubstituteByPosition(Document* doc, const char *text, Sci::Position *length) {
	delete[] _substituted;
	_substituted = 
#ifdef ICU_BUILD
		_utf32.SubstituteByPosition(text, length);
#else
		(doc->CodePage() == SC_CP_UTF8)
		? _utf8.SubstituteByPosition(text, length)
		: _ansi.SubstituteByPosition(text, length);
#endif
	return _substituted;
}

template <class CharT, class CharacterIterator>
char *BoostRegexSearch::EncodingDependent<CharT, CharacterIterator>::SubstituteByPosition(const char *text, Sci::Position *length) {
#ifdef ICU_BUILD
	return stringToCharPtr(_match.format((const UChar32*)CharTPtr(text), boost::format_all, _regex), length);
#else
	return stringToCharPtr(_match.format((const CharT*)CharTPtr(text), boost::format_all), length);
#endif
}

#ifdef ICU_BUILD
UChar32 *BoostRegexSearch::utf8ToUchar32(const char *utf8)
{
	int len = strlen(utf8) + 1;
	UChar32 *ws = new UChar32[len];
	size_t outLen = UTF32FromUTF8(utf8, strlen(utf8), reinterpret_cast<unsigned int *>(&ws[0]), len);
	ws[outLen] = 0;
	return ws;
}

char *BoostRegexSearch::stringToCharPtr(const std::basic_string<UChar32, std::char_traits<UChar32>, std::allocator<UChar32>>& str, Sci::Position *lengthRet)
{
	std::string s;
	icu::UnicodeString us = icu::UnicodeString::fromUTF32(str.c_str(), str.length());
	us.toUTF8String(s);
	const int charSize = s.length();
	char *c = new char[charSize + 1];
	for (int i = 0; i < charSize; ++i)
	{
		c[i] = s[i];
	}
	c[charSize] = 0;
	*lengthRet = charSize;
	return c;
}

#else	// #ifdef ICU_BUILD

wchar_t *BoostRegexSearch::utf8ToWchar(const char *utf8)
{
	size_t utf8Size = strlen(utf8);
	size_t wcharSize = UTF16Length(utf8, utf8Size);
	wchar_t *w = new wchar_t[wcharSize + 1];
	UTF16FromUTF8(utf8, utf8Size, w, wcharSize + 1);
	w[wcharSize] = 0;
	return w;
}

char *BoostRegexSearch::stringToCharPtr(const std::string& str, Sci::Position *lengthRet)
{
	size_t length = str.length();
	char *charPtr = new char[length + 1];
	memcpy_s(charPtr, length, str.c_str(), length);
	*lengthRet = length;
	return charPtr;
}

char *BoostRegexSearch::stringToCharPtr(const std::wstring& str, Sci::Position *lengthRet)
{
	size_t wcharSize = str.length();
	size_t charSize = UTF8Length(str.c_str(), wcharSize, true);
	char *c = new char[charSize + 1];
	UTF8FromUTF16(str.c_str(), wcharSize, c, charSize, true);
	c[charSize] = 0;
	*lengthRet = charSize;
	return c;
}
#endif
