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

#ifndef VX_V8_HANDLE_VAR_H
#define VX_V8_HANDLE_VAR_H

#include "v8.h"
#include "nkit/tools.h"
#include <string>

namespace vx
{
  class V8VarBuilder
  {
  public:
    typedef v8::Persistent<v8::Value> type;

    V8VarBuilder() {}

    ~V8VarBuilder()
    {
      object_.Dispose();
    }

    void InitAsDict()
    {
      v8::HandleScope handle_scope;
      object_.Dispose();
      object_ = v8::Persistent<v8::Object>::New(v8::Object::New());
    }

    void InitAsList()
    {
      v8::HandleScope handle_scope;
      object_.Dispose();
      object_ = v8::Persistent<v8::Array>::New(v8::Array::New());
    }

    void InitAsBoolean(std::string const & value)
    {
      v8::HandleScope handle_scope;
      object_.Dispose();
      object_ = v8::Persistent<v8::Boolean>::New(v8::Boolean::New(
        nkit::bool_cast(value)));
    }

    void InitAsBooleanFormat(std::string const & value, const std::string & )
    {
      InitAsBoolean(value);
    }

    void InitAsBooleanDefault()
    {
      InitAsBoolean(nkit::S_FALSE_);
    }

    void InitAsString(std::string const & value)
    {
      v8::HandleScope handle_scope;
      object_.Dispose();
      object_ = v8::Persistent<v8::String>::New(v8::String::New(value.c_str()));
    }

    void InitAsStringFormat(std::string const & value, const std::string & )
    {
      InitAsString(value);
    }

    void InitAsStringDefault()
    {
      InitAsString(nkit::S_EMPTY_);
    }

    void InitAsInteger(const std::string & value)
    {
#ifdef _WIN32
      int32_t i = !value.empty() ? strtol(value.c_str(), NULL, 10) : 0;
#else
      int64_t i = !value.empty() ? NKIT_STRTOLL(value.c_str(), NULL, 10) : 0;
#endif

      v8::HandleScope handle_scope;
      object_.Dispose();
      object_ = v8::Persistent<v8::Number>::New(v8::Number::New(i));
    }

    void InitAsIntegerFormat(std::string const & value, const std::string & )
    {
      InitAsInteger(value);
    }

    void InitAsIntegerDefault()
    {
      InitAsInteger(nkit::S_ZERO_);
    }

    void InitAsFloat(const std::string & value)
    {
      _InitAsFloatFormat(value, NKIT_FORMAT_DOUBLE);
    }

    void InitAsFloatFormat(std::string const & value,
        const std::string & format)
    {
      _InitAsFloatFormat(value, format.c_str());
    }

    void InitAsFloatDefault()
    {
      _InitAsFloatFormat(nkit::S_ZERO_, NKIT_FORMAT_DOUBLE);
    }

    void _InitAsFloatFormat(std::string const & value,
        const char * format)
    {
      double d(0.0);
      if (!value.empty())
      {
        if (NKIT_SSCANF(value.c_str(), format, &d) == 0)
          d = 0.0;
      }

      v8::HandleScope handle_scope;
      object_.Dispose();
      object_ = v8::Persistent<v8::Number>::New(v8::Number::New(d));
    }
#if !defined(_WIN32) && !defined(_WIN64)
    void InitAsDatetime(const std::string & value)
    {
      _InitAsDatetimeFormat(value, nkit::DATE_TIME_DEFAULT_FORMAT_);
    }
    void InitAsDatetimeFormat(const std::string & value,
        const std::string & format)
    {
      _InitAsDatetimeFormat(value, format.c_str());
    }

    void _InitAsDatetimeFormat(const std::string & value,
        const char * format)
    {
      struct tm _tm = {0,0,0,0,0,0,0,0,0,0,0};
      if (strptime(value.c_str(), format, &_tm) == NULL)
      {
        InitAsUndefined();
        return;
      }
      _tm.tm_isdst = 1;
      time_t t = mktime(&_tm);// - nkit::timezone_offset();

      v8::HandleScope handle_scope;
      object_.Dispose();
      object_ = v8::Persistent<v8::Value>::New(v8::Date::New(t*1000.0));
    }

    void InitAsDatetimeDefault()
    {
      v8::HandleScope handle_scope;
      object_.Dispose();
      object_ = v8::Persistent<v8::Value>::New(v8::Date::New(0));
    }
#endif

    void InitAsUndefined()
    {
      v8::HandleScope handle_scope;
      object_.Dispose();
      object_ = v8::Persistent<v8::Value>::New(v8::Undefined());
    }

    void SetDictKeyValue(std::string const & key, type const & var)
    {
      v8::HandleScope handle_scope;
      assert(object_->IsObject());
      v8::Handle<v8::Object> obj = v8::Handle<v8::Object>::Cast(object_);
      obj->Set(v8::String::New(key.c_str()), var);
    }

    void AppendToList(type const & obj)
    {
      v8::HandleScope handle_scope;
      assert(object_->IsArray());
      v8::Handle<v8::Array> arr = v8::Handle<v8::Array>::Cast(object_);
      arr->Set(arr->Length(), obj);
    }

    type const & get() const
    {
      return object_;
    }

  private:
    type object_;
  };

  inline std::string v8var_to_json(const v8::Handle<v8::Value> & var)
  {
    using namespace v8;
    Handle<Object> global = v8::Context::GetCurrent()->Global();
    Handle<Object> JSON = global->Get(String::New("JSON"))->ToObject();
    Handle<Function> JSON_stringify = Handle<Function>::Cast(
        JSON->Get(String::New("stringify")));
    Handle<Value> argv[] = { var, Null(), v8::String::New("  ") };
    String::Utf8Value ascii(JSON_stringify->Call(JSON, 3, argv));
    return *ascii;
  }
} // namespace vx

#endif // VX_V8_HANDLE_VAR_H
