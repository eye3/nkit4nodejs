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

#ifndef NKIT_TRANSCODE_H
#define NKIT_TRANSCODE_H

namespace nkit
{
  //----------------------------------------------------------------------------
  bool transcode(const std::string & enc_from, const std::string & enc_to,
          const std::string & from, std::string * to);

  //----------------------------------------------------------------------------
  class Transcoder
  {
    friend bool transcode(const std::string & enc_from,
            const std::string & enc_to,
            const std::string & from,
            std::string * to);

  private:
    typedef std::map<uint16_t, char> SingleUtf16ToCharMap;
    typedef bool (*SPECIAL_CHAR_CALLBACK)(char ch, std::string * out);

  public:
    Transcoder(const uint16_t * char_to_single_utf16_map);
    static void Build();
    static const Transcoder * Find(const std::string & name);
    void FillExpatEncodingInfo(int * map) const;
    bool FromUtf8(const std::string & src, std::string * out) const;
    bool FromUtf8(const std::string & src, SPECIAL_CHAR_CALLBACK cb,
            std::string * out) const;
    bool FromUtf8(const char * src, size_t size, std::string * out) const;
    bool ToUtf8(const std::string & src, std::string * out) const;

  private:
    bool Transcode(const std::string & src,
            const Transcoder & out_transcoder,std::string * out) const;
    bool FromUtf8(const char ** src, size_t * bytes_left,
            std::string * out) const;
    void AddMapping(uint16_t single_utf16_c, char c);
    bool GetChar(uint16_t single_utf16_c, char * c) const;

  private:
    const uint16_t * char_to_single_utf16_map_;
    std::vector<SingleUtf16ToCharMap> single_utf16_to_char_hash_table_;
  };

} // namespace nkit

#endif // NKIT_TRANSCODE_H
