/**
 * \file dispatch_queue.hpp
 *
 * \brief This header contains the implementation of a queue used for
 *        dispatching jobs sequentially
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_THREADING_DISPATCH_QUEUE_HPP
#define BIT_PLATFORM_THREADING_DISPATCH_QUEUE_HPP

#include "job.hpp" // job

#include <bit/stl/utilities/invoke.hpp>

#include <atomic>  // std::atomic<bool>
#include <condition_variable> // std::condition_variable
#include <mutex>   // std::mutex
#include <thread>  // std::thread
#include <utility> // std::forward

namespace bit {
  namespace platform {
    namespace detail {
      class job_queue;
    } // namespace detail

    ///////////////////////////////////////////////////////////////////////////
    /// \brief A dispatch queue that operates on jobs in a sequential,
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
      /// The remaining enqueued jobs will be invoked before this dispatch_queue
      /// comes to a full stop -- but any further calls to post_job will be
      /// silently ignored.
      void stop();

      /// \brief Waits for a job \p job to be completed
      ///
      /// The calling thread participates in executing jobs while waiting
      /// for \p job to complete.
      ///
      /// \param job the job to wait for
      void wait( job_handle job );

      //-----------------------------------------------------------------------

      /// \{
      /// \brief Posts a job in this dispatch queue
      ///
      /// \param parent the parent job
      /// \param fn the function to dispatch
      /// \param args the arguments to forward to the function
      template<typename Fn, typename...Args>
      void post( Fn&& fn, Args&&...args );
      template<typename Fn, typename...Args>
      void post( const job& parent, Fn&& fn, Args&&...args );
      /// \}

      /// \brief Posts a job in this dispatch queue
      ///
      /// \param job the job to post
      void post_job( job job );

      /// \{
      /// \brief Posts a job in this dispatch queue, waiting for the result
      ///
      /// \param parent the parent job
      /// \param fn the function to dispatch
      /// \param args the arguments to forward to the function
      /// \return the result of the function
      template<typename Fn, typename...Args>
      stl::invoke_result_t<Fn,Args...>
        post_and_wait( Fn&& fn, Args&&...args );
      template<typename Fn, typename...Args>
      stl::invoke_result_t<Fn,Args...>
        post_and_wait( const job& parent, Fn&& fn, Args&&...args );
      /// \}

      //-----------------------------------------------------------------------
      // Private Members
      //-----------------------------------------------------------------------
    private:

      std::unique_ptr<detail::job_queue> m_queue;
      std::thread             m_thread;
      std::mutex              m_mutex;
      std::condition_variable m_cv;
      std::atomic<bool>       m_is_running;

      //-----------------------------------------------------------------------
      // Private Modifiers
      //-----------------------------------------------------------------------
    private:

      /// \brief The thread function that operates on the job queue
      void run();

      /// \brief Gets a job from the current queue, if one exists -- or waits
      ///        until one becomes available
      ///
      /// \return the job
      job get_job();
    };

    //-------------------------------------------------------------------------
    // Free Functions
    //-------------------------------------------------------------------------

    /// \brief Posts a job for execution to \p queue
    ///
    /// \param queue the dispatch_queue to post the job to
    /// \param job the job to dispatch
    void post_job( dispatch_queue& queue, job job );

    /// \brief Waits for the job based on the handle
    ///
    /// \param queue the queue to wait on
    /// \param job the handle to wait for
    void wait( dispatch_queue& queue, job_handle job );

    //-------------------------------------------------------------------------

    /// \{
    /// \brief Creates and posts a job to the specified \p queue
    ///
    /// \param queue the dispatch_queue to post the job to
    /// \param parent the parent job
    /// \param fn the function to invoke
    /// \param args the arguments to forward to \p fn
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    void post( dispatch_queue& queue, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    void post( dispatch_queue& queue,
               const job& parent, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args>
    void post( dispatch_queue&, std::nullptr_t, Fn&&, Args&&... ) = delete;
    /// \}

    //-------------------------------------------------------------------------

    /// \{
    /// \brief Creates, posts, and waits for the completion of a specified
    ///        job.
    ///
    /// This makes the result appear synchronous, despite the fact that it
    /// may be invoked on a different thread.
    ///
    /// \param queue the dispatch_queue to post the job to
    /// \param parent the parent job
    /// \param fn the function to invoke
    /// \param args the arguments to forward to \p fn
    /// \return the result of the invocation
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    stl::invoke_result_t<Fn,Args...>
      post_and_wait( dispatch_queue& queue, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    stl::invoke_result_t<Fn,Args...>
      post_and_wait( dispatch_queue& queue,
                     const job& parent, Fn&& fn, Args&&...args );
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

      /// \brief Posts a job for execution to the active dispatch_queue
      ///
      /// \param job the job to dispatch
      void post_job( job job );

      /// \brief Waits for the job based on the handle
      ///
      /// \param job the handle to wait for
      void wait( job_handle job );

      //-----------------------------------------------------------------------

      /// \{
      /// \brief Creates and posts a job to the specified \p queue
      ///
      /// \param parent the parent job
      /// \param fn the function to invoke
      /// \param args the arguments to forward to \p fn
      template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
      void post( Fn&& fn, Args&&...args );
      template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
      void post( const job& parent, Fn&& fn, Args&&...args );
      template<typename Fn, typename...Args>
      void post( std::nullptr_t, Fn&&, Args&&... ) = delete;
      /// \}


    } // namespace this_dispatch_queue
  } // namespace platform
} // namespace bit

#include "detail/dispatch_queue.inl"

#endif /* BIT_PLATFORM_THREADING_DISPATCH_QUEUE_HPP */
