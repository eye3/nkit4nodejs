#ifndef NKIT_VX_DYNAMIC_BUILDER_H
#define NKIT_VX_DYNAMIC_BUILDER_H

#include "nkit/dynamic_json.h"
#include "nkit/xml2var.h"

namespace nkit
{
  class DynamicPolicy
  {
  private:
    friend class VarBuilder<DynamicPolicy>;

    typedef Dynamic type;

    DynamicPolicy(const detail::Options & options)
      : object_()
      //, options_(options)
    {
      NKIT_FORCE_USED(options);
    }

    ~DynamicPolicy() {}

    void InitAsBoolean( std::string const & value )
    {
      object_ = nkit::Dynamic(bool_cast(value));
    }

    void InitAsInteger( std::string const & value )
    {
      int64_t i = !value.empty() ? NKIT_STRTOLL(value.c_str(), NULL, 10) : 0;
      object_ = nkit::Dynamic(i);
    }

    void InitAsString( std::string const & value)
    {
      object_ = nkit::Dynamic(value);
    }

    void InitAsUndefined()
    {
      object_ = nkit::Dynamic();
    }

    void InitAsFloatFormat( std::string const & value, const char * format )
    {
      double d(0.0);
      if (!value.empty())
      {
        if (NKIT_SSCANF(value.c_str(), format, &d) == 0)
          d = 0.0;
      }
      object_ = nkit::Dynamic(d);
    }

    void InitAsDatetimeFormat( std::string const & value,
        const char * format )
    {
      object_ = nkit::Dynamic::DateTimeFromString(value, format);
    }

    void InitAsList()
    {
      object_ = nkit::Dynamic::List();
    }

    void InitAsDict()
    {
      object_ = nkit::Dynamic::Dict();
    }

    void ListCheck()
    {
      assert(object_.IsList());
    }

    void DictCheck()
    {
      assert(object_.IsDict());
    }

    void AppendToList( type const & obj )
    {
      object_.PushBack(obj);
    }

    void SetDictKeyValue( std::string const & key, type const & var )
    {
      object_[std::string(key)] = var;
    }

    type const & get() const
    {
      return object_;
    }

    std::string ToString() const
    {
      return DynamicToJson(object_);
    }

  private:
    type object_;
    //const detail::Options & options_;
  };

  typedef VarBuilder<DynamicPolicy> DynamicBuilder;

} // namespace nkit

#endif // NKIT_VX_DYNAMIC_BUILDER_H
