/*
   Copyright 2014 Boris T. Darchiev (boris.darchiev@gmail.com)

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

#ifndef VX_V8_VAR_BUILDER_H
#define VX_V8_VAR_BUILDER_H

#include <string>

#include "nkit/tools.h"

namespace vx
{
  class V8VarBuilder
  {
  public:
    typedef v8::Persistent<v8::Value> type;

    V8VarBuilder();
    ~V8VarBuilder();
    void InitAsDict();
    void InitAsList();
    void InitAsBoolean(std::string const & value);
    void InitAsBooleanFormat(std::string const & value, const std::string & );
    void InitAsBooleanDefault();
    void InitAsString(std::string const & value);
    void InitAsStringFormat(std::string const & value, const std::string & );
    void InitAsStringDefault();
    void InitAsInteger(const std::string & value);
    void InitAsIntegerFormat(std::string const & value, const std::string & );
    void InitAsIntegerDefault();
    void InitAsFloat(const std::string & value);
    void InitAsFloatFormat(std::string const & value,
        const std::string & format);
    void InitAsFloatDefault();
    void _InitAsFloatFormat(std::string const & value,
        const char * format);

#if !defined(_WIN32) && !defined(_WIN64)
    void InitAsDatetime(const std::string & value);
    void InitAsDatetimeFormat(const std::string & value,
        const std::string & format);
    void _InitAsDatetimeFormat(const std::string & value,
        const char * format);
    void InitAsDatetimeDefault();
#endif

    void InitAsUndefined();
    void SetDictKeyValue(std::string const & key, type const & var);
    void AppendToList(type const & obj);

    type const & get() const
    {
      return object_;
    }

  private:
    type object_;
#if !defined(_WIN32) && !defined(_WIN64)
    v8::Persistent<v8::Function> date_constructor_;
#endif
  };

  std::string v8var_to_json(const v8::Handle<v8::Value> & var);
} // namespace vx

#endif // VX_V8_VAR_BUILDER_H
