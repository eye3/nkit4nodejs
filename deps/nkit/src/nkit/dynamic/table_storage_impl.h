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

#ifndef __NKIT__STORAGE__IMPL__H__
#define __NKIT__STORAGE__IMPL__H__

namespace nkit
{
  //--------------------------------------------------------------------------
  class StorageImpl
  {
  public :
    StorageImpl()
        : grow_factor_(0), array_() { }

    explicit StorageImpl(const size_t grow_factor)
        : grow_factor_(grow_factor), array_() { }

    ~StorageImpl()
    {
#if defined(STORAGE_IMPL_PRINT_STATISTICS)
#endif // STORAGE_IMPL_STATISTICS
    }

    StorageImpl * clone() const
    {
      StorageImpl * result = new StorageImpl(grow_factor_);
      if (!result)
        abort_with_core("Low memory");
      result->array_ = array_;
      return result;
    }

    detail::Data * get(const size_t offset)
    {
      assert((offset * grow_factor_) <= array_.size());
      return &array_[0] + (offset * grow_factor_);
    }

    const detail::Data * get(const size_t offset) const
    {
      assert((offset * grow_factor_) <= array_.size());
      return &array_[0] + (offset * grow_factor_);
    }

    detail::Data * extend()
    {
      const size_t prev_size = array_.size();
      for (size_t s = 0; s != grow_factor_; ++s)
      {
        const detail::Data item = { 0 };
        array_.push_back(item);
      }
      return &array_[0] + prev_size;
    }

    void insert(const size_t offset, detail::Data * data)
    {
      assert(array_.size() != 0);
      array_.insert(array_.begin() + offset_to_pos(offset),
        data + 0, data + grow_factor_);
    }

    void remove(const size_t offset)
    {
      assert(array_.size() != 0);
      size_t const start = offset_to_pos(offset);
      size_t const end = start + grow_factor_;
      array_.erase(array_.begin() + start, array_.begin() + end);
    }

    void clear()
    {
       array_.clear();
       grow_factor_ = 0;
    }

    size_t size() const
    {
      return array_.size();
    }

    size_t grow_factor() const
    {
      return grow_factor_;
    }

    void set_grow_factor(const size_t grow_factor)
    {
      grow_factor_ = grow_factor;
    }

  private :
    size_t offset_to_pos(const size_t offset) const
    {
        return (offset * grow_factor_);
    }

    size_t grow_factor_;
    detail::DataVector array_;
  }; // class StorageImpl
} // namespace nkit

#endif
