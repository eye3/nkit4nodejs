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
#include "v8_var_builder.h"
#include "nkit/constants.h"

namespace vx
{
	using namespace v8;

  v8::Persistent<v8::Function> V8VarBuilder::date_constructor_;

  void V8VarBuilder::Init()
  {
    NanScope();
    Handle<Object> global = NanGetCurrentContext()->Global();
    NanAssignPersistent(date_constructor_,
        Handle<Function>::Cast(global->Get(NanNew("Date"))));
  }

	V8VarBuilder::V8VarBuilder()
	{
    NanAssignPersistent(object_, Local<Value>(NanNew<Object>()));
	}

	V8VarBuilder::~V8VarBuilder()
	{
		NanScope();
		NanDisposePersistent(object_);
	}

	void V8VarBuilder::InitAsDict()
	{
		NanScope();
		NanAssignPersistent(object_, Local<Value>(NanNew<Object>()));
	}

	void V8VarBuilder::InitAsList()
	{
		NanScope();
		NanAssignPersistent(object_, Local<Value>(NanNew<Array>()));
	}

	void V8VarBuilder::InitAsBoolean(std::string const & value)
	{
		NanScope();
		NanAssignPersistent(object_,
		        Local<Value>(NanNew(nkit::bool_cast(value))));
	}

	void V8VarBuilder::InitAsBooleanFormat(std::string const & value,
	    const std::string &)
	{
		InitAsBoolean(value);
	}

	void V8VarBuilder::InitAsBooleanDefault()
	{
		InitAsBoolean(nkit::S_FALSE_);
	}

	void V8VarBuilder::InitAsString(std::string const & value)
	{
		NanScope();
		NanAssignPersistent(object_, Local<Value>(NanNew(value.c_str())));
	}

	void V8VarBuilder::InitAsStringFormat(std::string const & value,
	    const std::string &)
	{
		InitAsString(value);
	}

	void V8VarBuilder::InitAsStringDefault()
	{
		InitAsString(nkit::S_EMPTY_);
	}

	void V8VarBuilder::InitAsInteger(const std::string & value)
	{
		int32_t i = !value.empty() ? strtol(value.c_str(), NULL, 10) : 0;

		NanScope();
		NanAssignPersistent(object_, Local<Value>(NanNew(i)));
	}

	void V8VarBuilder::InitAsIntegerFormat(std::string const & value,
	    const std::string &)
	{
		InitAsInteger(value);
	}

	void V8VarBuilder::InitAsIntegerDefault()
	{
		InitAsInteger(nkit::S_ZERO_);
	}

	void V8VarBuilder::InitAsFloat(const std::string & value)
	{
		_InitAsFloatFormat(value, NKIT_FORMAT_DOUBLE);
	}

	void V8VarBuilder::InitAsFloatFormat(std::string const & value,
	    const std::string & format)
	{
		_InitAsFloatFormat(value, format.c_str());
	}

	void V8VarBuilder::InitAsFloatDefault()
	{
		_InitAsFloatFormat(nkit::S_ZERO_, NKIT_FORMAT_DOUBLE);
	}

	void V8VarBuilder::_InitAsFloatFormat(std::string const & value,
	    const char * format)
	{
		double d(0.0);
		if (!value.empty())
		{
			if (NKIT_SSCANF(value.c_str(), format, &d) == 0)
				d = 0.0;
		}

		NanScope();
		NanAssignPersistent(object_, Local<Value>(NanNew(d)));
	}

	void V8VarBuilder::InitAsDatetime(const std::string & value)
	{
		_InitAsDatetimeFormat(value, nkit::DATE_TIME_DEFAULT_FORMAT_);
	}

	void V8VarBuilder::InitAsDatetimeFormat(const std::string & value,
	    const std::string & format)
	{
		_InitAsDatetimeFormat(value, format.c_str());
	}

	void V8VarBuilder::_InitAsDatetimeFormat(const std::string & value,
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

		time_t tz_offset_in_seconds = nkit::timezone_offset() / 60;
		char tz_sign = '-';
		if (tz_offset_in_seconds < 0)
		{
			tz_offset_in_seconds *= -1;
			tz_sign = '+';
		}

		char tz_offset_hours[3];
		sprintf(tz_offset_hours, "%02u",
		    static_cast<uint32_t>(tz_offset_in_seconds / 60));
		tz_offset_hours[2] = 0;

		char tz_offset_minutes[3];
		sprintf(tz_offset_minutes, "%02u",
		    static_cast<uint32_t>(tz_offset_in_seconds % 60));
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
		strncpy(date_time_buf+8, MONTHS[_tm.tm_mon], 3);
		strncpy(date_time_buf+26, &tz_sign, 1);
		strncpy(date_time_buf+27, tz_offset_hours, 2);
		strncpy(date_time_buf+29, tz_offset_minutes, 2);

		NanScope();
		Local<Value> argv[1] =
			{ NanNew<String>(std::string(date_time_buf, DATE_TIME_BUFFER_LENGTH)) };
		NanAssignPersistent(object_,
		        Local<Value>(NanNew(date_constructor_)->NewInstance(1, argv)));
	}

	void V8VarBuilder::InitAsDatetimeDefault()
	{
		NanScope();
		NanAssignPersistent(object_, Local<Value>(NanNew<Date>(0)));
	}

	void V8VarBuilder::InitAsUndefined()
	{
		NanScope();
		NanAssignPersistent(object_, Local<Value>(NanUndefined()));
	}

	void V8VarBuilder::SetDictKeyValue(std::string const & key, type const & var)
	{
		NanScope();
    Local<Value> object(NanNew(object_));
		assert(object->IsObject());
		Local<Object> obj = Local<Object>::Cast(object);
		obj->Set(NanNew(key.c_str()), NanNew(var));
	}

	void V8VarBuilder::AppendToList(type const & var)
	{
		NanScope();
		Local<Value> object(NanNew(object_));
		assert(object->IsArray());
		Local<Array> arr = Local<Array>::Cast(object);
		arr->Set(arr->Length(), NanNew(var));
	}

	std::string V8VarBuilder::ToString() const
	{
	    return v8var_to_json(object_);
	}

	std::string v8var_to_json(const Handle<Value> & var)
	{
		NanScope();
		Handle<Object> global = NanGetCurrentContext()->Global();
		Handle<Object> JSON = global->Get(NanNew<String>("JSON"))->ToObject();
		Handle<Function> JSON_stringify = Handle<Function>::Cast(
		    JSON->Get(NanNew<String>("stringify")));
		Handle<Value> argv[3] =
		  { var, NanNull(), NanNew<String>("  ") };
		String::Utf8Value ascii(JSON_stringify->Call(JSON, 3, argv));
		return *ascii;
	}

} // namespace vx
