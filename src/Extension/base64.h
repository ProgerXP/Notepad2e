/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See base64-license.txt for more details.
 */

#pragma once
#include <wtypes.h>

unsigned char * base64_encode(const unsigned char *src, size_t len,
                              size_t *out_len);
unsigned char * base64_decode(const unsigned char *src, size_t len,
                              size_t *out_len);

struct TBase64Data
{
  unsigned char initialized;
  unsigned char dtable[256];
};
typedef struct TBase64Data Base64Data;

struct TEncodingData;
typedef struct TEncodingData EncodingData;
struct TRecodingAlgorythm;
typedef struct TRecodingAlgorythm RecodingAlgorythm;

BOOL Base64_IsValidSequence(EncodingData* pED, const int requiredChars);
LPVOID Base64_InitAlgorythmData(const BOOL isEncoding);
void Base64_ReleaseAlgorythmData(LPVOID pData);
BOOL Base64_Encode(RecodingAlgorythm* pRA, EncodingData* pED, long* piCharsProcessed);
BOOL Base64_EncodeTail(RecodingAlgorythm* pRA, EncodingData* pED, long* piCharsProcessed);
BOOL Base64_Decode(RecodingAlgorythm* pRA, EncodingData* pED, long* piCharsProcessed);
