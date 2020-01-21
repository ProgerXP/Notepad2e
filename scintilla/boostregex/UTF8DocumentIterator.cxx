#include "UTF8DocumentIterator.h"

static unsigned char s_firstByteMask[7] = { 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01 };

void ReadCharacterFromUTF8(Scintilla::Document* _doc, const Sci::Position _pos, const Sci::Position _end,
	wchar_t* _character, int& _characterIndex, int& _utf8Length, int& _utf16Length)
{
	unsigned char currentChar = _doc->CharAt(_pos);
	if (currentChar & 0x80)
	{
		int mask = 0x40;
		int nBytes = 1;
			
		do 
		{
			mask >>= 1;
			++nBytes;
		} while (currentChar & mask);

		int result = currentChar & s_firstByteMask[nBytes];
		Sci::Position pos = _pos;
		_utf8Length = 1;
		// work out the unicode point, and count the actual bytes.
		// If a byte does not start with 10xxxxxx then it's not part of the 
		// the code. Therefore invalid UTF-8 encodings are dealt with, simply by stopping when 
		// the UTF8 extension bytes are no longer valid.
		while ((--nBytes) && (pos < _end) && (0x80 == ((currentChar = _doc->CharAt(++pos)) & 0xC0)))
		{
			result = (result << 6) | (currentChar & 0x3F);
			++_utf8Length;
		}

		if (result >= 0x10000)
		{
			result -= 0x10000;
			_utf16Length = 2;
			// UTF-16 Pair
			_character[0] = 0xD800 + (result >> 10);
			_character[1] = 0xDC00 + (result & 0x3FF);
				
		}
		else
		{
			_utf16Length = 1;
			_character[0] = static_cast<wchar_t>(result);
		}
	}
	else
	{
		_utf8Length = 1;
		_utf16Length = 1;
		_characterIndex = 0;
		_character[0] = static_cast<wchar_t>(currentChar);
	}
}
