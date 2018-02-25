/**
 * \file dispatch_queue.hpp
 *
 * \brief This header contains the implementation of a queue used for
 *        dispatching tasks sequentially
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_THREADING_DISPATCH_QUEUE_HPP
#define BIT_PLATFORM_THREADING_DISPATCH_QUEUE_HPP

#include <bit/platform/threading/task.hpp> // task
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
    /// \brief A dispatch queue that operates on tasks in a sequential,
    ///        thread-safe manner
    ///
    /// Posting an operation is thread-safe, and may be done from any thread.
    ///////////////////////////////////////////////////////////////////////////
    class dispatch_queue
    {
      //-----------------------------------------------------------------------
      // Constructors / Destructor / Assignment
      //-----------------------------------------------------------------------
    public:

      /// \brief Default-constructs this dispatch_queue with threads equal
      ///        to the number of logical cores on the system - 1
      dispatch_queue();

      // Deleted move constructor
      dispatch_queue( dispatch_queue&& other ) = delete;

      // Deleted copy constructor
      dispatch_queue( const dispatch_queue& other ) = delete;

      //-----------------------------------------------------------------------

      /// \brief Destructs and stops the dispatch queue from running
      ~dispatch_queue();

      //-----------------------------------------------------------------------

      // Deleted move assignment
      dispatch_queue& operator=( dispatch_queue&& other ) = delete;

      // Deleted copy assignment
      dispatch_queue& operator=( const dispatch_queue& other ) = delete;

      //-----------------------------------------------------------------------
      // Modifiers
      //-----------------------------------------------------------------------
    public:

      /// \brief Starts this dispatch_queue
      void start();

      /// \brief Signals to stop running this dispatch_queue
      ///
      /// The remaining enqueued tasks will be invoked before this dispatch_queue
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
      /// \brief Posts a task in this dispatch queue, waiting for the result
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

      std::unique_ptr<detail::task_queue> m_queue;
      std::thread             m_thread;
      std::mutex              m_mutex;
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
    /// \param queue the dispatch_queue to post the task to
    /// \param task the task to dispatch
    void post_task( dispatch_queue& queue, task task );

    /// \brief Waits for the task based on the handle
    ///
    /// \param queue the queue to wait on
    /// \param task the handle to wait for
    void wait( dispatch_queue& queue, task_handle task );

    //-------------------------------------------------------------------------

    /// \{
    /// \brief Creates and posts a task to the specified \p queue
    ///
    /// \param queue the dispatch_queue to post the task to
    /// \param parent the parent task
    /// \param fn the function to invoke
    /// \param args the arguments to forward to \p fn
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    void post( dispatch_queue& queue, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    void post( dispatch_queue& queue,
               const task& parent, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args>
    void post( dispatch_queue&, std::nullptr_t, Fn&&, Args&&... ) = delete;
    /// \}

    //-------------------------------------------------------------------------

    /// \{
    /// \brief Creates, posts, and waits for the completion of a specified
    ///        task.
    ///
    /// This makes the result appear synchronous, despite the fact that it
    /// may be invoked on a different thread.
    ///
    /// \param queue the dispatch_queue to post the task to
    /// \param parent the parent task
    /// \param fn the function to invoke
    /// \param args the arguments to forward to \p fn
    /// \return the result of the invocation
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    stl::invoke_result_t<Fn,Args...>
      post_and_wait( dispatch_queue& queue, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    stl::invoke_result_t<Fn,Args...>
      post_and_wait( dispatch_queue& queue,
                     const task& parent, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args>
    stl::invoke_result_t<Fn,Args...>
      post_and_wait( dispatch_queue&,
                     std::nullptr_t, Fn&&, Args&&... ) = delete;
    /// \}

    //-------------------------------------------------------------------------

    /// \brief This namespace is used to dispatch functions to the currently
    ///        active dispatch_queue without requiring passing the queue as
    ///        the active argument
    namespace this_dispatch_queue {

      /// \brief Posts a task for execution to the active dispatch_queue
      ///
      /// \param task the task to dispatch
      void post_task( task task );

      /// \brief Waits for the task based on the handle
      ///
      /// \param task the handle to wait for
      void wait( task_handle task );

      //-----------------------------------------------------------------------

      /// \{
      /// \brief Creates and posts a task to the specified \p queue
      ///
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


    } // namespace this_dispatch_queue
  } // namespace platform
} // namespace bit

#include "detail/dispatch_queue.inl"

#endif /* BIT_PLATFORM_THREADING_DISPATCH_QUEUE_HPP */
