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

#include <node/node_buffer.h>

#include "xml2var_builder_wrapper.h"

using namespace v8;

Persistent<Function> Xml2VarBuilderWrapper::constructor;

void Xml2VarBuilderWrapper::Init(Handle<Object> exports)
{
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);

  tpl->SetClassName(String::NewSymbol("Xml2VarBuilder"));

  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("feed"),
      FunctionTemplate::New(Feed)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("end"),
      FunctionTemplate::New(End)->GetFunction());

  constructor = Persistent<Function>::New(tpl->GetFunction());

  exports->Set(String::NewSymbol("Xml2VarBuilder"), constructor);
}

Xml2VarBuilderWrapper::Xml2VarBuilderWrapper(const Arguments& args)
{
  if (1 > args.Length())
  {
    ThrowException(String::New("Expected arguments"));
    return;
  }

  std::string error;
  if (node::Buffer::HasInstance(args[0]))
  {
    char* data = node::Buffer::Data(args[0]);
    size_t length = node::Buffer::Length(args[0]);

    std::string target_spec(data, length);
    gen_ = nkit::Xml2VarBuilder<vx::V8VarBuilder>::Create(target_spec, &error);
  }
  else if (args[0]->IsString())
  {
    std::string target_spec(*String::Utf8Value(args[0]));
    gen_ = nkit::Xml2VarBuilder<vx::V8VarBuilder>::Create(target_spec, &error);
  }
  else
  {
    ThrowException(String::New("Wrong type of arguments"));
    return;
  }

  if (!gen_)
  {
    ThrowException(String::New(error.c_str()));
    return;
  }
}

Handle<Value> Xml2VarBuilderWrapper::New(const Arguments& args)
{
  HandleScope scope;

  if (!args.IsConstructCall())
  {
    // Invoked as plain function `Xml2VarBuilder(...)`
    ThrowException(String::New("Cannot call constructor as function"));
    return scope.Close(Undefined());
  }

  // Invoked as constructor: `new Xml2VarBuilder(...)
  Xml2VarBuilderWrapper* obj = new Xml2VarBuilderWrapper(args);

  obj->Wrap(args.This());

  return args.This();
}

Handle<Value> Xml2VarBuilderWrapper::Feed(const Arguments& args)
{
  HandleScope scope;

  if (1 > args.Length())
  {
    ThrowException(String::New("Expected arguments"));
    return scope.Close(Undefined());
  }

  Xml2VarBuilderWrapper* obj = ObjectWrap::Unwrap<Xml2VarBuilderWrapper>(
      args.This());

  bool result = false;
  std::string error = "";
  if (node::Buffer::HasInstance(args[0]))
  {
    char* data = node::Buffer::Data(args[0]);
    size_t length = node::Buffer::Length(args[0]);

    result = obj->gen_->Feed(data, length, false, &error);
  }
  else if (args[0]->IsString())
  {
    String::Utf8Value utf8_value(args[0]);
    result = obj->gen_->Feed(*utf8_value, utf8_value.length(), false, &error);
  }
  else
  {
    ThrowException(String::New("Wrong type of arguments"));
    return scope.Close(Undefined());
  }

  if (!result)
  {
    ThrowException(String::New(error.c_str()));
    return scope.Close(Undefined());
  }

  return scope.Close(Undefined());
}

Handle<Value> Xml2VarBuilderWrapper::End(const v8::Arguments& args)
{
  HandleScope scope;

  Xml2VarBuilderWrapper* obj = ObjectWrap::Unwrap<Xml2VarBuilderWrapper>(
      args.This());

  std::string empty = "";
  std::string error;
  if (!obj->gen_->Feed(empty.c_str(), empty.size(), true, &error))
  {
    ThrowException(String::New(error.c_str()));
    return scope.Close(Undefined());
  }

  return scope.Close(obj->gen_->var());
}
