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

#ifndef __NKIT__DETAIL__PTHREAD__MUTEX__IMPL__H__
#define __NKIT__DETAIL__PTHREAD__MUTEX__IMPL__H__

#include <nkit/tools.h>

#include <errno.h>
#include <string.h>
#include <pthread.h>

namespace nkit
{
  class MutexImpl
  {
    MutexImpl(const MutexImpl &);
    MutexImpl & operator =(const MutexImpl &);
  public:
    MutexImpl() : m_()
    {
      if (unlikely(pthread_mutex_init(&m_, NULL) == -1))
        ::nkit::abort_with_core("pthread_mutex_init "
            + std::string(strerror(errno)));
    }
    ~MutexImpl()
    {
      if (unlikely(pthread_mutex_destroy(&m_) == -1))
        ::nkit::abort_with_core("pthread_mutex_destroy "
            + std::string(strerror(errno)));
    }

    inline void Lock()
    {
      if (unlikely(pthread_mutex_lock(&m_) == -1))
        ::nkit::abort_with_core("pthread_mutex_lock "
            + std::string(strerror(errno)));
    }

    inline void Unlock()
    {
      if (unlikely(pthread_mutex_unlock(&m_) == -1))
        ::nkit::abort_with_core("pthread_mutex_unlock "
            + std::string(strerror(errno)));
    }
  private:
    pthread_mutex_t m_;
  }; // class MutexImpl

} // namespace nkit

#endif

