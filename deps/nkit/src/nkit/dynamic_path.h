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

#ifndef __NKIT__DYNAMIC__PATH__H__
#define __NKIT__DYNAMIC__PATH__H__

#include "nkit/dynamic.h"
#include <algorithm>
#include <iterator>

namespace nkit
{
  //----------------------------------------------------------------------------
  class DynamicPath
  {
  //----------------------------------------------------------------------------
  public: // types
    class PathItem
    {
      friend class DynamicPath;

      enum Kind
      {
        UNDEF = 0,
        INDEX,
        KEY,
        INDEX_PTR,
        KEY_PTR
      };

    public:
      PathItem()
        : kind_(UNDEF)
        , key_("")
        , index_(0)
        , key_ptr_(NULL)
        , index_ptr_(NULL)
      {}

      PathItem(size_t index)
        : kind_(INDEX)
        , key_("")
        , index_(index)
        , key_ptr_(NULL)
        , index_ptr_(NULL)
      {}

      PathItem(const size_t * index_ptr)
        : kind_(INDEX_PTR)
        , key_("")
        , index_(0)
        , key_ptr_(NULL)
        , index_ptr_(index_ptr)
      {}

      PathItem(size_t * index_ptr)
        : kind_(INDEX_PTR)
        , key_("")
        , index_(0)
        , key_ptr_(NULL)
        , index_ptr_(index_ptr)
      {}

      PathItem(const char *key)
        : kind_(KEY)
        , key_(key)
        , index_(0)
        , key_ptr_(NULL)
        , index_ptr_(NULL)
      {}

      PathItem(const std::string &key)
        : kind_(KEY)
        , key_(key)
        , index_(0)
        , key_ptr_(NULL)
        , index_ptr_(NULL)
      {}

      PathItem(const std::string * key_ptr)
        : kind_(KEY_PTR)
        , key_("")
        , index_(0)
        , key_ptr_(key_ptr)
        , index_ptr_(NULL)
      {}

      PathItem(std::string * key_ptr)
        : kind_(KEY_PTR)
        , key_("")
        , index_(0)
        , key_ptr_(key_ptr)
        , index_ptr_(NULL)
      {}

      const Dynamic * Get(const Dynamic & root) const
      {
        const Dynamic * out = NULL;

        switch (kind_)
        {
        case KEY:
          if (!root.Get(key_, &out))
            return NULL;
          break;
        case KEY_PTR:
          if (!root.Get(*key_ptr_, &out))
            return NULL;
          break;
        case INDEX:
          if (!root.GetByIndex(index_, &out))
            return NULL;
          break;
        case INDEX_PTR:
          if (!root.GetByIndex(*index_ptr_, &out))
            return NULL;
          break;
        default:
          break;
        }

        return out;
      }

      bool operator < (const PathItem & right) const
      {
        return kind_ < right.kind_;
      }

      bool operator == (const PathItem & right) const
      {
        return kind_ == right.kind_;
      }

      operator std::string() const
      {
        std::string ret;
        Print(&ret);
        return ret;
      }

    private:
      bool IsIndex() const
      {
        return kind_ == INDEX || kind_ == INDEX_PTR;
      }

      bool IsKey() const
      {
        return kind_ == KEY || kind_ == KEY_PTR;
      }

      void Print(std::string * const out) const
      {
        switch (kind_)
        {
        case KEY:
          *out += key_;
          break;
        case KEY_PTR:
          *out += "&" + *key_ptr_;
          break;
        case INDEX:
          *out += "[" + string_cast(index_) + "]";
          break;
        case INDEX_PTR:
          *out += "[&" + string_cast(*index_ptr_) + "]";
          break;
        default:
          *out += "undefined";
          break;
        }
      }

    private:
      Kind kind_;
      std::string key_;
      size_t index_;
      const std::string * key_ptr_;
      const size_t * index_ptr_;
    }; // class PathItem

  //----------------------------------------------------------------------------
  private: // types
    typedef std::vector<const PathItem *> PathItemPtrs;
    typedef std::vector<PathItem> PathItems;

    //--------------------------------------------------------------------------
    enum State
    {
      DOT = 0,
      DOT_AFTER_BRACKET_CLOSE,
      BRACKET_OPEN,
      BRACKET_CLOSE,
      PARAM
    };

  //----------------------------------------------------------------------------
  public: // methods
    DynamicPath()
      : delimiter_('.')
    {
      MakeKeyChars();
    }

    DynamicPath(char delimiter)
      : delimiter_(delimiter)
    {
      MakeKeyChars();
    }

    DynamicPath(char delimiter, const char * path,
          const PathItem &a1 = PathItem(),
          const PathItem &a2 = PathItem(),
          const PathItem &a3 = PathItem(),
          const PathItem &a4 = PathItem(),
          const PathItem &a5 = PathItem())
      : delimiter_(delimiter)
    {
      MakeKeyChars();
      Set(std::string(path), a1, a2, a3, a4, a5);
    }

    DynamicPath(char delimiter, const std::string & path,
          const PathItem &a1 = PathItem(),
          const PathItem &a2 = PathItem(),
          const PathItem &a3 = PathItem(),
          const PathItem &a4 = PathItem(),
          const PathItem &a5 = PathItem())
      : delimiter_(delimiter)
    {
      MakeKeyChars();
      Set(path, a1, a2, a3, a4, a5);
    }

    void delimiter(char d)
    {
      delimiter_ = d;
      MakeKeyChars();
    }
    char delimiter() const { return delimiter_; }

    bool ok() const { return error().empty(); }

    const std::string & error() const { return error_; }

    const Dynamic * Get(const Dynamic & root) const
    {
      const Dynamic * current = & root;

      PathItems::const_iterator it = path_items_.begin(),
          end = path_items_.end();
      for (; it != end; ++it)
      {
        current = it->Get(*current);
        if (!current)
          return NULL;
      }

      return current;
    }

    operator std::string() const
    {
      std::string ret;
      Print(&ret);
      return ret;
    }

    bool operator < (const DynamicPath & right) const
    {
      // TODO: implement this method w/o comparing sizes
      if (path_items_.size() < right.path_items_.size())
        return true;
      else if (path_items_.size() > right.path_items_.size())
        return false;

      PathItems::const_iterator it1 = path_items_.begin(),
          end1 = path_items_.end(),
          it2 = right.path_items_.begin();
      return std::equal(it1, end1, it2, std::less<PathItem>());
    }

    bool operator == (const DynamicPath & right) const
    {
      if (path_items_.size() != right.path_items_.size())
        return false;
      PathItems::const_iterator it1 = path_items_.begin(),
          end1 = path_items_.end(),
          it2 = right.path_items_.begin();
      return std::equal(it1, end1, it2);
    }

    DynamicPath & operator / (const DynamicPath & path)
    {
      // FIXME need tests
      std::copy(path.path_items_.begin(), path.path_items_.end(),
          std::back_inserter(path_items_));
      return *this;
    }

    DynamicPath & operator / (const std::string & key)
    {
      path_items_.push_back(key);
      return *this;
    }

    DynamicPath & operator / (const std::string * const key)
    {
      path_items_.push_back(key);
      return *this;
    }

    DynamicPath & operator / (std::string * const key)
    {
      path_items_.push_back(key);
      return *this;
    }

    DynamicPath & operator / (const char * const key)
    {
      path_items_.push_back(key);
      return *this;
    }

    DynamicPath & operator / (const size_t index)
    {
      path_items_.push_back(index);
      return *this;
    }

    DynamicPath & operator / (const size_t * const index)
    {
      path_items_.push_back(index);
      return *this;
    }

    DynamicPath & operator / (size_t * const index)
    {
      path_items_.push_back(index);
      return *this;
    }

    std::string Print() const
    {
      std::string res;
      Print(&res);
      return res;
    }

    void Print(std::string * const out) const
    {
      PathItems::const_iterator pitem = path_items_.begin(),
          pend = path_items_.end();
      if (pitem != pend && pitem->IsIndex())
        *out += delimiter_;
      for (; pitem != pend; ++pitem)
      {
        if (!pitem->IsIndex())
          *out += delimiter_;
        pitem->Print(out);
      }
    }

  //----------------------------------------------------------------------------
  private: // methods
    void Set(const std::string &path, const PathItem &a1, const PathItem &a2,
        const PathItem &a3, const PathItem &a4, const PathItem &a5);
    bool AddPathItem(const std::string &path,
        PathItemPtrs::const_iterator &path_item,
        PathItemPtrs::const_iterator &pend,
        PathItem::Kind kind);
    bool InvalidPath(const std::string &path, const char * const c);
    bool FetchName(const char ** const s, const char * const end,
        std::string * const name);
    bool UpdateState(const char ** const c, State * const state);
    bool MakePath(const std::string &path, const PathItemPtrs & params);
    void MakeKeyChars()
    {
      key_chars_ = std::string("[]") + delimiter_;
    }

  //----------------------------------------------------------------------------
  private: // members
    std::string error_;
    PathItems path_items_;
    char delimiter_;
    std::string key_chars_;
  };

  //----------------------------------------------------------------------------
  inline std::string operator + (const std::string & s, const DynamicPath & p)
  {
    return s + static_cast<std::string>(p);
  }

} // namespace nkit


#endif // __NKIT__DYNAMIC__PATH__H__
