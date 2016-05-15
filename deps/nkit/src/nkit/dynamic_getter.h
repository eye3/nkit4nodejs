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

#ifndef __NKIT__DYNAMIC__JSON__CONFIG__H__
#define __NKIT__DYNAMIC__JSON__CONFIG__H__

#include <nkit/dynamic_json.h>
#include <nkit/dynamic_path.h>

namespace nkit
{
  inline Dynamic & operator << (Dynamic & d, const bool b)
  {
    d = Dynamic(b);
    return d;
  }

  inline Dynamic & operator << (Dynamic & d, const uint64_t v)
  {
    d = Dynamic(v);
    return d;
  }

  inline Dynamic & operator << (Dynamic & d, const int64_t v)
  {
    d = Dynamic(v);
    return d;
  }

  inline Dynamic & operator << (Dynamic & d, const uint32_t v)
  {
    d = Dynamic(v);
    return d;
  }

  inline Dynamic & operator << (Dynamic & d, const int32_t v)
  {
    d = Dynamic(v);
    return d;
  }

  inline Dynamic & operator << (Dynamic & d, const uint16_t v)
  {
    d = Dynamic(v);
    return d;
  }

  inline Dynamic & operator << (Dynamic & d, const int16_t v)
  {
    d = Dynamic(v);
    return d;
  }

  inline Dynamic & operator << (Dynamic & d, const uint8_t v)
  {
    d = Dynamic(v);
    return d;
  }

  inline Dynamic & operator << (Dynamic & d, const int8_t v)
  {
    d = Dynamic(v);
    return d;
  }

  inline Dynamic & operator << (Dynamic & d, const std::string & v)
  {
    d = Dynamic(v);
    return d;
  }

  inline Dynamic & operator << (Dynamic & d, const char * v)
  {
    d = Dynamic(v);
    return d;
  }

  template <typename K, typename V>
  inline typename std::map<K, V> & operator << (Dynamic & d,
      const typename std::map<K, V> & map)
  {
    d = Dynamic::Dict();
    typename std::map<K, V>::const_iterator it = map.begin(), end = map.end();
    for (; it != end; ++it)
    {
      Dynamic v;
      v << it->second;
      d[string_cast(it->first)] = v;
    }
    return d; // TODO: test all operator << (Dynamic & d, ...)
  }

  //----------------------------------------------------------------------------
  inline bool & operator << (bool & b, const Dynamic & d)
  {
    if (unlikely(d.IsString()))
      b = bool_cast(d.GetConstString());
    else
      b = d.GetBoolean();
    return b;
  }

  inline uint64_t & operator << (uint64_t & uv, const Dynamic & d)
  {
    uv = d.GetUnsignedInteger();
    return uv;
  }

  inline int64_t & operator << (int64_t & sv, const Dynamic & d)
  {
    sv = d.GetSignedInteger();
    return sv;
  }

  inline uint8_t & operator << (uint8_t & uv, const Dynamic & d)
  {
    uv = static_cast<uint8_t>(d.GetUnsignedInteger());
    return uv;
  }

  inline int8_t & operator << (int8_t & sv, const Dynamic & d)
  {
    sv = static_cast<int8_t>(d.GetSignedInteger());
    return sv;
  }

  inline uint16_t & operator << (uint16_t & uv, const Dynamic & d)
  {
    uv = static_cast<uint16_t>(d.GetUnsignedInteger());
    return uv;
  }

  inline int16_t & operator << (int16_t & sv, const Dynamic & d)
  {
    sv = static_cast<int16_t>(d.GetSignedInteger());
    return sv;
  }

  inline uint32_t & operator << (uint32_t & uv, const Dynamic & d)
  {
    uv = static_cast<uint32_t>(d.GetUnsignedInteger());
    return uv;
  }

  inline int32_t & operator << (int32_t & sv, const Dynamic & d)
  {
    sv = static_cast<int32_t>(d.GetSignedInteger());
    return sv;
  }

  inline double & operator << (double & v, const Dynamic & d)
  {
    v = d.GetFloat();
    return v;
  }

  inline std::string & operator << (std::string & v, const Dynamic & d)
  {
    if (d.IsString())
      v = d.GetConstString();
    else
      v = d.GetString();
    return v;
  }

  template <typename T>
  inline typename std::vector<T> & operator << (typename std::vector<T> & c,
      const Dynamic & d)
  {
    c.clear();
    if (likely(d.IsList()))
    {
      DLIST_FOREACH(item, d)
      {
        T i; i << *item;
        c.push_back(i);
      }
    }
    return c;
  }

  template <typename T>
  inline typename std::list<T> & operator << (typename std::list<T> & c,
      const Dynamic & d)
  {
    c.clear();
    if (likely(d.IsList()))
    {
      DLIST_FOREACH(item, d)
      {
        T i; i << *item;
        c.push_back(i);
      }
    }
    return c;
  }

  template <typename T>
  inline typename std::set<T> & operator << (typename std::set<T> & c,
      const Dynamic & d)
  {
    c.clear();
    if (likely(d.IsList()))
    {
      DLIST_FOREACH(item, d)
      {
        T i; i << *item;
        c.insert(i);
      }
    }
    return c;
  }

  template <typename K, typename V>
  inline typename std::map<K, V> & operator << (typename std::map<K, V> & map,
      const Dynamic & d)
  {
    map.clear();
    if (likely(d.IsDict()))
    {
      DDICT_FOREACH(pair, d)
      {
        K k; k << pair->first;
        V v; v << pair->second;
        std::swap(map[k], v);
      }
    }
    return map;
  }

  // Checker classes
  class NonEmptyString
  {
  public:
    bool Check(const std::string & s)
    {
      if (s.empty())
      {
        error_ = "is empty";
        return false;
      }

      return true;
    }

    bool Check(const StringSet & container)
    {
      return CheckSequence(container);
    }

    bool Check(const StringList & container)
    {
      return CheckSequence(container);
    }

    bool Check(const StringVector & container)
    {
      return CheckSequence(container);
    }

    const std::string & error() const
    {
      return error_;
    }

  private:
    template <typename T>
    bool CheckSequence(const T & container)
    {
      typename T::const_iterator s = container.begin(), end = container.end();
      for (size_t counter = 1; s != end; ++s, ++counter)
      {
        if (s->empty())
        {
          error_ = "LIST has empty string at position " + string_cast(counter);
          return false;
        }
      }

      return true;
    }

  private:
    std::string error_;
  };

  inline bool GetSubDynamic(const Dynamic & data,
      const DynamicPath & sub_path, std::string * out,
      std::string * NKIT_UNUSED(error))
  {
    out->clear();
    const Dynamic * v = sub_path.Get(data);
    if (!v || (!v->IsString() && !(*v)))
      return false;
    *out = v->GetString();
    return true;
  }

  inline bool GetSubDynamic(const Dynamic & data,
      const DynamicPath & sub_path, double * out,
      std::string * NKIT_UNUSED(error))
  {
    const Dynamic * v = sub_path.Get(data);
    if (!v)
      return false;
    *out = v->GetFloat();
    return true;
  }

  inline bool GetSubDynamic(const Dynamic & data,
      const DynamicPath & sub_path, int16_t * out,
      std::string * NKIT_UNUSED(error))
  {
    const Dynamic * v = sub_path.Get(data);
    if (!v)
      return false;
    *out = (int16_t)v->GetSignedInteger();
    return true;
  }

  inline bool GetSubDynamic(const Dynamic & data,
      const DynamicPath & sub_path, uint16_t * out,
      std::string * NKIT_UNUSED(error))
  {
    const Dynamic * v = sub_path.Get(data);
    if (!v)
      return false;
    *out = (uint16_t)v->GetUnsignedInteger();
    return true;
  }

  inline bool GetSubDynamic(const Dynamic & data,
      const DynamicPath & sub_path, int32_t * out,
      std::string * NKIT_UNUSED(error))
  {
    const Dynamic * v = sub_path.Get(data);
    if (!v)
      return false;
    *out = (int32_t)v->GetSignedInteger();
    return true;
  }

  inline bool GetSubDynamic(const Dynamic & data,
      const DynamicPath & sub_path, uint32_t * out,
      std::string * NKIT_UNUSED(error))
  {
    const Dynamic * v = sub_path.Get(data);
    if (!v)
      return false;
    *out = (uint32_t)v->GetUnsignedInteger();
    return true;
  }

  inline bool GetSubDynamic(const Dynamic & data,
      const DynamicPath & sub_path, int64_t * out,
      std::string * NKIT_UNUSED(error))
  {
    const Dynamic * v = sub_path.Get(data);
    if (!v)
      return false;
    *out = v->GetSignedInteger();
    return true;
  }

  inline bool GetSubDynamic(const Dynamic & data,
      const DynamicPath & sub_path, uint64_t * out,
      std::string * NKIT_UNUSED(error))
  {
    const Dynamic * v = sub_path.Get(data);
    if (!v)
      return false;
    *out = v->GetUnsignedInteger();
    return true;
  }

#ifdef __APPLE__
  inline bool GetSubDynamic(const Dynamic & data,
      const DynamicPath & sub_path, size_t * out,
      std::string * NKIT_UNUSED(error))
  {
    const Dynamic * v = sub_path.Get(data);
    if (!v)
      return false;
    *out = static_cast<size_t>(v->GetUnsignedInteger());
    return true;
  }
#endif

#ifdef NKIT_WINNT
  inline bool GetSubDynamic(const Dynamic & data,
      const DynamicPath & sub_path, unsigned long * out,
      std::string * NKIT_UNUSED(error))
  {
    const Dynamic * v = sub_path.Get(data);
    if (!v)
      return false;
    *out = static_cast<unsigned long>(v->GetUnsignedInteger());
    return true;
  }
#endif

  inline bool GetSubDynamic(const Dynamic & data,
      const DynamicPath & sub_path, bool * out, std::string * NKIT_UNUSED(error))
  {
    const Dynamic * v = sub_path.Get(data);
    if (!v)
      return false;
    *out << *v;
    return true;
  }

  inline bool GetSubDynamic(const Dynamic & data,
      const DynamicPath & sub_path, const Dynamic ** out,
      std::string * NKIT_UNUSED(error))
  {
    const Dynamic * v = sub_path.Get(data);
    if (!v)
      return false;
    *out = v;
    return true;
  }

  template <typename T>
  void AppendToSequence(T * out, const Dynamic & item)
  {
    out->push_back(item.GetString());
  }

  inline void AppendToSequence(StringSet * out, const Dynamic & item)
  {
    out->insert(item.GetString());
  }

  template <typename T>
  bool GetDynamicListData(const Dynamic & data,
      const DynamicPath & sub_path, T * out, std::string * error)
  {
    const Dynamic * v = sub_path.Get(data);
    if (!v)
      return false;

    if (!v->IsList())
    {
      if (error)
        *error = "Dynamic variable is not LIST";
      return false;
    }

    out->clear();

    DLIST_FOREACH(item, *v)
      AppendToSequence(out, *item);

    return true;
  }

  inline bool GetSubDynamic(const Dynamic & data, const DynamicPath & sub_path,
      StringVector * out, std::string * error = NULL)
  {
    return GetDynamicListData(data, sub_path, out, error);
  }

  inline bool GetSubDynamic(const Dynamic & data, const DynamicPath & sub_path,
      StringList * out, std::string * error = NULL)
  {
    return GetDynamicListData(data, sub_path, out, error);
  }

  inline bool GetSubDynamic(const Dynamic & data, const DynamicPath & sub_path,
      StringSet * out, std::string * error = NULL)
  {
    return GetDynamicListData(data, sub_path, out, error);
  }

  inline bool GetSubDynamic(const Dynamic & data,
      const DynamicPath & sub_path, StringMap * out,
      std::string * error = NULL)
  {
    const Dynamic * v = sub_path.Get(data);
    if (!v)
      return false;

    if (!v->IsDict())
    {
      if (error)
        *error = "Dynamic variable is not DICT";
      return false;
    }

    if (!(*v))
      return false;

    out->clear();

    DDICT_FOREACH(item, *v)
      (*out)[item->first] = item->second.GetString();
    return true;
  }

  inline bool GetSubDynamic(const Dynamic & data,
      const DynamicPath & sub_path, StringDynamicMap * out,
      std::string * error = NULL)
  {
    const Dynamic * v = sub_path.Get(data);
    if (!v)
      return false;

    if (!v->IsDict())
    {
      if (error)
        *error = "Dynamic variable is not DICT";
      return false;
    }

    if (!(*v))
      return false;

    out->clear();

    DDICT_FOREACH(item, *v)
      (*out)[item->first] = item->second;

    return true;
  }

  class DynamicGetter
  {
  public:
    DynamicGetter()
      : path_("__undefined__path__")
      , parent_(NULL)
      , error_(new StringList)
    {
    }

    explicit DynamicGetter(const std::string & location)
      : path_(location)
      , parent_(NULL)
      , error_(new StringList)
    {
      std::string error;
      data_ = DynamicFromJsonFile(path_, &error);
      if (!error.empty())
        SetError(error);
    }

    DynamicGetter(const std::string & json, const std::string & location)
      : path_(location.empty() ? "-" : location)
      , parent_(NULL)
      , error_(new StringList)
    {
      std::string error;
      data_ = DynamicFromJson(json, &error);
      if (!error.empty())
        SetError(error);
    }

    DynamicGetter(Dynamic data, const std::string & location = S_EMPTY_)
      : path_(location.empty() ? "-" : location)
      , parent_(NULL)
      , data_(data)
      , error_(new StringList)
    {}

    DynamicGetter(DynamicGetter & parent,
        const DynamicPath & sub_path)
      : path_(parent.path_)
      , parent_(&parent)
      , sub_path_(sub_path)
      , error_(parent.error_)
    {
      sub_path_.delimiter(parent.delimiter());
      const Dynamic * tmp = sub_path_.Get(parent.data_);
      if (tmp)
        data_ = *tmp;
      else
        SetError("Could not find option '" + dynamic_path() +
          "' in location '" + parent.file_path() + "'");
    }

    DynamicGetter(const DynamicPath & sub_path,
        Dynamic data, const std::string & location)
      : path_(location.empty() ? "-" : location)
      , parent_(NULL)
      , sub_path_(sub_path)
      , error_(new StringList)
    {
      const Dynamic * tmp = sub_path_.Get(data);
      if (tmp)
        data_ = *tmp;
      else
        SetError("Could not find option '" + dynamic_path() +
          "' in location '" + file_path() + "'");
    }

    explicit DynamicGetter(const DynamicGetter & rvalue)
    {
      Set(rvalue);
    }

    DynamicGetter & operator =(const DynamicGetter & rvalue)
    {
      if (this != &rvalue)
        Set(rvalue);
      return *this;
    }

    void delimiter(char d)
    {
      if (parent_)
        parent_->delimiter(d);
      sub_path_.delimiter(d);
    }

    char delimiter() const { return sub_path_.delimiter(); }

    std::string error() const
    {
      std::string error;
      join(*error_, "\n\t", "", "", &error);
      return error;
    }

    void SetError(const std::string & error)
    {
      if (parent_)
        parent_->SetError(error);
      else
        error_->push_front(error);
    }

    void ClearError()
    {
      error_->clear();
    }

    bool ok() const { return error_->empty(); }
    const Dynamic & data() const { return data_; }
    Dynamic & data() { return data_; }
    std::string file_path() const { return path_; }
    DynamicPath dynamic_path() const
    {
      DynamicPath empty_path(delimiter());
      return (parent_ ? parent_->dynamic_path() : empty_path) / sub_path_;
    }

    //--------------------------------------------------------------------------
    template <typename T>
    DynamicGetter & Get(const std::string & sub_path, T * out)
    {
      if (ok())
      {
        DynamicPath _sub_path(delimiter(), sub_path);
        if (!_sub_path.ok())
        {
          SetError("Wrong path '" +
              static_cast<std::string>(dynamic_path() / _sub_path) +
              "': " + _sub_path.error());
          return *this;
        }

        return Get(_sub_path, out);
      }

      return *this;
    }

    //--------------------------------------------------------------------------
    template <typename T>
    DynamicGetter & Get(const std::string & sub_path, T * out, const T & def)
    {
      if (ok())
      {
        DynamicPath _sub_path(delimiter(), sub_path);
        if (!_sub_path.ok())
        {
          SetError("Wrong path '" +
              static_cast<std::string>(dynamic_path() / _sub_path) +
              "': " + _sub_path.error());
          return *this;
        }

        return Get(_sub_path, out, def);
      }

      return *this;
    }

    //--------------------------------------------------------------------------
    template <typename T>
    DynamicGetter & Get(const char * sub_path, T * out)
    {
      return Get(std::string(sub_path), out);
    }

    //--------------------------------------------------------------------------
    template <typename T>
    DynamicGetter & Get(const char * sub_path, T * out, const T & def)
    {
      return Get(std::string(sub_path), out, def);
    }

    //--------------------------------------------------------------------------
    template <typename T>
    DynamicGetter & Get(const size_t index, T * out)
    {
      DynamicPath sub_path(delimiter(), "[%]", index);
      return Get(sub_path, out);
    }

    //--------------------------------------------------------------------------
    template <typename T>
    DynamicGetter & Get(const size_t index, T * out, const T & def)
    {
      DynamicPath sub_path(delimiter(), "[%]", index);
      return Get(sub_path, out, def);
    }

  private:
    //--------------------------------------------------------------------------
    DynamicGetter & Get(const DynamicPath & sub_path,
        DynamicGetter * sub_loader)
    {
      if (ok())
      {
        DynamicGetter result(*this, sub_path);
        if (result.ok())
          *sub_loader = result;
        else
          SetError("Could not find option '" +
            static_cast<std::string>(dynamic_path() / sub_path) +
              "' in location '" + file_path() + "'");
      }

      return *this;
    }

    //--------------------------------------------------------------------------
    template <typename T>
    DynamicGetter & Get(const DynamicPath & sub_path, std::vector<T> * out)
    {
      if (ok())
      {
        DynamicGetter sub_loader(*this, sub_path);
        if (!sub_loader.data().IsList())
        {
          SetError("Value of option '" +
            static_cast<std::string>(dynamic_path() / sub_path) +
              "' in location '" + file_path() + "' must be LIST");
        }
        else
        {
          out->clear();
          sub_loader.data().SaveTo(out);
        }
      }

      return *this;
    }

    //--------------------------------------------------------------------------
    template <typename T>
    DynamicGetter & Get(const DynamicPath & sub_path, std::list<T> * out)
    {
      if (ok())
      {
        DynamicGetter sub_loader(*this, sub_path);
        if (!sub_loader.data().IsList())
        {
          SetError("Value of option '" +
            static_cast<std::string>(dynamic_path() / sub_path) +
              "' in location '" + file_path() + "' must be LIST");
        }
        else
        {
          out->clear();
          sub_loader.data().SaveTo(out);
        }
      }

      return *this;
    }

    //--------------------------------------------------------------------------
    template <typename T>
    DynamicGetter & Get(const DynamicPath & sub_path, std::set<T> * out)
    {
      if (ok())
      {
        DynamicGetter sub_loader(*this, sub_path);
        if (!sub_loader.data().IsList())
        {
          SetError("Value of option '" +
            static_cast<std::string>(dynamic_path() / sub_path) +
              "' in location '" + file_path() + "' must be LIST");
        }
        else
        {
          out->clear();
          sub_loader.data().SaveTo(out);
        }
      }

      return *this;
    }

    //--------------------------------------------------------------------------
    template <typename T>
    DynamicGetter & Get(const DynamicPath & sub_path, NKIT_SHARED_PTR(T) * out)
    {
      if (ok())
      {
        DynamicGetter sub_loader(*this, sub_path);
        if (sub_loader.ok())
        {
          NKIT_SHARED_PTR(T) tmp(T::Create(sub_loader));
          if (sub_loader.ok())
            *out = tmp;
        }
      }
      return *this;
    }

    //--------------------------------------------------------------------------
    template <typename T>
    DynamicGetter & Get(const DynamicPath & sub_path,
      std::vector<NKIT_SHARED_PTR(T) > * out)
    {
      if (ok())
      {
        DynamicGetter sub_loader(*this, sub_path);
        if (!sub_loader.data().IsList())
          SetError("Value of option '" +
            static_cast<std::string>(dynamic_path() / sub_path) +
              "' in location '" + file_path() + "' must be LIST");
        else
        {
          std::vector<NKIT_SHARED_PTR(T) > vtmp;
          size_t size(sub_loader.data().size());
          for (size_t counter = 0; counter < size; ++counter)
          {
            NKIT_SHARED_PTR(T) tmp;
            sub_loader.Get(counter, &tmp);
            if (!sub_loader.ok())
              break;
            vtmp.push_back(tmp);
          }

          *out = vtmp;
        }
      }

      return *this;
    }

    //--------------------------------------------------------------------------
    template <typename T>
    DynamicGetter & Get(const DynamicPath & sub_path,
      std::list<NKIT_SHARED_PTR(T) > * out)
    {
      if (ok())
      {
        DynamicGetter sub_loader(*this, sub_path);
        if (!sub_loader.data().IsList())
          SetError("Value of option '" +
            static_cast<std::string>(dynamic_path() / sub_path) +
              "' in location '" + file_path() + "' must be LIST");
        else
        {
          std::list<NKIT_SHARED_PTR(T) > ltmp;
          size_t size(sub_loader.data().size());
          for (size_t counter = 0; counter < size; ++counter)
          {
            NKIT_SHARED_PTR(T) tmp;
            sub_loader.Get(counter, &tmp);
            if (!sub_loader.ok())
              break;
            ltmp.push_back(tmp);
          }

          *out = ltmp;
        }
      }

      return *this;
    }

    //--------------------------------------------------------------------------
    template <typename K, typename T>
    DynamicGetter & Get(const DynamicPath & sub_path,
        typename std::map<K, NKIT_SHARED_PTR(T) > * out)
    {
      if (ok())
      {
        DynamicGetter sub_loader(*this, sub_path);
        if (!sub_loader.data().IsDict())
          SetError("Value of option '" +
            static_cast<std::string>(dynamic_path() / sub_path) +
              "' in location '" + file_path() + "' must be DICT");
        else
        {
          std::map<K, NKIT_SHARED_PTR(T) > tmp_map;
          DDICT_FOREACH(pair, sub_loader.data())
          {
            NKIT_SHARED_PTR(T) tmp;
            sub_loader.Get(sub_loader.delimiter() + pair->first, &tmp);
            if (!sub_loader.ok())
              break;
            K k;
            k << pair->first;
            tmp_map[k] = tmp;
          }

          *out = tmp_map;
        }
      }

      return *this;
    }

    //--------------------------------------------------------------------------
    template <typename K, typename V>
    DynamicGetter & Get(const DynamicPath & sub_path,
      typename std::map<K, V> * out)
    {
      if (ok())
      {
        DynamicGetter sub_loader(*this, sub_path);
        if (!sub_loader.data().IsDict())
        {
          SetError("Value of option '" +
            static_cast<std::string>(dynamic_path() / sub_path) +
              "' in location '" + file_path() + "' must be DICT");
        }
        {
          *out << sub_loader.data();
        }
      }

      return *this;
    }

    //--------------------------------------------------------------------------
    DynamicGetter & Get(const DynamicPath & sub_path, const Dynamic ** out)
    {
      if (ok())
      {
        std::string error;
        if (!GetSubDynamic(data_, sub_path, out, &error))
        {
          if (error.empty())
            SetError("Could not find option '" +
                static_cast<std::string>(dynamic_path() / sub_path) +
                "' in location '" + file_path() + "'");
          else
            SetError("Wrong value of option '" +
                static_cast<std::string>(dynamic_path() / sub_path) +
                "' in location '" + file_path() + "': " + error);
        }
      }

      return *this;
    }

    //--------------------------------------------------------------------------
    template <typename T>
    DynamicGetter & Get(const DynamicPath & sub_path, T * out)
    {
      if (ok())
      {
        std::string error;
        if (!GetSubDynamic(data_, sub_path, out, &error))
        {
          if (error.empty())
            SetError("Could not find option '" +
                static_cast<std::string>(dynamic_path() / sub_path) +
                "' in location '" + file_path() + "'");
          else
            SetError("Wrong value of option '" +
                static_cast<std::string>(dynamic_path() / sub_path) +
                "' in location '" + file_path() + "': " + error);
        }
      }

      return *this;
    }

    //--------------------------------------------------------------------------
    template <typename T>
    DynamicGetter & Get(const DynamicPath & sub_path, T * out, const T & def)
    {
      if (ok())
      {
        Get(sub_path, out);
        if (!ok())
        {
          *out = def;
          ClearError();
        }
      }
      return *this;
    }

    //--------------------------------------------------------------------------
    template <typename T, typename Checker>
    DynamicGetter & Get(const DynamicPath & sub_path, T * out,
        Checker checker)
    {
      if (ok())
      {
        std::string error;
        if (!GetSubDynamic(data_, sub_path, out, &error))
        {
          if (error.empty())
            SetError("Could not find option '" +
                static_cast<std::string>(dynamic_path() / sub_path) +
                "' in location '" + file_path() + "'");
          else
            SetError("Wrong value of option '" +
                static_cast<std::string>(dynamic_path() / sub_path) +
                "' in location '" + file_path() + "': " + error);
        }

        if (!checker.Check(*out))
          SetError("Not valid option '" +
              static_cast<std::string>(dynamic_path() / sub_path) +
              "' in location '" + file_path() + "': " + checker.error());
      }

      return *this;
    }

    void Set(const DynamicGetter & from)
    {
      path_ = from.path_;
      parent_ = from.parent_;
      sub_path_ = from.sub_path_;
      data_ = from.data_;
      error_ = from.error_;
    }

  private:
    std::string path_;
    DynamicGetter * parent_;
    DynamicPath sub_path_;
    Dynamic data_;
    NKIT_SHARED_PTR(StringList) error_;
  };

  //----------------------------------------------------------------------------
  typedef bool (*DynamicChecker)(const Dynamic &);

  inline bool get(const Dynamic & var, const std::string & option,
      StringSet * const set, const DynamicChecker & checker,
      std::string * const error)
  {
    const Dynamic *v;
    if (!var.Get(option, &v))
    {
      *error = "Missing parameter '" + option + "'";
      return false;
    }

    DLIST_FOREACH(item, *v)
    {
      if (!checker(*item))
      {
        *error = "Invalid value of parameter '" + option + "'";
        return false;
      }

      set->insert(item->GetConstString());
    }

    return true;
  }

  inline bool get(const Dynamic & var, const std::string & name1,
      const std::string & name2, std::string * const value)
  {
    const Dynamic *v1, *v;
    if (var.Get(name1, &v1) && v1->Get(name2, &v))
    {
      *value = v->GetString();
      return true;
    }
    return false;
  }

  inline bool get(const Dynamic & var, const std::string & name1,
      const std::string & name2, const Dynamic ** const value)
  {
    const Dynamic * v1, *v2;
    if (var.Get(name1, &v1) && v1->Get(name2, &v2))
    {
      *value = v2;
      return true;
    }
    return false;
  }

  inline bool get(const Dynamic & var, const std::string & name,
      const Dynamic ** const value)
  {
    const Dynamic *v;
    if (var.Get(name, &v))
    {
      *value = v;
      return true;
    }
    return false;
  }

  inline bool get(const Dynamic & var, const std::string & name,
      std::string * const value)
  {
    const Dynamic *v;
    if (var.Get(name, &v))
    {
      if (v->IsString())
        *value = v->GetConstString();
      else
        *value = v->GetString();
      return true;
    }
    return false;
  }

  inline bool get(const Dynamic & var, const std::string & name,
      bool * const value)
  {
    const Dynamic *v;
    if (var.Get(name, &v))
    {
      *value << *v;
      //*value = v->GetBoolean();
      return true;
    }
    return false;
  }

  inline bool get(const Dynamic & var, const std::string & name,
      uint64_t * const value)
  {
    const Dynamic *v;
    if (var.Get(name, &v))
    {
      *value = v->GetUnsignedInteger();
      return true;
    }
    return false;
  }

  inline bool get(const Dynamic & var, const std::string & name,
      int64_t * const value)
  {
    const Dynamic *v;
    if (var.Get(name, &v))
    {
      *value = v->GetSignedInteger();
      return true;
    }
    return false;
  }

  inline bool get(const Dynamic & var, const std::string & name,
      double * const value)
  {
    const Dynamic *v;
    if (var.Get(name, &v))
    {
      *value = v->GetFloat();
      return true;
    }
    return false;
  }

  inline bool get(const Dynamic & var, const std::string & name,
      uint16_t * const value)
  {
    uint64_t tmp;
    if (!get(var, name, &tmp))
      return false;
    if (tmp > MAX_UINT16_VALUE)
      return false;
    *value = static_cast<uint16_t>(tmp);
    return true;
  }

  inline bool get(const Dynamic & var,const std::string & name,
      int16_t * const value)
  {
    int64_t tmp;
    if (!get(var, name, &tmp))
      return false;
    if (tmp > MAX_INT16_VALUE || tmp < MIN_INT16_VALUE)
      return false;
    *value = static_cast<int16_t>(tmp);
    return true;
  }

  inline bool get(const Dynamic & var, const std::string & name,
      uint32_t * const value)
  {
    uint64_t tmp;
    if (!get(var, name, &tmp))
      return false;
    if (tmp > MAX_UINT32_VALUE)
      return false;
    *value = static_cast<uint32_t>(tmp);
    return true;
  }

  inline bool get(const Dynamic & var,const std::string & name,
      int32_t * const value)
  {
    int64_t tmp;
    if (!get(var, name, &tmp))
      return false;
    if (tmp > MAX_INT32_VALUE || tmp < MIN_INT32_VALUE)
      return false;
    *value = static_cast<int32_t>(tmp);
    return true;
  }

  template <typename T>
  void get(const Dynamic & var, const std::string & name,
    T * const value, const T & _default)
  {
    if (!get(var, name, value))
      *value = _default;
  }

} // namespace nkit

#define __NKIT_OPTIONS_BEGIN(prefix, line) template <int n>                   \
  class prefix##Option : public prefix##Option<n-1> {                         \
  public:                                                                     \
    bool Init(DynamicGetter & config)                                         \
    {                                                                         \
      return prefix##Option<n-1>::Init(config);                               \
    }                                                                         \
    void SaveToDynamic(Dynamic * out) const {                                 \
      prefix##Option<n-1>::SaveToDynamic(out);                                \
    }                                                                         \
  };                                                                          \
  template <> class prefix##Option<line>                                      \
  {                                                                           \
  public:                                                                     \
    bool Init(DynamicGetter & config) { return config.ok(); }                 \
    void SaveToDynamic(Dynamic * ) const {}                                   \
  };                                                                          \

#define _NKIT_OPTIONS_BEGIN(prefix, line) __NKIT_OPTIONS_BEGIN(prefix, line)
#define NKIT_OPTIONS_BEGIN \
  _NKIT_OPTIONS_BEGIN(NKIT_OPTIONS_CLASS_NAME, __LINE__)

//------------------------------------------------------------------------------
#define __NKIT_OPTION(prefix, line, type, prop, option) template <>           \
    class prefix##Option<line> :                                              \
      public prefix##Option<line-1>                                           \
    {                                                                         \
    public:                                                                   \
      prefix##Option<line>()                                                  \
        : prop##_option_(option)                                              \
        , prop##_option_path_(option)                                         \
        , prop##_value_()                                                     \
      {}                                                                      \
      bool Init(DynamicGetter & config)                                       \
      {                                                                       \
        config.Get(prop##_option_path_, &prop##_value_);                      \
        return prefix##Option<line-1>::Init(config);                          \
      }                                                                       \
      void SaveToDynamic(Dynamic * dict) const                                \
      {                                                                       \
        Dynamic tmp;                                                          \
        nkit::VarToDynamic<type>::Save(prop##_value_, &tmp);                  \
        (*dict)[prop##_option_] = tmp;                                        \
        prefix##Option<line-1>::SaveToDynamic(dict);                          \
      }                                                                       \
      const type & prop() const { return prop##_value_; }                     \
      void prop(const type & v) { prop##_value_ = v; }                        \
    private:                                                                  \
      std::string prop##_option_;                                             \
      std::string prop##_option_path_;                                        \
      type prop##_value_;                                                     \
    };

#define _NKIT_OPTION(prefix, line, type, prop, option)                        \
  __NKIT_OPTION(prefix, line, type, prop, option)
#define NKIT_OPTION(type, prop, option)                                       \
  _NKIT_OPTION(NKIT_OPTIONS_CLASS_NAME, __LINE__, type, prop, option)

//------------------------------------------------------------------------------
#define __NKIT_OPTION_DEFAULT(prefix, line, type, prop, option, def_val)      \
    template <> class prefix##Option<line> : public prefix##Option<line-1>    \
    {                                                                         \
    public:                                                                   \
      prefix##Option<line>()                                                  \
        : prop##_option_(option)                                              \
        , prop##_option_path_(option)                                         \
        , prop##_def_val_(def_val)                                            \
        , prop##_value_(def_val)                                              \
      {}                                                                      \
      bool Init(DynamicGetter & config)                                       \
      {                                                                       \
        config.Get(prop##_option_path_, &prop##_value_, prop##_def_val_);     \
        return prefix##Option<line-1>::Init(config);                          \
      }                                                                       \
      void SaveToDynamic(Dynamic * dict) const                                \
      {                                                                       \
        Dynamic tmp;                                                          \
        nkit::VarToDynamic<type>::Save(prop##_value_, &tmp);                  \
        (*dict)[prop##_option_] = tmp;                                        \
        prefix##Option<line-1>::SaveToDynamic(dict);                          \
      }                                                                       \
      const type & prop() const { return prop##_value_; }                     \
      void prop(const type & v) { prop##_value_ = v; }                        \
    private:                                                                  \
      std::string prop##_option_;                                             \
      std::string prop##_option_path_;                                        \
      type prop##_def_val_;                                                   \
      type prop##_value_;                                                     \
    };

#define _NKIT_OPTION_DEFAULT(prefix, line, type, prop, option, def_val)       \
  __NKIT_OPTION_DEFAULT(prefix, line, type, prop, option, def_val)
#define NKIT_OPTION_DEFAULT(type, prop, option, def_val)                      \
  _NKIT_OPTION_DEFAULT                                                        \
    (NKIT_OPTIONS_CLASS_NAME, __LINE__, type, prop, option, def_val)

  //--------------------------------------------------------------------------
#define __NKIT_OPTIONS_END(prefix, line) class prefix :                       \
  public prefix##Option<line-1>                                               \
  {                                                                           \
  public:                                                                     \
    typedef NKIT_SHARED_PTR(prefix) Ptr;                                      \
    static Ptr Create()                                                       \
    {                                                                         \
      return Ptr(new prefix);                                                 \
    }                                                                         \
    static Ptr Create(const Dynamic & d, std::string * error)                 \
    {                                                                         \
      DynamicGetter getter(d);                                                \
      Ptr result = Create(getter);                                            \
      if (!result.get())                                                      \
        *error = getter.error();                                              \
      return result;                                                          \
    }                                                                         \
    static Ptr Create(const std::string & json, std::string * error)          \
    {                                                                         \
      Dynamic d = DynamicFromJson(json, error);                               \
      DynamicGetter getter(d);                                                \
      return Create(getter);                                                  \
    }                                                                         \
    static Ptr Create(DynamicGetter & getter)                                 \
    {                                                                         \
      Ptr result(new prefix);                                                 \
      if (!result->prefix##Option<line-1>::Init(getter))                      \
        return Ptr();                                                         \
      return result;                                                          \
    }                                                                         \
    void SaveToJson(std::string * result) const                               \
    {                                                                         \
      Dynamic dict = Dynamic::Dict();                                         \
      prefix##Option<line-1>::SaveToDynamic(&dict);                           \
      DynamicToJson(dict, result);                                            \
    }                                                                         \
    std::string SaveToJson() const                                            \
    {                                                                         \
      std::string result;                                                     \
      SaveToJson(&result);                                                    \
      return result;                                                          \
    }                                                                         \
    Dynamic SaveToDynamic() const                                             \
    {                                                                         \
      Dynamic result = Dynamic::Dict();                                       \
      prefix##Option<line-1>::SaveToDynamic(&result);                         \
      return result;                                                          \
    }                                                                         \
    void SaveToDynamic(Dynamic * out) const                                   \
    {                                                                         \
      *out = Dynamic::Dict();                                                 \
      prefix##Option<line-1>::SaveToDynamic(out);                             \
    }                                                                         \
  };

#define _NKIT_OPTIONS_END(prefix, line) \
    __NKIT_OPTIONS_END(prefix, line)
#define NKIT_OPTIONS_END _NKIT_OPTIONS_END(NKIT_OPTIONS_CLASS_NAME, __LINE__)

namespace nkit
{
  template <typename T>
  struct VarToDynamic
  {
    static void Save(const T & value, Dynamic * out)
    {
      *out = Dynamic(value);
    }
  };

  template <typename T>
  struct VarToDynamic<NKIT_SHARED_PTR(T)>
  {
    static void Save(const NKIT_SHARED_PTR(T) & value, Dynamic * out)
    {
      *out = Dynamic::Dict();
      if (value)
        value->SaveToDynamic(out);
    }
  };

  template <typename T>
  struct VarToDynamic<std::vector<NKIT_SHARED_PTR(T)> >
  {
    static void Save(const std::vector<NKIT_SHARED_PTR(T)> & vv, Dynamic * out)
    {
      *out = Dynamic::List();
      typename std::vector<NKIT_SHARED_PTR(T)>::const_iterator it = vv.begin(),
          end = vv.end();
      for (; it != end; ++it)
      {
        Dynamic item = Dynamic::Dict();
        (*it)->SaveToDynamic(&item);
        out->PushBack(item);
      }
    }
  };

  template <typename T>
  struct VarToDynamic<std::vector<T> >
  {
    static void Save(const std::vector<T> & vv, Dynamic * out)
    {
      *out = Dynamic::List();
      typename std::vector<T>::const_iterator it = vv.begin(),
          end = vv.end();
      for (; it != end; ++it)
      {
        out->PushBack(Dynamic(*it));
      }
    }
  };

  template <typename T>
  struct VarToDynamic<std::list<NKIT_SHARED_PTR(T)> >
  {
    static void Save(const std::list<NKIT_SHARED_PTR(T)> & vv, Dynamic * out)
    {
      *out = Dynamic::List();
      typename std::list<NKIT_SHARED_PTR(T)>::const_iterator it = vv.begin(),
          end = vv.end();
      for (; it != end; ++it)
      {
        Dynamic item = Dynamic::Dict();
        (*it)->SaveToDynamic(&item);
        out->PushBack(item);
      }
    }
  };

  template <typename T>
  struct VarToDynamic<std::list<T> >
  {
    static void Save(const std::list<T> & vv, Dynamic * out)
    {
      *out = Dynamic::List();
      typename std::list<T>::const_iterator it = vv.begin(),
          end = vv.end();
      for (; it != end; ++it)
      {
        out->PushBack(Dynamic(*it));
      }
    }
  };

  template <typename T>
  struct VarToDynamic<std::set<NKIT_SHARED_PTR(T)> >
  {
    static void Save(const std::set<NKIT_SHARED_PTR(T)> & vv, Dynamic * out)
    {
      *out = Dynamic::List();
      typename std::set<NKIT_SHARED_PTR(T)>::const_iterator it = vv.begin(),
          end = vv.end();
      for (; it != end; ++it)
      {
        Dynamic item = Dynamic::Dict();
        (*it)->SaveToDynamic(&item);
        out->PushBack(item);
      }
    }
  };

  template <typename T>
  struct VarToDynamic<std::set<T> >
  {
    static void Save(const std::set<T> & vv, Dynamic * out)
    {
      *out = Dynamic::List();
      typename std::set<T>::const_iterator it = vv.begin(),
          end = vv.end();
      for (; it != end; ++it)
      {
        out->PushBack(Dynamic(*it));
      }
    }
  };

  template <typename T>
  struct VarToDynamic<std::map<std::string, NKIT_SHARED_PTR(T)> >
  {
    static void Save(const std::map<std::string, NKIT_SHARED_PTR(T)> & map,
        Dynamic * out)
    {
      *out = Dynamic::Dict();
      typename std::map<std::string, NKIT_SHARED_PTR(T)>::const_iterator
          it = map.begin(), end = map.end();
      for (; it != end; ++it)
      {
        Dynamic item = Dynamic::Dict();
        it->second->SaveToDynamic(&item);
        (*out)[it->first] = item;
      }
    }
  };

  template <typename T>
  struct VarToDynamic<std::map<std::string, T> >
  {
    static void Save(const std::map<std::string, T> & map,
        Dynamic * out)
    {
      *out = Dynamic::Dict();
      typename std::map<std::string, T>::const_iterator
          it = map.begin(), end = map.end();
      for (; it != end; ++it)
      {
        (*out)[it->first] = Dynamic(it->second);
      }
    }
  };
}  // namespace nkit

#endif // __NKIT__DYNAMIC__JSON__CONFIG__H__
