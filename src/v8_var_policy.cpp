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

#include <node.h>
#include <nan.h>
#include "v8_var_policy.h"
#include "nkit/constants.h"

namespace nkit
{
  using namespace v8;

  Nan::Persistent<v8::Function> V8BuilderPolicy::date_constructor_;
  Nan::Persistent<v8::Value> V8BuilderPolicy::undefined_;

  void V8BuilderPolicy::Init()
  {
    Nan::HandleScope scope;

    Local<Object> global = Nan::GetCurrentContext()->Global();
    date_constructor_.Reset(
            Local<Function>::Cast(
                    global->Get(Nan::New("Date").ToLocalChecked())));
    undefined_.Reset(Nan::Undefined());
  }

  V8BuilderPolicy::V8BuilderPolicy(const detail::Options & NKIT_UNUSED(options))
    //: options_(options)
  {
    Nan::HandleScope scope;
    object_.Reset(Nan::New<Object>());
  }

  V8BuilderPolicy::~V8BuilderPolicy()
  {
    Nan::HandleScope scope;
    object_.Reset();
  }

  void V8BuilderPolicy::InitAsDict()
  {
    Nan::HandleScope scope;
    object_.Reset(Nan::New<Object>());
  }

  void V8BuilderPolicy::InitAsList()
  {
    Nan::HandleScope scope;
    object_.Reset(Nan::New<Array>());
  }

  void V8BuilderPolicy::ListCheck() const
  {
    assert(Local<Array>::Cast(Nan::New(object_))->IsArray());
  }

  void V8BuilderPolicy::DictCheck() const
  {
    assert(Local<Object>::Cast(Nan::New(object_))->IsObject());
  }

  void V8BuilderPolicy::InitAsBoolean(std::string const & value)
  {
    Nan::HandleScope scope;
    object_.Reset(
        Nan::New(nkit::bool_cast(value)));
  }

  void V8BuilderPolicy::InitAsString(std::string const & value)
  {
    Nan::HandleScope scope;
    object_.Reset(Nan::New(value).ToLocalChecked());
  }

  void V8BuilderPolicy::InitAsInteger(const std::string & value)
  {
    int32_t i = !value.empty() ? strtol(value.c_str(), NULL, 10) : 0;

    Nan::HandleScope scope;
    object_.Reset(Nan::New(i));
  }

  void V8BuilderPolicy::InitAsFloatFormat(std::string const & value,
      const char * format)
  {
    double d(0.0);
    if (!value.empty())
    {
      if (NKIT_SSCANF(value.c_str(), format, &d) == 0)
        d = 0.0;
    }

    Nan::HandleScope scope;
    object_.Reset(Nan::New(d));
  }

  void V8BuilderPolicy::InitAsDatetimeFormat(const std::string & value,
      const char * format)
  {
#if defined(_WIN32) || defined(_WIN64)
    struct tm _tm =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0};
#else
    struct tm _tm =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
#endif
    if (NKIT_STRPTIME(value.c_str(), format, &_tm) == NULL)
    {
      InitAsUndefined();
      return;
    }

    time_t tz_offset = nkit::timezone_offset() / 60;
    char tz_sign = '-';
    if (tz_offset < 0)
    {
      tz_offset *= -1;
      tz_sign = '+';
    }

    char tz_offset_hours[3];
    sprintf(tz_offset_hours, "%02u",
        static_cast<uint32_t>(tz_offset / 60));
    tz_offset_hours[2] = 0;

    char tz_offset_minutes[3];
    sprintf(tz_offset_minutes, "%02u",
        static_cast<uint32_t>(tz_offset % 60));
    tz_offset_minutes[2] = 0;

    static const char * WEEK_DAYS[7] =
    { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    static const char * MONTHS[12] =
    { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
        "Nov", "Dec" };
    static const size_t DATE_TIME_BUFFER_LENGTH = 32;
    char date_time_buf[DATE_TIME_BUFFER_LENGTH];
    strftime(date_time_buf, DATE_TIME_BUFFER_LENGTH,
        "WWW, %d MMM %Y %H:%M:%S ZHHMM", &_tm);

    strncpy(date_time_buf, WEEK_DAYS[_tm.tm_wday], 3);
    strncpy(date_time_buf + 8, MONTHS[_tm.tm_mon], 3);
    strncpy(date_time_buf + 26, &tz_sign, 1);
    strncpy(date_time_buf + 27, tz_offset_hours, 2);
    strncpy(date_time_buf + 29, tz_offset_minutes, 2);

    Nan::HandleScope scope;
    Local<Value> argv[1] = {
        Nan::New<String>(std::string(date_time_buf, DATE_TIME_BUFFER_LENGTH)).
          ToLocalChecked()
    };
    object_.Reset(
        Nan::New(date_constructor_)->NewInstance(1, argv));
  }

  void V8BuilderPolicy::InitAsUndefined()
  {
    Nan::HandleScope scope;
    object_.Reset(Nan::Undefined());
  }

  void V8BuilderPolicy::SetDictKeyValue(std::string const & key,
      type const & var)
  {
    Nan::HandleScope scope;
    Local<Value> object(Nan::New(object_));
    assert(object->IsObject());
    Local<Object> obj = Local<Object>::Cast(object);
    obj->Set(Nan::New(key).ToLocalChecked(), Nan::New(var));
  }

  void V8BuilderPolicy::AppendToList(type const & var)
  {
    Nan::HandleScope scope;
    Local<Value> object(Nan::New(object_));
    assert(object->IsArray());
    Local<Array> arr = Local<Array>::Cast(object);
    arr->Set(arr->Length(), Nan::New(var));
  }

  void V8BuilderPolicy::AppendToDictKeyList(std::string const & _key,
          type const & var)
  {
    Nan::HandleScope scope;
    Local<Value> object(Nan::New(object_));
    assert(object->IsObject());
    Local<Object> obj = Local<Object>::Cast(object);
    Local<String> key( Nan::New(_key).ToLocalChecked());
    if (obj->Has(key))
    {
      Local<Value> value = obj->Get(key);
      if (value->IsArray())
      {
        Local<Array> arr = Local<Array>::Cast(value);
        arr->Set(arr->Length(), Nan::New(var));
        return;
      }
    }

    Local<Array> arr(Nan::New<Array>());
    arr->Set(0, Nan::New(var));
    obj->Set(key, arr);
  }

  std::string V8BuilderPolicy::ToString() const
  {
    Nan::HandleScope scope;
    return v8var_to_json(Nan::New(object_));
  }

  std::string v8var_to_json(const Handle<Value> & var)
  {
    Nan::HandleScope scope;
    Local<Object> global = Nan::GetCurrentContext()->Global();
    Handle<Object> JSON = global->Get(Nan::New("JSON").ToLocalChecked())->ToObject();
    Handle<Function> JSON_stringify = Handle<Function>::Cast(
	  JSON->Get(Nan::New("stringify").ToLocalChecked()));
    Handle<Value> argv[3] = {
            var,
            Nan::Null(),
            Nan::New<String>("  ").ToLocalChecked()
    };
    String::Utf8Value ascii(JSON_stringify->Call(JSON, 3, argv));
    return *ascii;
  }

} // namespace vx
