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
#include "nkit/xml2var.h"
#include "nkit/var2xml.h"

namespace nkit
{
  std::string v8var_to_json(const v8::Handle<v8::Value> & var);

  class V8BuilderPolicy: Uncopyable
  {
  public:
    typedef v8::Persistent<v8::Value> type;

    static void Init();

    V8BuilderPolicy(const detail::Options & options);
    ~V8BuilderPolicy();

    static const type & GetUndefined()
    {
      return undefined_;
    }

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
    void AppendToDictKeyList(std::string const & key, type const & var);

    const type & get() const { return object_; }
    std::string ToString() const;
    void ListCheck() const;
    void DictCheck() const;

  private:
    type object_;
    //const detail::Options & options_;
    static v8::Persistent<v8::Function> date_constructor_;
    static v8::Persistent<v8::Value> undefined_;
  };

  ////--------------------------------------------------------------------------
  struct V8ReaderPolicy
  {
    typedef v8::Local<v8::Value> type;

    static const uint32_t END = -1;

    //--------------------------------------------------------------------------
    struct DictConstIterator
    {
      DictConstIterator()
        : dict_()
        , keys_()
        , size_(0)
        , pos_(END)
      {}

      DictConstIterator(const type & dict)
        : dict_()
        , keys_()
        , size_(0)
        , pos_(END)
      {
        NanAssignPersistent(dict_, v8::Local<v8::Object>::Cast(dict));
        NanAssignPersistent(keys_,
            NanNew(dict_)->GetOwnPropertyNames());
        size_ = NanNew(keys_)->Length();
        if (size_ > 0)
        {
          pos_ = 0;
          FetchKeyValue();
        }
      }

      DictConstIterator(const DictConstIterator & copy)
        : dict_()
		, keys_()
		, key_()
		, value_()
        , size_(copy.size_)
        , pos_(copy.pos_)
      {
        NanAssignPersistent(dict_, copy.dict_);
        NanAssignPersistent(keys_, copy.keys_);
        NanAssignPersistent(key_, copy.key_);
        NanAssignPersistent(value_, copy.value_);
      }

      DictConstIterator & operator = (const DictConstIterator & copy)
      {
        NanAssignPersistent(dict_, copy.dict_);
        NanAssignPersistent(keys_, copy.keys_);
        NanAssignPersistent(key_, copy.key_);
        NanAssignPersistent(value_, copy.value_);
        size_ = copy.size_;
        pos_ = copy.pos_;
        return *this;
      }

      ~DictConstIterator()
      {
        NanDisposePersistent(dict_);
        NanDisposePersistent(keys_);
        NanDisposePersistent(key_);
        NanDisposePersistent(value_);
      }

      void FetchKeyValue()
      {
        if (pos_ == END)
          return;

        NanAssignPersistent(key_, NanNew(keys_)->Get(pos_));
        NanAssignPersistent(value_, NanNew(dict_)->Get(NanNew(key_)));
      }

      bool operator != (const DictConstIterator & another)
      {
        return pos_ != another.pos_;
      }

      DictConstIterator & operator++()
      {
        if (unlikely(++pos_ >= size_))
          pos_ = END;
        else
          FetchKeyValue();
        return *this;
      }

      std::string first() const
      {
        if (pos_ == END)
          return S_EMPTY_;

        v8::String::Utf8Value tmp(NanNew(key_));
        std::string ret(*tmp, tmp.length());
        return ret;
      }

      type second() const
      {
        NanEscapableScope();
        if (unlikely(pos_ == END))
          return NanEscapeScope(v8::Local<v8::Value>(NanUndefined()));
        else
          return NanEscapeScope(NanNew(value_));
      }

      v8::Persistent<v8::Object> dict_;
      v8::Persistent<v8::Array> keys_;
      v8::Persistent<v8::Value> key_;
      v8::Persistent<v8::Value> value_;
      uint32_t size_;
      uint32_t pos_;
    };

    //--------------------------------------------------------------------------
    struct ListConstIterator
    {
      ListConstIterator()
        : list_()
        , size_(0)
        , pos_(END)
      {}

      ListConstIterator(const type & list)
        : list_()
        , size_(0)
        , pos_(0)
      {
        NanAssignPersistent(list_, v8::Local<v8::Array>::Cast(list));
        size_ = NanNew(list_)->Length();
        if (size_ == 0)
          pos_ = END;
      }

      ListConstIterator(const ListConstIterator & copy)
        : list_()
        , size_(copy.size_)
        , pos_(copy.pos_)
      {
        NanAssignPersistent(list_, copy.list_);
      }

      ListConstIterator & operator = (const ListConstIterator & copy)
      {
        NanAssignPersistent(list_, copy.list_);
        size_ = copy.size_;
        pos_ = copy.pos_;
        return *this;
      }

      ~ListConstIterator()
      {
        NanDisposePersistent(list_);
      }

      bool operator != (const ListConstIterator & another)
      {
        return pos_ != another.pos_;
      }

      ListConstIterator & operator++()
      {
        if (unlikely(++pos_ >= size_))
          pos_ = END;
        return *this;
      }

      type value() const
      {
        NanEscapableScope();
        if (unlikely(pos_ == END))
          return NanEscapeScope(v8::Local<v8::Value>(NanUndefined()));
        else
          return NanEscapeScope(NanNew(list_)->Get(pos_));
      }

      v8::Persistent<v8::Array> list_;
      uint32_t size_;
      uint32_t pos_;
    };

    //--------------------------------------------------------------------------
    static DictConstIterator begin_d(const type & data)
    {
      return DictConstIterator(data);
    }

    static DictConstIterator end_d(const type & data)
    {
      return DictConstIterator();
    }

    static ListConstIterator begin_l(const type & data)
    {
      return ListConstIterator(data);
    }

    static ListConstIterator end_l(const type & data)
    {
      return ListConstIterator();
    }

    static std::string First(const DictConstIterator & it)
    {
      return it.first();
    }

    static type Second(const DictConstIterator & it)
    {
      NanEscapableScope();
      return NanEscapeScope(it.second());
    }

    static type Value(const ListConstIterator & it)
    {
      NanEscapableScope();
      return NanEscapeScope(it.value());
    }

    static bool IsList(const type & data)
    {
      bool ret = data->IsArray();
      return ret;
    }

    static bool IsDict(const type & data)
    {
      bool ret = data->IsObject() && !(data->IsArray()
          || data->IsStringObject()
          || data->IsNumberObject()
          || data->IsBooleanObject()
          || data->IsDate()
          );
      return ret;
    }

    static bool IsString(const type & data)
    {
      return data->IsString() || data->IsStringObject();
    }

    static bool IsFloat(const type & data)
    {
      return (data->IsNumber() || data->IsNumberObject()) &&
          !(data->IsInt32() || data->IsUint32());
    }

    static bool IsDateTime(const type & data)
    {
      return data->IsDate();
    }

    static bool IsBool(const type & data)
    {
      return data->IsBoolean() || data->IsBooleanObject();
    }

    static std::string GetString(const type & data)
    {
      v8::String::Utf8Value tmp(data->ToString());
      std::string str(*tmp, tmp.length());
      return str;
    }

    static std::string GetStringAsDateTime(const type & data,
            const std::string & format)
    {
      NanScope();
      v8::Local<v8::Date> date = v8::Local<v8::Date>::Cast(data);
      double _timestamp = date->NumberValue();
      time_t timestamp = time_t(_timestamp) / 1000;
      struct tm loc;
      LOCALTIME_R(timestamp, &loc);
      static const size_t STRFTIME_BUF_SIZE = 256;
      char buf[STRFTIME_BUF_SIZE];
      size_t len = strftime(buf, STRFTIME_BUF_SIZE, format.c_str(), &loc);
      if (!len)
        return "Too big datetime format";
      return std::string(buf, len);
    }

    static std::string GetStringAsFloat(const type & data,
            size_t precision)
    {
      NanScope();
      return string_cast(data->ToNumber()->NumberValue(), precision);
    }

    static const std::string & GetStringAsBool(const type & data,
        const std::string & true_format, const std::string & false_format)
    {
      return data->ToBoolean()->BooleanValue() ?
          true_format: false_format;
    }

    static type GetByKey(const type & data, const std::string & _key,
        bool * found)
    {
      NanEscapableScope();
      v8::Local<v8::Object> dict = v8::Local<v8::Object>::Cast(data);
      v8::Local<v8::String> key = NanNew(_key.c_str());
      *found = dict->HasOwnProperty(key);
      if (likely(*found))
        return NanEscapeScope(dict->Get(NanNew(key)));
      else
        return NanEscapeScope(v8::Local<v8::Value>(NanUndefined()));
    }

    static v8::Persistent<v8::Function> date_constructor_;
  };

  typedef Var2XmlConverter<V8ReaderPolicy> V8ToXmlConverter;

} // namespace nkit

#endif // VX_V8_VAR_BUILDER_H
