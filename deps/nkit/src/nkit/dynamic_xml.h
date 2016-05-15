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

#ifndef NKIT_DYNAMIC_XML_H
#define NKIT_DYNAMIC_XML_H

#include "nkit/dynamic.h"

namespace nkit
{
  Dynamic DynamicFromAnyXml(const std::string & xml,
        const std::string & options,
        std::string * const root_name,
        std::string * const error);
  Dynamic DynamicFromAnyXml(const std::string & xml,
        const Dynamic & options,
        std::string * const root_name,
        std::string * const error);
  Dynamic DynamicFromAnyXmlFile(const std::string & path,
        const std::string & options,
        std::string * const root_name,
        std::string * const error);
  Dynamic DynamicFromAnyXmlFile(const std::string & path,
        const Dynamic & options,
        std::string * const root_name,
        std::string * const error);
  Dynamic DynamicFromXml(const std::string & xml,
      const Dynamic & options,
      const Dynamic & mapping,
      std::string * const error);
  Dynamic DynamicFromXml(const std::string & xml,
      const Dynamic & mapping,
      std::string * const error);
  Dynamic DynamicFromXml(const std::string & xml,
      const std::string & options,
      const std::string & mapping,
      std::string * const error);
  Dynamic DynamicFromXml(const std::string & xml,
      const std::string & mapping,
      std::string * const error);
  Dynamic DynamicFromXmlFile(const std::string & path,
      const std::string & options,
      const std::string & mapping,
      std::string * const error);
  Dynamic DynamicFromXmlFile(const std::string & path,
      const std::string & mapping,
      std::string * const error);
} // namespace nkit


#endif // NKIT_DYNAMIC_XML_H
