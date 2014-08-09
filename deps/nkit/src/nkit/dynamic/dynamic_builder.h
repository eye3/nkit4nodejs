#ifndef NKIT_VX_DYNAMIC_BUILDER_H
#define NKIT_VX_DYNAMIC_BUILDER_H

#include "nkit/dynamic_json.h"

namespace nkit
{
  //---------------------------------------------------------------------------
  class DynamicBuilder
  {
  public:
    typedef nkit::Dynamic type;

  public:
    void InitAsDict()
    {
      object_ = nkit::Dynamic::Dict();
    }

    void InitAsList()
    {
      object_ = nkit::Dynamic::List();
    }

    void InitAsBoolean(std::string const & value)
    {
      object_ = nkit::Dynamic(bool_cast(value));
    }

    void InitAsBooleanFormat(std::string const & value, const std::string & )
    {
      InitAsBoolean(value);
    }

    void InitAsBooleanDefault()
    {
      InitAsBoolean(nkit::S_FALSE_);
    }

    void InitAsString(std::string const & value)
    {
      object_ = nkit::Dynamic(value);
    }

    void InitAsStringFormat(std::string const & value, const std::string & )
    {
      InitAsString(value);
    }

    void InitAsStringDefault()
    {
      InitAsString(nkit::S_EMPTY_);
    }

    void InitAsInteger(const std::string & value)
    {
      int64_t i = !value.empty() ? NKIT_STRTOLL(value.c_str(), NULL, 10) : 0;
      object_ = nkit::Dynamic(i);
    }

    void InitAsIntegerFormat(const std::string & value, const std::string & )
    {
      InitAsInteger(value);
    }

    void InitAsIntegerDefault()
    {
      InitAsInteger(nkit::S_ZERO_);
    }

    void InitAsFloat(const std::string & value)
    {
      _InitAsFloatFormat(value, NKIT_FORMAT_DOUBLE);
    }

    void InitAsFloatFormat(const std::string & value,
        const std::string & format)
    {
      _InitAsFloatFormat(value, format.c_str());
    }

    void InitAsFloatDefault()
    {
      _InitAsFloatFormat(nkit::S_ZERO_, NKIT_FORMAT_DOUBLE);
    }

    void _InitAsFloatFormat(const std::string & value, const char * format)
    {
      double d(0.0);
      if (!value.empty())
      {
        if (NKIT_SSCANF(value.c_str(), format, &d) == 0)
          d = 0.0;
      }
      object_ = nkit::Dynamic(d);
    }
//#ifndef NKIT_WINNT
    void InitAsDatetime(const std::string & value)
    {
      object_ = nkit::Dynamic::DateTimeFromString(value,
          nkit::DATE_TIME_DEFAULT_FORMAT_);
    }

    void InitAsDatetimeFormat(const std::string & value,
        const std::string & format)
    {
      object_ = nkit::Dynamic::DateTimeFromString(value, format.c_str());
    }

    void InitAsDatetimeDefault()
    {
      object_ = nkit::Dynamic::DateTimeFromTimestamp(0);
    }
//#endif

    void InitAsUndefined()
    {
      object_ = nkit::Dynamic();
    }

    void SetDictKeyValue(const std::string & key,
        const type & obj)
    {
      assert(object_.IsDict());
      object_[key] = obj;
    }

    void AppendToList(const type & obj)
    {
      assert(object_.IsList());
      object_.PushBack(obj);
    }

    const type & get() const
    {
      return object_;
    }

  private:
    type object_;
  };

} // namespace nkit

#endif // NKIT_VX_DYNAMIC_BUILDER_H
