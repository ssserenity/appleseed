
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2010-2013 Francois Beaune, Jupiter Jazz Limited
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

// Interface header.
#include "thread.h"

// appleseed.foundation headers.
#ifdef _WIN32
#include "foundation/platform/windows.h"
#endif
#include "foundation/utility/log.h"

// boost headers.
#include "boost/date_time/posix_time/posix_time_types.hpp"

// Standard headers.
#include <cassert>

using namespace boost;

namespace foundation
{

//
// ProcessPriorityContext class implementation (Windows).
//
// Reference:
//
//   http://msdn.microsoft.com/en-us/library/windows/desktop/ms685100(v=vs.85).aspx
//

#ifdef _WIN32

namespace
{
    DWORD get_priority_class(const ProcessPriority priority)
    {
        switch (priority)
        {
          case ProcessPriorityLowest:   return IDLE_PRIORITY_CLASS;
          case ProcessPriorityLow:      return BELOW_NORMAL_PRIORITY_CLASS;
          case ProcessPriorityNormal:   return NORMAL_PRIORITY_CLASS;
          case ProcessPriorityHigh:     return ABOVE_NORMAL_PRIORITY_CLASS;
          case ProcessPriorityHighest:  return HIGH_PRIORITY_CLASS;
          default:
            assert(!"Should never happen.");
            return NORMAL_PRIORITY_CLASS;
        }
    }

    void set_current_process_priority_class(const DWORD priority_class, Logger* logger)
    {
        if (!SetPriorityClass(GetCurrentProcess(), priority_class))
        {
            if (logger)
            {
                LOG_WARNING(
                    *logger,
                    "failed to set process priority class to %lu (%lu).",
                    priority_class,
                    GetLastError());
            }
        }
    }
}

struct ProcessPriorityContext::Impl
{
    Logger* m_logger;
    DWORD   m_initial_priority_class;
};

ProcessPriorityContext::ProcessPriorityContext(
    const ProcessPriority   priority,
    Logger*                 logger)
  : impl(new Impl())
{
    impl->m_logger = logger;
    impl->m_initial_priority_class = GetPriorityClass(GetCurrentProcess());

    set_current_process_priority_class(
        get_priority_class(priority),
        impl->m_logger);
}

ProcessPriorityContext::~ProcessPriorityContext()
{
    set_current_process_priority_class(
        impl->m_initial_priority_class,
        impl->m_logger);

    delete impl;
}

#else

ProcessPriorityContext::ProcessPriorityContext(
    const ProcessPriority   priority,
    Logger*                 logger)
{
}

ProcessPriorityContext::~ProcessPriorityContext() {}

#endif


//
// ThreadPriorityContext class implementation (Windows).
//
// Reference:
//
//   http://msdn.microsoft.com/en-us/library/windows/desktop/ms685100(v=vs.85).aspx
//

#ifdef _WIN32

namespace
{
    int get_thread_priority_level(const ProcessPriority priority)
    {
        switch (priority)
        {
          case ProcessPriorityLowest:   return THREAD_PRIORITY_LOWEST;
          case ProcessPriorityLow:      return THREAD_PRIORITY_BELOW_NORMAL;
          case ProcessPriorityNormal:   return THREAD_PRIORITY_NORMAL;
          case ProcessPriorityHigh:     return THREAD_PRIORITY_ABOVE_NORMAL;
          case ProcessPriorityHighest:  return THREAD_PRIORITY_HIGHEST;
          default:
            assert(!"Should never happen.");
            return THREAD_PRIORITY_NORMAL;
        }
    }

    void set_current_thread_priority_level(const int thread_priority, Logger* logger)
    {
        if (!SetThreadPriority(GetCurrentThread(), thread_priority))
        {
            if (logger)
            {
                LOG_WARNING(
                    *logger,
                    "failed to set thread priority level to %d (%lu).",
                    thread_priority,
                    GetLastError());
            }
        }
    }
}

struct ThreadPriorityContext::Impl
{
    Logger* m_logger;
    int     m_initial_priority_level;
};

ThreadPriorityContext::ThreadPriorityContext(
    const ProcessPriority   priority,
    Logger*                 logger)
  : impl(new Impl())
{
    impl->m_logger = logger;
    impl->m_initial_priority_level = GetThreadPriority(GetCurrentThread());

    set_current_thread_priority_level(
        get_thread_priority_level(priority),
        impl->m_logger);
}

ThreadPriorityContext::~ThreadPriorityContext()
{
    set_current_thread_priority_level(
        impl->m_initial_priority_level,
        impl->m_logger);

    delete impl;
}

#else

ThreadPriorityContext::ThreadPriorityContext(
    const ProcessPriority   priority,
    Logger*                 logger)
{
}

ThreadPriorityContext::~ThreadPriorityContext() {}

#endif


//
// BenchmarkingThreadContext class implementation (Windows).
//

#ifdef _WIN32

struct BenchmarkingThreadContext::Impl
{
    ProcessPriorityContext  m_process_priority_context;
    ThreadPriorityContext   m_thread_priority_context;
    uint64                  m_thread_affinity_mask;

    explicit Impl(Logger* logger)
      : m_process_priority_context(ProcessPriorityHighest, logger)
      , m_thread_priority_context(ProcessPriorityHighest, logger)
      , m_thread_affinity_mask(SetThreadAffinityMask(GetCurrentThread(), 1))
    {
    }

    ~Impl()
    {
        SetThreadAffinityMask(GetCurrentThread(), m_thread_affinity_mask);
    }
};

BenchmarkingThreadContext::BenchmarkingThreadContext(Logger* logger)
  : impl(new Impl(logger))
{
}

BenchmarkingThreadContext::~BenchmarkingThreadContext()
{
    delete impl;
}

#else

BenchmarkingThreadContext::BenchmarkingThreadContext(Logger* logger) {}
BenchmarkingThreadContext::~BenchmarkingThreadContext() {}

#endif


//
// Utility free functions implementation.
//

void sleep(const uint32 ms)
{
    this_thread::sleep(posix_time::milliseconds(ms));
}

void yield()
{
    this_thread::yield();
}

}   // namespace foundation
