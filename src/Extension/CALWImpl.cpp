#include "CALWImpl.h"
#include <assert.h>
#include <algorithm>
#include <cctype>
#include <locale>

#ifdef __cplusplus
extern "C" {
#endif

#include "StringRecoding.h"
#include "Externals.h"
#include "LexerUtils.h"

#ifdef __cplusplus
}; // extern "C"
#endif

#include "Scintilla.h"

#ifdef __cplusplus
extern "C" {
#endif

  static unsigned char CHAR_SPACE = ' ';
  static unsigned char CHAR_EOL_R = '\r';
  static unsigned char CHAR_EOL_N = '\n';
  static unsigned char CHAR_FORCE_EOL = '\a';
  static unsigned char CHAR_FORCE_EOL_PROCESSED = '\b';
  static unsigned char CHAR_NEXT_PARAGRAPH = '\f';

  static LPCSTR lpstrWhiteSpaces = " \t";
  static LPCSTR lpstrWhiteSpacesAndEOLs = " \t\r\n";
  static LPCSTR lpstrStaticMarkerChars = ">=?*";        // #TODO: removed # (comment line in perl)
  static LPCSTR lpstrDynamicMarkerChars = ":).";
  static LPCSTR lpstrDigits = "0123456789";

  BOOL isStaticMarker(const unsigned char ch)
  {
    return IsCharFromString(lpstrStaticMarkerChars, ch);
  }

  BOOL isMarker(const unsigned char ch, EncodingData* pED)
  {
    const BOOL isDynamicMarker = TextBuffer_IsAnyCharAtPos_IgnoreSpecial(&pED->m_tb, lpstrDynamicMarkerChars, lpstrDigits, 0);
    return isStaticMarker(ch) || isDynamicMarker;
  }

  bool Prefix::IsPlain() const
  {
    return m_type == PrefixType::Plain;
  }

  bool Prefix::IsMarker() const
  {
    return IsMarkerStatic() || IsMarkerDynamic();
  }

  bool Prefix::IsMarkerStatic() const
  {
    return m_type == PrefixType::MarkerStatic;
  }

  bool Prefix::IsMarkerDynamic() const
  {
    return m_type == PrefixType::MarkerDynamic;
  }

  bool Prefix::IsComment() const
  {
    return m_type == PrefixType::Comment;
  }

  bool Prefix::IsCommentedMarker() const
  {
    return IsComment() && (GetLength() > 0) && isStaticMarker(*(m_data.rbegin() + CountTrailingWhiteSpaces()));
  }

  bool Prefix::IsEmpty() const
  {
    return m_data.size() == 0;
  }

  bool Prefix::IsInitialized() const
  {
    return m_isInitialized;
  }

  void Prefix::SetInitialized()
  {
    m_isInitialized = true;
  }

  void Prefix::Init(const Prefix& p)
  {
    SetString(p.GetString());
    SetType(p.GetType());
    SetInitialized();
  }

  PrefixType Prefix::GetType() const
  {
    return m_type;
  }

  void Prefix::SetType(const PrefixType type)
  {
    m_type = type;
    initWhitespace();
  }

  int Prefix::CountTrailingWhiteSpaces() const
  {
    int res = 0;
    int pos = m_data.size() - 1;
    while (pos > 0)
    {
      if (!IsCharFromString(lpstrWhiteSpaces, m_data.at(pos)))
      {
        break;
      }
      --pos;
      ++res;
    }
    return res;
  }

  void Prefix::initWhitespace()
  {
    const auto posWhitespace = m_data.find_first_of(lpstrWhiteSpaces);
    m_charWhitespace = (posWhitespace != std::string::npos) ? m_data.at(posWhitespace) : CHAR_SPACE;

    if (IsMarkerStatic())
    {
      const auto posMarker = m_data.find_first_of(lpstrStaticMarkerChars);
      if (posMarker != std::string::npos)
      {
        m_rangeMarker.pos1 = posMarker;
        m_rangeMarker.pos2 = m_rangeMarker.pos1 + 1;
      }
    }
    else if (IsMarkerDynamic())
    {
      const auto posMarkerStart = m_data.find_first_of(lpstrDigits, posWhitespace);
      const auto posMarkerEnd = m_data.find_first_of(lpstrDynamicMarkerChars, posWhitespace);
      if ((posMarkerStart != std::string::npos) && (posMarkerEnd != std::string::npos))
      {
        m_rangeMarker.pos1 = posMarkerStart;
        m_rangeMarker.pos2 = posMarkerEnd + 1;
      }
    }
  }

  std::string Prefix::GetString() const
  {
    return m_data;
  }

  void Prefix::SetString(const std::string s)
  {
    m_data = s;
    initWhitespace();
  }

  void Prefix::PushChar(const unsigned char ch)
  {
    m_data.push_back(ch);
    initWhitespace();
  }

  unsigned char Prefix::GetChar(const std::size_t pos, const int iLineIndex)
  {
    if (IsMarkerStatic() || IsCommentedMarker())
    {
      return (iLineIndex != 0) && m_rangeMarker.check(pos)
            ? CHAR_SPACE
            : m_data.at(pos);
    }
    else if (IsComment())
    {
      return m_data.at(pos);
    }
    else
    {
      return (iLineIndex != 0) ? m_charWhitespace : m_data.at(pos);
    }
  }

  int Prefix::GetLength() const
  {
    return m_data.size();
  }

  void Prefix::rtrim()
  {
    m_data.erase(std::find_if(m_data.rbegin(), m_data.rend(), [](char ch)
    {
      return !std::isspace(ch);
    }).base(), m_data.end());
  }

  std::shared_ptr<Prefix> Paragraph::createPrefix()
  {
    return std::make_shared<Prefix>(Prefix());
  }

  Paragraph::Paragraph() : prefix(createPrefix()){}

  BOOL CALWData::updateCharsProcessed(long* piCharsProcessed, const int iCharCount) const
  {
    if (piCharsProcessed)
    {
      (*piCharsProcessed) += iCharCount;
    }
    return TRUE;
  }

  BOOL CALWData::GetTrailingEOLLength() const
  {
    switch (iEOLMode)
    {
    case SC_EOL_CRLF:
      return 1;
    case SC_EOL_LF:
    case SC_EOL_CR:
      return 0;
    }
    assert(0);
    return 0;
  }

  std::shared_ptr<Paragraph> CALWData::addParagraph()
  {
    m_paragraphs.emplace_back(new Paragraph);
    m_iActiveParagraphIndex = m_paragraphs.size() - 1;
    return *m_paragraphs.rbegin();
  }

  std::shared_ptr<Paragraph> CALWData::nextParagraph()
  {
    ++m_iActiveParagraphIndex;
    if (m_iActiveParagraphIndex < m_paragraphs.size())
    {
      return m_paragraphs.at(m_iActiveParagraphIndex);
    }
    else
    {
      m_iActiveParagraphIndex = -1;
      return nullptr;
    }
  }

  std::shared_ptr<const Paragraph> CALWData::prevParagraph() const
  {
    if (m_iActiveParagraphIndex - 1 >= 0)
    {
      return m_paragraphs.at(m_iActiveParagraphIndex - 1);
    }
    else
    {
      return nullptr;
    }
  }

  CALWData::CALWData(const int iAdditionalData1, const int iAdditionalData2, const int iAdditionalData3)
  {
    longLineLimit = iAdditionalData1;
    lexerId = iAdditionalData2;
    iEOLMode = iAdditionalData3;
    iTrailingEOLLength = GetTrailingEOLLength();
    iSingleLineCommentPrefixLength = n2e_GetSingleLineCommentPrefixLength(lexerId);
    for (int i = 0; i < 3; ++i)
    {
      ps[i].relativeLineIndexPrefixProcessed = -1;
    }
    m_cp = addParagraph();
  }

  BOOL CALWData::IsEOL(const unsigned char ch) const
  {
    if ((ch == CHAR_FORCE_EOL) || (ch == CHAR_FORCE_EOL_PROCESSED))
      return TRUE;

    switch (iEOLMode)
    {
    case SC_EOL_CRLF:
      return ch == CHAR_EOL_N;
    case SC_EOL_LF:
      return ch == CHAR_EOL_N;
    case SC_EOL_CR:
      return ch == CHAR_EOL_R;
    }
    assert(0);
    return FALSE;
  }

  void CALWData::InitPass(LPVOID pRA, const int /*iPassIndex*/)
  {
    m_iActiveParagraphIndex = 0;
    m_cp = (m_paragraphs.size() > 0) ? *m_paragraphs.cbegin() : nullptr;
  }

  BOOL CALWData::RunPass0(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
  {
    const unsigned char ch = TextBuffer_PopChar(&pED->m_tb);
    int iCharCount = 1 + (!IsTrailingEOL(iEOLMode, ch, &pED->m_tb) ? TextBuffer_GetCharSequenceLength(&pED->m_tb, ch, 0) : 0);
    BOOL skipChars = FALSE;

    if (!skipChars)
    {
      for (int i = 0; i < iCharCount; ++i)
      {
        TextBuffer_PushChar(&pED->m_tbRes, ch);
      }
      TextBuffer_OffsetPos(&pED->m_tb, iCharCount - 1);
    }

    if (IsTrailingEOL(iEOLMode, ch, &pED->m_tb) && !skipChars)
    {
      ++ps[pRA->iPassIndex].relativeLineIndex;
    }

    return updateCharsProcessed(piCharsProcessed, iCharCount);
  }

  std::string CALWData::readLinePrefix(EncodingData* pED, const char ch, const int count, bool& isCommentLine) const
  {
    std::string res;

    const BOOL isWhiteSpace = IsCharFromString(lpstrWhiteSpaces, ch);
    const int iCommentOffset = count - 1 + (isWhiteSpace ? iSingleLineCommentPrefixLength : 0);
    isCommentLine = n2e_IsSingleLineCommentStyleAtPos(NULL, lexerId, iCommentOffset, pED);

    res += ch;
    if (isCommentLine)
    {
      const int iWhiteSpacesAfterComment = TextBuffer_CountWhiteSpaces(&pED->m_tb, iCommentOffset);
      for (int i = 0; i < iCommentOffset + iWhiteSpacesAfterComment; ++i)
      {
        res += TextBuffer_PopChar(&pED->m_tb);
      }
    }
    else
    {
      const int iCount = TextBuffer_CountWhiteSpaces(&pED->m_tb, 0);
      for (int i = 0; i < iCount; ++i)
      {
        res += TextBuffer_PopChar(&pED->m_tb);
      }
    }
    return res;
  }

  bool CALWData::saveStaticMarkerPrefix(EncodingData* pED, int& iCharsProcessed)
  {
    const int iWhiteSpacesBeforeMarker = TextBuffer_CountWhiteSpaces(&pED->m_tb, 0);
    const auto res = isStaticMarker(TextBuffer_GetCharAt(&pED->m_tb, iWhiteSpacesBeforeMarker));
    if (res)
    {
      m_cp->prefix->SetType(PrefixType::MarkerStatic);
      for (int i = 0; i < iWhiteSpacesBeforeMarker; ++i)
      {
        m_cp->prefix->PushChar(TextBuffer_PopChar(&pED->m_tb));
      }
      m_cp->prefix->PushChar(TextBuffer_PopChar(&pED->m_tb));
      const int iWhiteSpacesAfterMarker = TextBuffer_CountWhiteSpaces(&pED->m_tb, 0);
      for (int i = 0; i < iWhiteSpacesAfterMarker; ++i)
      {
        m_cp->prefix->PushChar(TextBuffer_PopChar(&pED->m_tb));
      }
      m_cp->prefix->SetInitialized();
      iCharsProcessed = m_cp->prefix->GetLength();
    }
    return res;
  }

  bool CALWData::saveDynamicMarkerPrefix(EncodingData* pED, int& iCharsProcessed)
  {
    const int iWhiteSpacesBeforeMarker = TextBuffer_CountWhiteSpaces(&pED->m_tb, 0);
    const auto res = TextBuffer_IsAnyCharAtPos_IgnoreSpecial(&pED->m_tb, lpstrDynamicMarkerChars, lpstrDigits, iWhiteSpacesBeforeMarker);
    if (res)
    {
      m_cp->prefix->SetType(PrefixType::MarkerDynamic);
      for (int i = 0; i < iWhiteSpacesBeforeMarker; ++i)
      {
        m_cp->prefix->PushChar(TextBuffer_PopChar(&pED->m_tb));
      }
      while (IsCharFromString(lpstrDigits, TextBuffer_GetChar(&pED->m_tb))
        || IsCharFromString(lpstrDynamicMarkerChars, TextBuffer_GetChar(&pED->m_tb)))
      {
        m_cp->prefix->PushChar(TextBuffer_PopChar(&pED->m_tb));
      }
      const int iWhiteSpacesAfterMarker = TextBuffer_CountWhiteSpaces(&pED->m_tb, 0);
      for (int i = 0; i < iWhiteSpacesAfterMarker; ++i)
      {
        m_cp->prefix->PushChar(TextBuffer_PopChar(&pED->m_tb));
      }
      m_cp->prefix->SetInitialized();
      iCharsProcessed = m_cp->prefix->GetLength();
    }
    return res;
  }

  bool CALWData::savePlainPrefix(EncodingData* pED, const char ch, const int iCharCount, int& iCharsProcessed)
  {
    bool res = IsCharFromString(lpstrWhiteSpaces, ch);
    if (res)
    {
      m_cp->prefix->SetType(PrefixType::Plain);
      for (int i = 0; i < iCharCount; ++i)
      {
        m_cp->prefix->PushChar(ch);
      }
      TextBuffer_OffsetPos(&pED->m_tb, iCharCount);
      m_cp->prefix->SetInitialized();
      iCharsProcessed = m_cp->prefix->GetLength();
    }
    return res;
  }

  bool CALWData::isCommentStyleOnNextLine(EncodingData* pED, int& iCharsProcessed) const
  {
    char ch = TextBuffer_GetChar(&pED->m_tb);
    int iCharCount = TextBuffer_GetCharSequenceLength(&pED->m_tb, ch, 0);
    if (GetTrailingEOLLength() > 0)
    {
      ch = TextBuffer_GetCharAt(&pED->m_tb, 1);
      iCharCount = TextBuffer_GetCharSequenceLength(&pED->m_tb, ch, 1);
    }
    const BOOL isWhiteSpace = IsCharFromString(lpstrWhiteSpaces, ch);
    const int iCommentOffset = GetTrailingEOLLength() + iCharCount + (isWhiteSpace ? iSingleLineCommentPrefixLength : 0);
    const bool res = (n2e_IsSingleLineCommentStyleAtPos(NULL, lexerId, iCommentOffset, pED));
    if (res)
    {
      const int iWhiteSpacesAfterComment = TextBuffer_CountWhiteSpaces(&pED->m_tb, iCommentOffset);
      iCharsProcessed += iCommentOffset + iWhiteSpacesAfterComment;
    }
    return res;
  }

  bool CALWData::isStaticMarkerOnNextLine(EncodingData* pED, const bool isCurrentPrefixMarker, int& iCharsProcessed) const
  {
    int iOffset = GetTrailingEOLLength();
    iCharsProcessed = iOffset + TextBuffer_CountWhiteSpaces(&pED->m_tb, iOffset);
    bool res = isStaticMarker(TextBuffer_GetCharAt(&pED->m_tb, iCharsProcessed));
    if (res && isCurrentPrefixMarker)
    {
      iCharsProcessed += 1; // static marker itself
      iCharsProcessed += TextBuffer_CountWhiteSpaces(&pED->m_tb, iCharsProcessed);
    }
    return res;
  }

  bool CALWData::saveCommentPrefix(EncodingData* pED, const char ch, const int iCharCount, int& iCharsProcessed)
  {
    const BOOL isWhiteSpace = IsCharFromString(lpstrWhiteSpaces, ch);
    const int iCommentOffset = iCharCount + (isWhiteSpace ? iSingleLineCommentPrefixLength : 0);
    const BOOL isSingleLineComment = n2e_IsSingleLineCommentStyleAtPos(NULL, lexerId, iCommentOffset, pED);
    const BOOL isEmptyLine = isSingleLineComment
      ? TextBuffer_IsWhiteSpaceLine(&pED->m_tb, iCommentOffset, NULL)
      : FALSE;
    const auto res = isSingleLineComment;
    if (res)
    {
      const int iWhiteSpacesAfterComment = TextBuffer_CountWhiteSpaces(&pED->m_tb, iCommentOffset);
      for (int i = 0; i < iCommentOffset + iWhiteSpacesAfterComment; ++i)
      {
        m_cp->prefix->PushChar(TextBuffer_PopChar(&pED->m_tb));
      }
      saveStaticMarkerPrefix(pED, iCharsProcessed);
      m_cp->prefix->SetType(PrefixType::Comment);
      if ((TextBuffer_GetTailLength(&pED->m_tb) == 0))
      {
        m_cp->prefix->rtrim();
      }
      m_cp->prefix->SetInitialized();
      iCharsProcessed = m_cp->prefix->GetLength();
    }
    return res;
  }

  typedef std::pair<bool, int> TestLineResult;

  BOOL CALWData::RunPass1(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
  {
    BOOL skipChars = FALSE;
    const unsigned char ch = TextBuffer_GetChar(&pED->m_tb);
    int iCharCount = (!IsTrailingEOL(iEOLMode, ch, &pED->m_tb) ? TextBuffer_GetCharSequenceLength(&pED->m_tb, ch, 0) : 0);
    int iCharsProcessed = iCharCount;
    if (!m_cp->prefix->IsInitialized())
    {
      skipChars = saveCommentPrefix(pED, ch, iCharCount, iCharsProcessed)
        || saveStaticMarkerPrefix(pED, iCharsProcessed)
        || saveDynamicMarkerPrefix(pED, iCharsProcessed)
        || savePlainPrefix(pED, ch, iCharCount, iCharsProcessed);

      iCharCount = m_cp->prefix->GetLength();
      m_cp->prefix->SetInitialized();
      ps[pRA->iPassIndex].relativeLineIndexPrefixProcessed = ps[pRA->iPassIndex].relativeLineIndex;
    }
    if (!skipChars)
    {
      TextBuffer_OffsetPos(&pED->m_tb, 1);
      if (iCharCount == 0)
      {
        iCharsProcessed = 1;
        iCharCount = 1;
      }
    }

    if (IsEOLChar(ch))
    {
      ++ps[pRA->iPassIndex].relativeLineIndex;

      TestLineResult _isCommentStyleOnNextLine;
      _isCommentStyleOnNextLine.first = isCommentStyleOnNextLine(pED, _isCommentStyleOnNextLine.second);
      TestLineResult _isStaticMarkerOnNextLine;
      _isStaticMarkerOnNextLine.first = isStaticMarkerOnNextLine(pED, m_cp->prefix->IsMarker(), _isStaticMarkerOnNextLine.second);
      TestLineResult _isEmptyNextLine;
      _isEmptyNextLine.first = TextBuffer_IsWhiteSpaceLine(&pED->m_tb, GetTrailingEOLLength(), &_isEmptyNextLine.second);
      if (_isEmptyNextLine.first)
      {
        _isEmptyNextLine.second += GetTrailingEOLLength();
      }

      if (_isCommentStyleOnNextLine.first
        && m_cp->prefix->IsComment()
        && !m_cp->prefix->IsCommentedMarker())
      {
        iCharsProcessed += _isCommentStyleOnNextLine.second;
        TextBuffer_OffsetPos(&pED->m_tb, _isCommentStyleOnNextLine.second);

        if (TextBuffer_IsWhiteSpaceLine(&pED->m_tb, 0, NULL))
        {
          TextBuffer_PushChar(&pED->m_tbRes, CHAR_FORCE_EOL_PROCESSED);
          if (TextBuffer_GetTailLength(&pED->m_tb) == 0)
            TextBuffer_PushChar(&pED->m_tbRes, CHAR_FORCE_EOL_PROCESSED);
        }
        else if (isMarker(TextBuffer_GetChar(&pED->m_tb), pED))
        {
          TextBuffer_PushChar(&pED->m_tbRes, CHAR_FORCE_EOL_PROCESSED);
        }
        else if (TextBuffer_GetHeadLength(&pED->m_tbRes) > 0)
        {
          if (TextBuffer_GetCharAt(&pED->m_tbRes, -1) != CHAR_SPACE)
          {
            TextBuffer_PushChar(&pED->m_tbRes,
              IsEOL(TextBuffer_GetCharAt(&pED->m_tbRes, -1))
              ? CHAR_FORCE_EOL_PROCESSED
              : CHAR_SPACE);
          }
        }
      }
      else if (!_isCommentStyleOnNextLine.first
        && m_cp->prefix->IsPlain()
        && TextBuffer_IsWhiteSpaceLine(&pED->m_tb, GetTrailingEOLLength(), &_isCommentStyleOnNextLine.second))
      {
        const int iSkippedChars = GetTrailingEOLLength() + _isCommentStyleOnNextLine.second;
        iCharsProcessed += 1 + iSkippedChars;
        TextBuffer_OffsetPos(&pED->m_tb, iSkippedChars);

        TextBuffer_PushChar(&pED->m_tbRes, CHAR_FORCE_EOL);
      }
      else if (_isStaticMarkerOnNextLine.first)
      {
        if (m_cp->prefix->IsMarker() && (_isStaticMarkerOnNextLine.second == m_cp->prefix->GetLength()))
        {
          iCharsProcessed += _isStaticMarkerOnNextLine.second;
          TextBuffer_OffsetPos(&pED->m_tb, _isStaticMarkerOnNextLine.second);

          TextBuffer_PushChar(&pED->m_tbRes, CHAR_FORCE_EOL);
        }
        else
        {
          const auto prevPrefix(prevParagraph() ? prevParagraph()->prefix : nullptr);
          if (m_cp->prefix->IsMarker() && prevPrefix && prevPrefix->IsMarker() && (_isStaticMarkerOnNextLine.second < m_cp->prefix->GetLength()))
          {
            iCharsProcessed += _isStaticMarkerOnNextLine.second - 1;
            TextBuffer_OffsetPos(&pED->m_tb, _isStaticMarkerOnNextLine.second);
            m_cp = addParagraph();
            m_cp->prefix->Init(*prevPrefix);
          }
          else
          {
            const int iSkippedChars = IsEOL(TextBuffer_GetChar(&pED->m_tb)) ? GetTrailingEOLLength() : 0;
            iCharsProcessed += 1 + iSkippedChars;
            TextBuffer_OffsetPos(&pED->m_tb, iSkippedChars);
            m_cp = addParagraph();
          }
          TextBuffer_PushChar(&pED->m_tbRes, CHAR_NEXT_PARAGRAPH);
        }          
      }
      else if (TextBuffer_GetCharAt(&pED->m_tbRes, -1) == CHAR_FORCE_EOL)
      {
        const int iSkippedChars = GetTrailingEOLLength();
        iCharsProcessed += 1 + iSkippedChars;
        TextBuffer_OffsetPos(&pED->m_tb, iSkippedChars);

        m_cp = addParagraph();
        TextBuffer_PushChar(&pED->m_tbRes, CHAR_NEXT_PARAGRAPH);
      }
      else if (TextBuffer_GetTailLength(&pED->m_tb) > 0)
      {
        if (!m_cp->prefix->IsEmpty() && _isEmptyNextLine.first)
        {
          const int iSkippedChars = 1 + _isEmptyNextLine.second;
          iCharsProcessed += iSkippedChars;
          TextBuffer_OffsetPos(&pED->m_tb, iSkippedChars);

          m_cp = addParagraph();
          TextBuffer_PushChar(&pED->m_tbRes, CHAR_NEXT_PARAGRAPH);
        }
        else if (!_isCommentStyleOnNextLine.first && !_isStaticMarkerOnNextLine.first && !_isEmptyNextLine.first
          && TextBuffer_GetCharAt(&pED->m_tbRes, -1) != CHAR_NEXT_PARAGRAPH)
        {
          const int iSkippedChars = GetTrailingEOLLength();
          iCharsProcessed += 1 + iSkippedChars;
          TextBuffer_OffsetPos(&pED->m_tb, iSkippedChars);

          const int iSkippedWhitespaces = TextBuffer_CountWhiteSpaces(&pED->m_tb, 0);
          iCharsProcessed += iSkippedWhitespaces;
          TextBuffer_OffsetPos(&pED->m_tb, iSkippedWhitespaces);            

          TextBuffer_PushChar(&pED->m_tbRes, CHAR_SPACE);
        }
        else if (!m_cp->prefix->IsEmpty())
        {
          const int iSkippedChars = GetTrailingEOLLength();
          iCharsProcessed += iSkippedChars;
          TextBuffer_OffsetPos(&pED->m_tb, iSkippedChars);

          m_cp = addParagraph();
          TextBuffer_PushChar(&pED->m_tbRes, CHAR_NEXT_PARAGRAPH);
        }
        else
        {
          TextBuffer_PushChar(&pED->m_tbRes, CHAR_FORCE_EOL_PROCESSED);
        }
      }
      else
      {
        TextBuffer_PushChar(&pED->m_tbRes, ch);
      }

      skipChars = TRUE;
    }
    else if (isMarker(ch, pED))
    {
      previosLineUseMarker = TRUE;
    }

    if (!skipChars)
    {
      for (int i = 0; i < iCharCount; ++i)
      {
        TextBuffer_PushChar(&pED->m_tbRes, ch);
      }
      TextBuffer_OffsetPos(&pED->m_tb, iCharCount - 1);
      iCharsProcessed = iCharCount;
    }

    return updateCharsProcessed(piCharsProcessed, iCharsProcessed);
  }

  void CALWData::addEOL(EncodingData* pED)
  {
    TextBuffer_AddEOL(&pED->m_tbRes, iEOLMode);
    iLineOffset = 0;
    ++iLineIndex;
  }

  BOOL CALWData::RunPass2(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
  {
    const unsigned char ch = TextBuffer_GetChar(&pED->m_tb);

    if (ch == CHAR_NEXT_PARAGRAPH)
    {
      addEOL(pED);

      TextBuffer_PopChar(&pED->m_tb);
      ++*piCharsProcessed;

      iLineIndex = 0;
      m_cp = nextParagraph();
    }
    else if (ch == CHAR_FORCE_EOL)
    {
      TextBuffer_PopChar(&pED->m_tb);
      addEOL(pED);
      if (m_cp->prefix->IsMarker())
        iLineIndex = 0;
      return updateCharsProcessed(piCharsProcessed, 1);
    }

    BOOL isPrefixInitialized = FALSE;
    auto prefixLength = m_cp->prefix->GetLength();
    if ((iLineOffset == 0) && (prefixLength > 0))
    {
      if ((ch == CHAR_FORCE_EOL) || (ch == CHAR_FORCE_EOL_PROCESSED)
        || ((ch == CHAR_NEXT_PARAGRAPH) && (TextBuffer_GetTailLength(&pED->m_tb) > 0) && IsEOL(TextBuffer_GetCharAt(&pED->m_tb, 1))))
      {
        prefixLength -= m_cp->prefix->CountTrailingWhiteSpaces();
      }
      for (int i = 0; i < prefixLength; ++i)
      {
        TextBuffer_PushChar(&pED->m_tbRes, m_cp->prefix->GetChar(i, iLineIndex));
        ++iLineOffset;
      }
    }

    if (ch == CHAR_NEXT_PARAGRAPH)
    {
      return updateCharsProcessed(piCharsProcessed, 0);
    }

    auto prefixFirstLineLength = prefixLength;
    prefixLength = m_cp->prefix->IsMarker() ? m_cp->prefix->GetLength() : 0;

    int iCharsProcessed = 0;
    int iWordByteCount = 0;
    const int iWordLength = TextBuffer_GetWordLength(&pED->m_tb, iEncoding, &iWordByteCount);
    if ((iWordCount == 0)
        || (iLineOffset + iWordLength <= longLineLimit))
    {
      if (m_cp->prefix->IsMarker()
        && ((pED->m_tbRes.m_iPos > 0) && IsEOL(TextBuffer_GetCharAt(&pED->m_tbRes, -1))))
      {
        int markerPrefixLength = m_cp->prefix->GetLength();
        if (markerPrefixLength > 0)
        {
          if ((ch == CHAR_FORCE_EOL) || (ch == CHAR_FORCE_EOL_PROCESSED))
          {
            markerPrefixLength -= m_cp->prefix->CountTrailingWhiteSpaces();
          }
          for (int i = 0; i < markerPrefixLength; ++i)
          {
            TextBuffer_PushChar(&pED->m_tbRes, m_cp->prefix->GetChar(i, iLineIndex));
            ++iLineOffset;
          }
        }
        prefixFirstLineLength = markerPrefixLength;
      }
      BOOL isMarker = FALSE;
      BOOL isWhiteSpace = FALSE;

      if ((iWordByteCount == 1)
        && ((ch == CHAR_FORCE_EOL)
            || (ch == CHAR_FORCE_EOL_PROCESSED)))
      {
        iLineOffset = 0;
        iWordCount = 0;
        if (IsEOL(TextBuffer_GetCharAt(&pED->m_tbRes, -(prefixFirstLineLength + 1))))
        {
          TextBuffer_PushChar(&pED->m_tb, CHAR_FORCE_EOL_PROCESSED);
          TextBuffer_DecPos(&pED->m_tb);
        }
        if (TextBuffer_GetChar(&pED->m_tb) != CHAR_FORCE_EOL_PROCESSED)
        {
          TextBuffer_PushChar(&pED->m_tb, CHAR_FORCE_EOL_PROCESSED);
          if ((TextBuffer_GetTailLength(&pED->m_tb) > 0)
            || ((TextBuffer_GetTailLength(&pED->m_tb) > 1) && !IsEOL(TextBuffer_GetCharAt(&pED->m_tbRes, -1))))
          {
            TextBuffer_DecPos(&pED->m_tb);
            addEOL(pED);
          }
          else
          {
            iCharsProcessed += 1;
          }
        }
        else 
        {
          TextBuffer_IncPos(&pED->m_tb);
          iCharsProcessed += 1;
          if (TextBuffer_GetTailLength(&pED->m_tb) > 0)
          {
            addEOL(pED);
          }
        }
      }
      else
      {
        for (int i = 1; i <= iWordByteCount; ++i)
        {
          const BOOL isDynamicMarker = TextBuffer_IsAnyCharAtPos_IgnoreSpecial(&pED->m_tb, lpstrDynamicMarkerChars, lpstrDigits, 0);
          const unsigned char ch = TextBuffer_PopChar(&pED->m_tb);
          const BOOL isStaticMarker = IsCharFromString(lpstrStaticMarkerChars, ch);
          isMarker = isStaticMarker || isDynamicMarker;
          isWhiteSpace = IsCharFromString(lpstrWhiteSpaces, ch);
          TextBuffer_PushChar(&pED->m_tbRes, ch);
          ++iCharsProcessed;
          ++iLineOffset;
          if (IsEOLChar(ch))
          {
            if (IsTrailingEOL(iEOLMode, ch, &pED->m_tb))
            {
              if (iEOLMode == SC_EOL_CRLF)
              {
                TextBuffer_PushChar(&pED->m_tbRes, TextBuffer_PopChar(&pED->m_tb));
                ++iCharsProcessed;
              }
            }

            iLineOffset = 0;
            iWordCount = 0;
          }
        }
      }
      if (!isMarker && !isWhiteSpace)
      {
        iWordCount = (iLineOffset == 0) ? 0 : iWordCount + 1;
      }
      if (!isPrefixInitialized && (iWordByteCount != iWordLength))
      {
        iLineOffset -= iWordByteCount - iWordLength;
      }
    }
    else
    {
      iLineOffset = longLineLimit;
    }

    if (!isPrefixInitialized
        && (iLineOffset >= longLineLimit)
        && (TextBuffer_GetTailLength(&pED->m_tb) > 0))
    {
      iLineOffset = 0;
      iWordCount = 0;
      if (!IsEOL(TextBuffer_GetChar(&pED->m_tb))
        || m_cp->prefix->IsComment())
      {
        if (TextBuffer_GetCharAt(&pED->m_tbRes, -1) == CHAR_SPACE)
        {
          TextBuffer_DecPos(&pED->m_tbRes);
        }
        const auto chNext = TextBuffer_GetChar(&pED->m_tb);
        if (chNext != CHAR_NEXT_PARAGRAPH)
          addEOL(pED);
        if (IsEOL(chNext))
        {
          const int offset = 1 + (((chNext == CHAR_FORCE_EOL) || (chNext == CHAR_FORCE_EOL_PROCESSED)) ? 0 : GetTrailingEOLLength());
          TextBuffer_OffsetPos(&pED->m_tb, offset);
          iCharsProcessed += offset;
        }
        // skip trailing space
        else if (TextBuffer_GetChar(&pED->m_tb) == CHAR_SPACE)
        {
          TextBuffer_PopChar(&pED->m_tb);
          ++iCharsProcessed;
        }
      }
    }

    return updateCharsProcessed(piCharsProcessed, iCharsProcessed);
  }

#ifdef __cplusplus
}; // extern "C"
#endif
