/**
 * \file concurrent_task_scheduler.hpp
 *
 * \brief This header contains implementation information for a multithreaded
 *        task concurrent_task_scheduler system
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_CONCURRENCY_SCHEDULERS_CONCURRENT_TASK_SCHEDULER_HPP
#define BIT_PLATFORM_CONCURRENCY_SCHEDULERS_CONCURRENT_TASK_SCHEDULER_HPP

#include "task.hpp" // task
#include "task_scheduler.hpp" // task_scheduler

#include <atomic>  // std::atomic
#include <cstdlib> // std::size_t
#include <condition_variable> // std::condition_variable
#include <mutex>   // std::mutex
#include <thread>  // std::thread
#include <vector>  // std::vector
#include <utility> // std::forward

namespace bit {
  namespace platform {

    namespace detail {
      class task_queue;
    } // namespace detail

    std::ptrdiff_t worker_thread_id();

    /// \brief Tag used for assigning affinity to threads in the
    ///        task_concurrent_task_scheduler
    struct assign_affinity_t{};

    /// \brief Constant used for tag dispatching assigning affinity
    constexpr assign_affinity_t assign_affinity = {};

    ///////////////////////////////////////////////////////////////////////////
    /// \brief A concurrent_task_scheduler for managing the task-system.
    ///
    /// This uses a work-stealing queue system for stored tasks.
    ///
    /// \note Only the thread that creates and runs this concurrent_task_scheduler (typically
    ///       from the main message pump) is allowed to stop or destroy this
    ///       concurrent_task_scheduler.
    ///////////////////////////////////////////////////////////////////////////
    class concurrent_task_scheduler : public task_scheduler
    {
      //-----------------------------------------------------------------------
      // Constructors / Destructor / Assignment
      //-----------------------------------------------------------------------
    public:

      /// \brief Default-constructs this task_concurrent_task_scheduler with threads equal
      ///        to the number of logical cores on the system - 1
      concurrent_task_scheduler();

      /// \brief Constructs the task_concurrent_task_scheduler to use \p threads worker
      ///        threads
      ///
      /// \param threads the number of worker threads
      explicit concurrent_task_scheduler( std::size_t threads );

      /// \brief Constructs this task_concurrent_task_scheduler with threads equal
      ///        to the number of logical cores on the system - 1, and
      ///        assigns affinity to each core
      explicit concurrent_task_scheduler( assign_affinity_t );

      /// \brief Constructs \p threads worker threads, each assigned
      ///        affinity to new cores
      ///
      /// \param threads the number of worker threads
      explicit concurrent_task_scheduler( assign_affinity_t, std::size_t threads );

      // Deleted move constructor
      concurrent_task_scheduler( concurrent_task_scheduler&& other ) = delete;

      // Deleted copy constructor
      concurrent_task_scheduler( const concurrent_task_scheduler& other ) = delete;

      //-----------------------------------------------------------------------

      /// \brief Destructs and stops the concurrent_task_scheduler from running
      ~concurrent_task_scheduler();

      //-----------------------------------------------------------------------

      // Deleted move assignment
      concurrent_task_scheduler& operator=( concurrent_task_scheduler&& other ) = delete;

      // Deleted copy assignment
      concurrent_task_scheduler& operator=( const concurrent_task_scheduler& other ) = delete;

      //-----------------------------------------------------------------------
      // Modifiers
      //-----------------------------------------------------------------------
    public:

      /// \brief Runs this task_concurrent_task_scheduler, invoking \p fn each iteration
      ///
      /// \note The calling thread becomes a worker_thread that invokes \p fn
      ///       before attempting to do work with tasks. Care should be taken
      ///       to not exhaust task input and starve the worker threads;
      ///       otherwise the system may never return.
      ///
      /// \param fn The function to invoke each iteration
      template<typename Fn>
      void run( Fn&& fn );

      /// \brief Signals to stop running this task_concurrent_task_scheduler
      ///
      /// The remaining enqueued tasks will be invoked before this concurrent_task_scheduler
      /// comes to a full stop. Any tasks being posted after this will result in
      /// std::terminate being called
      void stop();

      /// \brief Waits for a task \p task to be completed
      ///
      /// The calling thread participates in executing tasks while waiting
      /// for \p task to complete.
      ///
      /// \param task the task to wait for
      void wait( task_handle task ) override;

      //-----------------------------------------------------------------------

      /// \brief Posts a task in this concurrent_task_scheduler
      ///
      /// \param task the task to post
      void post_task( task task ) override;

      //-----------------------------------------------------------------------
      // Private Members
      //-----------------------------------------------------------------------
    private:

      // Threading
      std::vector<std::thread>         m_threads;
      std::vector<detail::task_queue*> m_queues;
#ifndef NDEBUG
      std::thread::id                  m_owner;
#endif

      // Locking
      std::atomic<std::size_t> m_running_threads;
      std::mutex               m_lock;
      std::condition_variable  m_cv;
      bool                     m_running;

      // Initialization
      bool m_set_affinity;

      //-----------------------------------------------------------------------
      // Private Capacity
      //-----------------------------------------------------------------------
    private:

      /// \brief Queries whether there are any remaining tasks in any of the
      ///        current queues
      ///
      /// \return \c true if there are any tasks remaining in any queue
      bool has_remaining_tasks() const noexcept;

      //-----------------------------------------------------------------------
      // Private Modifiers
      //-----------------------------------------------------------------------
    private:

      /// \brief Starts this task_concurrent_task_scheduler for the first time
      void start();

      /// \brief Makes a worker thread with the specified thread \p index
      ///
      /// \return the worker thread
      std::thread make_worker_thread( std::ptrdiff_t index );

      /// \brief Gets a task either through the current thread's queue, or
      ///        from stealing from another active thread
      ///
      /// \return the task
      task get_task();

      /// \brief Pushes a task onto this queue
      ///
      /// \param task the task to push
      void push_task( task task );

      /// \brief Helps in processing tasks while a condition is met
      ///
      /// \param condition the condition to check for
      template<typename Condition>
      void help_while( Condition&& condition );

      /// \brief Helps in processing tasks while the specified task \p j is
      ///        unavailable for processing
      ///
      /// \note This function exists to break infinite template recursion
      ///       caused by calling help_while with a lambda from inside of
      ///       help_while. Each lambda produces a new, unique type causing
      ///       unlimited recursion otherwise.
      ///
      /// \param j the task to check for availability
      void help_while_unavailable( const task& j );

      /// \brief Performs the basic work cycle
      void do_work();
    };

  } // namespace platform
} // namespace bit

#include "detail/concurrent_task_scheduler.inl"

#endif /* BIT_PLATFORM_CONCURRENCY_SCHEDULERS_CONCURRENT_TASK_SCHEDULER_HPP */
