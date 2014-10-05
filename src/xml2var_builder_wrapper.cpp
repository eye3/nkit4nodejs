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

using namespace v8;

Persistent<Function> Xml2VarBuilderWrapper::constructor;

void Xml2VarBuilderWrapper::Init(Handle<Object> exports)
{
  // Prepare constructor template
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(
          Xml2VarBuilderWrapper::New);
  tpl->SetClassName(NanNew("Xml2VarBuilder"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  NODE_SET_PROTOTYPE_METHOD(tpl, "feed", Xml2VarBuilderWrapper::Feed);
  NODE_SET_PROTOTYPE_METHOD(tpl, "end", Xml2VarBuilderWrapper::End);
  NanAssignPersistent(constructor, tpl->GetFunction());
  exports->Set(NanNew("Xml2VarBuilder"), NanNew(constructor));
}

NAN_METHOD(Xml2VarBuilderWrapper::New)
{
  NanScope();

  if (!args.IsConstructCall())
  {
    // Invoked as plain function `Xml2VarBuilder(...)`
    return NanThrowError("Can't call constructor as a function");
  }

  if (1 > args.Length())
    return NanThrowError("Expected arguments");

  std::string mappings;
  std::string error;
  if (node::Buffer::HasInstance(args[0]))
  {
    char* data = node::Buffer::Data(args[0]);
    size_t length = node::Buffer::Length(args[0]);
    mappings.assign(data, length);
  }
  else if (args[0]->IsString())
    mappings.assign(*String::Utf8Value(args[0]));
  else if (args[0]->IsObject())
    mappings.assign(vx::v8var_to_json(args[0]));
  else
    return NanThrowError("Wrong type of arguments");

  nkit::Xml2VarBuilder<vx::V8VarBuilder>::Ptr builder =
          nkit::Xml2VarBuilder<vx::V8VarBuilder>::Create(
      mappings, &error);

  if (!builder)
    return NanThrowError(error.c_str());

  Xml2VarBuilderWrapper* obj = new Xml2VarBuilderWrapper(builder);

  obj->Wrap(args.This());

  NanReturnValue(args.This());
}

NAN_METHOD(Xml2VarBuilderWrapper::Feed)
{
  NanScope();

  if (1 > args.Length())
    return NanThrowError("Expected arguments");

  Xml2VarBuilderWrapper* obj = ObjectWrap::Unwrap<Xml2VarBuilderWrapper>(
      args.This());

  bool result = false;
  std::string error = "";
  if (node::Buffer::HasInstance(args[0]))
  {
    char* data = node::Buffer::Data(args[0]);
    size_t length = node::Buffer::Length(args[0]);

    result = obj->builder_->Feed(data, length, false, &error);
  }
  else if (args[0]->IsString())
  {
    String::Utf8Value utf8_value(args[0]);
    result = obj->builder_->Feed(*utf8_value, utf8_value.length(), false, &error);
  }
  else
    return NanThrowTypeError("Wrong type of arguments");

  if (!result)
    return NanThrowError(error.c_str());

  NanReturnUndefined();
}

NAN_METHOD(Xml2VarBuilderWrapper::End)
{
  NanScope();

  Xml2VarBuilderWrapper* obj = ObjectWrap::Unwrap<Xml2VarBuilderWrapper>(
      args.This());

  std::string empty = "";
  std::string error;
  if (!obj->builder_->Feed(empty.c_str(), empty.size(), true, &error))
    return NanThrowError(error.c_str());

  NanReturnValue(obj->builder_->var());
}
