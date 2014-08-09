#ifndef NKIT_VX_STR2ID_H
#define NKIT_VX_STR2ID_H

#include <string.h>

#include <cstdlib>
#include <string>
#include <map>

namespace nkit
{
  //---------------------------------------------------------------------------
  class String2IdMap
  {
  private:
    struct CompareStrings
    {
      bool operator ()(const char * s1, const char * s2) const
      {
        return strcmp(s1, s2) < 0;
      }
    };

    typedef std::map<char *, size_t, CompareStrings> Name2Id;

  public:
    String2IdMap() : max_element_id_(1) {}

    String2IdMap(const String2IdMap & from)
    {
      Set(from);
    }

    ~String2IdMap()
    {
      Clear();
    }

    String2IdMap & operator =(const String2IdMap & from)
    {
      if (this != &from)
        Set(from);

      return *this;
    }

    size_t GetId(const char * str)
    {
      Name2Id::const_iterator it = name2id_.find(const_cast<char *>(str)), end =
          name2id_.end();
      if (it != end)
        return it->second;
      size_t element_id = max_element_id_++;
      name2id_.insert(std::make_pair(strdup(str), element_id));
      return element_id;
    }

    std::string GetString(size_t id) const
    {
      Name2Id::const_iterator it = name2id_.begin(), end = name2id_.end();
      for (; it != end; ++it)
        if (it->second == id)
          return std::string(it->first);
      return std::string("");
    }

  private:
    void Clear()
    {
      max_element_id_ = 1;

      Name2Id::const_iterator it = name2id_.begin(), end = name2id_.end();
      for (; it != end; ++it)
        free(it->first);
    }

    void Set(const String2IdMap & from)
    {
      Clear();
      max_element_id_ = from.max_element_id_;

      Name2Id::const_iterator it = from.name2id_.begin(), end =
          from.name2id_.end();
      for (; it != end; ++it)
        name2id_.insert(std::make_pair(strdup(it->first), it->second));
    }

  private:
    Name2Id name2id_;
    size_t max_element_id_;
  };
} // namespace nkit

#endif // NKIT_VX_STR2ID_H
