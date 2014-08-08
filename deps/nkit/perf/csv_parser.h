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

#ifndef CSV_PARSER_H_
#define CSV_PARSER_H_

#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <nkit/types.h>

namespace nkit
{
  class CSVIterator
  {
  public:
    typedef std::input_iterator_tag iterator_category;
    typedef StringVector value_type;
    typedef std::size_t difference_type;
    typedef StringVector* pointer;
    typedef StringVector& reference;

    CSVIterator(std::istream & str) :
        stream_(str.good() ? &str : NULL)
    {
      ++(*this);
    }

    CSVIterator() :
        stream_(NULL)
    {}

    CSVIterator& operator++()
    {
      if (stream_)
      {
        std::string line;
        std::getline(*stream_, line);

        if (line[line.size()-1] == '\r')
          line.reserve(line.size()-1);

        std::stringstream lineStream(line);
        std::string cell;

        row_.clear();
        while (std::getline(lineStream, cell, ','))
          row_.push_back(cell);

        stream_ = stream_->good() ? stream_ : NULL;
      }
      return *this;
    }

    CSVIterator operator++(int)
    {
      CSVIterator tmp(*this);
      ++(*this);
      return tmp;
    }

    const StringVector & operator*() const
    {
      return row_;
    }

    StringVector & operator*()
    {
      return row_;
    }

    StringVector const* operator->() const
    {
      return &row_;
    }

    bool operator==(const CSVIterator & rhs)
    {
      return ((this == &rhs) ||
        ((this->stream_ == NULL) && (rhs.stream_ == NULL)));
    }

    bool operator!=(const CSVIterator & rhs)
    {
      return !((*this) == rhs);
    }

  private:
    std::istream * stream_;
    StringVector row_;
  };

} // namespace nkit

#endif /* CSV_PARSER_H_ */
