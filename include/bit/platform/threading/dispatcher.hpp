/**
 * \file task_dispatcher.hpp
 *
 * \brief This header contains implementation information for a multithreaded
 *        task dispatcher system
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_THREADING_DISPATCHER_HPP
#define BIT_PLATFORM_THREADING_DISPATCHER_HPP

#include <bit/platform/threading/task.hpp> // task
#include <bit/stl/utilities/invoke.hpp>

#include <cstdlib> // std::size_t
#include <atomic>  // std::atomic
#include <thread>  // std::thread
#include <mutex>   // std::mutex
#include <condition_variable> // std::condition_variable
#include <vector>  // std::vector
#include <tuple>   // std::tuple

namespace bit {
  namespace platform {
    namespace detail {
      class task_queue;

      template<typename T>
      struct post_task_and_wait_impl;

      template<typename T>
      struct post_task_and_wait_this_impl;

    } // namespace detail

    std::ptrdiff_t worker_thread_id();

    /// \brief Tag used for assigning affinity to threads in the
    ///        task_dispatcher
    struct assign_affinity_t{};

    /// \brief Constant used for tag dispatching assigning affinity
    constexpr assign_affinity_t assign_affinity = {};

    ///////////////////////////////////////////////////////////////////////////
    /// \brief A dispatcher for managing the task-system.
    ///
    /// This uses a work-stealing queue system for stored tasks.
    ///
    /// \note Only the thread that creates and runs this dispatcher (typically
    ///       from the main message pump) is allowed to stop or destroy this
    ///       dispatcher.
    ///////////////////////////////////////////////////////////////////////////
    class dispatcher
    {
      //-----------------------------------------------------------------------
      // Constructors / Destructor / Assignment
      //-----------------------------------------------------------------------
    public:

      /// \brief Default-constructs this task_dispatcher with threads equal
      ///        to the number of logical cores on the system - 1
      dispatcher();

      /// \brief Constructs the task_dispatcher to use \p threads worker
      ///        threads
      ///
      /// \param threads the number of worker threads
      explicit dispatcher( std::size_t threads );

      /// \brief Constructs this task_dispatcher with threads equal
      ///        to the number of logical cores on the system - 1, and
      ///        assigns affinity to each core
      explicit dispatcher( assign_affinity_t );

      /// \brief Constructs \p threads worker threads, each assigned
      ///        affinity to new cores
      ///
      /// \param threads the number of worker threads
      explicit dispatcher( assign_affinity_t, std::size_t threads );

      // Deleted move constructor
      dispatcher( dispatcher&& other ) = delete;

      // Deleted copy constructor
      dispatcher( const dispatcher& other ) = delete;

      //-----------------------------------------------------------------------

      /// \brief Destructs and stops the dispatcher from running
      ~dispatcher();

      //-----------------------------------------------------------------------

      // Deleted move assignment
      dispatcher& operator=( dispatcher&& other ) = delete;

      // Deleted copy assignment
      dispatcher& operator=( const dispatcher& other ) = delete;

      //-----------------------------------------------------------------------
      // Modifiers
      //-----------------------------------------------------------------------
    public:

      /// \brief Runs this task_dispatcher, invoking \p fn each iteration
      ///
      /// \note The calling thread becomes a worker_thread that invkes \p fn
      ///       before attempting to do work with tasks. Care should be taken
      ///       to not exhaust task input and starve the worker threads;
      ///       otherwise the system may never return.
      ///
      /// \param fn The function to invoke each iteration
      template<typename Fn>
      void run( Fn&& fn );

      /// \brief Signals to stop running this task_dispatcher
      ///
      /// The remaining enqueued tasks will be invoked before this dispatcher
      /// comes to a full stop. Any tasks being posted after this will result in
      /// std::terminate being called
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
      /// \brief Posts a task in this dispatcher
      ///
      /// \param parent the parent task
      /// \param fn the function to dispatch
      /// \param args the arguments to forward to the function
      template<typename Fn, typename...Args>
      void post( Fn&& fn, Args&&...args );
      template<typename Fn, typename...Args>
      void post( const task& parent, Fn&& fn, Args&&...args );
      /// \}

      /// \brief Posts a task in this dispatcher
      ///
      /// \param task the task to post
      void post_task( task task );

      /// \{
      /// \brief Posts a task in this dispatcher, waiting for the result
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
      // Private Members
      //-----------------------------------------------------------------------
    private:

      std::vector<std::thread>        m_threads;
      std::vector<detail::task_queue*> m_queues;
      std::thread::id                 m_owner;
      std::mutex                      m_lock;
      std::condition_variable         m_cv;
      std::atomic<std::size_t>        m_running_threads;
      bool                            m_running;
      bool                            m_set_affinity;

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

      /// \brief Starts this task_dispatcher for the first time
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

    //-------------------------------------------------------------------------
    // Free Functions
    //-------------------------------------------------------------------------

    /// \brief Posts a task for execution to \p dispatcher
    ///
    /// \param dispatcher the dispatcher to post the task to
    /// \param task the task to dispatch
    void post_task( dispatcher& dispatcher, task task );

    /// \brief Waits for the task based on the handle
    ///
    /// \param dispatcher the dispatcher to wait on
    /// \param task the handle to wait for
    void wait( dispatcher& dispatcher, task_handle task );

    //-------------------------------------------------------------------------

    /// \{
    /// \brief Creates and posts a task to the specified \p dispatcher
    ///
    /// \param dispatcher the dispatcher to post the task to
    /// \param parent the parent task
    /// \param fn the function to invoke
    /// \param args the arguments to forward to \p fn
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    void post( dispatcher& dispatcher, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    void post( dispatcher& dispatcher,
               const task& parent, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args>
    void post( dispatcher&, std::nullptr_t, Fn&&, Args&&... ) = delete;
    /// \}

    //-------------------------------------------------------------------------

    /// \{
    /// \brief Creates, posts, and waits for the completion of a specified
    ///        task.
    ///
    /// This makes the result appear synchronous, despite the fact that it
    /// may be invoked on a different thread.
    ///
    /// \param dispatcher the dispatcher to post the task to
    /// \param parent the parent task
    /// \param fn the function to invoke
    /// \param args the arguments to forward to \p fn
    /// \return the result of the invocation
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    stl::invoke_result_t<Fn,Args...>
      post_and_wait( dispatcher& dispatcher, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    stl::invoke_result_t<Fn,Args...>
      post_and_wait( dispatcher& dispatcher,
                     const task& parent, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args>
    stl::invoke_result_t<Fn,Args...>
      post_and_wait( dispatcher&,
                     std::nullptr_t, Fn&&, Args&&... ) = delete;
    /// \}

    //-------------------------------------------------------------------------

    /// \brief This namespace is used to dispatch functions to the currently
    ///        active dispatcher without requiring passing the dispatcher as
    ///        the active argument
    namespace this_dispatcher {

      /// \brief Posts a task for execution to the active dispatcher
      ///
      /// \param task the task to dispatch
      void post_task( task task );

      /// \brief Waits for the task based on the handle
      ///
      /// \param task the handle to wait for
      void wait( task_handle task );

      //-----------------------------------------------------------------------

      /// \{
      /// \brief Creates and posts a task to the specified \p dispatcher
      ///
      /// \param dispatcher the dispatcher to post the task to
      /// \param parent the parent task
      /// \param fn the function to invoke
      /// \param args the arguments to forward to \p fn
      template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
      void post( Fn&& fn, Args&&...args );
      template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
      void post( const task& parent, Fn&& fn, Args&&...args );
      template<typename Fn, typename...Args>
      void post( std::nullptr_t, Fn&&, Args&&... ) = delete;
      /// \}

      //-----------------------------------------------------------------------

      /// \{
      /// \brief Creates, posts, and waits for the completion of a specified
      ///        task.
      ///
      /// This makes the result appear synchronous, despite the fact that it
      /// may be invoked on a different thread.
      ///
      /// \param dispatcher the dispatcher to post the task to
      /// \param parent the parent task
      /// \param fn the function to invoke
      /// \param args the arguments to forward to \p fn
      /// \return the result of the invocation
      template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
      stl::invoke_result_t<Fn,Args...>
        post_and_wait( Fn&& fn, Args&&...args );
      template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
      stl::invoke_result_t<Fn,Args...>
        post_and_wait( const task& parent, Fn&& fn, Args&&...args );
      template<typename Fn, typename...Args>
      stl::invoke_result_t<Fn,Args...>
        post_and_wait( std::nullptr_t, Fn&&, Args&&... ) = delete;
      /// \}

    } // namespace this_dispatcher
  } // namespace thread
} // namespace bit

#include "detail/dispatcher.inl"

#endif /* BIT_PLATFORM_THREADING_DISPATCHER_HPP */
