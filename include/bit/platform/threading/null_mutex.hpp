/**
 * \file null_mutex.hpp
 *
 * \brief This header contains an implementation of a null Lockable
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_THREADING_NULL_MUTEX_HPP
#define BIT_PLATFORM_THREADING_NULL_MUTEX_HPP

#include <chrono>

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief A mutex that follows the null-object-pattern.
    ///
    /// This is used for single-threading in a generic API.
    //////////////////////////////////////////////////////////////////////////
    class null_mutex
    {
      template<typename Rep, typename Period>
      using duration = std::chrono::duration<Rep,Period>;

      template<typename Clock, typename Duration>
      using time_point = std::chrono::time_point<Clock,Duration>;

      //----------------------------------------------------------------------
      // Constructors
      //----------------------------------------------------------------------
    public:

      /// \brief Default-constructs a null_mutex
      constexpr null_mutex() = default;

      null_mutex( const null_mutex& ) = delete;
      null_mutex( null_mutex&& ) = delete;

      //----------------------------------------------------------------------
      // Exclusive Locking
      //----------------------------------------------------------------------
    public:

      /// \brief 'Locks' this null_mutex
      constexpr void lock() noexcept;

      /// \brief Attempts to 'lock' this null_mutex
      ///
      /// \return \c true
      constexpr bool try_lock() noexcept;

      /// \brief Attempts to 'lock' this null_mutex for \p duration
      ///
      /// \param duration the timeout duration
      /// \return \c true
      template<typename Rep, typename Period>
      constexpr bool try_lock_for( const duration<Rep,Period>& duration ) noexcept;

      /// \brief Attempts to 'lock' this null_mutex until \p time
      ///
      /// \param time the time point to stop trying
      /// \return \c true
      template<typename Clock, typename Duration>
      constexpr bool try_lock_until( const time_point<Clock,Duration>& time ) noexcept;

      /// \brief 'Unlocks' this null_mutex
      constexpr void unlock() noexcept;

      //----------------------------------------------------------------------
      // Shared Locking
      //----------------------------------------------------------------------
    public:

      /// \copydoc null_mutex::lock()
      constexpr void lock_shared() noexcept;

      /// \copydoc null_mutex::try_lock_shared()
      constexpr bool try_lock_shared() noexcept;

      /// \copydoc null_mutex::try_lock_shared_for( const duration<Rep,Period>& )
      template<typename Rep, typename Period>
      constexpr bool try_lock_shared_for( const duration<Rep,Period>& duration ) noexcept;

      /// \copydoc null_mutex::try_lock_shared_until( const time_point<Clock,Duration>& )
      template<typename Clock, typename Duration>
      constexpr bool try_lock_shared_until( const time_point<Clock,Duration>& time ) noexcept;

      /// \copydoc null_mutex::unlock_shared()
      constexpr void unlock_shared() noexcept;
    };

  } // namespace platform
} // namespace bit

#include "detail/null_mutex.inl"

#endif /* BIT_PLATFORM_THREADING_NULL_MUTEX_HPP */
