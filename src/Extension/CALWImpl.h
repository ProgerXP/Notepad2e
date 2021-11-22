#pragma once

#include <WTypes.h>
#include <map>
#include <memory>
#include <vector>
#include "StringRecoding-fwd.h"

#include "CommentAwareLineWrapping.h"

struct PassState
{
  int relativeLineIndex = 0;
  int relativeLineIndexPrefixProcessed = -1;
  std::string currentPrefix;
};

struct Prefix
{
  bool m_isComment = false;
  bool m_isEmptyLine = false;
  std::string m_data;

public:
  bool IsComment() const;
  bool IsEmptyLineComment() const;
  void SetComment(const bool isComment, const bool isEmptyLine);
  int CountTrailingWhiteSpaces() const;
  void SetString(const std::string s);
  void PushChar(const unsigned char ch);
  unsigned char GetChar(const int i);
  int GetLength() const;
};

struct Paragraph
{
  std::shared_ptr<Prefix> prefixMinimal;
  std::shared_ptr<Prefix> prefixFirstLine;
  std::shared_ptr<Prefix> prefixMarkerLine;

  Paragraph();
};

struct CALWData
{
public:
  int longLineLimit = 0;
  int lexerId = 0;
  int iEOLMode = 0;
  int iTrailingEOLLength = 0;

  PassState ps[3];

  BOOL previosLineUseMarker = FALSE;

  BOOL initLine = FALSE;
  BOOL skipNextEOL = FALSE;
  int iLineOffset = 0;
  int iWordCount = 0;
  int iSingleLineCommentPrefixLength = 0;

  std::vector<std::shared_ptr<Paragraph>> m_paragraphs;

protected:
  std::shared_ptr<Paragraph> m_cp;

  std::shared_ptr<Paragraph> addParagraph();
  std::shared_ptr<Prefix> createPrefix() const;
  std::string readLinePrefix(EncodingData* pED, const char ch, const int count, bool& isCommentLine) const;
  BOOL updateCharsProcessed(long* piCharsProcessed, int iCharsProcessed) const;
  
  BOOL IsEOL(const unsigned char ch) const;
  BOOL GetTrailingEOLLength() const;

public:
  CALWData(const int iAdditionalData1, const int iAdditionalData2, const int iAdditionalData3);
  BOOL RunPass0(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);
  BOOL RunPass1(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);
  BOOL RunPass2(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);
};
