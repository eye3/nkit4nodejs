#ifndef NKIT_VX_DYNAMIC_BUILDER_H
#define NKIT_VX_DYNAMIC_BUILDER_H

#include "nkit/dynamic_json.h"
#include "nkit/xml2var.h"
#include "nkit/var2xml.h"

namespace nkit
{
  class DynamicBuilderPolicy
  {
  private:
    friend class VarBuilder<DynamicBuilderPolicy>;

  public:
    typedef Dynamic type;

    static const type & GetUndefined()
    {
      return D_NONE;
    }

  private:
    DynamicBuilderPolicy(const detail::Options & options)
      : object_()
    {
      NKIT_FORCE_USED(options);
    }

    ~DynamicBuilderPolicy() {}

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

    void AppendToDictKeyList( std::string const & key, type const & var )
    {
      Dynamic * list_value;
      if (object_.Get(key, &list_value) && list_value->IsList())
        list_value->PushBack(var);
      else
        SetDictKeyValue(key, DLIST(var));
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
  };

  typedef VarBuilder<DynamicBuilderPolicy> DynamicBuilder;

  //----------------------------------------------------------------------------
  struct DynamicReaderPolicy
  {
    typedef Dynamic type;
    typedef Dynamic::DictConstIterator DictConstIterator;
    typedef Dynamic::ListConstIterator ListConstIterator;

    static DictConstIterator begin_d(const Dynamic & data)
    {
      return data.begin_d();
    }

    static DictConstIterator end_d(const Dynamic & data)
    {
      return data.end_d();
    }

    static ListConstIterator begin_l(const Dynamic & data)
    {
      return data.begin_l();
    }

    static ListConstIterator end_l(const Dynamic & data)
    {
      return data.end_l();
    }

    static std::string First(const DictConstIterator & it)
    {
      return it->first;
    }

    static Dynamic Second(const DictConstIterator & it)
    {
      return it->second;
    }

    static const Dynamic & Value(const ListConstIterator & it)
    {
      return *it;
    }

    static bool IsList(const Dynamic & data)
    {
      return data.IsList();
    }

    static bool IsDict(const Dynamic & data)
    {
      return data.IsDict();
    }

    static bool IsString(const Dynamic & data)
    {
      return data.IsString();
    }

    static bool IsFloat(const Dynamic & data)
    {
      return data.IsFloat();
    }

    static bool IsDateTime(const Dynamic & data)
    {
      return data.IsDateTime();
    }

    static bool IsBool(const Dynamic & data)
    {
      return data.IsBool();
    }

    static std::string GetString(const Dynamic & data)
    {
      return data.GetString();
    }

    static std::string GetStringAsDateTime(const Dynamic & data,
            const std::string & format)
    {
      return data.GetString(format.c_str());
    }

    static std::string GetStringAsFloat(const Dynamic & data,
            size_t precision)
    {
      return string_cast(data.GetFloat(), precision);
    }

    static const std::string & GetStringAsBool(const Dynamic & data,
        const std::string & bool_true_format,
        const std::string & bool_false_format)
    {
      return data ? bool_true_format: bool_false_format;
    }

    static Dynamic GetByKey(const Dynamic & data, const std::string & key,
        bool * found)
    {
      const Dynamic * ret = NULL;
      *found = data.Get(key, &ret);
      if (*found)
        return *ret;
      else
        return Dynamic();
    }
  };

  typedef Var2XmlConverter<DynamicReaderPolicy> Dynamic2XmlConverter;

} // namespace nkit

#endif // NKIT_VX_DYNAMIC_BUILDER_H
