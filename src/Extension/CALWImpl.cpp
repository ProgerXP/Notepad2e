#include "CALWImpl.h"
#include <assert.h>
#include <algorithm>

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
  static LPCSTR lpstrWhiteSpaces = " \t";
  static LPCSTR lpstrWhiteSpacesAndEOLs = " \t\r\n";
  static LPCSTR lpstrStaticMarkerChars = "#>=?*";
  static LPCSTR lpstrDynamicMarkerChars = ":).";
  static LPCSTR lpstrDigits = "0123456789";

  bool Prefix::IsComment() const
  {
    return m_isComment;
  }

  bool Prefix::IsEmptyLineComment() const
  {
    return m_isEmptyLine;
  }

void Prefix::SetComment(const bool isComment, const bool isEmptyLine)
  {
    m_isComment = isComment;
    m_isEmptyLine = isEmptyLine;
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

  void Prefix::SetString(const std::string s)
  {
    m_data = s;
  }

  void Prefix::PushChar(const unsigned char ch)
  {
    m_data.push_back(ch);
  }

  unsigned char Prefix::GetChar(const int i)
  {
    return m_data.at(i);
  }

  int Prefix::GetLength() const
  {
    return m_data.size();
  }

  Paragraph::Paragraph() {}

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
    return *m_paragraphs.rbegin();
  }

  std::shared_ptr<Prefix> CALWData::createPrefix() const
  {
    return std::make_shared<Prefix>(Prefix());
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

  BOOL isMarker(const unsigned char ch, EncodingData* pED)
  {
    const BOOL isStaticMarker = IsCharFromString(lpstrStaticMarkerChars, ch);
    const BOOL isDynamicMarker = TextBuffer_IsAnyCharAtPos_IgnoreSpecial(&pED->m_tb, lpstrDynamicMarkerChars, lpstrDigits, 0);
    return isStaticMarker || isDynamicMarker;
  }

  BOOL CALWData::IsEOL(const unsigned char ch) const
  {
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

  BOOL CALWData::RunPass0(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
  {
    const unsigned char ch = TextBuffer_PopChar(&pED->m_tb);
    int iCharCount = 1 + (!IsTrailingEOL(iEOLMode, ch, &pED->m_tb) ? TextBuffer_GetCharSequenceLength(&pED->m_tb, ch, 0) : 0);
    BOOL skipChars = FALSE;

    if (!IsEOLChar(ch) && (ps[pRA->iPassIndex].relativeLineIndex > ps[pRA->iPassIndex].relativeLineIndexPrefixProcessed))
    {
      const BOOL isWhiteSpace = IsCharFromString(lpstrWhiteSpaces, ch);
      const int iCommentOffset = iCharCount - 1 + (isWhiteSpace ? iSingleLineCommentPrefixLength : 0);
      const BOOL isSingleLineComment = n2e_IsSingleLineCommentStyleAtPos(NULL, lexerId, iCommentOffset, pED);
      if (isSingleLineComment && !TextBuffer_IsWhiteSpaceLine(&pED->m_tb, iCommentOffset, NULL))
      {
        const int iPrefixWhiteSpacesAfterComment = m_cp->prefixMinimal ? m_cp->prefixMinimal->CountTrailingWhiteSpaces() : 1000;
        const int iWhiteSpacesAfterComment = min(
          iPrefixWhiteSpacesAfterComment,
          TextBuffer_CountWhiteSpaces(&pED->m_tb, iCommentOffset)
        );
        if (m_cp->prefixMinimal || (iWhiteSpacesAfterComment < iPrefixWhiteSpacesAfterComment))
        {
          m_cp->prefixMinimal = createPrefix();
          m_cp->prefixMinimal->PushChar(ch);

          TextBuffer_PushChar(&pED->m_tbRes, ch);
          iCharCount = 1;
          for (int i = 0; i < iCommentOffset + iWhiteSpacesAfterComment; ++i)
          {
            const auto ch2 = TextBuffer_PopChar(&pED->m_tb);
            m_cp->prefixMinimal->PushChar(ch2);
            TextBuffer_PushChar(&pED->m_tbRes, ch2);
            ++iCharCount;
          }
        }
        else
        {
          TextBuffer_PushChar(&pED->m_tbRes, ch);
          iCharCount = 1;
          for (int i = 0; i < 1 + iCommentOffset + iWhiteSpacesAfterComment; ++i)
          {
            TextBuffer_PushChar(&pED->m_tbRes, TextBuffer_PopChar(&pED->m_tb));
            ++iCharCount;
          }
        }
        ps[pRA->iPassIndex].relativeLineIndexPrefixProcessed = ps[pRA->iPassIndex].relativeLineIndex;
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

  BOOL CALWData::RunPass1(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
  {
    BOOL skipChars = FALSE;
    BOOL skipInitLineCheck = FALSE;
    const unsigned char ch = TextBuffer_PopChar(&pED->m_tb);
    int iCharCount = 1 + (!IsTrailingEOL(iEOLMode, ch, &pED->m_tb) ? TextBuffer_GetCharSequenceLength(&pED->m_tb, ch, 0) : 0);
    int iCharsProcessed = 0;
    if (((ps[pRA->iPassIndex].relativeLineIndex == 0) && !m_cp->prefixFirstLine)
      || ((ps[pRA->iPassIndex].relativeLineIndex > 0) && m_cp->prefixFirstLine->IsEmptyLineComment()))
    {
      if ((ps[pRA->iPassIndex].relativeLineIndex > 0) && IsEOLChar(ch))
      {
        ++(*piCharsProcessed);

        if ((TextBuffer_GetTailLength(&pED->m_tb) == 0) && (TextBuffer_GetHeadLength(&pED->m_tbRes) == 0))
        {
          TextBuffer_PushChar(&pED->m_tbRes, CHAR_FORCE_EOL_PROCESSED);
        }
        return TRUE;
      }
      m_cp->prefixFirstLine = createPrefix();
      const BOOL isWhiteSpace = IsCharFromString(lpstrWhiteSpaces, ch);
      const int iCommentOffset = iCharCount - 1 + (isWhiteSpace ? iSingleLineCommentPrefixLength : 0);
      const BOOL isSingleLineComment = n2e_IsSingleLineCommentStyleAtPos(NULL, lexerId, iCommentOffset, pED);
      const BOOL isEmptyLine = isSingleLineComment
        ? TextBuffer_IsWhiteSpaceLine(&pED->m_tb, iCommentOffset, NULL)
        : FALSE;
      m_cp->prefixFirstLine->SetComment(isSingleLineComment, isEmptyLine);
      if (isSingleLineComment)
      {
        const int iWhiteSpacesAfterComment = m_cp->prefixMinimal ? m_cp->prefixMinimal->CountTrailingWhiteSpaces() : TextBuffer_CountWhiteSpaces(&pED->m_tb, iCommentOffset);
        m_cp->prefixFirstLine->PushChar(ch);
        for (int i = 0; i < iCommentOffset + iWhiteSpacesAfterComment; ++i)
        {
          m_cp->prefixFirstLine->PushChar(TextBuffer_PopChar(&pED->m_tb));
        }
        iCharCount = m_cp->prefixFirstLine->GetLength();
        skipChars = TRUE;

        if ((TextBuffer_GetTailLength(&pED->m_tb) == 0) && (TextBuffer_GetHeadLength(&pED->m_tbRes) == 0))
        {
          TextBuffer_PushChar(&pED->m_tbRes, CHAR_FORCE_EOL_PROCESSED);
        }
      }
      else if (isWhiteSpace)
      {
        for (int i = 0; i < iCharCount; ++i)
        {
          m_cp->prefixFirstLine->PushChar(ch);
        }
        TextBuffer_OffsetPos(&pED->m_tb, iCharCount - 1);
        skipChars = TRUE;
      }
      ps[pRA->iPassIndex].relativeLineIndexPrefixProcessed = ps[pRA->iPassIndex].relativeLineIndex;
    }
    else if ((ps[pRA->iPassIndex].relativeLineIndex > 0)
      && (ps[pRA->iPassIndex].relativeLineIndex > ps[pRA->iPassIndex].relativeLineIndexPrefixProcessed))
    {
      if (m_cp->prefixFirstLine->IsComment())
      {
        const int iWhiteSpaces = TextBuffer_CountWhiteSpaces(&pED->m_tb, 0);
        const BOOL isSingleLineComment =
          n2e_IsSingleLineCommentStyleAtPos(NULL, lexerId,
          (IsCharFromString(lpstrWhiteSpacesAndEOLs, ch) ? 0 : -1) + iWhiteSpaces + iSingleLineCommentPrefixLength,
            pED);
        if (isSingleLineComment)
        {
          if ((iLineOffset == 0) && (iWhiteSpaces == 0) && !IsEOLChar(TextBuffer_GetCharAt(&pED->m_tb, -1)))
          {
            TextBuffer_DecPos(&pED->m_tb);
            --(*piCharsProcessed);
          }
          iCharCount = iWhiteSpaces + 1 + iSingleLineCommentPrefixLength;
          TextBuffer_OffsetPos(&pED->m_tb, iCharCount - 1);

          skipChars = TRUE;
          skipInitLineCheck = TRUE;
          ps[pRA->iPassIndex].relativeLineIndexPrefixProcessed = ps[pRA->iPassIndex].relativeLineIndex;

          int iLineLength = 0;
          const BOOL isWhiteSpaceLine = TextBuffer_IsWhiteSpaceLine(&pED->m_tb, 0, &iLineLength);
          if (isWhiteSpaceLine)
          {
            if (TextBuffer_GetCharAt(&pED->m_tbRes, -1) == CHAR_SPACE)
            {
              TextBuffer_OffsetPos(&pED->m_tbRes, -1);
            }
            TextBuffer_PushChar(&pED->m_tbRes, CHAR_FORCE_EOL);
            const int offset = ((TextBuffer_GetTailLength(&pED->m_tb) > 0) ? 1 : 0) + std::min<int>(GetTrailingEOLLength(), TextBuffer_GetTailLength(&pED->m_tb));
            TextBuffer_OffsetPos(&pED->m_tb, offset);
            iCharsProcessed += offset;
            initLine = TRUE;
            ++ps[pRA->iPassIndex].relativeLineIndex;
          }
        }
      }
      else if ((m_cp->prefixFirstLine->GetLength() > 0)
        && (ch == CHAR_SPACE) && (iCharCount >= m_cp->prefixFirstLine->GetLength()))
      {
        TextBuffer_OffsetPos(&pED->m_tb, iCharCount - 1);
        skipChars = TRUE;
      }
    }

    if (IsEOLChar(ch) || skipInitLineCheck)
    {
      if (!initLine)
      {
        ++ps[pRA->iPassIndex].relativeLineIndex;

        const BOOL isEOLAtPosition = (iTrailingEOLLength == 0) ? TRUE : IsEOLChar(TextBuffer_GetChar(&pED->m_tb));
        const int spacesAfterPosition = TextBuffer_GetCharSequenceLength(&pED->m_tb, CHAR_SPACE, iTrailingEOLLength);
        int iLineLength = 0;
        const BOOL isWhiteSpaceLine = TextBuffer_IsWhiteSpaceLine(&pED->m_tb, iTrailingEOLLength, &iLineLength);

        initLine = !skipNextEOL &&
          (((m_cp->prefixFirstLine->GetLength() == 0) && !isWhiteSpaceLine) || isEOLAtPosition);

        if (isWhiteSpaceLine)
        {
          TextBuffer_PushChar(&pED->m_tbRes, ch);
          int count = 0;
          if (iEOLMode == SC_EOL_CRLF)
          {
            TextBuffer_PushChar(&pED->m_tbRes, TextBuffer_GetChar(&pED->m_tb));
            count += 1;
          }
          count += iLineLength;
          TextBuffer_OffsetPos(&pED->m_tb, count);
          iCharCount += count;
          initLine = FALSE;
          skipNextEOL = TRUE;
          skipChars = TRUE;
        }
        else if (initLine)
        {
          const BOOL commentAtPosition = n2e_IsSingleLineCommentStyleAtPos(NULL, lexerId, iTrailingEOLLength + spacesAfterPosition + iSingleLineCommentPrefixLength, pED);
          const int markerOffset = iTrailingEOLLength + spacesAfterPosition + (commentAtPosition ? iSingleLineCommentPrefixLength : 0);
          int iLineLength = 0;
          if (isEOLAtPosition && TextBuffer_IsWhiteSpaceLine(&pED->m_tb, markerOffset, &iLineLength))
          {
            TextBuffer_PushChar(&pED->m_tbRes, CHAR_FORCE_EOL);
            iCharCount += markerOffset + iTrailingEOLLength + iLineLength;
            TextBuffer_OffsetPos(&pED->m_tb, markerOffset + iTrailingEOLLength + iLineLength);
            skipChars = TRUE;
          }
          else if (isEOLAtPosition
            && TextBuffer_IsAnyCharAtPos_IgnoreSpecial(&pED->m_tb, lpstrStaticMarkerChars, lpstrWhiteSpaces, markerOffset))
          {
            TextBuffer_PushChar(&pED->m_tbRes, ch);
            if (iEOLMode == SC_EOL_CRLF)
            {
              TextBuffer_PushChar(&pED->m_tbRes, TextBuffer_GetChar(&pED->m_tb));
            }
            ps[pRA->iPassIndex].relativeLineIndexPrefixProcessed = ps[pRA->iPassIndex].relativeLineIndex;
            initLine = FALSE;
            if (!previosLineUseMarker && !commentAtPosition)
            {
              previosLineUseMarker = TRUE;
              iCharCount += iTrailingEOLLength;
              TextBuffer_OffsetPos(&pED->m_tb, iTrailingEOLLength);
            }
            else
            {
              iCharCount += markerOffset;
              TextBuffer_OffsetPos(&pED->m_tb, markerOffset);
            }
            skipChars = TRUE;
          }
          else if ((pED->m_tbRes.m_iPos > 0)
            && (TextBuffer_GetCharAt(&pED->m_tbRes, -1) != CHAR_SPACE))
          {
            TextBuffer_PushChar(&pED->m_tbRes, CHAR_SPACE);
          }
        }
        if (!isWhiteSpaceLine && IsEOL(ch))
        {
          skipNextEOL = FALSE;
          previosLineUseMarker = FALSE;
        }
      }
    }
    else if (initLine)
    {
      initLine &= (ch == CHAR_SPACE);
    }
    else if (isMarker(ch, pED))
    {
      previosLineUseMarker = TRUE;
    }

    if (!skipChars)
    {
      for (int i = 0; i < iCharCount; ++i)
      {
        if (!initLine)
        {
          TextBuffer_PushChar(&pED->m_tbRes, ch);
        }
      }
      TextBuffer_OffsetPos(&pED->m_tb, iCharCount - 1);
    }
    iCharsProcessed += iCharCount;

    return updateCharsProcessed(piCharsProcessed, iCharsProcessed);
  }
  
  BOOL CALWData::RunPass2(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
  {
    const unsigned char ch = TextBuffer_GetChar(&pED->m_tb);

    BOOL isPrefixInitialized = FALSE;
    auto prefixLength = m_cp->prefixFirstLine->GetLength();
    if ((iLineOffset == 0) && (prefixLength > 0) && !m_cp->prefixMarkerLine)
    {
      if ((ch == CHAR_FORCE_EOL) || (ch == CHAR_FORCE_EOL_PROCESSED))
      {
        prefixLength -= m_cp->prefixFirstLine->CountTrailingWhiteSpaces();
      }
      for (int i = 0; i < prefixLength; ++i)
      {
        TextBuffer_PushChar(&pED->m_tbRes, m_cp->prefixFirstLine->GetChar(i));
        ++iLineOffset;
      }
    }
    auto prefixFirstLineLength = prefixLength;
    prefixLength = m_cp->prefixMarkerLine ? m_cp->prefixMarkerLine->GetLength() : 0;

    int iCharsProcessed = 0;
    int iWordByteCount = 0;
    const int iWordLength = TextBuffer_GetWordLength(&pED->m_tb, iEncoding, &iWordByteCount);
    if ((iWordCount == 0)
        || (iLineOffset + iWordLength <= longLineLimit))
    {
      if (m_cp->prefixMarkerLine
        && ((pED->m_tbRes.m_iPos > 0) && IsEOL(TextBuffer_GetCharAt(&pED->m_tbRes, -1))))
      {
        int markerPrefixLength = m_cp->prefixMarkerLine->GetLength();
        if (markerPrefixLength > 0)
        {
          if ((ch == CHAR_FORCE_EOL) || (ch == CHAR_FORCE_EOL_PROCESSED))
          {
            markerPrefixLength -= m_cp->prefixMarkerLine->CountTrailingWhiteSpaces();
          }
          for (int i = 0; i < markerPrefixLength; ++i)
          {
            TextBuffer_PushChar(&pED->m_tbRes, m_cp->prefixMarkerLine->GetChar(i));
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
            TextBuffer_AddEOL(&pED->m_tbRes, iEOLMode);
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
            TextBuffer_AddEOL(&pED->m_tbRes, iEOLMode);
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
          if ((iWordLength >= 1)
            && isMarker
            && ((prefixFirstLineLength == 0)
              || (iLineOffset == prefixFirstLineLength)))
          {
            TextBuffer_PushChar(&pED->m_tbRes, ch);
            ++iCharsProcessed;
            ++iLineOffset;

            m_cp->prefixMarkerLine = createPrefix();
            if (prefixFirstLineLength > 0)
            {
              for (int j = 0; j < prefixFirstLineLength; ++j)
              {
                m_cp->prefixMarkerLine->PushChar(m_cp->prefixFirstLine->GetChar(j));
              }
            }
            else
            {
              for (int j = 0; j < iLineOffset - 1; ++j)
              {
                m_cp->prefixMarkerLine->PushChar(TextBuffer_GetCharAt(&pED->m_tb, j - iLineOffset));
              }
            }
            m_cp->prefixMarkerLine->PushChar(CHAR_SPACE);

            if (isDynamicMarker)
            {
              while (IsCharFromString(lpstrDigits, TextBuffer_GetChar(&pED->m_tb))
                    || IsCharFromString(lpstrDynamicMarkerChars, TextBuffer_GetChar(&pED->m_tb)))
              {
                const unsigned char ch1 = TextBuffer_PopChar(&pED->m_tb);
                TextBuffer_PushChar(&pED->m_tbRes, ch1);
                m_cp->prefixMarkerLine->PushChar(CHAR_SPACE); // space chars under NNN)
                ++iCharsProcessed;
                ++iLineOffset;
              }
            }

            const unsigned char chNext = TextBuffer_GetChar(&pED->m_tb);
            if (IsCharFromString(lpstrWhiteSpaces, chNext))
            {
              const int markerPostfixLength = TextBuffer_GetCharSequenceLength(&pED->m_tb, chNext, 0);
              for (int i = 0; i < markerPostfixLength; ++i)
              {
                m_cp->prefixMarkerLine->PushChar(chNext);
                TextBuffer_PushChar(&pED->m_tbRes, chNext);
                ++iCharsProcessed;
                ++iLineOffset;
              }
              TextBuffer_OffsetPos(&pED->m_tb, markerPostfixLength);
            }
            isPrefixInitialized = TRUE;
            break;
          }
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
            m_cp->prefixMarkerLine.reset();
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
      if (!IsEOLChar(TextBuffer_GetChar(&pED->m_tb))
        || m_cp->prefixFirstLine->IsComment())
      {
        if (TextBuffer_GetCharAt(&pED->m_tbRes, -1) == CHAR_SPACE)
        {
          TextBuffer_DecPos(&pED->m_tbRes);
        }
        TextBuffer_AddEOL(&pED->m_tbRes, iEOLMode);
        if (TextBuffer_IsEOL(&pED->m_tb, iEOLMode))
        {
          TextBuffer_OffsetPos(&pED->m_tb, 1 + GetTrailingEOLLength());
          iCharsProcessed += 1 + GetTrailingEOLLength();
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
