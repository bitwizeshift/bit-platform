/**
 * \file serial_task_scheduler
 *
 * \brief This header contains the implementation of a queue used for
 *        dispatching tasks sequentially
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_CONCURRENCY_SCHEDULERS_SERIAL_TASK_SCHEDULER_HPP
#define BIT_PLATFORM_CONCURRENCY_SCHEDULERS_SERIAL_TASK_SCHEDULER_HPP

#include "task.hpp"           // task
#include "task_scheduler.hpp" // task_scheduler

#include <atomic>  // std::atomic<bool>
#include <condition_variable> // std::condition_variable
#include <memory>  // std::unique_ptr
#include <mutex>   // std::mutex
#include <thread>  // std::thread

namespace bit {
  namespace platform {
    namespace detail {
      class task_queue;
    } // namespace detail

    ///////////////////////////////////////////////////////////////////////////
    /// \brief A task scheduler that operates on tasks in a serial and
    ///        thread-safe manner
    ///
    /// Posting a task is thread-safe, and may be done from any thread.
    ///////////////////////////////////////////////////////////////////////////
    class serial_task_scheduler final : public task_scheduler
    {
      //-----------------------------------------------------------------------
      // Constructors / Destructor / Assignment
      //-----------------------------------------------------------------------
    public:

      /// \brief Default-constructs this serial_task_scheduler with threads equal
      ///        to the number of logical cores on the system - 1
      serial_task_scheduler();

      // Deleted move constructor
      serial_task_scheduler( serial_task_scheduler&& other ) = delete;

      // Deleted copy constructor
      serial_task_scheduler( const serial_task_scheduler& other ) = delete;

      //-----------------------------------------------------------------------

      /// \brief Destructs and stops the dispatch queue from running
      ~serial_task_scheduler();

      //-----------------------------------------------------------------------

      // Deleted move assignment
      serial_task_scheduler& operator=( serial_task_scheduler&& other ) = delete;

      // Deleted copy assignment
      serial_task_scheduler& operator=( const serial_task_scheduler& other ) = delete;

      //-----------------------------------------------------------------------
      // Modifiers
      //-----------------------------------------------------------------------
    public:

      /// \brief Starts this serial_task_scheduler
      void start();

      /// \brief Signals to stop running this serial_task_scheduler
      ///
      /// The remaining enqueued tasks will be invoked before this serial_task_scheduler
      /// comes to a full stop -- but any further calls to post_task will be
      /// silently ignored.
      void stop();

      //-----------------------------------------------------------------------
      // Posting / Waiting (virtual)
      //-----------------------------------------------------------------------
    public:

      /// \brief Posts a task in this dispatch queue
      ///
      /// \param task the task to post
      void post_task( task task ) override;

      /// \brief Waits for a task \p task to be completed
      ///
      /// The calling thread participates in executing tasks while waiting
      /// for \p task to complete.
      ///
      /// \param task the task to wait for
      void wait( task_handle task ) override;

      //-----------------------------------------------------------------------
      // Private Member Types
      //-----------------------------------------------------------------------
    private:

      struct worker_thread
      {
        std::thread                         thread;
        std::unique_ptr<detail::task_queue> task_queue;
      };

      //-----------------------------------------------------------------------
      // Private Members
      //-----------------------------------------------------------------------
    private:

      // Thread Management
      worker_thread   m_worker;
#ifndef NDEBUG
      std::thread::id m_owner;  // The thread that constructed this scheduler
#endif

      // Locking
      std::mutex              m_mutex; // mutex used for waiting on CV
      std::condition_variable m_cv;
      std::atomic<bool>       m_is_running;

      //-----------------------------------------------------------------------
      // Private Modifiers
      //-----------------------------------------------------------------------
    private:

      /// \brief The thread function that operates on the task queue
      void run();

      /// \brief Gets a task from the current queue, if one exists -- or waits
      ///        until one becomes available
      ///
      /// \return the task
      task get_task();
    };
  } // namespace platform
} // namespace bit

#endif /* BIT_PLATFORM_CONCURRENCY_SCHEDULERS_SERIAL_TASK_SCHEDULER_HPP */
