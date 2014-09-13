#ifndef NKIT_VX_DYNAMIC_BUILDER_H
#define NKIT_VX_DYNAMIC_BUILDER_H

#include "nkit/dynamic_json.h"
#include "nkit/vx.h"

namespace nkit
{
  class DynamicPolicy
  {
  private:
    friend class VarBuilder<DynamicPolicy>;

    typedef Dynamic type;

    DynamicPolicy(): object_() {}
    ~DynamicPolicy() {}

    void _InitAsBoolean( std::string const & value )
    {
      object_ = nkit::Dynamic(bool_cast(value));
    }

    void _InitAsInteger( std::string const & value )
    {
      int64_t i = !value.empty() ? NKIT_STRTOLL(value.c_str(), NULL, 10) : 0;
      object_ = nkit::Dynamic(i);
    }

    void _InitAsString( std::string const & value)
    {
      object_ = nkit::Dynamic(value);
    }

    void _InitAsUndefined()
    {
      object_ = nkit::Dynamic();
    }

    void _InitAsFloatFormat( std::string const & value, const char * format )
    {
      double d(0.0);
      if (!value.empty())
      {
        if (NKIT_SSCANF(value.c_str(), format, &d) == 0)
          d = 0.0;
      }
      object_ = nkit::Dynamic(d);
    }

    void _InitAsDatetimeFormat( std::string const & value,
        const char * format )
    {
      object_ = nkit::Dynamic::DateTimeFromString(value, format);
    }

    void _InitAsList()
    {
      object_ = nkit::Dynamic::List();
    }

    void _InitAsDict()
    {
      object_ = nkit::Dynamic::Dict();
    }

    void _ListCheck()
    {
      assert(object_.IsList());
    }

    void _DictCheck()
    {
      assert(object_.IsDict());
    }

    void _AppendToList( type const & obj )
    {
      object_.PushBack(obj);
    }

    void _SetDictKeyValue( std::string const & key, type const & var )
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
  };

  typedef VarBuilder<DynamicPolicy> DynamicBuilder;

} // namespace nkit

#endif // NKIT_VX_DYNAMIC_BUILDER_H
