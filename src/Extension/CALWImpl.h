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

enum class PrefixType
{
  Plain,
  MarkerStatic,
  MarkerDynamic,
  Comment
};

struct Range
{
public:
  std::size_t pos1 = std::string::npos;
  std::size_t pos2 = std::string::npos;

  bool check(const std::size_t pos) const {
    return (pos1 != std::string::npos) && (pos2 != std::string::npos)
      && (pos >= pos1) && (pos < pos2);
  }
};

struct Prefix
{
  bool m_isInitialized = false;
  PrefixType m_type = PrefixType::Plain;
  bool m_isEmptyLine = false;
  std::string m_data;
  unsigned m_charWhitespace = ' ';
  Range m_rangeMarker;

  void initWhitespace();

public:
  bool IsPlain() const;
  bool IsMarker() const;
  bool IsMarkerStatic() const;
  bool IsMarkerDynamic() const;
  bool IsComment() const;
  bool IsCommentedMarker() const;
  bool IsEmpty() const;
  bool IsInitialized() const;

  void SetInitialized();
  void Init(const Prefix& p);
  PrefixType GetType() const;
  void SetType(const PrefixType type);
  int CountTrailingWhiteSpaces() const;
  std::string GetString() const;
  void SetString(const std::string s);
  void PushChar(const unsigned char ch);
  unsigned char GetChar(const std::size_t pos, const int iLineIndex);
  int GetLength() const;

  void rtrim();
};

struct Paragraph
{
  Paragraph();

  static std::shared_ptr<Prefix> createPrefix();

  std::shared_ptr<Prefix> prefix;
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

  BOOL skipNextEOL = FALSE;
  int iLineOffset = 0;
  int iWordCount = 0;
  int iSingleLineCommentPrefixLength = 0;
  int iLineIndex = 0;                       // line index within paragraph

protected:
  std::vector<std::shared_ptr<Paragraph>> m_paragraphs;
  int m_iActiveParagraphIndex = 0;
  std::shared_ptr<Paragraph> m_cp;

  std::shared_ptr<Paragraph> addParagraph();
  std::shared_ptr<Paragraph> nextParagraph();
  std::shared_ptr<const Paragraph> prevParagraph() const;
  std::string readLinePrefix(EncodingData* pED, const char ch, const int count, bool& isCommentLine) const;
  BOOL updateCharsProcessed(long* piCharsProcessed, int iCharsProcessed) const;
  void addEOL(EncodingData* pED);
  
  BOOL IsEOL(const unsigned char ch) const;
  BOOL GetTrailingEOLLength() const;

  bool isCommentStyleOnNextLine(EncodingData* pED, int& iCharsProcessed) const;
  bool isStaticMarkerOnNextLine(EncodingData* pED, const bool isCurrentPrefixMarker, int& iCharsProcessed) const;
  bool savePlainPrefix(EncodingData* pED, const char ch, const int iCharCount, int& iCharsProcessed);
  bool saveStaticMarkerPrefix(EncodingData* pED, int& iCharsProcessed);
  bool saveDynamicMarkerPrefix(EncodingData* pED, int& iCharsProcessed);
  bool saveCommentPrefix(EncodingData* pED, const char ch, const int iCharCount, int& iCharsProcessed);

public:
  CALWData(const int iAdditionalData1, const int iAdditionalData2, const int iAdditionalData3);

  void InitPass(LPVOID pRA, const int iPassIndex);
  BOOL RunPass0(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);
  BOOL RunPass1(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);
  BOOL RunPass2(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);
};
