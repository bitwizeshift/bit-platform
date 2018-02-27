/**
 * \file waitable_event.hpp
 *
 * \brief This header contains a synchronization primitive for waiting
 *        on an event
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_CONCURRENCY_UTILITIES_WAITABLE_EVENT_HPP
#define BIT_PLATFORM_CONCURRENCY_UTILITIES_WAITABLE_EVENT_HPP

#include <chrono> // std::chrono::duration, std::chrono::time_point
#include <mutex>  // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief A simple waitable event that helps manage waiting for a valid
    ///        signal condition
    //////////////////////////////////////////////////////////////////////////
    class waitable_event
    {
      //----------------------------------------------------------------------
      // Constructors / Assignment
      //----------------------------------------------------------------------
    public:

      /// \brief Default constructs a waitable_event that is not yet signaled
      waitable_event();

      // Deleted move constructor
      waitable_event( waitable_event&& other ) = delete;

      // Deleted copy constructor
      waitable_event( const waitable_event& other ) = delete;

      //----------------------------------------------------------------------

      // Deleted move assignment
      waitable_event& operator=( waitable_event&& other ) = delete;

      // Deleted copy assignment
      waitable_event& operator=( const waitable_event& other ) = delete;

      //----------------------------------------------------------------------
      // Waiting
      //----------------------------------------------------------------------
    public:

      /// \brief Blocks the current thread until it is signaled
      void wait();

      /// \brief Blocks the current thread until it is signaled, or until the
      ///        specified duration has been waited for
      ///
      /// \param duration the amount of time to wait for
      /// \return \c true if the event was woken up because it was signaled
      template<typename Rep, typename Period>
      bool wait_for( const std::chrono::duration<Rep,Period>& duration );

      /// \brief Blocks the current thread until it is signaled, or until the
      ///        specified \p time_out has been reached
      ///
      /// \param time_out the time to wait until
      /// \return \c true if the event was woken up because it was signaled
      template<typename Clock, typename Duration>
      bool wait_until( const std::chrono::time_point<Clock,Duration>& time_point );

      //----------------------------------------------------------------------
      // Signaling
      //----------------------------------------------------------------------
    public:

      /// \brief Signals for this waitable_event to stop waiting
      void signal();

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    private:

      std::mutex              m_mutex;
      std::condition_variable m_cv;
      bool                    m_signal;
    };

  } // namespace platform
} // namespace bit

#include "detail/waitable_event.inl"

#endif /* BIT_PLATFORM_CONCURRENCY_UTILITIES_WAITABLE_EVENT_HPP */
