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
    NanScope();

    std::string options("{}");
    if (1 > args.Length())
      return NanThrowError("Expected JavaScript structure"
          " and/or options object");
    else if (2 <= args.Length())
    {
      if (!parse_object(args[1], &options))
        return NanThrowError("Options parameter must be JSON-string or Object");
    }

    std::string ret, error;

    Dynamic op = DynamicFromJson(options, &error);
    if (!op.IsDict())
      return NanThrowError("Options parameter must be JSON-string or Object");

    if (!V8ToXmlConverter::Process(options, args[0], &ret, &error))
      return NanThrowError(error.c_str());

    Dynamic * as_buffer;
    bool to_string = op.Get("asBuffer", &as_buffer) ? !*as_buffer : false;

    if (to_string
        && istrequal(op["encoding"].GetConstString(), std::string("utf-8")))
      NanReturnValue(NanNew(ret));
    else
      NanReturnValue(NanNewBufferHandle(ret.c_str(), ret.size()));
  }

  //------------------------------------------------------------------------------
  Persistent<Function> Xml2VarBuilderWrapper::constructor;

  //------------------------------------------------------------------------------
  void Xml2VarBuilderWrapper::Init(Handle<Object> exports)
  {
    // Prepare constructor template
    Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(
            Xml2VarBuilderWrapper::New);
    tpl->SetClassName(NanNew("Xml2VarBuilder"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(tpl, "feed", Xml2VarBuilderWrapper::Feed);
    NODE_SET_PROTOTYPE_METHOD(tpl, "end", Xml2VarBuilderWrapper::End);
    NODE_SET_PROTOTYPE_METHOD(tpl, "get", Xml2VarBuilderWrapper::Get);
    NanAssignPersistent(constructor, tpl->GetFunction());
    exports->Set(NanNew("Xml2VarBuilder"), NanNew(constructor));
    exports->Set(NanNew("var2xml"),
        NanNew<FunctionTemplate>(var2xml)->GetFunction());

  }

  //------------------------------------------------------------------------------
  NAN_METHOD(Xml2VarBuilderWrapper::New)
  {
    NanScope();

    if (!args.IsConstructCall())
    {
      // Invoked as plain function `Xml2VarBuilder(...)`
      return NanThrowError("Can't call constructor as a function");
    }

    std::string options("{}"), mappings;
    if (1 > args.Length())
      return NanThrowError("Expected one or two arguments:"
          " 1) mappings or 2) options and mappings");
    else if (1 == args.Length())
    {
      if (!parse_object(args[0], &mappings))
        return NanThrowError("Mappings parameter must be JSON-string or Object");
    }
    else
    {
      if (!parse_object(args[0], &options))
        return NanThrowError("Options parameter must be JSON-string or Object");
      if (!parse_object(args[1], &mappings))
        return NanThrowError("Mappings parameter must be JSON-string or Object");
    }

    std::string error;
    Xml2VarBuilder<V8VarBuilder>::Ptr builder =
        Xml2VarBuilder<V8VarBuilder>::Create(options, mappings, &error);

    if (!builder)
      return NanThrowError(error.c_str());

    Xml2VarBuilderWrapper* obj = new Xml2VarBuilderWrapper(builder);
    obj->Wrap(args.This());
    NanReturnValue(args.This());
  }

  //------------------------------------------------------------------------------
  NAN_METHOD(Xml2VarBuilderWrapper::Feed)
  {
    NanScope();

    if (1 > args.Length())
      return NanThrowError("Expected String or Buffer parameter");

    Xml2VarBuilderWrapper* obj = ObjectWrap::Unwrap<Xml2VarBuilderWrapper>(
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
  NAN_METHOD(Xml2VarBuilderWrapper::Get)
  {
    NanScope();

    if (1 > args.Length())
      return NanThrowError("Expected mapping name: String or Buffer");

    Xml2VarBuilderWrapper* obj = ObjectWrap::Unwrap<Xml2VarBuilderWrapper>(
        args.This());

    Local<Object> result;
    std::string error;
    if (node::Buffer::HasInstance(args[0]))
    {
      char* str;
      size_t length;
      get_buffer_data(args[0], &str, &length);
      std::string mapping_name(str, length);
      result = Local<Object>::Cast(
          NanNew<Value>(obj->builder_->var(mapping_name)));
    }
    else if (args[0]->IsString())
    {
      String::Utf8Value utf8_value(args[0]);
      std::string mapping_name(*utf8_value, utf8_value.length());
      result = Local<Object>::Cast(
          NanNew<Value>(obj->builder_->var(mapping_name)));
    }
    else
      return NanThrowTypeError("Expected mapping name: String or Buffer");

    NanReturnValue(result);
  }

  //------------------------------------------------------------------------------
  NAN_METHOD(Xml2VarBuilderWrapper::End)
  {
    NanScope();

    Xml2VarBuilderWrapper* obj = ObjectWrap::Unwrap<Xml2VarBuilderWrapper>(
        args.This());

    std::string empty = "";
    std::string error;
    if (!obj->builder_->Feed(empty.c_str(), empty.size(), true, &error))
      return NanThrowError(error.c_str());

    StringList mapping_names(obj->builder_->mapping_names());

    Local<Object> result = NanNew<Object>();
    StringList::const_iterator mapping_name = mapping_names.begin(),
        end = mapping_names.end();
    for (; mapping_name != end; ++mapping_name)
    {
      Local<Value> item = NanNew(obj->builder_->var(*mapping_name));
      result->Set(NanNew(mapping_name->c_str()), item);
    }

    NanReturnValue(result);
  }

}  // namespace nkit
