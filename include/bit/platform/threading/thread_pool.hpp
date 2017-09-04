/**
 * \file thread_pool.hpp
 *
 * \brief todo: fill in documentation
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */

#ifndef BIT_PLATFORM_THREADING_THREAD_POOL_HPP
#define BIT_PLATFORM_THREADING_THREAD_POOL_HPP

#include "waitable_event.hpp"
#include "concurrent_queue.hpp"

#include <thread> // std::thread
#include <future> // std::packaged_task

#include <bit/stl/utility.hpp>     // stl::apply
#include <bit/stl/type_traits.hpp> // stl::invoke_result_t

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \concept ThreadPool
    ///
    /// \brief This concept defines the required interface and semantics
    ///        expected of a thread pool
    ///
    /// A \c ThreadPool has the ability to dispatch function calls onto a
    /// pool of worker threads that will never return; or alternatively wait
    /// for the return result with a blocking call
    ///
    /// The \c ThreadPool requirement expects the following.
    ///
    /// Provided:
    ///
    /// \c t - an instance of \c ThreadPool,
    /// \c f - an invokable object/function
    /// \c args - arguments to pass to the function (possibly 0)
    ///
    /// the following expressions must be well-formed with the expected
    /// side-effects:
    ///
    /// \code
    /// t.post( f, args... )
    /// \endcode
    /// \c t posts a job to the ThreadPool that will be executed whenever
    /// possible. Both \c f and \c args... are copied before being executed,
    /// as if by calling an imaginary function \c decay_copy.
    ///
    /// \code
    /// auto v = t.post_and_wait( f, args... )
    /// \endcode
    /// \c t posts a job to the ThreadPool that will be executed whenever
    /// possible. The call to this function blocks until \p f finishes
    /// executing; at which point the return value of \c f is returned and
    /// stored in \c v. Both \c f and \c args... are copied before being
    /// executed, as if by calling an imaginary function \c decay_copy.
    ///
    /// where \c decay_copy is defined as
    ///
    /// \code
    /// template<typename T>
    /// std::decay_t<T> decay_copy(T&& v) { return std::forward<T>(v); }
    /// \endcode
    //////////////////////////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////////////////////
    /// \brief A basic thread pool system that uses a concurrent queue for
    ///        managing tasks to deploy
    ///
    /// This type is templated on the type of allocator to be used.
    ///
    /// \tparam Allocator the allocator for the thread data
    /// \satisfies ThreadPool
    //////////////////////////////////////////////////////////////////////////
    template<typename Allocator>
    class basic_thread_pool
    {
      //----------------------------------------------------------------------
      // Constructors / Destructor / Assignment
      //----------------------------------------------------------------------
    public:

      /// \brief Default constructs a thread pool with threads equal to the
      ///        number of active cores on the current system
      basic_thread_pool();

      /// \brief Constructs a thread pool with the specified \p capacity
      ///
      /// \param capacity the number of threads to support
      explicit basic_thread_pool( std::size_t capacity );

      /// \brief Constructs a thread pool using the specified \p allocator
      ///
      /// This defaults the number of threads in the pool to the number of
      /// active cores on the current system
      ///
      /// \param allocator the allocator to use
      explicit basic_thread_pool( const Allocator& allocator );

      /// \brief Constructs a thread pool with the specified \p capacity
      ///        using the specified \p allocator
      ///
      /// \param capacity the number of threads to support
      /// \param allocator the allocator to use
      explicit basic_thread_pool( std::size_t capacity,
                                  const Allocator& allocator );

      /// \brief Default move-construction
      basic_thread_pool( basic_thread_pool&& other ) = default;

      // Deleted copy constructor
      basic_thread_pool( const basic_thread_pool& other ) = delete;

      //----------------------------------------------------------------------

      /// \brief Waits for all dispatched threads
      ~basic_thread_pool();

      //----------------------------------------------------------------------

      // Deleted move assignment
      basic_thread_pool& operator=( basic_thread_pool&& other ) = delete;

      // Deleted copy assignment
      basic_thread_pool& operator=( const basic_thread_pool& other ) = delete;

      //----------------------------------------------------------------------
      // Modifiers
      //----------------------------------------------------------------------
    public:

      /// \brief Posts a function that will be executed by the thread pool
      ///        when a thread becomes available
      ///
      /// The function is called by decaying the arguments and performing a
      /// copy
      ///
      /// \param fn the function to execute
      /// \param args the arguments to forward to the function
      template<typename Fn, typename...Args>
      void post( Fn&& fn, Args&&...args );

      /// \brief Posts a function and waits for to be executed by this thread
      ///        pool
      ///
      /// \param fn the function to execute
      /// \param args the arguments to forward to the function
      /// \return the result from the function posted
      template<typename Fn, typename...Args>
      stl::invoke_result_t<std::decay_t<Fn>> post_and_wait( Fn&& fn,
                                                            Args&&...args );

      //----------------------------------------------------------------------
      // Private Member Types
      //----------------------------------------------------------------------
    private:

      using value_type = std::packaged_task<void()>;

      using thread_container       = std::vector<std::thread,Allocator>;
      using pending_jobs_container = concurrent_queue<value_type,std::mutex,Allocator>;

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    private:

      thread_container       m_threads;
      pending_jobs_container m_queue;
      bool                   m_is_running;
    };

    using thread_pool = basic_thread_pool<std::allocator<char>>;

    //////////////////////////////////////////////////////////////////////////
    /// \brief A thread pool that dispatches a thread for each job
    ///
    /// \satisfies ThreadPool
    //////////////////////////////////////////////////////////////////////////
    class unlimited_thread_pool
    {
      //----------------------------------------------------------------------
      // Constructor / Assignment
      //----------------------------------------------------------------------
    public:

      /// \brief Default constructible
      explicit unlimited_thread_pool() = default;

      /// \brief Default move constructible
      ///
      /// \param other the other unlimited_thread_pool
      unlimited_thread_pool( unlimited_thread_pool&& other ) noexcept = default;

      // Deleted copy constructor
      unlimited_thread_pool( const unlimited_thread_pool& other ) = delete;

      //----------------------------------------------------------------------

      // Deleted move assignment operator
      unlimited_thread_pool& operator=( unlimited_thread_pool&& ) = delete;

      // Deleted copy assignment operator
      unlimited_thread_pool& operator=( const unlimited_thread_pool& ) = delete;

      //----------------------------------------------------------------------
      // Modifiers
      //----------------------------------------------------------------------
    public:

      /// \brief Posts a function that will be executed by the thread pool
      ///        when a thread becomes available. On this thread pool,
      ///        all threads are executed immediately.
      ///
      /// The function is called by decaying the arguments and performing a
      /// copy
      ///
      /// \param fn the function to execute
      /// \param args the arguments to forward to the function
      template<typename Fn, typename...Args>
      void post( Fn&& fn, Args&&...args );

      /// \brief Posts a function and waits for to be executed by this thread
      ///        pool
      ///
      /// \param fn the function to execute
      /// \param args the arguments to forward to the function
      /// \return the result from the function posted
      template<typename Fn, typename...Args>
      stl::invoke_result_t<std::decay_t<Fn>> post_and_wait( Fn&& fn,
                                                            Args&&...args );
    };

    //////////////////////////////////////////////////////////////////////////
    /// \brief A simple wrapper to satisfy the thread-pool concept without
    ///        actually using threads.
    ///
    /// All functions are dispatched synchronously on the calling thread,
    /// rather than asynchronously.
    ///
    /// \satisfies ThreadPool
    //////////////////////////////////////////////////////////////////////////
    class sequential_thread_pool
    {
      //----------------------------------------------------------------------
      // Constructor / Assignment
      //----------------------------------------------------------------------
    public:

      /// \brief Default constructible
      explicit sequential_thread_pool() = default;

      /// \brief Default move constructible
      ///
      /// \param other the other sequential_thread_pool
      sequential_thread_pool( sequential_thread_pool&& other ) noexcept = default;

      // Deleted copy constructor
      sequential_thread_pool( const sequential_thread_pool& other ) = delete;

      //----------------------------------------------------------------------

      // Deleted move assignment operator
      sequential_thread_pool& operator=( sequential_thread_pool&& ) = delete;

      // Deleted copy assignment operator
      sequential_thread_pool& operator=( const sequential_thread_pool& ) = delete;

      //----------------------------------------------------------------------
      // Modifiers
      //----------------------------------------------------------------------
    public:

      /// \brief Posts a function that will be executed immediately in the
      ///        calling thread
      ///
      /// The function is called by decaying the arguments and performing a
      /// copy
      ///
      /// \param fn the function to execute
      /// \param args the arguments to forward to the function
      template<typename Fn, typename...Args>
      void post( Fn&& fn, Args&&...args );

      /// \brief Posts a function and waits for to be executed by this thread
      ///        pool
      ///
      /// \param fn the function to execute
      /// \param args the arguments to forward to the function
      /// \return the result from the function posted
      template<typename Fn, typename...Args>
      stl::invoke_result_t<std::decay_t<Fn>> post_and_wait( Fn&& fn,
                                                            Args&&...args );
    };
  } // namespace platform
} // namespace bit

#include "detail/thread_pool.inl"

#endif /* BIT_PLATFORM_THREADING_THREAD_POOL_HPP */
