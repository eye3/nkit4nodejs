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

#include <node.h>
#include "v8_var_builder.h"
#include "nkit/constants.h"
//#include "nkit/logger_brief.h"

namespace vx
{
	using namespace v8;

	V8VarBuilder::V8VarBuilder()
	{
#if !defined(_WIN32) && !defined(_WIN64)
		HandleScope handle_scope;
		Handle<Object> global = Context::GetCurrent()->Global();
		date_constructor_ = Persistent<Function>::New(
		    Handle<Function>::Cast(global->Get(String::New("Date"))));
#endif
	}

	V8VarBuilder::~V8VarBuilder()
	{
		HandleScope handle_scope;
		object_.Dispose();
#if !defined(_WIN32) && !defined(_WIN64)
		date_constructor_.Dispose();
#endif
	}

	void V8VarBuilder::InitAsDict()
	{
		HandleScope handle_scope;
		object_.Dispose();
		object_ = Persistent<Object>::New(Object::New());
	}

	void V8VarBuilder::InitAsList()
	{
		HandleScope handle_scope;
		object_.Dispose();
		object_ = Persistent<Array>::New(Array::New());
	}

	void V8VarBuilder::InitAsBoolean(std::string const & value)
	{
		HandleScope handle_scope;
		object_.Dispose();
		object_ = Persistent<Boolean>::New(Boolean::New(nkit::bool_cast(value)));
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
		HandleScope handle_scope;
		object_.Dispose();
		object_ = Persistent<String>::New(String::New(value.c_str()));
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
#ifdef _WIN32
		int32_t i = !value.empty() ? strtol(value.c_str(), NULL, 10) : 0;
#else
		int64_t i = !value.empty() ? NKIT_STRTOLL(value.c_str(), NULL, 10) : 0;
#endif

		HandleScope handle_scope;
		object_.Dispose();
		object_ = Persistent<Number>::New(Number::New(i));
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

		HandleScope handle_scope;
		object_.Dispose();
		object_ = Persistent<Number>::New(Number::New(d));
	}

#if !defined(_WIN32) && !defined(_WIN64)
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
		struct tm _tm =
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		if (strptime(value.c_str(), format, &_tm) == NULL)
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
		sprintf(tz_offset_hours, "%02u", static_cast<uint32_t>(tz_offset_in_seconds / 60));
		tz_offset_hours[2] = 0;

		char tz_offset_minutes[3];
		sprintf(tz_offset_minutes, "%02u", static_cast<uint32_t>(tz_offset_in_seconds % 60));
		tz_offset_minutes[2] = 0;
		//CINFO(tz_offset_hours << " " << tz_offset_minutes);

		static const char * WEEK_DAYS[7] =
			{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
		static const char * MONTHS[12] =
			{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
		    "Nov", "Dec" };
		static const size_t DATE_TIME_BUFFER_LENGTH = 31;
		char date_time_buf[DATE_TIME_BUFFER_LENGTH];
		strftime(date_time_buf, DATE_TIME_BUFFER_LENGTH, "WWW, %d MMM %Y %H:%M:%S ZHHMM", &_tm);
		strncpy(date_time_buf, WEEK_DAYS[_tm.tm_wday], 3);
		strncpy(date_time_buf+8, MONTHS[_tm.tm_mon], 3);
		strncpy(date_time_buf+26, &tz_sign, 1);
		strncpy(date_time_buf+27, tz_offset_hours, 2);
		strncpy(date_time_buf+29, tz_offset_minutes, 2);
		//CINFO(std::string(date_time_buf, DATE_TIME_BUFFER_LENGTH));
		HandleScope handle_scope;
		Handle<Value> argv[] =
			{ String::New(date_time_buf, DATE_TIME_BUFFER_LENGTH) };
		object_.Dispose();
		object_ = Persistent<Value>::New(date_constructor_->NewInstance(1, argv));
	}

	void V8VarBuilder::InitAsDatetimeDefault()
	{
		HandleScope handle_scope;
		object_.Dispose();
		object_ = Persistent<Value>::New(Date::New(0));
	}
#endif

	void V8VarBuilder::InitAsUndefined()
	{
		HandleScope handle_scope;
		object_.Dispose();
		object_ = Persistent<Value>::New(Undefined());
	}

	void V8VarBuilder::SetDictKeyValue(std::string const & key, type const & var)
	{
		HandleScope handle_scope;
		assert(object_->IsObject());
		Handle<Object> obj = Handle<Object>::Cast(object_);
		obj->Set(String::New(key.c_str()), var);
	}

	void V8VarBuilder::AppendToList(type const & obj)
	{
		HandleScope handle_scope;
		assert(object_->IsArray());
		Handle<Array> arr = Handle<Array>::Cast(object_);
		arr->Set(arr->Length(), obj);
	}

	std::string v8var_to_json(const Handle<Value> & var)
	{
		HandleScope handle_scope;
		Handle<Object> global = Context::GetCurrent()->Global();
		Handle<Object> JSON = global->Get(String::New("JSON"))->ToObject();
		Handle<Function> JSON_stringify = Handle<Function>::Cast(
		    JSON->Get(String::New("stringify")));
		Handle<Value> argv[] =
		{ var, Null(), String::New("  ") };
		String::Utf8Value ascii(JSON_stringify->Call(JSON, 3, argv));
		return *ascii;
	}

} // namespace vx
