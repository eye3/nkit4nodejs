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

#ifndef __NKIT__DETAIL__WINNT__MUTEX__IMPL__H__
#define __NKIT__DETAIL__WINNT__MUTEX__IMPL__H__

#include <windows.h>
#include <nkit/tools.h>

namespace nkit
{
  class MutexImpl
  {
    MutexImpl(const MutexImpl &);
    MutexImpl & operator=(const MutexImpl &);
  public:
    MutexImpl() : m_(INVALID_HANDLE_VALUE)
    {
      if ((m_ = CreateMutex(NULL, FALSE, NULL)) == NULL)
        ::nkit::abort_with_core("CreateMutex "
            + ::nkit::string_cast((int)GetLastError()));
    }

    ~MutexImpl()
    {
      if (CloseHandle(m_) == FALSE)
        ::nkit::abort_with_core("CloseHandle "
            + ::nkit::string_cast((int)GetLastError()));
    }

    void Lock()
    {
      if (WaitForSingleObject(m_, INFINITE) == WAIT_ABANDONED)
        ::nkit::abort_with_core("WaitForSingleObject "
            + ::nkit::string_cast((int)GetLastError()));
    }

    void Unlock()
    {
      if (ReleaseMutex(m_) == FALSE)
        ::nkit::abort_with_core("ReleaseMutex "
            + ::nkit::string_cast((int)GetLastError()));
    }
  private:
    HANDLE m_;
  };
} // namespace nkit

#endif
