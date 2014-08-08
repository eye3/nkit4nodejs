#include "nkit/tools.h"
#include "nkit/detail/encodings.h"

namespace nkit
{
  static const size_t ALL_CHARS_LEN = 0x100;

  struct Encoding
  {
    const char * encoding_name;
    const int map[ALL_CHARS_LEN];
  };

#include "vx/encodings.inc"

  bool find_encoding(const char * name, XML_Encoding * info)
  {
    size_t encoding_count = sizeof(encodings) / sizeof(Encoding);
    for (size_t e = 0; e < encoding_count; ++e)
    {
      const Encoding & encoding = encodings[e];
      if (NKIT_STRCASECMP(encoding.encoding_name, name) == 0)
      {
        for (size_t i = 0; i < ALL_CHARS_LEN; ++i)
          info->map[i] = encoding.map[i];
        return true;
      }
    }

    return false;
  }

} // namespace nkit
