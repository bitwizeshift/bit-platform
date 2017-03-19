/**
 * \file spin_lock.hpp
 *
 * \brief This header contains an implementation for a
 *        spin lock
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */

#ifndef BIT_PLATFORM_THREADING_SPIN_LOCK_HPP
#define BIT_PLATFORM_THREADING_SPIN_LOCK_HPP

#include <atomic>

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief An implementation of a busy-waiting spin lock
    ///
    /// This uses atomics, and thus may be heavier for systems with software
    /// atomics like older ARM architectures.
    //////////////////////////////////////////////////////////////////////////
    class spin_lock
    {
      //------------------------------------------------------------------------
      // Public Constructor
      //------------------------------------------------------------------------
    public:

      /// \brief Constructs a basic SpinLock
      spin_lock() noexcept;

      spin_lock( const spin_lock& ) = delete;
      spin_lock( spin_lock&& ) = delete;

      //------------------------------------------------------------------------
      // Locking
      //------------------------------------------------------------------------
    public:

      /// \brief Locks the spin_lock
      void lock() noexcept;

      /// \brief Tries the spin_lock, returning whether the lock is acquired
      ///
      /// \return \c true if the lock is acquired
      bool try_lock() noexcept;

      /// \brief Unlocks the spin_lock
      void unlock() noexcept;

      //------------------------------------------------------------------------
      // Private Members
      //------------------------------------------------------------------------
    private:

      std::atomic_flag m_lock; ///< The atomic flag to use for locking
    };

  } // namespace platform
} // namespace bit

#endif /* BIT_PLATFORM_THREADING_SPIN_LOCK_HPP */
