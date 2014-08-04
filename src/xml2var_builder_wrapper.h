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

#ifndef XML2VAR_BUILDER_H
#define XML2VAR_BUILDER_H

#include <node/node.h>
#include <boost/shared_ptr.hpp>

#include "nkit/vx.h"
#include "v8_var_builder.h"

class Xml2VarBuilderWrapper: public node::ObjectWrap
{
public:
  static void Init(v8::Handle<v8::Object> exports);

private:
  explicit Xml2VarBuilderWrapper(const v8::Arguments& args);
  ~Xml2VarBuilderWrapper()
  {}

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> Feed(const v8::Arguments& args);
  static v8::Handle<v8::Value> End(const v8::Arguments& args);

  static v8::Persistent<v8::Function> constructor;

  nkit::Xml2VarBuilder<vx::V8VarBuilder>::Ptr gen_;
};

#endif // XML2VAR_BUILDER_H
