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
#include <iostream>
#include <memory>
#include <stdexcept>
#include <tuple>
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
#include "../lexlib/CharacterCategory.h"
#include "Document.h"
#include "UniConversion.h"
#include "BoostRegexSearch.h"
#include <boost/algorithm/string/replace.hpp>

#include <map>
#include "Selection.h"
#include "Style.h"
#include "UniqueString.h"
#include "LineMarker.h"
#include "Indicator.h"
#include "ContractionState.h"
#include "ViewStyle.h"
#include "PositionCache.h"
#include "EditModel.h"
#include "MarginView.h"
#include "EditView.h"
#include "KeyMap.h"
#include "Editor.h"

#ifdef ICU_BUILD
#include <boost/regex/icu.hpp>
#include "UTF32DocumentIterator.h"
#else
#include <boost/regex.hpp>
#include <boost/function.hpp>
#include "UTF8DocumentIterator.h"
#include "AnsiDocumentIterator.h"
#endif

#define CP_UTF8 65001
#define SC_CP_UTF8 65001

using namespace Scintilla;
using namespace boost;

class BoostRegexReplace : public RegexReplaceBase
{
public:
	BoostRegexReplace(){}
	
	virtual ~BoostRegexReplace(){}
	
	Sci::Position ReplaceText(void* editor, Document* doc, const bool regexp, Sci::Position startPosition, Sci::Position endPosition, const char *regex,
		const char* regexReplaceString, const regexReplaceFilterFunc& filterFunc, int filterMode, bool caseSensitive, bool word, bool wordStart, int sciSearchFlags, Sci::Position *lengthRet, Sci::Position* counterRet) override;

private:
	class ReplaceParameters;

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
		void ReplaceText(ReplaceParameters& search);
	private:
		void ReplaceTextImpl(ReplaceParameters& search);

	public:
#ifdef ICU_BUILD
		typedef CharT Char;
		typedef boost::u32regex Regex;
#else
		typedef CharT Char;
		typedef basic_regex<CharT> Regex;
#endif
		typedef match_results<CharacterIterator> MatchResults;
	private:
		Regex _regex;
		std::string _lastRegexString;
		int _lastCompileFlags;
	};
	
	class ReplaceParameters {
	public:
		Editor* _editor;
		Document* _document;
		int _filterMode = 0;
		regexReplaceFilterFunc _filterFunc;
		const char *_regexString;
		const char* _regexReplaceString;
		int _compileFlags;
		Sci::Position _startPosition;
		Sci::Position _endPosition;
		regex_constants::match_flag_type _boostRegexFlags;
		int _direction;
		bool _is_allowed_empty;
		bool _is_allowed_empty_at_start_position;
		bool _skip_windows_line_end_as_one_character;
		int counter = 0;
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
	
	int _lastDirection;
};

#ifdef SCI_NAMESPACE
namespace Scintilla
{
#endif

#ifdef SCI_OWNREGEX
namespace Scintilla
{
	RegexReplaceBase *CreateRegexReplace(CharClassify* /* charClassTable */)
	{
		return new BoostRegexReplace();
	}
}
#endif

#ifdef SCI_NAMESPACE
}
#endif

extern std::string g_exceptionMessage;

/**
 * Find text in document, supporting both forward and backward
 * searches (just pass startPosition > endPosition to do a backward search).
 */

Sci::Position BoostRegexReplace::ReplaceText(void* editor, Document* doc, const bool regexp, Sci::Position startPosition, Sci::Position endPosition, const char *regexString,
	const char* regexReplaceString, const regexReplaceFilterFunc& filterFunc, int filterMode, bool caseSensitive, bool word, bool wordStart, int sciSearchFlags, Sci::Position *lengthRet, Sci::Position* counterRet)
{
	g_exceptionMessage.clear();
	try {
		ReplaceParameters search{};

		search._editor = (Scintilla::Editor*)editor;
		search._document = doc;
		search._filterMode = filterMode;
		search._filterFunc = filterFunc;

		if (startPosition > endPosition
			|| (startPosition == endPosition && _lastDirection < 0))  // If we search in an empty region, suppose the direction is the same as last search (this is only important to verify if there can be an empty match in that empty region).
		{
			search._startPosition = endPosition;
			search._endPosition = startPosition;
			search._direction = -1;
		}
		else
		{
			search._startPosition = startPosition;
			search._endPosition = endPosition;
			search._direction = 1;
		}
		_lastDirection = search._direction;

		// Range endpoints should not be inside DBCS characters, but just in case, move them.
		search._startPosition = doc->MovePositionOutsideChar(search._startPosition, 1, false);
		search._endPosition = doc->MovePositionOutsideChar(search._endPosition, 1, false);

		const bool isUtf8 = (doc->CodePage() == SC_CP_UTF8);
		search._compileFlags = regex_constants::ECMAScript | (caseSensitive ? 0 : regex_constants::icase);

		std::string _regexString = regexString;
		std::string _regexReplaceString = regexReplaceString;
		if (regexp)
		{
			boost::replace_all(_regexReplaceString, "\\r", "\r");
			boost::replace_all(_regexReplaceString, "\\n", "\n");
			boost::replace_all(_regexReplaceString, "\\r\\n", "\r\n");
		}
		else
		{
			if (word)
				_regexString = "\\<" + _regexString + "\\>";
			else if (wordStart)
				_regexString = "\\<" + _regexString;
			else
				_regexString = "\\Q" + _regexString + "\\E";
		}
		search._regexString = _regexString.data();
		search._regexReplaceString = _regexReplaceString.data();
		search._boostRegexFlags = regexp
			? regex_constants::match_not_dot_newline
			: regex_constants::format_literal;

		if (regexp)
		{
			if ((search._startPosition > 0) && !doc->IsLineStartPosition(search._startPosition))
				search._boostRegexFlags |= regex_constants::match_not_bol;
			if ((search._endPosition < doc->Length()) && !doc->IsLineEndPosition(search._endPosition))
				search._boostRegexFlags |= regex_constants::match_not_eol;
		}

#ifdef ICU_BUILD
			_utf32.ReplaceText(search);
#else
		isUtf8 ? _utf8.ReplaceText(search)
		       : _ansi.ReplaceText(search);
#endif

		*counterRet = search.counter;
		return 0;
	}

	catch (regex_error& ex)
	{
		// -1 is normally used for not found, -2 is used here for invalid regex
		g_exceptionMessage = ex.what();
		return -2;
	}

	catch (...)
	{
		g_exceptionMessage = "Unexpected exception while searching";
		return -3;
	}
}

template <class CharT, class CharacterIterator>
void BoostRegexReplace::EncodingDependent<CharT, CharacterIterator>::ReplaceText(ReplaceParameters& search)
{
	compileRegex(search._regexString, search._compileFlags);
	return ReplaceTextImpl(search);
}

template <class Container, class CharT, class String>
class MyOutIterator
{
public:
	Container* m_container = nullptr;
	Sci::Position m_pos = 0;

	MyOutIterator(Container* _container, const Sci::Position& _pos)
		: m_container(_container), m_pos(_pos){}

	MyOutIterator& operator=(const MyOutIterator& _right)
	{
		m_container = _right.m_container;
		m_pos = _right.m_pos;
		return *this;
	}
	MyOutIterator& operator*()
	{
		return *this;
	}
	MyOutIterator& operator++(int)
	{
		m_pos++;
		return *this;
	}
	MyOutIterator& operator--(int)
	{
		m_pos++;
		return *this;
	}
	MyOutIterator& operator=(CharT c)
	{
		return *this;
	}
	void save(const Sci::Position& posFrom, const Sci::Position& posTo, const String& to)
	{
		m_container->save(m_pos + posFrom, m_pos + posTo, to);
	}
};

template <class CharT, class String>
struct ReplaceData
{
	std::map<size_t, String> m_replacements;
	std::list<std::tuple<Sci::Position, Sci::Position, size_t>> m_replacementPositions;

	virtual ~ReplaceData()
	{
		m_replacements.clear();
	}
	MyOutIterator<ReplaceData, CharT, String> begin(const Sci::Position& pos)
	{
		return MyOutIterator<ReplaceData, CharT, String>(this, pos);
	}
	void save(const Sci::Position& posFrom, const Sci::Position& posTo, const String& to)
	{
		const auto hash = std::hash<String>{}(to);
		if (m_replacements.find(hash) == m_replacements.cend())
			m_replacements[hash] = to;
		m_replacementPositions.push_back(std::make_tuple(posFrom, posTo, hash));
	}
};

template <class CharT, class CharTPtr, class Match, class String, class Container>
struct CRegexCustomFormatter
{
#ifdef ICU_BUILD
	typedef CharT Char;
	typedef boost::u32regex Regex;
#else
	typedef CharT Char;
	typedef basic_regex<CharT> Regex;
#endif

	Editor* m_editor = nullptr;
	Document* m_document = nullptr;
	int m_searchMode = 0;
	regexReplaceFilterFunc m_filterFunc;

	String m_replacement;
	std::map<String, int> m_captures;

	CRegexCustomFormatter(Editor* editor, Document* document, const int searchMode, const regexReplaceFilterFunc& filterFunc, const std::string& replacement, const bool plainFormat)
		: m_editor(editor), m_document(document), m_searchMode(searchMode), m_filterFunc(filterFunc)
	{
		m_replacement = String(replacement.begin(), replacement.end());
		if (plainFormat)
			return;
		Regex expression(CharTPtr("[\\\\$](\\d+)"));
		boost::smatch what;
		auto start = replacement.begin();
		auto end = replacement.end();
		while (boost::regex_search(start, end, what, expression))
		{
			m_captures[String(what[0].begin(), what[0].end())] = std::stoi(what[1]);
			start = what[0].second;
		}
		int actualCaptureIndex = 0;
		for (const auto& k : m_captures)
			m_captures[k.first] = actualCaptureIndex++;
	}
	
	MyOutIterator<Container, CharT, String> operator()(const Match& m, MyOutIterator<Container, CharT, String> it) const
	{
		const int pos = m[0].first.pos();
		if (!m_filterFunc(m_editor, pos, m_searchMode))
			return it;

		if (!m_captures.empty())
		{
			String res(m_replacement.begin(), m_replacement.end());
			for (const auto& cap : m_captures)
			{
				const auto& capturedValue = m[cap.second].str();
				size_t pos = res.find(cap.first);
				while (pos != std::string::npos)
				{
					res.replace(pos, cap.first.length(), capturedValue);
					pos = res.find(cap.first, pos);
				}
			}
			it.save(pos, pos + m.length(), res);
			return it;
		}
		it.save(pos, pos + m.length(), m_replacement);
		return it;
	}
};

template <class CharT, class CharacterIterator>
void BoostRegexReplace::EncodingDependent<CharT, CharacterIterator>::ReplaceTextImpl(ReplaceParameters& search)
{
	CharacterIterator endIterator(search._document, search._endPosition, search._endPosition);
	CharacterIterator baseIterator(search._document, 0, search._endPosition);
	Sci::Position next_search_from_position = search._startPosition;

	using match_type = boost::match_results<CharacterIterator>;
	using string_type = std::basic_string<CharT>;

	ReplaceData<CharT, string_type> replaceData;
	CRegexCustomFormatter<CharT, CharTPtr, match_type, string_type, ReplaceData<CharT, string_type>>
		formatter(search._editor, search._document, search._filterMode, search._filterFunc, search._regexReplaceString, search._boostRegexFlags == regex_constants::format_literal);

	auto doc = search._document;

	boost::regex_replace(
		replaceData.begin(search._startPosition),
		CharacterIterator(doc, next_search_from_position, search._endPosition),
		endIterator,
		_regex,
		formatter,
		search._boostRegexFlags | boost::regex_constants::format_no_copy);

	doc->BeginUndoAction();
	Sci::Position offset = 0;
	for (const auto& t : replaceData.m_replacementPositions)
	{
		const auto& posFrom = std::get<0>(t);
		const auto& posTo = std::get<1>(t);
		const auto& hash = std::get<2>(t);
		const auto& stringTo = replaceData.m_replacements[hash];
		const auto targetStart = offset + posFrom;
		const auto targetEnd = offset + posTo;
		search._editor->WndProc(SCI_SETTARGETSTART, targetStart, 0);
		search._editor->WndProc(SCI_SETTARGETEND, targetEnd, 0);
		search._editor->WndProc(SCI_REPLACETARGET, -1, (sptr_t)stringTo.c_str());
		offset += stringTo.length() - (posTo - posFrom);
	}
	doc->EndUndoAction();

	search.counter = replaceData.m_replacementPositions.size();
	return;
}

template <class CharT, class CharacterIterator>
void BoostRegexReplace::EncodingDependent<CharT, CharacterIterator>::compileRegex(const char *regex, const int compileFlags)
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

#ifdef ICU_BUILD
UChar32 *BoostRegexReplace::utf8ToUchar32(const char *utf8)
{
	int len = strlen(utf8) + 1;
	UChar32 *ws = new UChar32[len];
	size_t outLen = UTF32FromUTF8(utf8, strlen(utf8), reinterpret_cast<unsigned int *>(&ws[0]), len);
	ws[outLen] = 0;
	return ws;
}

char *BoostRegexReplace::stringToCharPtr(const std::basic_string<UChar32, std::char_traits<UChar32>, std::allocator<UChar32>>& str, Sci::Position *lengthRet)
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

wchar_t *BoostRegexReplace::utf8ToWchar(const char *utf8)
{
	size_t utf8Size = strlen(utf8);
	size_t wcharSize = UTF16Length(utf8, utf8Size);
	wchar_t *w = new wchar_t[wcharSize + 1];
	UTF16FromUTF8(utf8, utf8Size, w, wcharSize + 1);
	w[wcharSize] = 0;
	return w;
}

char *BoostRegexReplace::stringToCharPtr(const std::string& str, Sci::Position *lengthRet)
{
	size_t length = str.length();
	char *charPtr = new char[length + 1];
	memcpy_s(charPtr, length, str.c_str(), length);
	*lengthRet = length;
	return charPtr;
}

char *BoostRegexReplace::stringToCharPtr(const std::wstring& str, Sci::Position *lengthRet)
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
