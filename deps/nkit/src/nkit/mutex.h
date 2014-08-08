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

#ifndef __NKIT__THREADING__MUTEX__IMPL__H__
#define __NKIT__THREADING__MUTEX__IMPL__H__

#include <nkit/detail/push_options.h>

#if defined(NKIT_PTHREAD)
#  include <nkit/threading/pthread_mutex.h>
#elif defined(NKIT_WINNT)
#  include <nkit/threading/winnt_mutex.h>
#endif


namespace nkit
{
  class Mutex
  {
    Mutex(const Mutex &);
    Mutex & operator=(const Mutex &);
  public:
    Mutex() : impl_() {}
    ~Mutex() {}

    inline void Lock() { impl_.Lock(); }
    inline void Unlock() { impl_.Unlock(); }

  private:
    MutexImpl impl_;
  }; // class Mutex

  template <typename T>
  class LockGuard
  {
  public:
    explicit LockGuard(T & m)
      : mutex(m)
    {
      mutex.Lock();
    }

    ~LockGuard()
    {
      mutex.Unlock();
    }

  private:
    T & mutex;
  }; // struct ScopedLock

} // namespace nkit

#endif // __NKIT__THREADING__MUTEX__IMPL__H__
