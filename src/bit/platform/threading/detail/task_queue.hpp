/**
 * \file task_queue.hpp
 *
 * \brief TODO: Add description
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef SRC_BIT_PLATFORM_THREADING_DETAIL_TASK_QUEUE_HPP
#define SRC_BIT_PLATFORM_THREADING_DETAIL_TASK_QUEUE_HPP

#include <bit/platform/threading/task.hpp> // task
#include <array>   // std::array
#include <mutex>   // std::mutex
#include <array>   // std::array
#include <cstddef> // std::ptrdiff_t

namespace bit {
  namespace platform {
    namespace detail {

    //=========================================================================
    // task_queue
    //=========================================================================

    ///////////////////////////////////////////////////////////////////////////
    ///
    ///
    ///////////////////////////////////////////////////////////////////////////
    class task_queue
    {
      //-----------------------------------------------------------------------
      // Constructor
      //-----------------------------------------------------------------------
    public:

      task_queue();

      //-----------------------------------------------------------------------
      // Modifiers
      //-----------------------------------------------------------------------
    public:

      /// \brief Pushes a new task into the queue
      ///
      /// \param j the task to push
      void push( task j );

      /// \brief Pops a task from the front of this task_queue
      ///
      /// \return the popped task, or nullptr on failure
      task pop();

      /// \brief Steals a task from the back of this task_queue
      ///
      /// \return the stolen task, or nullptr on failure
      task steal();

      //-----------------------------------------------------------------------
      // Capacity
      //-----------------------------------------------------------------------
    public:

      /// \brief Queries whether this task_queue is empty
      ///
      /// \return \c true when empty
      bool empty() const noexcept;

      //-----------------------------------------------------------------------
      // Private Members
      //-----------------------------------------------------------------------
    private:

      static auto constexpr max_tasks = task::max_tasks;

      std::array<task,max_tasks> m_tasks;
      std::ptrdiff_t             m_bottom;
      std::ptrdiff_t             m_top;
      mutable std::mutex         m_lock;
    };

    } // namespace detail
  } // namespace platform
} // namespace bit

#endif /* SRC_BIT_PLATFORM_THREADING_DETAIL_TASK_QUEUE_HPP */
