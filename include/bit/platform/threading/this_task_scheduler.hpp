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

      //-----------------------------------------------------------------------

      /// \{
      /// \brief Posts a given function \p fn with the specified arguments
      ///        \p args as a task in this scheduler
      ///
      /// \param parent the task to designate as parent
      /// \param fn the function to post
      /// \param args the arguments to forward
      template<typename Fn, typename...Args,
               typename=std::enable_if_t<stl::is_invocable<Fn,Args...>::value>>
      static void post( Fn&& fn, Args&&...args );
      template<typename Fn, typename...Args,
               typename=std::enable_if_t<stl::is_invocable<Fn,Args...>::value>>
      static void post( const task& parent, Fn&& fn, Args&&...args );
      /// \}

    }; // class this_task_scheduler
  } // namespace platform
} // namespace bit

#include "detail/this_task_scheduler.inl"

#endif /* BIT_PLATFORM_THREADING_THIS_TASK_SCHEDULER_HPP */
