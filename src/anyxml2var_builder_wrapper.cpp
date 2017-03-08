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

#include "anyxml2var_builder_wrapper.h"

#include "nkit/xml2var.h"

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
  Nan::Persistent<Function> AnyXml2VarBuilderWrapper::constructor;

  //------------------------------------------------------------------------------
  void AnyXml2VarBuilderWrapper::Init(Handle<Object> exports)
  {
    Nan::HandleScope scope;

    // Prepare constructor template
    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(
            AnyXml2VarBuilderWrapper::New);
    tpl->SetClassName(Nan::New("AnyXml2VarBuilder").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    Nan::SetPrototypeMethod(tpl, "feed", AnyXml2VarBuilderWrapper::Feed);
    Nan::SetPrototypeMethod(tpl, "end", AnyXml2VarBuilderWrapper::End);
    Nan::SetPrototypeMethod(tpl, "get", AnyXml2VarBuilderWrapper::Get);
    Nan::SetPrototypeMethod(tpl, "root_name",
            AnyXml2VarBuilderWrapper::GetRootName);
    constructor.Reset(tpl->GetFunction());
    exports->Set(Nan::New("AnyXml2VarBuilder").ToLocalChecked(),
        tpl->GetFunction());
  }

  //------------------------------------------------------------------------------
  NAN_METHOD(AnyXml2VarBuilderWrapper::New)
  {
    Nan::HandleScope scope;

    if (!info.IsConstructCall())
    {
      // Invoked as plain function `AnyXml2VarBuilder(...)`
      return Nan::ThrowError("Can't call constructor as a function");
    }

    std::string options("{}"), mappings;
    if (1 != info.Length())
      return Nan::ThrowError("Expected exactly one argument - options");
    else
    {
      if (!parse_object(info[0], &options))
        return Nan::ThrowError("Options parameter must be JSON-string or Object");
    }

    std::string error;
    AnyXml2VarBuilder<V8VarBuilder>::Ptr builder =
        AnyXml2VarBuilder<V8VarBuilder>::Create(options, &error);

    if (!builder)
      return Nan::ThrowError(error.c_str());

    AnyXml2VarBuilderWrapper* obj = new AnyXml2VarBuilderWrapper(builder);
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  }

  //------------------------------------------------------------------------------
  NAN_METHOD(AnyXml2VarBuilderWrapper::Feed)
  {
    Nan::HandleScope scope;

    if (1 > info.Length())
      return Nan::ThrowError("Expected String or Buffer parameter");

    AnyXml2VarBuilderWrapper* obj = ObjectWrap::Unwrap<AnyXml2VarBuilderWrapper>(
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
  NAN_METHOD(AnyXml2VarBuilderWrapper::Get)
  {
    Nan::HandleScope scope;

    AnyXml2VarBuilderWrapper* obj = ObjectWrap::Unwrap<AnyXml2VarBuilderWrapper>(
        info.This());
    Local<Object> result = Local<Object>::Cast(
        Nan::New<Value>(obj->builder_->var()));
    info.GetReturnValue().Set(result);
  }

  //------------------------------------------------------------------------------
  NAN_METHOD(AnyXml2VarBuilderWrapper::GetRootName)
  {
    Nan::HandleScope scope;

    AnyXml2VarBuilderWrapper* obj = ObjectWrap::Unwrap<AnyXml2VarBuilderWrapper>(
        info.This());
    Local<String> result =
            Nan::New<String>(obj->builder_->root_name()).ToLocalChecked();
    info.GetReturnValue().Set(result);
  }

  //------------------------------------------------------------------------------
  NAN_METHOD(AnyXml2VarBuilderWrapper::End)
  {
    Nan::HandleScope scope;

    AnyXml2VarBuilderWrapper* obj = ObjectWrap::Unwrap<AnyXml2VarBuilderWrapper>(
        info.This());

    std::string empty = "";
    std::string error;
    if (!obj->builder_->Feed(empty.c_str(), empty.size(), true, &error))
      return Nan::ThrowError(error.c_str());

    Local<Value> result = Nan::New(obj->builder_->var());

    info.GetReturnValue().Set(result);
  }

}  // namespace nkit
