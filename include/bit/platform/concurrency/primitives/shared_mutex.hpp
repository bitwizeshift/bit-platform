/**
 * \file shared_mutex.hpp
 *
 * \brief This header adds the missing shared_mutex implementation
 *        that was ommited from the c++14 standard. If c++17 mode
 *        is enabled, it uses std::shared_mutex; otherwise it
 *        limits std::shared_timed_mutex to a strictly lockable API
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */

#ifndef BIT_PLATFORM_CONCURRENCY_PRIMITIVES_SHARED_MUTEX_HPP
#define BIT_PLATFORM_CONCURRENCY_PRIMITIVES_SHARED_MUTEX_HPP

#if __cplusplus < 201402L
# error This file requires c++14 standard conformance or higher
#endif

#include <shared_mutex>

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief A mutex that supports both unique and shared locking
    ///
    /// This was an omission from the c++14 standard that has been rectified
    /// in c++17. As such, with c++17 mode enabled, it defaults to
    /// std::shared_mutex.
    //////////////////////////////////////////////////////////////////////////
#if __cplusplus >= 201703L
    using shared_mutex = std::shared_mutex;
#else
    class shared_mutex : std::shared_timed_mutex
    {
    public:

      using std::shared_timed_mutex::shared_timed_mutex;

      using std::shared_timed_mutex::lock;
      using std::shared_timed_mutex::try_lock;
      using std::shared_timed_mutex::unlock;

      using std::shared_timed_mutex::lock_shared;
      using std::shared_timed_mutex::try_lock_shared;
      using std::shared_timed_mutex::unlock_shared;
    };
#endif

  } // namespace platform
} // namespace bit

#endif /* BIT_PLATFORM_CONCURRENCY_PRIMITIVES_SHARED_MUTEX_HPP */
