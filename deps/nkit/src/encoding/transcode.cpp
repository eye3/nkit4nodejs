/*
   Copyright 2010-2014 Boris T. Darchiev (boris.darchiev@gmail.com)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "nkit/tools.h"
#include "nkit/transcode.h"

#ifdef max
#undef max
#endif

#define ALL_CHARS_LEN 0x100

namespace nkit
{
  //----------------------------------------------------------------------------
  bool    utf16_to_utf8(char *utf8, const uint16_t *utf16,
            uint8_t *shorts_read = NULL);
  bool    utf8_to_utf16(uint16_t *utf16, const char *utf8,
            uint8_t *bytes_read = NULL);
  uint8_t utf8_size(char first_utf8_byte);

  //----------------------------------------------------------------------------
  struct CharToSingleUtf16Map
  {
    const uint16_t codepage_id;
    const uint16_t map[ALL_CHARS_LEN];
  };

  //----------------------------------------------------------------------------
  struct Lang
  {
    const char * name;
    const uint16_t codepage_id;
  };

  const Lang * get_langs()
  {
    static const Lang langs[] =
    {
#include "encoding/langs.inc"
    };

    return langs;
  }

  //----------------------------------------------------------------------------
  typedef std::map<uint16_t, Transcoder> Transcoders;
  Transcoders & get_all_transcoders()
  {
    static Transcoders all_transcoders;
    return all_transcoders;
  }

  //----------------------------------------------------------------------------
  void Transcoder::Build()
  {
    static const CharToSingleUtf16Map codepages[] =
    {
#include "encoding/encodings.inc"
    };
    Transcoders & all_transcoders = get_all_transcoders();
    for (size_t i = 0; codepages[i].codepage_id != 0; ++i)
    {
      const CharToSingleUtf16Map & codepage = codepages[i];
      Transcoder transcoder(codepage.map);
      all_transcoders.insert(std::make_pair(codepage.codepage_id, transcoder));
    }
  }

  //----------------------------------------------------------------------------
  Transcoder::Transcoder(const uint16_t * char_to_single_utf16_map)
    : char_to_single_utf16_map_(char_to_single_utf16_map)
    , single_utf16_to_char_hash_table_(ALL_CHARS_LEN)
  {

    for (size_t i = 0; i < ALL_CHARS_LEN; ++i)
      AddMapping(char_to_single_utf16_map_[i], i);
  }

  //----------------------------------------------------------------------------
  const Transcoder * Transcoder::Find(const std::string & name)
  {
    const Lang * all_langs = get_langs();
    for (size_t l = 0; all_langs[l].name != NULL; ++l)
    {
      if (NKIT_STRCASECMP(all_langs[l].name, name.c_str()) == 0)
      {
        const Transcoders & all_transcoders = get_all_transcoders();
        Transcoders::const_iterator it =
                  all_transcoders.find(all_langs[l].codepage_id);
        if (it != all_transcoders.end())
          return & it->second;
      }
    }

    return NULL;
  }

  //----------------------------------------------------------------------------
  void Transcoder::FillExpatEncodingInfo(int * map) const
  {
    for (size_t i = 0; i < ALL_CHARS_LEN; ++i)
      map[i] = char_to_single_utf16_map_[i];
  }

  //----------------------------------------------------------------------------
  bool transcode(const std::string & enc_from, const std::string & enc_to,
          const std::string & from, std::string * to)
  {
    const Transcoder * tr_from = Transcoder::Find(enc_from);
    if (!tr_from)
      return false;
    const Transcoder * tr_to = Transcoder::Find(enc_to);
    if (!tr_to)
      return false;
    return tr_from->Transcode(from, *tr_to, to);
  }

  //----------------------------------------------------------------------------
  bool Transcoder::Transcode(const std::string & src,
          const Transcoder & out_transcoder,
          std::string * out) const
  {
    uint16_t utf16[2] = {0,0};
    std::string::const_iterator it = src.begin(), end = src.end();
    for (; it != end; ++it)
    {
      uint8_t ch = *it;
      utf16[0] = char_to_single_utf16_map_[ch];
      utf16[1] = 0;
      char c;
      if (!out_transcoder.GetChar(utf16[0], &c))
        return false;
      out->push_back(c);
    }
    return true;
  }

  //----------------------------------------------------------------------------
  bool Transcoder::FromUtf8(const char ** src, size_t * bytes_left,
          std::string * out) const
  {
    size_t simbol_length = utf8_size(**src);
    if (simbol_length > *bytes_left)
      return false;
    uint16_t utf16_char[2] = {0,0};
    if (!utf8_to_utf16(utf16_char, *src))
      return false;
    char c;
    if (!GetChar(utf16_char[0], &c))
      return false;
    out->push_back(c);
    *bytes_left -= simbol_length;
    *src += simbol_length;
    return true;
  }

  //----------------------------------------------------------------------------
  bool Transcoder::FromUtf8(const std::string & src, SPECIAL_CHAR_CALLBACK cb,
              std::string * out) const
  {
    const char * p = src.data();
    size_t bytes_left = src.size();
    while (bytes_left > 0)
    {
      if (unlikely(cb(*p, out)))
      {
        bytes_left--;
        p++;
      }
      else if (!FromUtf8(&p, &bytes_left, out))
        return false;
    }
    return true;
  }

  //----------------------------------------------------------------------------
  bool Transcoder::FromUtf8(const std::string & src, std::string * out) const
  {
    const char * p = src.data();
    size_t bytes_left = src.size();
    while (bytes_left > 0)
    {
      if (!FromUtf8(&p, &bytes_left, out))
        return false;
    }
    return true;
  }

  bool Transcoder::FromUtf8(const char * src, size_t size,
      std::string * out) const
  {
    return FromUtf8(&src, &size, out);
  }

  bool Transcoder::ToUtf8(const std::string & src, std::string * out) const
  {
    uint16_t utf16[2] = {0,0};
    char utf8[6] = {0,0,0,0,0,0};
    std::string::const_iterator c = src.begin(), end = src.end();
    for (; c != end; ++c)
    {
      uint8_t ch = *c;
      utf16[0] = char_to_single_utf16_map_[ch];
      utf16[1] = 0;
      uint8_t bytes_written = 0;
      if (!utf16_to_utf8(utf8, utf16, &bytes_written))
        return false;
      out->append(utf8, bytes_written);
    }
    return true;
  }

  //----------------------------------------------------------------------------
  void Transcoder::AddMapping(uint16_t single_utf16_c, char c)
  {
    single_utf16_to_char_hash_table_[
            single_utf16_c % ALL_CHARS_LEN][single_utf16_c] = c;
  }

  bool Transcoder::GetChar(uint16_t single_utf16_c, char * c) const
  {
    const SingleUtf16ToCharMap & mapping =
            single_utf16_to_char_hash_table_[single_utf16_c % ALL_CHARS_LEN];
    SingleUtf16ToCharMap::const_iterator it = mapping.find(single_utf16_c);
    if (unlikely(it == mapping.end()))
      return false;
    *c = it->second;
    return true;
  }

  //----------------------------------------------------------------------------
  int32_t utf16_to_ucs4(uint32_t *ucs4, const uint16_t *utf16,
    uint8_t * shorts_read = NULL)
  {
    if (utf16[0] < 0xD800)
    {
      *ucs4 = utf16[0];
      if (unlikely(shorts_read))
        *shorts_read = 1;
      return true;
    }

    if (utf16[0] < 0xDC00) {
      if ((utf16[1] >= 0xDC00) && (utf16[1] < 0xE000))
      {
        *ucs4 = (((utf16[0] - 0xD800) << 10)
          + (utf16[1] - 0xDC00)) + 0x00010000;
        if (unlikely(shorts_read))
          *shorts_read = 2;
        return true;
      }
      else return false;
    }

    if (utf16[0] < 0xFFFE)
    {
      *ucs4 = utf16[0];
      if (unlikely(shorts_read))
        *shorts_read = 1;
      return true;
    }

    return false;
  }

  //----------------------------------------------------------------------------
  bool ucs4_to_utf8(char *utf8, uint32_t ucs4, uint8_t *bytes_written)
  {
    if (ucs4 < 0x00000080)
    {
      utf8[0] = ucs4;
      *bytes_written = 1;
      return true;
    }

    if (ucs4 < 0x00000800)
    {
      utf8[0] = 0xc0 + (ucs4 >> 6);      // div 2^6
      utf8[1] = 0x80 + (ucs4 & 0x3F);    // mod 2^6
      *bytes_written = 2;
      return true;
    }

    // in UTF8 N.2 values in the range 0000 D800 to 0000 DFFF
    // shall be excluded from conversion.
    // (I don't see the need of this, because this range is
    // only relevant in UTF16 to decode 32-bit-values)
    // -----> 97-09-27 Marcus Mueller

    if (ucs4 < 0x00010000)
    {
      utf8[0] = 0xe0 + (ucs4 >> 12);
      utf8[1] = 0x80 + ((ucs4 >> 6) & 0x3F);
      utf8[2] = 0x80 + (ucs4 & 0x3F);
      *bytes_written = 3;
      return true;
    }

    if (ucs4 < 0x00200000)
    {
      utf8[0] = 0xf0 + (ucs4 >> 18);
      utf8[1] = 0x80 + ((ucs4 >> 12) & 0x3F);
      utf8[2] = 0x80 + ((ucs4 >> 6) & 0x3F);
      utf8[3] = 0x80 + (ucs4 & 0x3F);
      *bytes_written = 4;
      return true;
    }

    if (ucs4 < 0x04000000)
    {
      utf8[0] = 0xf8 + (ucs4 >> 24);
      utf8[1] = 0x80 + ((ucs4 >> 18) & 0x3F);
      utf8[2] = 0x80 + ((ucs4 >> 12) & 0x3F);
      utf8[3] = 0x80 + ((ucs4 >> 6) & 0x3F);
      utf8[4] = 0x80 + (ucs4 & 0x3F);
      *bytes_written = 5;
      return true;
    }

    if (ucs4 < 0x80000000)
    {
      utf8[0] = 0xfc + (ucs4 >> 30);
      utf8[1] = 0x80 + ((ucs4 >> 24) & 0x3F);
      utf8[2] = 0x80 + ((ucs4 >> 18) & 0x3F);
      utf8[3] = 0x80 + ((ucs4 >> 12) & 0x3F);
      utf8[4] = 0x80 + ((ucs4 >> 6) & 0x3F);
      utf8[5] = 0x80 + (ucs4 & 0x3F);
      *bytes_written = 6;
      return true;
    }

    *bytes_written = 0;
    return false;
  }

  //----------------------------------------------------------------------------
  bool ucs4_to_utf16(uint16_t *utf16, uint32_t ucs4)
  {
    if (ucs4 <= 0x0000FFFD)
    {
      utf16[0] = (uint16_t)ucs4;
      //utf16[1] = 0l;
      return true;
    }

    if ((ucs4 >= 0x00010000) && (ucs4 <= 0x0010FFFF))
    {
      utf16[0] = ((ucs4 - 0x00010000) >> 10) + 0xD800;
      utf16[1] = ((ucs4 - 0x00010000) & 0x3FF) + 0xDC00;
      //utf16[2] = 0l;
      return true;
    }

    return false;
  }

  //----------------------------------------------------------------------------
  int32_t utf8_to_ucs4(uint32_t *ucs4, const char *utf8, uint8_t *bytes_read)
  {
    register const uint8_t *utf = (uint8_t*)utf8;
    register uint8_t read = 0;
    register uint32_t RC = 0;

    if ((utf[0]) && ((utf[0] & 0xC0) != 0x80))
    {
      if (utf[0] < 0xC0)
      {
        RC = utf[0];
        read = 1;
      }
      else if ((utf[1] & 0xC0) == 0x80)
      {
        if (utf[0] < 0xE0)
        {
          RC = (((utf[0] - 0xC0) << 6)
            + (utf[1] - 0x80));
          read = 2;
        }
        else if ((utf[2] & 0xC0) == 0x80)
        {
          if (utf[0] < 0xF0)
          {
            RC = (((utf[0] - 0xE0) << 12)
              + ((utf[1] - 0x80) << 6)
              + (utf[2] - 0x80));
            read = 3;
          }
          else if ((utf[3] & 0xC0) == 0x80)
          {
            if (utf[0] < 0xF8)
            {
              RC = (((utf[0] - 0xF0) << 18)
                + ((utf[1] - 0x80) << 12)
                + ((utf[2] - 0x80) << 6)
                + (utf[3] - 0x80));
              read = 4;
            }
            else if ((utf[4] & 0xC0) == 0x80)
            {
              if (utf[0] < 0xFC)
              {
                RC = (((utf[0] - 0xF8) << 24)
                  + ((utf[1] - 0x80) << 18)
                  + ((utf[2] - 0x80) << 12)
                  + ((utf[3] - 0x80) << 6)
                  + (utf[4] - 0x80));
                read = 4;
              }
              else if ((utf[5] & 0xC0) == 0x80)
              {
                if (utf[0] < 0xFE)
                {
                  RC = (((utf[0] - 0xFC) << 30)
                    + ((utf[1] - 0x80) << 24)
                    + ((utf[2] - 0x80) << 18)
                    + ((utf[3] - 0x80) << 12)
                    + ((utf[4] - 0x80) << 6)
                    + (utf[5] - 0x80));
                  read = 5;
                }
              }
            }
          }
        }
      }
    }

    if (read)
    {
      if (bytes_read) *bytes_read = read;
      if (ucs4) *ucs4 = RC;
      return true;
    }

    return false;
  }

  //----------------------------------------------------------------------------
  bool utf8_to_utf16(uint16_t *utf16, const char *utf8, uint8_t *bytes_read)
  {
    uint32_t character;
    if (!utf8_to_ucs4(&character, utf8, bytes_read))
      return false;
    return ucs4_to_utf16(utf16, character);
  }

  //----------------------------------------------------------------------------
  bool utf16_to_utf8(char *utf8, const uint16_t *utf16, uint8_t * bytes_written)
  {
    uint32_t character;
    if (!utf16_to_ucs4(&character, utf16))
      return false;
    return ucs4_to_utf8(utf8, character, bytes_written);
  }

  //----------------------------------------------------------------------------
  uint8_t utf8_size(char first_utf8_byte)
  {
    register uint8_t RC = 0;
    register uint8_t first = (uint8_t)first_utf8_byte;

    if (first < 0x80)
      RC = 1;
    else if (first < 0xC0) // 10xxxxxx
      RC = 0;             // ERROR
    else if (first < 0xE0) // 110xxxxx
      RC = 2;
    else if (first < 0xF0) // 1110xxxx
      RC = 3;
    else if (first < 0xF8) // 11110xxx
      RC = 4;
    else if (first < 0xFC) // 111110xx
      RC = 5;
    else if (first < 0xFE) // 1111110x
      RC = 6;
    else                  // ERROR
      RC = 0;

    return(RC);
  }
} // namespace nkit
