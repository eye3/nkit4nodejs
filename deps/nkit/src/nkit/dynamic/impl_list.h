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

#include <iterator>

namespace nkit
{
  namespace detail
  {
    //--------------------------------------------------------------------------
    class SharedVector : public Shared<DynamicVector>
    {
    public:
      static SharedVector * Get(const Data & data)
      {
        return data.shared_vector_;
      }
    };

    //--------------------------------------------------------------------------
    template <>
    class Impl<detail::LIST> : public ImplDefault
    {
    public:
      static void Create(Dynamic & v)
      {
        Reset(&v, new detail::SharedVector());
      }

      static void OP_CLEAR(Dynamic & v)
      {
        GetVector(v.data_).clear();
      }

      static Dynamic OP_CLONE(const Data & v)
      {
        Dynamic result(Dynamic::List());
        const DynamicVector & vect = GetVector(v);
        DynamicVector::const_iterator it = vect.begin(), end = vect.end();
        for (; it != end; ++it)
          result.PushBack(it->Clone());
        return result;
      }

      static void OP_DEC_REF_DYNAMIC(Dynamic & v)
      {
        detail::SharedVector * shared = GetSharedPtr(v.data_);
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
        detail::SharedVector * shared = GetSharedPtr(data);
        if (shared->DecRef() == 0)
        {
          delete shared;
          data.i64_ = 0;
        }
      }

      static DynamicVector::const_iterator Begin(const Dynamic & v)
      {
        return GetVector(v.data_).begin();
      }

      static DynamicVector::const_iterator End(const Dynamic & v)
      {
        return GetVector(v.data_).end();
      }

      static DynamicVector::const_reverse_iterator Rbegin(const Dynamic & v)
      {
        return GetVector(v.data_).rbegin();
      }

      static DynamicVector::const_reverse_iterator Rend(const Dynamic & v)
      {
        return GetVector(v.data_).rend();
      }

      static DynamicVector::iterator Begin(Dynamic & v)
      {
        return GetVector(v.data_).begin();
      }

      static DynamicVector::iterator End(Dynamic & v)
      {
        return GetVector(v.data_).end();
      }

      static DynamicVector::reverse_iterator Rbegin(Dynamic & v)
      {
        return GetVector(v.data_).rbegin();
      }

      static DynamicVector::reverse_iterator Rend(Dynamic & v)
      {
        return GetVector(v.data_).rend();
      }

      static bool OP_GET_BOOL(const Data & v)
      {
        return !OP_IS_EMPTY(v);
      }

      static bool OP_IS_EMPTY(const Data & v)
      {
        return GetVector(v).empty();
      }

      static size_t OP_GET_SIZE(const Data & v)
      {
        return GetVector(v).size();
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

      static size_t FindStringPosI(const Dynamic & v, const std::string & str)
      {
        const DynamicVector & vect = GetVector(v.data_);
        size_t counter = 0;
        DynamicVector::const_iterator it = vect.begin(), end = vect.end();
        for (; it != end; ++it)
        {
          if (NKIT_STRCASECMP(it->GetConstString().c_str(), str.c_str()) == 0)
            return counter;
          ++counter;
        }

        return Dynamic::npos;
      }

      static size_t GetIndexOf(const Dynamic & list, const Dynamic & v)
      {
        const DynamicVector & vect = GetVector(list.data_);
        size_t counter = 0;
        DynamicVector::const_iterator it = vect.begin(), end = vect.end();
        for (; it != end; ++it)
        {
          if (*it == v)
            return counter;
          ++counter;
        }

        return Dynamic::npos;
      }

      static const Dynamic & GetItem(const Dynamic & v, const size_t pos)
      {
        const DynamicVector & arr = GetVector(v.data_);
        if (arr.size() <= pos)
          return D_NONE;
        return arr[pos];
      }

      static Dynamic & GetItem(Dynamic & v, const size_t pos)
      {
        DynamicVector & arr = GetVector(v.data_);
        if (arr.size() <= pos)
          return D_NONE;
        return arr[pos];
      }

      static bool GetItem(const Dynamic & v, const size_t pos,
          const Dynamic ** out)
      {
        const DynamicVector & arr = GetVector(v.data_);
        if (arr.size() <= pos)
          return false;
        *out = & arr[pos];
        return true;
      }

      static bool GetItem(Dynamic & v, const size_t pos, Dynamic ** out)
      {
        DynamicVector & arr = GetVector(v.data_);
        if (arr.size() <= pos)
          return false;
        *out = & arr[pos];
        return true;
      }

      static void Join(const Dynamic & v, const std::string & delimiter,
          const std::string & prefix, const std::string & postfix,
          std::string * out)
      {
        const DynamicVector & container = GetVector(v.data_);
        DynamicVector::const_iterator it = container.begin(),
            end = container.end();
        bool first(true);
        for (; it != end; ++it)
        {
          if (!first)
            *out += delimiter;
          else
            first = false;
          *out += prefix;
          if (it->IsString())
            *out += it->GetConstString();
          else
            *out += it->GetString();
          *out += postfix;
        }
      }

      static void Join(const Dynamic & v, const std::string & delimiter,
          const std::string & prefix, const std::string & postfix,
          const char * format, std::string * out)
      {
        const DynamicVector & container = GetVector(v.data_);
        DynamicVector::const_iterator it = container.begin(),
            end = container.end();
        bool first(true);
        for (; it != end; ++it)
        {
          if (!first)
            *out += delimiter;
          else
            first = false;
          *out += prefix;
          *out += it->GetString(format);
          *out += postfix;
        }
      }

      template <typename T>
      static void Join(const Dynamic & v, const std::string & delimiter,
          const std::string & prefix, const std::string & postfix,
          T & formatter, std::string * out)
      {
        const DynamicVector & container = GetVector(v.data_);
        DynamicVector::const_iterator it = container.begin(),
            end = container.end();
        bool first(true);
        for (; it != end; ++it)
        {
          if (!first)
            *out += delimiter;
          else
            first = false;
          *out += prefix;
          *out += formatter(*it);
          *out += postfix;
        }
      }

      static void Extend(Data & v, const Data & rv)
      {
        DynamicVector & to = GetVector(v);
        const DynamicVector & from = GetVector(rv);
        std::copy(from.begin(), from.end(), std::back_inserter(to));
      }

      static void PushBack(Dynamic & v, const Dynamic & rv)
      {
        GetVector(v.data_).push_back(rv);
      }

      static void PushFront(Dynamic & v, const Dynamic & rv)
      {
        DynamicVector & to = GetVector(v.data_);
        to.insert(to.begin(), rv);
      }

      static void PopBack(Dynamic & v)
      {
        DynamicVector & from = GetVector(v.data_);
        if (!from.empty())
          from.pop_back();
      }

      static void PopFront(Dynamic & v)
      {
        DynamicVector & from = GetVector(v.data_);
        if (!from.empty())
          from.erase(from.begin());
      }

      static void Erase(Dynamic & self, const size_t & pos)
      {
        DynamicVector & v = GetVector(self.data_);
        assert(v.size() > pos);
        v.erase(v.begin() + pos);
      }

      static void Erase(Dynamic & self, const size_t & from, const size_t & to)
      {
        DynamicVector & v = GetVector(self.data_);
        size_t size = v.size();
        if (from >= size)
          return;

        v.erase(v.begin() + from, (to >= size ? v.end(): (v.begin() + to)));
      }

      static void Erase(Dynamic & self, const SizeSet & pos_set)
      {
        DynamicVector & v = GetVector(self.data_);
        SizeSet::const_reverse_iterator it = pos_set.rbegin(),
            end = pos_set.rend();
        for (; it != end; ++it)
        {
          assert(v.size() > *it);
          v.erase(v.begin() + *it);
        }
      }

      static const Dynamic & GetFront(const Dynamic & v)
      {
        const DynamicVector & from = GetVector(v.data_);
        return *from.begin();
      }

      static Dynamic & GetFront(Dynamic & v)
      {
        DynamicVector & from = GetVector(v.data_);
        return *from.begin();
      }

      static const Dynamic & GetBack(const Dynamic & v)
      {
        const DynamicVector & from = GetVector(v.data_);
        return from.back();
      }

      static Dynamic & GetBack(Dynamic & v)
      {
        DynamicVector & from = GetVector(v.data_);
        return from.back();
      }

      template<typename T>
      static bool OP_EQ(const Dynamic & lv, const Dynamic & rv)
      {
        if (!rv.IsList())
          return false;
        const DynamicVector & l = GetVector(lv.data_);
        const DynamicVector & r = GetVector(rv.data_);
        return (l.size() == r.size())
            && std::equal(l.begin(), l.end(), r.begin());
      }

      template<typename T>
      static void OP_ADD(Data & v, const Data & rv)
      {
        Extend(v, rv);
      }

      template<typename T>
      static void OP_SUB(Data & v, const Data & rv)
      {
        uint64_t count = T::OP_GET_UINT(rv);
        size_t truncate_by = (size_t) count;
        DynamicVector & old = GetVector(v);
        size_t old_size(old.size());
        size_t new_size =
            (old_size <= truncate_by) ? 0 : old_size - truncate_by;
        old.resize(new_size, Dynamic());
      }

      template<typename T>
      static bool OP_MUL(Data & v, const Data & rv)
      {
        DynamicVector & old = GetVector(v);
        int32_t repeat_count = (uint32_t) T::OP_GET_INT(rv);
        DynamicVector tmp;
        tmp.reserve(old.size() * repeat_count);
        for (int32_t i = 0; i < repeat_count; ++i)
          std::copy(old.begin(), old.end(), std::back_inserter(tmp));
        old.swap(tmp);
        return true;
      }

      template<typename T>
      static bool OP_DIV(Data & v, const Data & rv)
      {
        uint64_t count = T::OP_GET_UINT(rv);
        size_t divide_by = (size_t) count;
        if (divide_by == 0)
          return false;
        DynamicVector & old = GetVector(v);
        size_t old_size(old.size());
        size_t new_size = (old_size - (old_size % (size_t) divide_by))
            / (size_t) divide_by;
        old.resize(new_size, Dynamic());
        return true;
      }

    private:
      static const DynamicVector & GetVector(const Data & v)
      {
        return GetSharedPtr(v)->GetRef();
      }

      static DynamicVector & GetVector(Data & v)
      {
        return GetSharedPtr(v)->GetRef();
      }

      static const detail::SharedVector * GetSharedPtr(const Data & data)
      {
        return detail::SharedVector::Get(data);
      }

      static detail::SharedVector * GetSharedPtr(Data & data)
      {
        return detail::SharedVector::Get(data);
      }

      static void Reset(Dynamic * v, detail::SharedVector * const list)
      {
        v->type_ = LIST;
        v->data_.shared_vector_ = list;
      }
    };

    //--------------------------------------------------------------------------
    class ConstVectorAdapter
    {
    public:
      typedef DynamicVector::const_iterator iterator;
      typedef DynamicVector::const_reverse_iterator reverse_iterator;
      typedef DynamicVector::const_iterator const_iterator;
      typedef DynamicVector::const_reverse_iterator const_reverse_iterator;

      ConstVectorAdapter(const Dynamic & list)
        : list_(list)
      {}

      bool ok() const { return list_.IsList(); }

      const_iterator begin() const
      {
        if (list_.IsList())
          return detail::Impl<detail::LIST>::Begin(list_);
        else
          return empty_list_.end();
      }

      const_iterator end() const
      {
        if (list_.IsList())
          return detail::Impl<detail::LIST>::End(list_);
        else
          return empty_list_.end();
      }

      const_reverse_iterator rbegin() const
      {
        if (list_.IsList())
          return detail::Impl<detail::LIST>::Rbegin(list_);
        else
          return empty_list_.rend();
      }

      const_reverse_iterator rend() const
      {
        if (list_.IsList())
          return detail::Impl<detail::LIST>::Rend(list_);
        else
          return empty_list_.rend();
      }

    private:
      const Dynamic & list_;
      static const DynamicVector empty_list_;
    };

    //--------------------------------------------------------------------------
    class VectorAdapter
    {
    public:
      typedef DynamicVector::iterator iterator;
      typedef DynamicVector::reverse_iterator reverse_iterator;
      typedef DynamicVector::const_iterator const_iterator;
      typedef DynamicVector::const_reverse_iterator const_reverse_iterator;

      VectorAdapter(Dynamic & list)
        : list_(list)
      {}

      bool ok() const { return list_.IsList(); }

      iterator begin() const
      {
        if (list_.IsList())
          return detail::Impl<detail::LIST>::Begin(list_);
        else
        {
          assert(empty_list_.empty());
          return empty_list_.end();
        }
      }

      iterator end() const
      {
        if (list_.IsList())
          return detail::Impl<detail::LIST>::End(list_);
        else
        {
          assert(empty_list_.empty());
          return empty_list_.end();
        }
      }

      reverse_iterator rbegin() const
      {
        if (list_.IsList())
          return detail::Impl<detail::LIST>::Rbegin(list_);
        else
          return empty_list_.rend();
      }

      reverse_iterator rend() const
      {
        if (list_.IsList())
          return detail::Impl<detail::LIST>::Rend(list_);
        else
          return empty_list_.rend();
      }

    private:
      Dynamic & list_;
      static DynamicVector empty_list_;
    };

    //--------------------------------------------------------------------------
    class DynamicListBuilder
    {
    public:
      DynamicListBuilder() : list_(Dynamic::List()) {}

      DynamicListBuilder & operator << (const Dynamic & v)
      {
        list_.PushBack(v);
        return *this;
      }

      template <typename T>
      DynamicListBuilder & operator << (const T v)
      {
        list_.PushBack(Dynamic(v));
        return *this;
      }

      Dynamic list() const { return list_; }

    private:
      Dynamic list_;
    };
  } // namespace detail
} // namespace nkit

#define DLIST(x) (( nkit::detail::DynamicListBuilder() << x ).list())
#define MAKE_NAME_WITH_LINE_(name, line) name##line
#define MAKE_NAME_WITH_LINE(name, line) MAKE_NAME_WITH_LINE_(name,line)

#define DLIST_FOREACH(item, container) \
  nkit::detail::ConstVectorAdapter \
    MAKE_NAME_WITH_LINE(a, __LINE__)(container);\
  nkit::detail::ConstVectorAdapter::const_iterator\
    MAKE_NAME_WITH_LINE(end, __LINE__) = \
      MAKE_NAME_WITH_LINE(a, __LINE__).end();\
  for(nkit::detail::ConstVectorAdapter::const_iterator item =\
        MAKE_NAME_WITH_LINE(a, __LINE__).begin();\
      item != MAKE_NAME_WITH_LINE(end, __LINE__);\
      ++item)

#define DLIST_FOREACH_MUTABLE(item, container) \
    nkit::detail::VectorAdapter \
      MAKE_NAME_WITH_LINE(a, __LINE__)(container);\
    nkit::detail::VectorAdapter::iterator\
      MAKE_NAME_WITH_LINE(end, __LINE__) = \
        MAKE_NAME_WITH_LINE(a, __LINE__).end();\
    for(nkit::detail::VectorAdapter::iterator item =\
          MAKE_NAME_WITH_LINE(a, __LINE__).begin();\
        item != MAKE_NAME_WITH_LINE(end, __LINE__);\
        ++item)

#define DLIST_REVERSE_FOREACH(item, container) \
  nkit::detail::ConstVectorAdapter \
    MAKE_NAME_WITH_LINE(a, __LINE__)(container);\
  nkit::detail::ConstVectorAdapter::const_reverse_iterator\
    MAKE_NAME_WITH_LINE(rend, __LINE__) = \
      MAKE_NAME_WITH_LINE(a, __LINE__).rend();\
  for(nkit::detail::ConstVectorAdapter::const_reverse_iterator item =\
        MAKE_NAME_WITH_LINE(a, __LINE__).rbegin();\
      item != MAKE_NAME_WITH_LINE(rend, __LINE__);\
      ++item)

#define DLIST_REVERSE_FOREACH_MUTABLE(item, container) \
  nkit::detail::VectorAdapter \
    MAKE_NAME_WITH_LINE(a, __LINE__)(container);\
  nkit::detail::VectorAdapter::reverse_iterator\
    MAKE_NAME_WITH_LINE(rend, __LINE__) = \
      MAKE_NAME_WITH_LINE(a, __LINE__).rend();\
  for(nkit::detail::VectorAdapter::reverse_iterator item =\
        MAKE_NAME_WITH_LINE(a, __LINE__).rbegin();\
      item != MAKE_NAME_WITH_LINE(rend, __LINE__);\
      ++item)
