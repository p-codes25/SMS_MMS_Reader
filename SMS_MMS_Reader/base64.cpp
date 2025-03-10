/* 
   base64.cpp and base64.h

   base64 encoding and decoding with C++.

   Version: 1.01.00

   Copyright (C) 2004-2017 René Nyffenegger

   *** NOTE: Modified by p_codes25 to use _T() macros and the like,
   for Visual C/C++ UNICODE support. Also modified decode function to
   return a vector<unsigned char> so it can return binary blocks. ***

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

#include "stdafx.h"
#include "base64.h"
#include <iostream>

static const std::wstring base64_chars = 
             _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/");

static inline unsigned char Base64ToIndex(TCHAR uChar)
{
	if (uChar >= _T('A') && uChar <= _T('Z'))
		return uChar - 'A';						// 0..25
	else if (uChar >= _T('a') && uChar <= _T('z'))
		return uChar - _T('a') + 26;				// 26..51
	else if (uChar >= _T('0') && uChar <= _T('9'))
		return uChar - _T('0') + 52;				// 52..61
	else if (uChar == _T('+'))
		return 62;
	else if (uChar == _T('/'))
		return 63;
	else
	{
#ifdef _DEBUG
		ASSERT(FALSE);
#endif
		return (unsigned char)-1;
	}
}

static inline bool is_base64(TCHAR c) {
  return (_istalnum(c) || (c == '+') || (c == '/'));
}

// Not currently used, but could be used if we support exporting of SMS/MMS messages back to XML...
std::wstring base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::wstring ret;
  int i = 0;
  int j = 0;
  TCHAR char_array_3[3];
  TCHAR char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = _T('\0');

    char_array_4[0] = ( char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += _T('=');

  }

  return ret;

}

// TBD: this could probably use some performance-tuning... e.g. preallocate the output vector somehow?
std::vector<unsigned char> base64_decode(std::wstring const& encoded_string) {
  size_t in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  TCHAR char_array_4[4];
  unsigned char char_array_3[3];
  std::vector<unsigned char> ret;

  // Use a regular C pointer for speed...
  LPCTSTR pszIn = encoded_string.c_str();

  while (in_len-- && ( *pszIn != _T('=')) && is_base64(*pszIn))
  {
    char_array_4[i++] = *pszIn++;
	in_++;

    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = (TCHAR) Base64ToIndex(char_array_4[i]);

      char_array_3[0] = ( char_array_4[0] << 2       ) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) +   char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret.push_back(char_array_3[i]);
      i = 0;
    }
  }

  if (i) {
    for (j = 0; j < i; j++)
      char_array_4[j] = (TCHAR) Base64ToIndex(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

    for (j = 0; (j < i - 1); j++)
		ret.push_back(char_array_3[j]);
  }

  return ret;
}
