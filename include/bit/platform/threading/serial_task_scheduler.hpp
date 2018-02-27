/**
 * \file serial_task_scheduler
 *
 * \brief This header contains the implementation of a queue used for
 *        dispatching tasks sequentially
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_THREADING_SERIAL_TASK_SCHEDULER_HPP
#define BIT_PLATFORM_THREADING_SERIAL_TASK_SCHEDULER_HPP

#include "task.hpp" // task

#include <bit/stl/utilities/invoke.hpp>

#include <atomic>  // std::atomic<bool>
#include <condition_variable> // std::condition_variable
#include <mutex>   // std::mutex
#include <thread>  // std::thread
#include <utility> // std::forward

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
    class serial_task_scheduler
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

      /// \brief Waits for a task \p task to be completed
      ///
      /// The calling thread participates in executing tasks while waiting
      /// for \p task to complete.
      ///
      /// \param task the task to wait for
      void wait( task_handle task );

      //-----------------------------------------------------------------------

      /// \{
      /// \brief Posts a task in this dispatch queue
      ///
      /// \param parent the parent task
      /// \param fn the function to dispatch
      /// \param args the arguments to forward to the function
      template<typename Fn, typename...Args>
      void post( Fn&& fn, Args&&...args );
      template<typename Fn, typename...Args>
      void post( const task& parent, Fn&& fn, Args&&...args );
      /// \}

      /// \brief Posts a task in this dispatch queue
      ///
      /// \param task the task to post
      void post_task( task task );

      /// \{
      /// \brief Posts a task in this dispatch serial scheduler, waiting for the result
      ///
      /// \param parent the parent task
      /// \param fn the function to dispatch
      /// \param args the arguments to forward to the function
      /// \return the result of the function
      template<typename Fn, typename...Args>
      stl::invoke_result_t<Fn,Args...>
        post_and_wait( Fn&& fn, Args&&...args );
      template<typename Fn, typename...Args>
      stl::invoke_result_t<Fn,Args...>
        post_and_wait( const task& parent, Fn&& fn, Args&&...args );
      /// \}

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

    //-------------------------------------------------------------------------
    // Free Functions
    //-------------------------------------------------------------------------

    /// \brief Posts a task for execution to \p queue
    ///
    /// \param queue the serial_task_scheduler to post the task to
    /// \param task the task to dispatch
    void post_task( serial_task_scheduler& queue, task task );

    /// \brief Waits for the task based on the handle
    ///
    /// \param queue the queue to wait on
    /// \param task the handle to wait for
    void wait( serial_task_scheduler& queue, task_handle task );

    //-------------------------------------------------------------------------

    /// \{
    /// \brief Creates and posts a task to the specified \p queue
    ///
    /// \param queue the serial_task_scheduler to post the task to
    /// \param parent the parent task
    /// \param fn the function to invoke
    /// \param args the arguments to forward to \p fn
    template<typename Fn, typename...Args,
             typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    void post( serial_task_scheduler& queue, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args,
             typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    void post( serial_task_scheduler& queue,
               const task& parent, Fn&& fn, Args&&...args );
    /// \}

    //-------------------------------------------------------------------------

    /// \{
    /// \brief Creates, posts, and waits for the completion of a specified
    ///        task.
    ///
    /// This makes the result appear synchronous, despite the fact that it
    /// may be invoked on a different thread.
    ///
    /// \param queue the serial_task_scheduler to post the task to
    /// \param parent the parent task
    /// \param fn the function to invoke
    /// \param args the arguments to forward to \p fn
    /// \return the result of the invocation
    template<typename Fn, typename...Args,
             typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    stl::invoke_result_t<Fn,Args...>
      post_and_wait( serial_task_scheduler& queue, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args,
             typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    stl::invoke_result_t<Fn,Args...>
      post_and_wait( serial_task_scheduler& queue,
                     const task& parent, Fn&& fn, Args&&...args );
    /// \}
  } // namespace platform
} // namespace bit

#include "detail/serial_task_scheduler.inl"

#endif /* BIT_PLATFORM_THREADING_SERIAL_TASK_SCHEDULER_HPP */
