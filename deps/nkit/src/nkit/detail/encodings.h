#ifndef VX_ENCODINGS_H
#define VX_ENCODINGS_H

#include "expat.h"

namespace nkit
{
  bool find_encoding(const char * name, XML_Encoding * info);

} // namespace nkit

#endif // VX_ENCODINGS_H
