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

#ifndef __NKIT__REF__COUNT__PTR__H__
#define __NKIT__REF__COUNT__PTR__H__

#include <nkit/types.h>

namespace nkit
{
  namespace detail
  {
    template<typename T>
    class ref_count_ptr
    {
      template<typename D> friend class ref_count_ptr;

    public:
      explicit ref_count_ptr() : obj_(NULL), counter_(NULL){}

      explicit ref_count_ptr(T * ptr) : obj_(ptr), counter_(new uint64_t(1)) {}

      ref_count_ptr(const ref_count_ptr<T> & from)
        : obj_(from.obj_)
        , counter_(from.counter_)
      {
        increment();
      }

      // D is a class, derived from T
      template<typename D>
      ref_count_ptr(const ref_count_ptr<D> & from)
        : obj_(static_cast<T*>(from.obj_))
        , counter_(from.counter_)
      {
        increment();
      }

      ~ref_count_ptr()
      {
        reset();
      }

      void reset()
      {
        if (counter_ != NULL)
        {
          if (--(*counter_) == 0)
          {
            delete obj_;
            delete counter_;
          }
          obj_ = NULL;
          counter_ = NULL;
        }
      }

      ref_count_ptr<T> & operator =(ref_count_ptr const & from)
      {
        if (&from != this)
        {
          reset();
          obj_ = from.obj_;
          counter_ = from.counter_;
          increment();
        }
        return *this;
      }

      operator bool() const
      {
        return obj_ != NULL;
      }

      T * operator ->() const
      {
        return obj_;
      }

      T * get() const
      {
        return obj_;
      }

      T & operator *() const
      {
        return *obj_;
      }

      bool operator ==(ref_count_ptr const & rv) const
      {
        return obj_ == rv.obj_;
      }

      bool operator <(ref_count_ptr const & rv) const
      {
        return obj_ < rv.obj_;
      }

    private:
      void increment()
      {
        if (counter_ != NULL)
          ++(*counter_);
      }

    private:
      T * obj_;
      uint64_t * counter_;
    };  // class ref_count_ptr
  }// namespace detail
} // namespace nkit

#endif // __NKIT__REF__COUNT__PTR__H__
