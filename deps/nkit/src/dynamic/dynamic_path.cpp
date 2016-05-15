/*
   Copyright 2010-2015 Boris T. Darchiev (boris.darchiev@gmail.com)

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

#include "nkit/dynamic_path.h"
#include "nkit/logger_brief.h"

namespace nkit
{
  //----------------------------------------------------------------------------
  void DynamicPath::Set(const std::string &path, const PathItem &a1,
      const PathItem &a2, const PathItem &a3, const PathItem &a4,
      const PathItem &a5)
  {
    if ( unlikely( (path.size() == 1) && (path[0] == delimiter_) ) )
    {
      path_items_.clear();
      return;
    }

    PathItemPtrs in;
    if (a1.kind_ != PathItem::UNDEF) in.push_back(&a1);
    if (a2.kind_ != PathItem::UNDEF) in.push_back(&a2);
    if (a3.kind_ != PathItem::UNDEF) in.push_back(&a3);
    if (a4.kind_ != PathItem::UNDEF) in.push_back(&a4);
    if (a5.kind_ != PathItem::UNDEF) in.push_back(&a5);
    MakePath(path, in);
  }

  //----------------------------------------------------------------------------
  bool DynamicPath::AddPathItem(const std::string &path,
      PathItemPtrs::const_iterator &path_item,
      PathItemPtrs::const_iterator &pend,
      PathItem::Kind kind)
  {
    if (path_item == pend)
    {
      error_ = "Not enough arguments for path '" + path + "'";
      return false;
    }
    bool ok = ( (kind == PathItem::INDEX) &&
          ((*path_item)->kind_ == PathItem::INDEX
              || (*path_item)->kind_ == PathItem::INDEX_PTR ))
     || ( (kind == PathItem::KEY) &&
          ((*path_item)->kind_ == PathItem::KEY
              || (*path_item)->kind_ == PathItem::KEY_PTR ));
    if (!ok)
    {
      error_ = "Wrong argument type for path '" + path + "'";
      return false;
    }

    path_items_.push_back(**path_item);
    ++path_item;
    return true;
  }

  //--------------------------------------------------------------------------
  bool DynamicPath::InvalidPath(const std::string &path, const char * const c)
  {
    error_ = "Invalid path '" + path + "' at "
        + string_cast(static_cast<uint64_t>(c - path.c_str() + 1));
    return false;
  }

  //--------------------------------------------------------------------------
  bool DynamicPath::FetchName(const char ** const s, const char * const end,
      std::string * const name)
  {
    const char * begin = *s;

    if (begin == end || *begin == ']')
      return false;

    while (*s != end && !strchr(key_chars_.c_str(), **s))
      ++(*s);

    if (begin == *s)
      return false;

    name->assign(begin, *s);
    return true;
  }

  //--------------------------------------------------------------------------
  bool DynamicPath::UpdateState(const char ** const c, State * const state)
  {
    if (**c == delimiter_)
    {
      *state = (*state == BRACKET_CLOSE) ? DOT_AFTER_BRACKET_CLOSE : DOT;
    }
    else
    {
      switch(**c)
      {
      case '[':
        if (*state == DOT_AFTER_BRACKET_CLOSE)
          return false;
        *state = BRACKET_OPEN;
        break;
      case ']':
        if (*state != BRACKET_OPEN && *state != PARAM)
          return false;
        *state = BRACKET_CLOSE;
        break;
      case '%':
        *state = PARAM;
        break;
      default:
        return false;
      }
    }

    ++(*c);

    return true;
  }

  //--------------------------------------------------------------------------
  bool DynamicPath::MakePath(const std::string &path,
          const PathItemPtrs & params)
  {
    if (path.empty())
      return true;

    const char * c = path.c_str();
    const char *end = c + path.length();

    State state(DOT);
    if (*c == delimiter_)
      ++c;
    else if (*c == '[')
    {
      state = BRACKET_OPEN;
      ++c;
    }

    if (c == end)
      return InvalidPath(path, path.c_str());

    std::string name;
    size_t index(0);
    PathItemPtrs::const_iterator param = params.begin(), pend = params.end();
    while (c != end)
    {
      switch (state)
      {
      case DOT_AFTER_BRACKET_CLOSE:
        if (!UpdateState(&c, &state))
          return InvalidPath(path, c);
        if (state == PARAM)
          if (!AddPathItem(path, param, pend, PathItem::KEY))
            return false;
        break;
      case DOT:
        if (!UpdateState(&c, &state))
        {
          if (!FetchName(&c, end, &name))
            return InvalidPath(path, c);
          path_items_.push_back(name);
          if (c == end)
            break;
          if (!UpdateState(&c, &state))
            return InvalidPath(path, c);
        }
        else if (state != PARAM)
          return InvalidPath(path, c);
        else if (!AddPathItem(path, param, pend, PathItem::KEY))
          return false;
        break;
      case BRACKET_OPEN:
        if (!UpdateState(&c, &state))
        {
          for (index = 0; c != end && *c >= '0' && *c <= '9'; ++c)
            index = index * 10 + size_t(*c - '0');
          if (c == end || *c != ']')
            return InvalidPath(path, c);
          path_items_.push_back(index);
          if (!UpdateState(&c, &state))
            return InvalidPath(path, c);
        }
        else if (state != PARAM)
          return InvalidPath(path, c);
        else if (!AddPathItem(path, param, pend, PathItem::INDEX))
          return false;
        break;
      case BRACKET_CLOSE:
        if (!UpdateState(&c, &state))
          return InvalidPath(path, c);
        break;
      case PARAM:
        if (!UpdateState(&c, &state))
          return InvalidPath(path, c);
        break;
      default:
        return InvalidPath(path, c);
      }
    }

    if (c != end && (state == BRACKET_OPEN || state == DOT))
      return InvalidPath(path, c);

    return true;
  }

} // namespace nkit
