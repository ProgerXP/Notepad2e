/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005-2011, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See base64-license.txt for more details.
 */

#include "base64.h"
#include <assert.h>
#include "StringRecoding.h"

static const unsigned char base64_table[65] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
* base64_encode - Base64 encode
* @src: Data to be encoded
* @len: Length of the data to be encoded
* @out_len: Pointer to output length variable, or %NULL if not used
* Returns: Allocated buffer of out_len bytes of encoded data,
* or %NULL on failure
*
* Caller is responsible for freeing the returned buffer. Returned buffer is
* nul terminated to make it easier to use as a C string. The nul terminator is
* not included in out_len.
*/
unsigned char * base64_encode(const unsigned char *src, size_t len,
                              size_t *out_len)
{
  unsigned char *out, *pos;
  const unsigned char *end, *in;
  size_t olen;
  int line_len;

  olen = len * 4 / 3 + 4; /* 3-byte blocks to 4-byte */
  olen += olen / 72; /* line feeds */
  olen++; /* nul termination */
  if (olen < len)
    return NULL; /* integer overflow */
  out = malloc(olen);
  if (out == NULL)
    return NULL;

  end = src + len;
  in = src;
  pos = out;
  line_len = 0;
  while (end - in >= 3)
  {
    *pos++ = base64_table[in[0] >> 2];
    *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
    *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
    *pos++ = base64_table[in[2] & 0x3f];
    in += 3;
    line_len += 4;
    if (line_len >= 72)
    {
      *pos++ = '\n';
      line_len = 0;
    }
  }

  if (end - in)
  {
    *pos++ = base64_table[in[0] >> 2];
    if (end - in == 1)
    {
      *pos++ = base64_table[(in[0] & 0x03) << 4];
      *pos++ = '=';
    }
    else
    {
      *pos++ = base64_table[((in[0] & 0x03) << 4) |
        (in[1] >> 4)];
      *pos++ = base64_table[(in[1] & 0x0f) << 2];
    }
    *pos++ = '=';
    line_len += 4;
  }

  if (line_len)
    *pos++ = '\n';

  *pos = '\0';
  if (out_len)
    *out_len = pos - out;
  return out;
}


/**
* base64_decode - Base64 decode
* @src: Data to be decoded
* @len: Length of the data to be decoded
* @out_len: Pointer to output length variable
* Returns: Allocated buffer of out_len bytes of decoded data,
* or %NULL on failure
*
* Caller is responsible for freeing the returned buffer.
*/
unsigned char * base64_decode(const unsigned char *src, size_t len,
                              size_t *out_len)
{
  unsigned char dtable[256], *out, *pos, block[4], tmp;
  size_t i, count, olen;
  int pad = 0;

  memset(dtable, 0x80, 256);
  for (i = 0; i < sizeof(base64_table) - 1; i++)
    dtable[base64_table[i]] = (unsigned char)i;
  dtable['='] = 0;

  count = 0;
  for (i = 0; i < len; i++)
  {
    if (dtable[src[i]] != 0x80)
      count++;
  }

  if (count == 0 || count % 4)
    return NULL;

  olen = count / 4 * 3;
  pos = out = malloc(olen);
  if (out == NULL)
    return NULL;

  count = 0;
  for (i = 0; i < len; i++)
  {
    tmp = dtable[src[i]];
    if (tmp == 0x80)
      continue;

    if (src[i] == '=')
      pad++;
    block[count] = tmp;
    count++;
    if (count == 4)
    {
      *pos++ = (block[0] << 2) | (block[1] >> 4);
      *pos++ = (block[1] << 4) | (block[2] >> 2);
      *pos++ = (block[2] << 6) | block[3];
      count = 0;
      if (pad)
      {
        if (pad == 1)
          pos--;
        else if (pad == 2)
          pos -= 2;
        else
        {
          /* Invalid padding */
          free(out);
          return NULL;
        }
        break;
      }
    }
  }

  *out_len = pos - out;
  return out;
}

BOOL CheckRequiredBase64DigitsAvailable(LPCSTR pCurTextOrigin, LPCSTR pCurText, const long iCurTextLength, const long nChars)
{
  BOOL res = FALSE;
  if (iCurTextLength - (pCurText - pCurTextOrigin) >= nChars)
  {
    long i = 0;
    res = TRUE;
    for (i = 0; i < nChars; ++i)
    {
      res &= (strchr(base64_table, pCurText[i]) != 0);
      if (!res)
      {
        break;
      }
    }
  }
  return res;
}

BOOL Base64_IsValidSequence(EncodingData* pED, const int requiredChars)
{
  return CheckRequiredBase64DigitsAvailable(pED->m_tb.m_ptr, pED->m_tb.m_ptr + pED->m_tb.m_iPos, pED->m_tb.m_iMaxPos, requiredChars);
}

static Base64Data b64data = { 0 };

LPVOID Base64_InitAlgorythmData(const BOOL isEncoding)
{
  if (!isEncoding)
  {
    if (!b64data.initialized)
    {
      memset(b64data.dtable, 0x80, 256);
      for (int i = 0; i < sizeof(base64_table) - 1; i++)
      {
        b64data.dtable[base64_table[i]] = (unsigned char)i;
      }
      b64data.dtable['='] = 0;
      b64data.initialized = 1;
    }
    return (LPVOID)&b64data;
  }
  else
  {
    return NULL;
  }
}

void Base64_ReleaseAlgorythmData(LPVOID pData)
{
}

long Base64_GetEncodedCharLength(unsigned char ch)
{
  if (Is8BitEncodingMode())
  {
    return isascii(ch) ? 1 : 2;
  }
  return 1;
}

BOOL Base64_Encode(RecodingAlgorythm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  unsigned char chInput[4] = { 0 };
  for (int i = 0; i < _countof(chInput) - 1; ++i)
  {
    chInput[i] = TextBuffer_PopChar(&pED->m_tb);
  }
  TextBuffer_PushChar(&pED->m_tbRes, base64_table[chInput[0] >> 2]);
  TextBuffer_PushChar(&pED->m_tbRes, base64_table[((chInput[0] & 0x03) << 4) | (chInput[1] >> 4)]);
  TextBuffer_PushChar(&pED->m_tbRes, base64_table[((chInput[1] & 0x0f) << 2) | (chInput[2] >> 6)]);
  TextBuffer_PushChar(&pED->m_tbRes, base64_table[chInput[2] & 0x3f]);

  if (piCharsProcessed)
  {
    int iCharsProcessed = 0;
    if (Is8BitEncodingMode())
    {
      for (int i = 0; i < _countof(chInput) - 1; ++i)
      {
        iCharsProcessed += Base64_GetEncodedCharLength(chInput[i]);
      }
    }
    else
    {
      iCharsProcessed = _countof(chInput) - 1;
    }
    (*piCharsProcessed) += iCharsProcessed;
  }
  return TRUE;
}

BOOL Base64_EncodeTail(RecodingAlgorythm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  long iCharsProcessed = 0;
  unsigned char chInput[4] = { 0 };
  int i = 0;

  chInput[0] = TextBuffer_PopChar(&pED->m_tb);
  iCharsProcessed += Base64_GetEncodedCharLength(chInput[0]);
  TextBuffer_PushChar(&pED->m_tbRes, base64_table[chInput[0] >> 2]);

  if (TextBuffer_GetTailLength(&pED->m_tb) == 0)
  {
    TextBuffer_PushChar(&pED->m_tbRes, base64_table[(chInput[0] & 0x03) << 4]);
    TextBuffer_PushChar(&pED->m_tbRes, '=');
  }
  else
  {
    chInput[1] = TextBuffer_PopChar(&pED->m_tb);
    iCharsProcessed += Base64_GetEncodedCharLength(chInput[1]);
    TextBuffer_PushChar(&pED->m_tbRes, base64_table[((chInput[0] & 0x03) << 4) | (chInput[1] >> 4)]);
    TextBuffer_PushChar(&pED->m_tbRes, base64_table[(chInput[1] & 0x0f) << 2]);
  }
  TextBuffer_PushChar(&pED->m_tbRes, '=');

  if (piCharsProcessed)
  {
    (*piCharsProcessed) += iCharsProcessed;
  }
  return TRUE;
}

BOOL Base64_Decode(RecodingAlgorythm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  Base64Data* pData = (Base64Data*)pRA->data;
  assert(pData);

  unsigned char chInput[5] = { 0 };
  unsigned char block[5] = { 0 };
  int pad = 0;
  for (int i = 0; i < _countof(chInput) - 1; ++i)
  {
    chInput[i] = (unsigned char)TextBuffer_PopChar(&pED->m_tb);
    block[i] = pData->dtable[chInput[i]];
    if (chInput[i] == '=')
    {
      pad++;
    }
  }

  TextBuffer_PushChar(&pED->m_tbRes, (block[0] << 2) | (block[1] >> 4));
  switch (pad)
  {
  case 0:
    TextBuffer_PushChar(&pED->m_tbRes, (block[1] << 4) | (block[2] >> 2));
    TextBuffer_PushChar(&pED->m_tbRes, (block[2] << 6) | block[3]);
    break;
  case 1:
    TextBuffer_PushChar(&pED->m_tbRes, (block[1] << 4) | (block[2] >> 2));
    break;
  case 2:
    break;
  default:
    assert(FALSE);
    return FALSE;   // Invalid padding
  }

  if (piCharsProcessed)
  {
    (*piCharsProcessed) += _countof(chInput) - 1;
  }
  return TRUE;
}
