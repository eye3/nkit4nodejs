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

#include "nkit/vx.h"
#include "nkit/dynamic/dynamic_builder.h"
#include "nkit/dynamic_xml.h"

namespace nkit
{
  Dynamic DynamicFromXml(const std::string & xml,
        const std::string & fields_mapping, std::string * const error)
  {
    Xml2VarBuilder<DynamicBuilder>::Ptr d_gen = Xml2VarBuilder<
        DynamicBuilder>::Create(fields_mapping, error);
    if(!d_gen)
      return Dynamic();
    if (!d_gen->Feed(xml.c_str(), xml.length(), true, error))
      return Dynamic();
    return d_gen->var();
  }

  Dynamic DynamicFromXmlFile(const std::string & path,
        const std::string & fields_mapping, std::string * const error)
  {
    std::string xml;
    if (!path.empty() && !text_file_to_string(path, &xml))
    {
      *error = "Could not open file: '" + path + "'";
      return Dynamic();
    }

    if (xml.empty())
    {
      *error = "Could not open file: '" + path + "'";
      return Dynamic();
    }

    return DynamicFromXml(xml, fields_mapping, error);
  }
} // namespace nkit
