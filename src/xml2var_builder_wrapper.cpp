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

#include "nkit/logger_brief.h"

#include <node_buffer.h>

#include "xml2var_builder_wrapper.h"

#include "nkit/xml2var.h"
#include "nkit/var2xml.h"

namespace nkit
{
  using namespace v8;

  //------------------------------------------------------------------------------
  template <typename T>
  void get_buffer_data(const T & buffer, char ** data, size_t * len)
  {
  #if NODE_MINOR_VERSION == 8
      Local<Object> obj = Local<Object>::Cast(buffer);
      *data = node::Buffer::Data(obj);
      *len = node::Buffer::Length(obj);
  #else
      *data = node::Buffer::Data(buffer);
      *len = node::Buffer::Length(buffer);
  #endif
  }

  //------------------------------------------------------------------------------
  template <typename T>
  bool parse_object(const T & arg, std::string * out)
  {
    if (node::Buffer::HasInstance(arg))
    {
      char* data;
      size_t length;
      get_buffer_data(arg, &data, &length);
      out->assign(data, length);
    }
    else if (arg->IsString())
      out->assign(*String::Utf8Value(arg));
    else if (arg->IsObject())
      out->assign(v8var_to_json(arg));
    else
      return false;
    return true;
  }

  //------------------------------------------------------------------------------
  NAN_METHOD(var2xml)
  {
    Nan::HandleScope scope;

    std::string options("{}");
    if (1 > info.Length())
      return Nan::ThrowError("Expected JavaScript structure"
          " and/or options object");
    else if (2 <= info.Length())
    {
      if (!parse_object(info[1], &options))
        return Nan::ThrowError("Options parameter must be JSON-string or Object");
    }

    std::string ret, error;

    Dynamic op = DynamicFromJson(options, &error);
    if (!op.IsDict())
      return Nan::ThrowError("Options parameter must be JSON-string or Object");

    if (!V8ToXmlConverter::Process(options, info[0], &ret, &error))
      return Nan::ThrowError(error.c_str());

    Dynamic * as_buffer;
    bool to_string = op.Get("as_buffer", &as_buffer) ? !*as_buffer : false;

    if (to_string
        && istrequal(op["encoding"].GetConstString(), std::string("utf-8")))
      info.GetReturnValue().Set(Nan::New(ret).ToLocalChecked());
    else
      info.GetReturnValue().Set(Nan::CopyBuffer(ret.c_str(),ret.size()).
              ToLocalChecked());
  }

  //------------------------------------------------------------------------------
  Nan::Global<Function> Xml2VarBuilderWrapper::constructor;

  //------------------------------------------------------------------------------
  void Xml2VarBuilderWrapper::Init(Handle<Object> exports)
  {
    Nan::HandleScope scope;

    // Prepare constructor template
    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(
            Xml2VarBuilderWrapper::New);
    tpl->SetClassName(Nan::New("Xml2VarBuilder").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    Nan::SetPrototypeMethod(tpl, "feed", Xml2VarBuilderWrapper::Feed);
    Nan::SetPrototypeMethod(tpl, "end", Xml2VarBuilderWrapper::End);
    Nan::SetPrototypeMethod(tpl, "get", Xml2VarBuilderWrapper::Get);
    constructor.Reset(tpl->GetFunction());
    exports->Set(Nan::New("Xml2VarBuilder").ToLocalChecked(), Nan::New(constructor));
    exports->Set(Nan::New("var2xml").ToLocalChecked(),
        Nan::New<FunctionTemplate>(var2xml)->GetFunction());

  }

  //------------------------------------------------------------------------------
  NAN_METHOD(Xml2VarBuilderWrapper::New)
  {
    Nan::HandleScope scope;

    if (!info.IsConstructCall())
    {
      // Invoked as plain function `Xml2VarBuilder(...)`
      return Nan::ThrowError("Can't call constructor as a function");
    }

    std::string options("{}"), mappings;
    if (1 > info.Length())
      return Nan::ThrowError("Expected one or two arguments:"
          " 1) mappings or 2) options and mappings");
    else if (1 == info.Length())
    {
      if (!parse_object(info[0], &mappings))
        return Nan::ThrowError("Mappings parameter must be JSON-string or Object");
    }
    else
    {
      if (!parse_object(info[0], &options))
        return Nan::ThrowError("Options parameter must be JSON-string or Object");
      if (!parse_object(info[1], &mappings))
        return Nan::ThrowError("Mappings parameter must be JSON-string or Object");
    }

    std::string error;
    StructXml2VarBuilder<V8VarBuilder>::Ptr builder =
        StructXml2VarBuilder<V8VarBuilder>::Create(options, mappings, &error);

    if (!builder)
      return Nan::ThrowError(error.c_str());

    Xml2VarBuilderWrapper* obj = new Xml2VarBuilderWrapper(builder);
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  }

  //------------------------------------------------------------------------------
  NAN_METHOD(Xml2VarBuilderWrapper::Feed)
  {
    Nan::HandleScope scope;

    if (1 > info.Length())
      return Nan::ThrowError("Expected String or Buffer parameter");

    Xml2VarBuilderWrapper* obj = ObjectWrap::Unwrap<Xml2VarBuilderWrapper>(
        info.This());

    bool result = false;
    std::string error;
    if (node::Buffer::HasInstance(info[0]))
    {
      char* data;
      size_t length;
      get_buffer_data(info[0], &data, &length);

      result = obj->builder_->Feed(data, length, false, &error);
    }
    else if (info[0]->IsString())
    {
      String::Utf8Value utf8_value(info[0]);
      result = obj->builder_->Feed(
          *utf8_value, utf8_value.length(), false, &error);
    }
    else
      return Nan::ThrowTypeError("Expected String or Buffer parameter");

    if (!result)
      return Nan::ThrowError(error.c_str());

    info.GetReturnValue().Set(Nan::Undefined());
  }

  //------------------------------------------------------------------------------
  NAN_METHOD(Xml2VarBuilderWrapper::Get)
  {
    Nan::HandleScope scope;

    if (1 > info.Length())
      return Nan::ThrowError("Expected mapping name: String or Buffer");

    Xml2VarBuilderWrapper* obj = ObjectWrap::Unwrap<Xml2VarBuilderWrapper>(
        info.This());

    Local<Object> result;
    std::string error;
    if (node::Buffer::HasInstance(info[0]))
    {
      char* str;
      size_t length;
      get_buffer_data(info[0], &str, &length);
      std::string mapping_name(str, length);
      result = Local<Object>::Cast(
          Nan::New<Value>(obj->builder_->var(mapping_name)));
    }
    else if (info[0]->IsString())
    {
      String::Utf8Value utf8_value(info[0]);
      std::string mapping_name(*utf8_value, utf8_value.length());
      result = Local<Object>::Cast(
          Nan::New<Value>(obj->builder_->var(mapping_name)));
    }
    else
      return Nan::ThrowTypeError("Expected mapping name: String or Buffer");

    info.GetReturnValue().Set(result);
  }

  //------------------------------------------------------------------------------
  NAN_METHOD(Xml2VarBuilderWrapper::End)
  {
    Nan::HandleScope scope;

    Xml2VarBuilderWrapper* obj = ObjectWrap::Unwrap<Xml2VarBuilderWrapper>(
        info.This());

    std::string empty = "";
    std::string error;
    if (!obj->builder_->Feed(empty.c_str(), empty.size(), true, &error))
      return Nan::ThrowError(error.c_str());

    StringList mapping_names(obj->builder_->mapping_names());

    Local<Object> result = Nan::New<Object>();
    StringList::const_iterator mapping_name = mapping_names.begin(),
        end = mapping_names.end();
    for (; mapping_name != end; ++mapping_name)
    {
      Local<Value> item = Nan::New(obj->builder_->var(*mapping_name));
      result->Set(Nan::New(*mapping_name).ToLocalChecked(), item);
    }

    info.GetReturnValue().Set(result);
  }

}  // namespace nkit
