/**
 * \file this_task_scheduler.hpp
 *
 * \brief TODO: Add description
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_THREADING_THIS_TASK_SCHEDULER_HPP
#define BIT_PLATFORM_THREADING_THIS_TASK_SCHEDULER_HPP

#include "task.hpp"
#include "task_scheduler.hpp"

#include <bit/stl/utilities/invoke.hpp> // bit::stl::invoke_result_t

#include <utility> // std::move

namespace bit {
  namespace platform {

    ///////////////////////////////////////////////////////////////////////////
    /// \brief This current task scheduler
    ///
    /// All functionality in here affects the currently active task scheduler
    ///
    ///////////////////////////////////////////////////////////////////////////
    class this_task_scheduler {

      ~this_task_scheduler() = delete;

      //-----------------------------------------------------------------------
      // Posting / Waiting
      //-----------------------------------------------------------------------
    public:

      /// \brief Posts a task in this dispatch queue
      ///
      /// \param task the task to post
      static void post_task( task task );

      /// \brief Waits for a given task in this dispatch queue
      ///
      /// \param task the task handle to wait on
      static void wait( task_handle task );

      //-----------------------------------------------------------------------

      /// \{
      /// \brief Posts a given function \p fn with the specified arguments
      ///        \p args as a task in this scheduler
      ///
      /// \param parent the task to designate as parent
      /// \param fn the function to post
      /// \param args the arguments to forward
      template<typename Fn, typename...Args>
      static void post( Fn&& fn, Args&&...args );
      template<typename Fn, typename...Args>
      static void post( const task& parent, Fn&& fn, Args&&...args );
      /// \}

      //-----------------------------------------------------------------------

      /// \{
      /// \brief Posts a task in this dispatch serial scheduler, waiting for the result
      ///
      /// \param parent the parent task
      /// \param fn the function to dispatch
      /// \param args the arguments to forward to the function
      /// \return the result of the function
      template<typename Fn, typename...Args>
      static stl::invoke_result_t<Fn,Args...>
        post_and_wait( Fn&& fn, Args&&...args );
      template<typename Fn, typename...Args>
      static stl::invoke_result_t<Fn,Args...>
        post_and_wait( const task& parent, Fn&& fn, Args&&...args );
      /// \}

    }; // class this_task_scheduler
  } // namespace platform
} // namespace bit

//=============================================================================
// Inline Definitions : this_task_scheduler
//=============================================================================

//-----------------------------------------------------------------------------
// Posting / Waiting
//-----------------------------------------------------------------------------

inline void bit::platform::this_task_scheduler::post_task( task task )
{
  auto scheduler = task_scheduler::g_active_scheduler;

  assert( scheduler != nullptr );

  scheduler->post_task( std::move(task) );
}

inline void bit::platform::this_task_scheduler::wait( task_handle task )
{
  auto scheduler = task_scheduler::g_active_scheduler;

  assert( scheduler != nullptr );

  scheduler->wait( task );
}

//-----------------------------------------------------------------------------

template<typename Fn, typename...Args>
inline void bit::platform::this_task_scheduler::post( Fn&& fn, Args&&...args )
{
  auto scheduler = task_scheduler::g_active_scheduler;

  assert( scheduler != nullptr );

  scheduler->post( std::forward<Fn>(fn), std::forward<Args>(args)... );
}

template<typename Fn, typename...Args>
inline void bit::platform::this_task_scheduler
  ::post( const task& parent, Fn&& fn, Args&&...args )
{
  auto scheduler = task_scheduler::g_active_scheduler;

  assert( scheduler != nullptr );

  scheduler->post( parent, std::forward<Fn>(fn), std::forward<Args>(args)... );
}

//-----------------------------------------------------------------------------


template<typename Fn, typename...Args>
inline bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::this_task_scheduler::post_and_wait( Fn&& fn, Args&&...args )
{
  auto scheduler = task_scheduler::g_active_scheduler;

  assert( scheduler != nullptr );

  scheduler->post_and_wait( std::forward<Fn>(fn),
                            std::forward<Args>(args)... );
}

template<typename Fn, typename...Args>
inline bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::this_task_scheduler
  ::post_and_wait( const task& parent, Fn&& fn, Args&&...args )
{
  auto scheduler = task_scheduler::g_active_scheduler;

  assert( scheduler != nullptr );

  scheduler->post_and_wait( parent,
                            std::forward<Fn>(fn),
                            std::forward<Args>(args)... );
}

#endif /* BIT_PLATFORM_THREADING_THIS_TASK_SCHEDULER_HPP */
