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
  Persistent<Function> AnyXml2VarBuilderWrapper::constructor;

  //------------------------------------------------------------------------------
  void AnyXml2VarBuilderWrapper::Init(Handle<Object> exports)
  {
    // Prepare constructor template
    Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(
            AnyXml2VarBuilderWrapper::New);
    tpl->SetClassName(NanNew("AnyXml2VarBuilder"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(tpl, "feed", AnyXml2VarBuilderWrapper::Feed);
    NODE_SET_PROTOTYPE_METHOD(tpl, "end", AnyXml2VarBuilderWrapper::End);
    NODE_SET_PROTOTYPE_METHOD(tpl, "get", AnyXml2VarBuilderWrapper::Get);
    NODE_SET_PROTOTYPE_METHOD(tpl, "root_name", AnyXml2VarBuilderWrapper::GetRootName);
    NanAssignPersistent(constructor, tpl->GetFunction());
    exports->Set(NanNew("AnyXml2VarBuilder"), NanNew(constructor));
  }

  //------------------------------------------------------------------------------
  NAN_METHOD(AnyXml2VarBuilderWrapper::New)
  {
    NanScope();

    if (!args.IsConstructCall())
    {
      // Invoked as plain function `AnyXml2VarBuilder(...)`
      return NanThrowError("Can't call constructor as a function");
    }

    std::string options("{}"), mappings;
    if (1 != args.Length())
      return NanThrowError("Expected exactly one argument - options");
    else
    {
      if (!parse_object(args[0], &options))
        return NanThrowError("Options parameter must be JSON-string or Object");
    }

    std::string error;
    AnyXml2VarBuilder<V8VarBuilder>::Ptr builder =
        AnyXml2VarBuilder<V8VarBuilder>::Create(options, &error);

    if (!builder)
      return NanThrowError(error.c_str());

    AnyXml2VarBuilderWrapper* obj = new AnyXml2VarBuilderWrapper(builder);
    obj->Wrap(args.This());
    NanReturnValue(args.This());
  }

  //------------------------------------------------------------------------------
  NAN_METHOD(AnyXml2VarBuilderWrapper::Feed)
  {
    NanScope();

    if (1 > args.Length())
      return NanThrowError("Expected String or Buffer parameter");

    AnyXml2VarBuilderWrapper* obj = ObjectWrap::Unwrap<AnyXml2VarBuilderWrapper>(
        args.This());

    bool result = false;
    std::string error;
    if (node::Buffer::HasInstance(args[0]))
    {
      char* data;
      size_t length;
      get_buffer_data(args[0], &data, &length);

      result = obj->builder_->Feed(data, length, false, &error);
    }
    else if (args[0]->IsString())
    {
      String::Utf8Value utf8_value(args[0]);
      result = obj->builder_->Feed(
          *utf8_value, utf8_value.length(), false, &error);
    }
    else
      return NanThrowTypeError("Expected String or Buffer parameter");

    if (!result)
      return NanThrowError(error.c_str());

    NanReturnUndefined();
  }

  //------------------------------------------------------------------------------
  NAN_METHOD(AnyXml2VarBuilderWrapper::Get)
  {
    NanScope();

    AnyXml2VarBuilderWrapper* obj = ObjectWrap::Unwrap<AnyXml2VarBuilderWrapper>(
        args.This());
    Local<Object> result = Local<Object>::Cast(
        NanNew<Value>(obj->builder_->var()));
    NanReturnValue(result);
  }

  //------------------------------------------------------------------------------
  NAN_METHOD(AnyXml2VarBuilderWrapper::GetRootName)
  {
    NanScope();

    AnyXml2VarBuilderWrapper* obj = ObjectWrap::Unwrap<AnyXml2VarBuilderWrapper>(
        args.This());
    Local<String> result = NanNew<String>(obj->builder_->root_name());
    NanReturnValue(result);
  }

  //------------------------------------------------------------------------------
  NAN_METHOD(AnyXml2VarBuilderWrapper::End)
  {
    NanScope();

    AnyXml2VarBuilderWrapper* obj = ObjectWrap::Unwrap<AnyXml2VarBuilderWrapper>(
        args.This());

    std::string empty = "";
    std::string error;
    if (!obj->builder_->Feed(empty.c_str(), empty.size(), true, &error))
      return NanThrowError(error.c_str());

    Local<Value> result = NanNew(obj->builder_->var());

    NanReturnValue(result);
  }

}  // namespace nkit
