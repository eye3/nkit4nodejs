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
#include "nkit/vx.h"

namespace nkit
{
  std::string v8var_to_json(const v8::Handle<v8::Value> & var);

  class V8VarPolicy: Uncopyable
  {
  public:
    typedef v8::Persistent<v8::Value> type;

    static void Init();

    V8VarPolicy(const detail::Options & options);
    ~V8VarPolicy();
    void InitAsDict();
    void InitAsList();
    void InitAsBoolean(std::string const & value);
    void InitAsString(std::string const & value);
    void InitAsInteger(const std::string & value);
    void InitAsFloatFormat(std::string const & value,
        const char * format);
    void InitAsDatetimeFormat(const std::string & value,
        const char * format);
    void InitAsUndefined();
    void SetDictKeyValue(std::string const & key, type const & var);
    void AppendToList(type const & obj);

    type const & get() const { return object_; }
    std::string ToString() const;
    void ListCheck() const;
    void DictCheck() const;

  private:
    type object_;
    const detail::Options & options_;
    static v8::Persistent<v8::Function> date_constructor_;
  };
} // namespace nkit

#endif // VX_V8_VAR_BUILDER_H
