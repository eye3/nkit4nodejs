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

namespace nkit
{
  namespace detail
  {
    //--------------------------------------------------------------------------
    class SharedMap : public Shared<StringDynamicMap>
    {
    public:
      static SharedMap * Get(const Data & data)
      {
        return data.shared_map_;
      }
    };

    //--------------------------------------------------------------------------
    template <>
    class Impl<detail::DICT> : public ImplDefault
    {
    public:
      static void Create(Dynamic & v)
      {
        Reset(&v, new detail::SharedMap());
      }

      static void OP_CLEAR(Dynamic & v)
      {
        GetMap(v.data_).clear();
      }

      static Dynamic OP_CLONE(const Data & v)
      {
        Dynamic result(Dynamic::Dict());
        const StringDynamicMap & map = GetMap(v);
        StringDynamicMap::const_iterator it = map.begin(), end = map.end();
        for (; it != end; ++it)
          result[it->first] = it->second.Clone();
        return result;
      }

      static void OP_DEC_REF_DYNAMIC(Dynamic & v)
      {
        detail::SharedMap * shared = GetSharedPtr(v.data_);
        if (shared->DecRef() == 0)
        {
          delete shared;
          v.Reset();
        }
      }

      static void OP_INC_REF_DATA(Data & data)
      {
        GetSharedPtr(data)->IncRef();
      }

      static void OP_DEC_REF_DATA(Data & data)
      {
        detail::SharedMap * shared = GetSharedPtr(data);
        if (shared->DecRef() == 0)
        {
          delete shared;
          data.i64_ = 0;
        }
      }

      static bool OP_GET_BOOL(const Data & v)
      {
        return !OP_IS_EMPTY(v);
      }

      static bool OP_IS_EMPTY(const Data & v)
      {
        return GetMap(v).empty();
      }

      static size_t OP_GET_SIZE(const Data & v)
      {
        return GetMap(v).size();
      }

      static std::string OP_GET_STRING(const Data & v,
          const char * NKIT_UNUSED(format))
      {
        return OP_GET_CONST_STRING(v);
      }

      static std::string OP_GET_STRING(const Dynamic & v,
          const char * NKIT_UNUSED(format))
      {
        return OP_GET_CONST_STRING(v.data_);
      }

      template<typename T>
      static bool OP_EQ(const Dynamic & lv, const Dynamic & rv)
      {
        if (!rv.IsDict())
          return false;
        const StringDynamicMap & l = GetMap(lv.data_);
        const StringDynamicMap & r = GetMap(rv.data_);
        return (l.size() == r.size())
            && std::equal(l.begin(), l.end(), r.begin());
      }

      static Dynamic & Set(Dynamic & v, const std::string & key,
          const Dynamic & item)
      {
        detail::SharedMap * shared = GetSharedPtr(v.data_);

        StringDynamicMap & map = shared->GetRef();
        Dynamic & value = map[key];
        value = item;
        return value;
      }

      static Dynamic & Get(Dynamic & v, const std::string & key)
      {
        StringDynamicMap & map = GetMap(v.data_);
        return map[key];
      }

      static const Dynamic & Get(const Dynamic & v, const std::string & key)
      {
        Dynamic * result;
        if (!Get(const_cast<Dynamic & >(v), key, &result))
          return D_NONE;
        return *result;
      }

      static bool Get(const Dynamic & v, const std::string & key,
          const Dynamic ** const value)
      {
        Dynamic * result;
        if (!Get(const_cast<Dynamic & >(v), key, &result))
          return false;
        *value = result;
        return true;
      }

      static bool Get(Dynamic & v, const std::string & key,
          Dynamic ** const value)
      {
        StringDynamicMap & map = GetMap(v.data_);
        StringDynamicMap::iterator it = map.find(key);
        if (it == map.end())
          return false;
        *value = &(it->second);
        return true;
      }

      static StringDynamicMap::const_iterator Begin(const Dynamic & v)
      {
        return GetMap(v.data_).begin();
      }

      static StringDynamicMap::const_iterator End(const Dynamic & v)
      {
        return GetMap(v.data_).end();
      }

      static StringDynamicMap::iterator Begin(Dynamic & v)
      {
        return GetMap(v.data_).begin();
      }

      static StringDynamicMap::iterator End(Dynamic & v)
      {
        return GetMap(v.data_).end();
      }

      static StringDynamicMap::const_iterator Find(const Dynamic & v,
          const std::string & key)
      {
        return GetMap(v.data_).find(key);
      }

      static void GetKeys(const Dynamic & v, StringSet * keys)
      {
        keys->clear();
        const StringDynamicMap & map = GetMap(v.data_);
        StringDynamicMap::const_iterator it = map.begin(), end = map.end();
        for (; it != end; ++it)
          keys->insert(it->first);
      }

      static void DeleteByKey(Dynamic & v, const std::string & key)
      {
        StringDynamicMap & map = GetMap(v.data_);
        StringDynamicMap::iterator it = map.find(key);
        if (it != map.end())
          map.erase(it);
      }

      static void Update(Dynamic & v, const Dynamic & rv)
      {
        if (rv.IsDict())
        {
          StringDynamicMap & to = GetMap(v.data_);
          const StringDynamicMap & from = GetMap(rv.data_);
          StringDynamicMap::const_iterator i = from.begin();
          StringDynamicMap::const_iterator end = from.end();
          for (; i != end; ++i)
          {
            const Dynamic & _from = i->second;
            Dynamic & _to = to[i->first];
            if (_to.IsDict() && _from.IsDict())
              Update(_to, _from);
            else
              _to = _from;
          }
        }
      }

    private:
      static const StringDynamicMap & GetMap(const Data & v)
      {
        return GetSharedPtr(v)->GetRef();
      }

      static StringDynamicMap & GetMap(Data & v)
      {
        return GetSharedPtr(v)->GetRef();
      }

      static const detail::SharedMap * GetSharedPtr(const Data & data)
      {
        return detail::SharedMap::Get(data);
      }

      static detail::SharedMap * GetSharedPtr(Data & data)
      {
        return detail::SharedMap::Get(data);
      }

      static void Reset(Dynamic * v, detail::SharedMap * const dict)
      {
        v->type_ = DICT;
        v->data_.shared_map_ = dict;
      }
    };

    //--------------------------------------------------------------------------
    class ConstMapAdapter
    {
    public:
      typedef StringDynamicMap::const_iterator iterator;
      typedef StringDynamicMap::const_iterator const_iterator;

      ConstMapAdapter(const Dynamic & dict)
        : hash_(dict)
      {}

      const_iterator begin() const
      {
        if (hash_.IsDict())
          return detail::Impl<detail::DICT>::Begin(hash_);
        else
          return empty_map_.end();
      }

      const_iterator end() const
      {
        if (hash_.IsDict())
          return detail::Impl<detail::DICT>::End(hash_);
        else
          return empty_map_.end();
      }

    private:
      const Dynamic & hash_;
      static const StringDynamicMap empty_map_;
    };

    //--------------------------------------------------------------------------
    class MapAdapter
    {
    public:
      typedef StringDynamicMap::iterator iterator;
      typedef StringDynamicMap::const_iterator const_iterator;

      MapAdapter(Dynamic & dict)
        : hash_(dict)
      {}

      iterator begin() const
      {
        if (hash_.IsDict())
          return detail::Impl<detail::DICT>::Begin(hash_);
        else
          return empty_map_.end();
      }

      iterator end() const
      {
        if (hash_.IsDict())
          return detail::Impl<detail::DICT>::End(hash_);
        else
          return empty_map_.end();
      }

    private:
      Dynamic & hash_;
      static StringDynamicMap empty_map_;
    };

    //--------------------------------------------------------------------------
    class DynamicDictBuilder
    {
    public:
      DynamicDictBuilder() : dict_(Dynamic::Dict()) {}

      DynamicDictBuilder & operator << (const char * v)
      {
        if (key_.empty())
          key_ = v;
        else
        {
          dict_[key_] = Dynamic(v);
          key_.clear();
        }
        return *this;
      }

      DynamicDictBuilder & operator << (const std::string & v)
      {
        if (key_.empty())
          key_ = v;
        else
        {
          dict_[key_] = Dynamic(v);
          key_.clear();
        }
        return *this;
      }

      DynamicDictBuilder & operator << (const Dynamic & v)
      {
        if (key_.empty())
          key_ = v.GetString();
        else
        {
          dict_[key_] = v;
          key_.clear();
        }
        return *this;
      }

      template <typename T>
      DynamicDictBuilder & operator << (const T v)
      {
        if (key_.empty())
          key_ = Dynamic(v).GetString();
        else
        {
          dict_[key_] = Dynamic(v);
          key_.clear();
        }
        return *this;
      }

      Dynamic dict() const
      {
        assert(key_.empty());
        return dict_;
      }

    private:
      Dynamic dict_;
      std::string key_;
    };
  } // namespace detail
} // namespace nkit

#define DDICT(x) (( nkit::detail::DynamicDictBuilder() << x ).dict())

#define DDICT_FOREACH(item, container) \
  nkit::detail::ConstMapAdapter\
    MAKE_NAME_WITH_LINE(a, __LINE__)(container);\
  nkit::detail::ConstMapAdapter::const_iterator\
    MAKE_NAME_WITH_LINE(end, __LINE__) = \
      MAKE_NAME_WITH_LINE(a, __LINE__).end();\
  for(nkit::detail::ConstMapAdapter::const_iterator item =\
        MAKE_NAME_WITH_LINE(a, __LINE__).begin();\
      item != MAKE_NAME_WITH_LINE(end, __LINE__); ++item)

#define DDICT_FOREACH_MUTABLE(item, container) \
  nkit::detail::MapAdapter\
    MAKE_NAME_WITH_LINE(a, __LINE__)(container);\
  nkit::detail::MapAdapter::iterator\
    item = MAKE_NAME_WITH_LINE(a, __LINE__).begin(),\
    MAKE_NAME_WITH_LINE(end, __LINE__) = \
      MAKE_NAME_WITH_LINE(a, __LINE__).end();\
  for(nkit::detail::MapAdapter::iterator item =\
        MAKE_NAME_WITH_LINE(a, __LINE__).begin();\
      item != MAKE_NAME_WITH_LINE(end, __LINE__); ++item)
