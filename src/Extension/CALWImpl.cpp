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

  BOOL isStaticMarker(const unsigned char ch)
  {
    return IsCharFromString(lpstrStaticMarkerChars, ch);
  }

  BOOL isMarker(const unsigned char ch, EncodingData* pED)
  {
    const BOOL isDynamicMarker = TextBuffer_IsAnyCharAtPos_RequireSpecial(&pED->m_tb, lpstrDynamicMarkerChars, lpstrDigits, 0);
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
    return IsComment() && ((m_subtype == PrefixType::MarkerStatic) || (m_subtype == PrefixType::MarkerDynamic));
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
    SetType(p.GetType(), p.GetContentOffset());
    SetInitialized();
  }

  PrefixType Prefix::GetType() const
  {
    return m_type;
  }

  void Prefix::SetType(const PrefixType type, const int iContentOffset)
  {
    m_type = type;
    SetContentOffset(iContentOffset);
    initWhitespace();
  }

  void Prefix::SetSubType(const PrefixType type)
  {
    m_subtype = type;
  }

  int Prefix::GetContentOffset() const
  {
    return m_iContentOffset;
  }

  void Prefix::SetContentOffset(const int iOffset)
  {
    m_iContentOffset = iOffset;
  }

  int Prefix::CountLeadingWhiteSpaces() const
  {
    const auto it = std::find_if(m_data.cbegin(), m_data.cend(), [&](const char& ch) {
      return !IsCharFromString(lpstrWhiteSpaces, ch);
      });
    return std::distance(m_data.cbegin(), it);
  }

  int Prefix::CountTrailingWhiteSpaces() const
  {
    const auto it = std::find_if(m_data.crbegin(), m_data.crend(), [&](const char& ch) {
      return !IsCharFromString(lpstrWhiteSpaces, ch);
      });
    return std::distance(m_data.crbegin(), it);
  }

  void Prefix::SetupLeadingWhiteSpaces(const struct Prefix& originPrefix)
  {
    m_data.erase(0, CountLeadingWhiteSpaces());
    m_data.insert(0, originPrefix.GetString().substr(0, originPrefix.CountLeadingWhiteSpaces()).c_str());
  }

  void Prefix::initWhitespace()
  {
    const auto posWhitespace = m_data.find_first_of(lpstrWhiteSpaces, m_iContentOffset);
    m_charWhitespace = (posWhitespace != std::string::npos) ? m_data.at(posWhitespace) : CHAR_SPACE;

    if (IsMarkerStatic() || (m_subtype == PrefixType::MarkerStatic))
    {
      const auto posMarker = m_data.find_first_of(lpstrStaticMarkerChars, m_iContentOffset);
      if (posMarker != std::string::npos)
      {
        m_rangeMarker.pos1 = posMarker;
        m_rangeMarker.pos2 = m_rangeMarker.pos1 + 1;
      }
    }
    else if (IsMarkerDynamic() || (m_subtype == PrefixType::MarkerDynamic))
    {
      const auto posMarkerStart = m_data.find_first_of(lpstrDigits, (posWhitespace != std::string::npos) ? posWhitespace : m_iContentOffset);
      const auto posMarkerEnd = m_data.find_first_of(lpstrDynamicMarkerChars, (posMarkerStart != std::string::npos) ? posMarkerStart : m_iContentOffset);
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
    if (IsMarker() || IsCommentedMarker())
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

  void Paragraph::SaveOffset(const int offset, const int offsetOrigin)
  {
    mapCharOffset[offset] = offsetOrigin;
  }

  int Paragraph::FindOffset(const int offset) const
  {
    const auto it = mapCharOffset.find(offset);
    return (it != mapCharOffset.cend()) ? it->second : -1;
  }

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
    return ::GetTrailingEOLLength(iEOLMode);
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
    return (m_iActiveParagraphIndex > 0) ? m_paragraphs.at(m_iActiveParagraphIndex - 1) : nullptr;
  }

  std::shared_ptr<const Paragraph> CALWData::prevParagraphByPrefixLength(const int currentPrefixLength) const
  {
    int iActiveParagraphIndex = m_iActiveParagraphIndex - 1;
    int iMinLengthParagraphIndex = iActiveParagraphIndex;
    while ((iActiveParagraphIndex >= 0)
      && (m_paragraphs.at(iActiveParagraphIndex)->prefix->GetLength() > currentPrefixLength))
    {
      iMinLengthParagraphIndex = (m_paragraphs.at(iActiveParagraphIndex)->prefix->GetLength() < m_paragraphs.at(iMinLengthParagraphIndex)->prefix->GetLength())
        ? iActiveParagraphIndex : iMinLengthParagraphIndex;
      --iActiveParagraphIndex;
    }
    return (iActiveParagraphIndex >= 0)
      ? m_paragraphs.at(iActiveParagraphIndex)
      : (iMinLengthParagraphIndex >= 0)
      ? m_paragraphs.at(iMinLengthParagraphIndex)
      : nullptr;
  }

  CALWData::CALWData(const int iAdditionalData1, const int iAdditionalData2, const int iAdditionalData3)
  {
    longLineLimit = iAdditionalData1;
    lexerId = iAdditionalData2;
    iEOLMode = LOWORD(iAdditionalData3);
    breakOnEOL = HIWORD(iAdditionalData3);
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

  void CALWData::InitPass(RecodingAlgorithm* pRA)
  {
    pRA->iResultEnd = 0;
    m_iActiveParagraphIndex = 0;
    m_cp = (m_paragraphs.size() > 0) ? *m_paragraphs.cbegin() : nullptr;
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
      m_cp->prefix->SetType(PrefixType::MarkerStatic, m_cp->prefix->GetContentOffset());
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
    const auto res = TextBuffer_IsAnyCharAtPos_RequireSpecial(&pED->m_tb, lpstrDynamicMarkerChars, lpstrDigits, iWhiteSpacesBeforeMarker);
    if (res)
    {
      m_cp->prefix->SetType(PrefixType::MarkerDynamic, m_cp->prefix->GetContentOffset());
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
      m_cp->prefix->SetType(PrefixType::Plain, 0);
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

  BOOL CALWData::isCommentStyleOnThisLine(EncodingData* pED) const
  {
    int res = -1;
    const int iLineHeadLength = TextBuffer_GetLineHeadLength(&pED->m_tb);
    const char ch = TextBuffer_GetCharAt(&pED->m_tb, -iLineHeadLength);
    const BOOL isWhiteSpace = IsCharFromString(lpstrWhiteSpaces, ch);
    const int iCharCount = TextBuffer_GetCharSequenceLength(&pED->m_tb, ch, 0);
    const int iCommentOffset = -iLineHeadLength + iCharCount + (isWhiteSpace ? iSingleLineCommentPrefixLength : 0);
    if (n2e_IsSingleLineCommentStyleAtPos(NULL, lexerId, iCommentOffset, pED))
    {
      return TRUE;
    }
    return FALSE;
  }

  BOOL CALWData::isStaticMarkerOnThisLine(EncodingData* pED) const
  {
    int res = -1;
    const int iLineHeadLength = TextBuffer_GetLineHeadLength(&pED->m_tb);
    const int iCharCount = TextBuffer_CountWhiteSpaces(&pED->m_tb, -iLineHeadLength);
    return isStaticMarker(TextBuffer_GetCharAt(&pED->m_tb, -iLineHeadLength + iCharCount));
  }

  BOOL CALWData::isDynamicMarkerOnThisLine(EncodingData* pED) const
  {
    int res = -1;
    const int iLineHeadLength = TextBuffer_GetLineHeadLength(&pED->m_tb);
    const int iCharCount = TextBuffer_CountWhiteSpaces(&pED->m_tb, -iLineHeadLength);
    return TextBuffer_IsAnyCharAtPos_RequireSpecial(&pED->m_tb, lpstrDynamicMarkerChars, lpstrDigits, -iLineHeadLength + iCharCount);
  }


  CLineAttribute CALWData::isCommentStyleOnNextLine(EncodingData* pED) const
  {
    int res = -1;
    int leadingSpaces = 0;
    int trailingSpaces = 0;
    char ch = TextBuffer_GetChar(&pED->m_tb);
    int iCharCount = TextBuffer_GetCharSequenceLength(&pED->m_tb, ch, 0);
    if (GetTrailingEOLLength() > 0)
    {
      ch = TextBuffer_GetCharAt(&pED->m_tb, 1);
      iCharCount = TextBuffer_GetCharSequenceLength(&pED->m_tb, ch, 1);
    }
    const BOOL isWhiteSpace = IsCharFromString(lpstrWhiteSpaces, ch);
    const int iCommentOffset = GetTrailingEOLLength() + iCharCount + (isWhiteSpace ? iSingleLineCommentPrefixLength : 0);
    if (n2e_IsSingleLineCommentStyleAtPos(NULL, lexerId, iCommentOffset, pED))
    {
      const int iWhiteSpacesAfterComment = TextBuffer_CountWhiteSpaces(&pED->m_tb, iCommentOffset);
      leadingSpaces = iCommentOffset - iSingleLineCommentPrefixLength;
      trailingSpaces = iWhiteSpacesAfterComment;
      res = iCommentOffset + iWhiteSpacesAfterComment;
    }
    return { res, leadingSpaces, trailingSpaces };
  }

  CLineAttribute CALWData::isStaticMarkerOnNextLine(EncodingData* pED, const int _offset) const
  {
    int res = -1;
    int leadingSpaces = 0;
    int trailingSpaces = 0;
    int iOffset = _offset + GetTrailingEOLLength();
    int iCharsProcessed = iOffset + TextBuffer_CountWhiteSpaces(&pED->m_tb, iOffset);
    if (isStaticMarker(TextBuffer_GetCharAt(&pED->m_tb, iCharsProcessed)))
    {
      leadingSpaces = iOffset;
      iCharsProcessed += 1; // static marker itself
      trailingSpaces = TextBuffer_CountWhiteSpaces(&pED->m_tb, iCharsProcessed);
      iCharsProcessed += trailingSpaces;
      res = iCharsProcessed;
    }
    return { res, leadingSpaces, trailingSpaces };
  }

  bool CALWData::saveCommentPrefix(EncodingData* pED, const char ch, const int iCharCount, int& iCharsProcessed)
  {
    const BOOL isWhiteSpace = IsCharFromString(lpstrWhiteSpaces, ch);
    const int iCommentOffset = iSingleLineCommentPrefixLength + (isWhiteSpace ? iCharCount : 0);
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

      m_cp->prefix->SetContentOffset(m_cp->prefix->GetLength());
      m_cp->prefix->SetSubType(saveStaticMarkerPrefix(pED, iCharsProcessed)
        ? PrefixType::MarkerStatic
        : saveDynamicMarkerPrefix(pED, iCharsProcessed)
        ? PrefixType::MarkerDynamic
        : PrefixType::Plain);

      m_cp->prefix->SetType(PrefixType::Comment, m_cp->prefix->GetContentOffset());
      if ((TextBuffer_GetTailLength(&pED->m_tb) == 0))
      {
        m_cp->prefix->rtrim();
      }
      m_cp->prefix->SetInitialized();
      iCharsProcessed = m_cp->prefix->GetLength();
      if (auto prevPar = prevParagraph())
      {
        if (prevPar->prefix->IsComment() == m_cp->prefix->IsComment())
        {
          m_cp->prefix->SetupLeadingWhiteSpaces(*prevPar->prefix);
        }
      }
    }
    return res;
  }

  TNextLineParams CALWData::checkNextLine(EncodingData* pED) const
  {
    TNextLineParams res;
    res.isComment = isCommentStyleOnNextLine(pED);
    if (res.isComment)
      res.isCommentedStaticMarker = isStaticMarkerOnNextLine(pED, res.isComment.GetOffset() - GetTrailingEOLLength());
    else
      res.isStaticMarker = isStaticMarkerOnNextLine(pED);
    int lineOffset = 0;
    res.isEmptyLine = TextBuffer_IsWhiteSpaceLine(&pED->m_tb, GetTrailingEOLLength(), &lineOffset)
                        ? CLineAttribute(lineOffset + GetTrailingEOLLength(), lineOffset, 0)
                        : CLineAttribute();

    return res;
  }
  
  BOOL CALWData::RunPass0(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
  {
    BOOL skipChars = FALSE;
    const unsigned char ch = TextBuffer_GetChar(&pED->m_tb);
    int iCharCount = (!IsTrailingEOL(iEOLMode, ch, &pED->m_tb) ? TextBuffer_GetCharSequenceLength(&pED->m_tb, ch, 0) : 0);
    int iCharsProcessed = iCharCount;
    if (IsCharFromString(lpstrWhiteSpaces, ch)
      && (((TextBuffer_GetLineHeadLength(&pED->m_tb) == 0) && m_cp->prefix->IsInitialized() && m_cp->prefix->IsPlain() && !isCommentStyleOnThisLine(pED) && !isStaticMarkerOnThisLine(pED) && !isDynamicMarkerOnThisLine(pED))
        || ((iCharCount > 1) && TextBuffer_GetLineHeadLength(&pED->m_tb) > 0)
        || (TextBuffer_GetLineTailLength(&pED->m_tb) == iCharCount)))
    {
      skipChars = TRUE;
      iCharCount = (iCharCount > 1)
        && (TextBuffer_GetLineTailLength(&pED->m_tb) > iCharCount)
        && (TextBuffer_GetLineHeadLength(&pED->m_tb) != 0)
        ? iCharCount - 1 : iCharCount;
      iCharsProcessed = iCharCount;
      TextBuffer_OffsetPos(&pED->m_tb, iCharCount);
    }
    else if (!m_cp->prefix->IsInitialized())
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
      int _offset = 0;
      ++ps[pRA->iPassIndex].relativeLineIndex;

      const TNextLineParams lineParams = checkNextLine(pED);
      if (lineParams.isComment && !lineParams.isCommentedStaticMarker
         && m_cp->prefix->IsComment()
         && ((!m_cp->prefix->IsCommentedMarker() && m_cp->prefix->CountTrailingWhiteSpaces() == lineParams.isComment.GetTrailingWhiteSpaces())
           || (!m_cp->prefix->IsCommentedMarker() && TextBuffer_IsWhiteSpaceLine(&pED->m_tb, lineParams.isComment.GetOffset() - GetTrailingEOLLength(), nullptr))
            || (m_cp->prefix->IsCommentedMarker()
                 && !TextBuffer_IsWhiteSpaceLine(&pED->m_tb, lineParams.isComment.GetOffset(), NULL)
                 && (m_cp->prefix->GetLength() - m_cp->prefix->CountLeadingWhiteSpaces()  == lineParams.isComment.GetOffset() - lineParams.isComment.GetLeadingWhiteSpaces()))
              )
        )
      {
        iCharsProcessed += lineParams.isComment.GetOffset();
        TextBuffer_OffsetPos(&pED->m_tb, lineParams.isComment.GetOffset());

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
              (IsEOL(TextBuffer_GetCharAt(&pED->m_tbRes, -1)) || (TextBuffer_GetCharAt(&pED->m_tbRes, -1) == CHAR_NEXT_PARAGRAPH))
              ? CHAR_FORCE_EOL_PROCESSED
              : CHAR_SPACE);
          }
        }
      }
      else if (!lineParams.isComment
        && m_cp->prefix->IsPlain()
        && TextBuffer_IsWhiteSpaceLine(&pED->m_tb, GetTrailingEOLLength(), &_offset))
      {
        const int iSkippedChars = GetTrailingEOLLength() + _offset;
        iCharsProcessed += 1 + iSkippedChars;
        TextBuffer_OffsetPos(&pED->m_tb, iSkippedChars);

        TextBuffer_PushChar(&pED->m_tbRes, CHAR_FORCE_EOL);
      }
      else if (lineParams.isCommentedStaticMarker || lineParams.isStaticMarker)
      {
        const auto prefixIsMarker = lineParams.isCommentedStaticMarker ? m_cp->prefix->IsCommentedMarker() : m_cp->prefix->IsMarker();
        const auto markerOffset = lineParams.isCommentedStaticMarker ? lineParams.isCommentedStaticMarker.GetOffset() : lineParams.isStaticMarker.GetOffset();
          bool processed = false;
          if (markerOffset - GetTrailingEOLLength() < m_cp->prefix->GetLength())
          {
          const auto _prevParagraph = prevParagraphByPrefixLength(markerOffset - GetTrailingEOLLength());
            const auto prevPrefix(_prevParagraph ? _prevParagraph->prefix : nullptr);
            const auto prevPrefixIsMarker = lineParams.isCommentedStaticMarker ? (prevPrefix && prevPrefix->IsCommentedMarker()) : (prevPrefix && prevPrefix->IsMarker());
            if (prefixIsMarker && prevPrefixIsMarker)
            {
              iCharsProcessed += markerOffset;
              TextBuffer_OffsetPos(&pED->m_tb, markerOffset);
              m_cp = addParagraph();
              m_cp->prefix->Init(*prevPrefix);
              processed = true;
            }
          }
          if (!processed)
          {
            const int iSkippedChars = GetTrailingEOLLength();
            iCharsProcessed += iSkippedChars;
            TextBuffer_OffsetPos(&pED->m_tb, iSkippedChars);
            m_cp = addParagraph();
          }
          TextBuffer_PushChar(&pED->m_tbRes, CHAR_NEXT_PARAGRAPH);
        }
      else if ((TextBuffer_GetHeadLength(&pED->m_tbRes) > 0)
        && (TextBuffer_GetCharAt(&pED->m_tbRes, -1) == CHAR_FORCE_EOL))
      {
        const int iSkippedChars = GetTrailingEOLLength();
        iCharsProcessed += iSkippedChars;
        TextBuffer_OffsetPos(&pED->m_tb, iSkippedChars);

        m_cp = addParagraph();
        TextBuffer_PushChar(&pED->m_tbRes, CHAR_NEXT_PARAGRAPH);
      }
      else if (TextBuffer_GetTailLength(&pED->m_tb) > 0)
      {
        if (!m_cp->prefix->IsEmpty() && lineParams.isEmptyLine)
        {
          const int iSkippedChars = lineParams.isEmptyLine.GetOffset();
          iCharsProcessed += iSkippedChars;
          TextBuffer_OffsetPos(&pED->m_tb, iSkippedChars);

          m_cp = addParagraph();
          TextBuffer_PushChar(&pED->m_tbRes, CHAR_NEXT_PARAGRAPH);
        }
        else if (!lineParams.isComment && !lineParams.isStaticMarker && !lineParams.isEmptyLine
          && (TextBuffer_GetHeadLength(&pED->m_tbRes) > 0)
          && (TextBuffer_GetCharAt(&pED->m_tbRes, -1) != CHAR_NEXT_PARAGRAPH)
          && (
              (m_cp->prefix->IsPlain() || m_cp->prefix->IsMarkerStatic()) && (m_cp->prefix->GetLength() == TextBuffer_CountWhiteSpaces(&pED->m_tb, GetTrailingEOLLength()))
            )
          )
        {
          const int iSkippedChars = GetTrailingEOLLength();
          iCharsProcessed += iSkippedChars;
          TextBuffer_OffsetPos(&pED->m_tb, iSkippedChars);

          const int iSkippedWhitespaces = TextBuffer_CountWhiteSpaces(&pED->m_tb, 0);
          iCharsProcessed += iSkippedWhitespaces;
          TextBuffer_OffsetPos(&pED->m_tb, iSkippedWhitespaces);            

          TextBuffer_PushChar(&pED->m_tbRes, CHAR_SPACE);
        }
        else
        {
          const int iSkippedChars = GetTrailingEOLLength();
          iCharsProcessed += iSkippedChars;
          TextBuffer_OffsetPos(&pED->m_tb, iSkippedChars);

          m_cp = addParagraph();
          TextBuffer_PushChar(&pED->m_tbRes, CHAR_NEXT_PARAGRAPH);
        }
      }
      else
      {
        TextBuffer_PushChar(&pED->m_tbRes, ch);
      }

      skipChars = TRUE;
    }

    if (breakOnEOL)
    {
      if (m_iActiveParagraphIndex < 2)
      {
        m_cp->SaveOffset(pED->m_tbRes.m_iPos, pED->m_tr.m_iSelStart + pED->m_tb.m_iPos - (skipChars ? 0 : 1));
      }
      else
      {
        pED->m_tr.m_iPositionCurrent = pED->m_tr.m_iSelEnd;
        pRA->iResultEnd = 1;
        skipChars = TRUE;
      }
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

  int CALWData::addPrefix(const unsigned char ch, EncodingData* pED)    // returns actual prefix length
  {
    auto prefixLength = m_cp->prefix->GetLength();
    if ((ch == CHAR_FORCE_EOL) || (ch == CHAR_FORCE_EOL_PROCESSED)
      || ((ch == CHAR_NEXT_PARAGRAPH) && (TextBuffer_GetTailLength(&pED->m_tb) > 0) && TextBuffer_IsWhiteSpaceLine(&pED->m_tb, 0, nullptr)))//&& IsEOL(TextBuffer_GetCharAt(&pED->m_tb, 1))))
    {
      prefixLength -= m_cp->prefix->CountTrailingWhiteSpaces();
    }
    for (int i = 0; i < prefixLength; ++i)
    {
      TextBuffer_PushChar(&pED->m_tbRes, m_cp->prefix->GetChar(i, iLineIndex));
      ++iLineOffset;
    }
    return prefixLength;
  }

  void CALWData::saveResultEnd(RecodingAlgorithm* pRA, EncodingData* pED, const int offset) const
  {
    auto res = m_cp->FindOffset(pED->m_tb.m_iPos) + offset;
    if (res == -1)
      res = pRA->iResultEndBackup;
    pRA->iResultEnd = res;
  }

  BOOL CALWData::RunPass1(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
  {
    const unsigned char ch = TextBuffer_GetChar(&pED->m_tb);

    if (ch == CHAR_NEXT_PARAGRAPH)
    {
      if ((pED->m_tb.m_iPos == 0) || (TextBuffer_GetCharAt(&pED->m_tb, -1) == CHAR_FORCE_EOL_PROCESSED))
      {
        addPrefix(ch, pED);
      }
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
      if ((iLineIndex > 0) && isMarker(ch, pED))
      {
        TextBuffer_OffsetPos(&pED->m_tb, -1);
        int iWordByteCountOrigin = 0;
        TextBuffer_GetWordRLength(&pED->m_tb, iEncoding, &iWordByteCountOrigin);
        TextBuffer_OffsetPos(&pED->m_tb, -iWordByteCountOrigin);
        int iBackOffset = -(1 + iWordByteCountOrigin);

        TextBuffer_OffsetPos(&pED->m_tbRes, -(1 + GetTrailingEOLLength()));
        int iWordByteCount = 0;
        TextBuffer_GetWordRLength(&pED->m_tbRes, iEncoding, &iWordByteCount);
        TextBuffer_OffsetPos(&pED->m_tbRes, -iWordByteCount);
        if (TextBuffer_GetLineHeadLength(&pED->m_tbRes) + iWordByteCount >= longLineLimit)
        {
          iLineOffset = TextBuffer_GetLineHeadLength(&pED->m_tbRes) + iWordByteCount;
          TextBuffer_OffsetPos(&pED->m_tbRes, iWordByteCount);
          TextBuffer_OffsetPos(&pED->m_tb, iWordByteCountOrigin);
          TextBuffer_PushChar(&pED->m_tbRes, CHAR_SPACE);
          iBackOffset += iWordByteCountOrigin;
        }
        else
        {
          const auto lineIndexBackup = iLineIndex;
          addEOL(pED);
          iLineIndex = lineIndexBackup;
        }
        if (TextBuffer_GetChar(&pED->m_tb) == CHAR_SPACE)
        {
          TextBuffer_PopChar(&pED->m_tb);
          ++iBackOffset;
        }
        
        return updateCharsProcessed(piCharsProcessed, iBackOffset);
      }

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

    if (breakOnEOL && (iLineIndex == 1) && (prefixLength == iLineOffset))
    {
      saveResultEnd(pRA, pED, 0);
      return updateCharsProcessed(piCharsProcessed, 0);
    }

    if (ch == CHAR_NEXT_PARAGRAPH)
    {
      if (breakOnEOL)
      {
        saveResultEnd(pRA, pED, 0);
      }
      return updateCharsProcessed(piCharsProcessed, 0);
    }

    auto prefixFirstLineLength = prefixLength;
    prefixLength = m_cp->prefix->GetLength();

    int iCharsProcessed = 0;
    int iWordByteCount = 0;
    const int iWordLength = TextBuffer_GetWordLength(&pED->m_tb, iEncoding, &iWordByteCount);
    if ((iWordCount == 0)
        || (iLineOffset + iWordLength <= longLineLimit) || isMarker(ch, pED))
    {
      if (m_cp->prefix->IsMarker()
        && ((TextBuffer_GetHeadLength(&pED->m_tbRes) > 0) && IsEOL(TextBuffer_GetCharAt(&pED->m_tbRes, -1))))
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
            || ((TextBuffer_GetTailLength(&pED->m_tb) > 1) && (TextBuffer_GetHeadLength(&pED->m_tbRes) > 0) && !IsEOL(TextBuffer_GetCharAt(&pED->m_tbRes, -1))))
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
          const BOOL isDynamicMarker = TextBuffer_IsAnyCharAtPos_RequireSpecial(&pED->m_tb, lpstrDynamicMarkerChars, lpstrDigits, 0);
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
      const auto chNext = TextBuffer_GetChar(&pED->m_tb);
      if (!isMarker(chNext, pED))
      {
        iLineOffset = 0;
        iWordCount = 0;
        if (!IsEOL(TextBuffer_GetChar(&pED->m_tb))
          || m_cp->prefix->IsComment())
        {
          if ((TextBuffer_GetHeadLength(&pED->m_tbRes) > 0) && (TextBuffer_GetCharAt(&pED->m_tbRes, -1) == CHAR_SPACE))
          {
            TextBuffer_DecPos(&pED->m_tbRes);
          }
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
    }

    if (TextBuffer_GetTailLength(&pED->m_tb) == 0)
    {
      saveResultEnd(pRA, pED, 0);
    }

    return updateCharsProcessed(piCharsProcessed, iCharsProcessed);
  }

#ifdef __cplusplus
}; // extern "C"
#endif
