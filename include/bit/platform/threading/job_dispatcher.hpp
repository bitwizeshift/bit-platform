/**
 * \file job_dispatcher.hpp
 *
 * \brief This header contains implementation information for a multithreaded
 *        job dispatcher system
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_THREADING_JOB_DISPATCHER_HPP
#define BIT_PLATFORM_THREADING_JOB_DISPATCHER_HPP

#include <bit/stl/utilities/byte.hpp>
#include <bit/stl/utilities/uninitialized_storage.hpp> // bit::stl::destroy_at
#include <bit/stl/utilities/invoke.hpp>                // bit::stl::invoke
#include <bit/stl/utilities/assert.hpp>                // BIT_ASSERT

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
      class job_queue;

      template<typename T>
      struct post_job_and_wait_impl;

      template<typename T>
      struct post_job_and_wait_this_impl;

      void* allocate_job();

      template<typename T>
      std::decay_t<T> decay_copy( T&& v ) { return std::forward<T>(v); }
    } // namespace detail

    std::ptrdiff_t worker_thread_id();

    //////////////////////////////////////////////////////////////////////////
    /// \brief A job is the unit of dispatch used in the job_system
    ///
    //////////////////////////////////////////////////////////////////////////
    class job
    {
      //----------------------------------------------------------------------
      // Observers
      //----------------------------------------------------------------------
    public:

      /// \brief Returns whether this job has completed
      ///
      /// \return \c true if the job has completed
      bool completed() const noexcept;

      /// \brief Returns whether this job is available for execution
      ///
      /// A job is considered available only if all the child tasks have
      /// finished executing first
      ///
      /// \return \c true if this job is available to be executed
      bool available() const noexcept;

      //----------------------------------------------------------------------
      // Element Access
      //----------------------------------------------------------------------
    public:

      /// \brief Returns the parent of this job, if any
      ///
      /// \note This returns nullptr for jobs with no parent
      ///
      /// \return the parent of the job
      job* parent() const noexcept;

      //----------------------------------------------------------------------
      // Modifiers
      //----------------------------------------------------------------------
    public:

      /// \brief Detaches the current active job to run indefinitely in the
      ///        background
      void execute();

      //----------------------------------------------------------------------
      // Private Constructors
      //----------------------------------------------------------------------
    private:

      /// \brief Default-constructs this job
      job();

      // Deleted move construction
      job( job&& other ) = delete;

      // Deleted copy construction
      job( const job& other ) = delete;

      //----------------------------------------------------------------------

      // Deleted move assignment
      job& operator=( job&& other ) = delete;

      // Deleted copy assignment
      job& operator=( const job& other ) = delete;

      /// \brief Constructs a job from a given \p function to invoke
      ///
      /// \param fn the function to invoke
      /// \param
      template<typename Fn, typename...Args>
      explicit job( Fn&& fn, Args&&...args );

      /// \brief Constructs a job from a given \p function to invoke
      ///
      /// \param fn the function to invoke
      template<typename Fn, typename...Args>
      explicit job( job* parent, Fn&& fn, Args&&...args );

      //----------------------------------------------------------------------
      // Private Modifiers
      //----------------------------------------------------------------------
    private:

      /// \brief Finalizes this job
      void finalize();

      /// \brief Stores arguments for the function to use
      ///
      /// \param args the arguments
      template<typename...Args>
      void store_arguments( Args&&...args );

      //----------------------------------------------------------------------
      // Private Member Types
      //----------------------------------------------------------------------
    private:

      ////////////////////////////////////////////////////////////////////////
      /// \brief An underlying storage type that converts the unused padding
      ///        of this job into a tuple of arguments
      ////////////////////////////////////////////////////////////////////////
      class storage_type
      {
        //--------------------------------------------------------------------
        // Constructor
        //--------------------------------------------------------------------
      public:

        /// \brief Constructs the storage type from the specified memory
        ///        address
        ///
        /// \param ptr the memory address for the storage
        storage_type( void* ptr );

        //--------------------------------------------------------------------
        // Modifiers
        //--------------------------------------------------------------------
      public:

        /// \brief Sets the values of the storage type, forwarding the args
        ///
        /// \param args the arguments to store
        template<typename...Ts, typename...Args>
        void set( Args&&...args );

        //--------------------------------------------------------------------
        // Observers
        //--------------------------------------------------------------------
      public:

        /// \brief Gets the values of the storage type as a tuple
        ///
        /// \return the storage values as a tuple
        template<typename...Ts>
        std::tuple<Ts...>& get() const;

        //--------------------------------------------------------------------
        // Private Members
        //--------------------------------------------------------------------
      private:

        void* m_ptr;
      };

      using atomic_type   = std::atomic<std::size_t>;
      using function_type = void(*)( void* );
      using move_function_type = void(*)(void*,void*);

      //----------------------------------------------------------------------
      // Static Private Members
      //----------------------------------------------------------------------
    private:

      // TODO: Determine actual cache line size
      static constexpr std::size_t cache_line_size = 64;

      static constexpr std::size_t padding_size
        = cache_line_size - sizeof(job*) - sizeof(function_type) - sizeof(atomic_type);

      template<typename T>
      static constexpr std::size_t max_storage_size = padding_size - alignof(T);

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    private:

      job*          m_parent;
      function_type m_function;
      atomic_type   m_unfinished;
      stl::byte     m_padding[padding_size];

      //----------------------------------------------------------------------
      // Static Functions
      //----------------------------------------------------------------------
    private:

      /// \brief The function being wrapped in the job object
      ///
      /// \param padding pointer to the padding to convert to arguments
      template<typename...Types>
      static void function( void* padding );

      /// \brief The implementation of the above function that forwards all
      ///        stored arguments to the underlying function type
      ///
      /// \param tuple the tuple of arguments
      template<typename Tuple, std::size_t...Idxs>
      static void function_inner( Tuple&& tuple, std::index_sequence<Idxs...> );

      //----------------------------------------------------------------------

      /// \{
      /// \brief Destructs the underlying arguments stored in the padding
      ///
      /// If the type is trivially destructible, this is a no-op
      ///
      /// \param storage the underlying storage to destruct
      template<typename...Types>
      static void destruct_args( storage_type& storage, std::true_type );
      template<typename...Types>
      static void destruct_args( storage_type& storage, std::false_type );
      /// \}

      //----------------------------------------------------------------------
      // Friends
      //----------------------------------------------------------------------
    private:

      template<typename Fn, typename...Args>
      friend const job* make_job( Fn&&, Args&&... );

      template<typename Fn, typename...Args>
      friend const job* make_job( const job*, Fn&&, Args&&... );

      friend void* detail::allocate_job();
    };

    /// \brief Gets the currently active job, if a job is being processed.
    ///        Otherwise this returns \c nullptr
    ///
    /// \return the active job, or nullptr
    job* this_job();

    /// \brief Tag used for assigning affinity to threads in the
    ///        job_dispatcher
    struct assign_affinity_t{};

    /// \brief Constant used for tag dispatching assigning affinity
    constexpr assign_affinity_t assign_affinity = {};

    //////////////////////////////////////////////////////////////////////////
    /// \brief A dispatcher for managing the job-system.
    ///
    /// This uses a work-stealing queue system for stored jobs.
    ///
    /// \note Only the thread that creates and runs this queue (typically
    ///       from the main message pump) is allowed to stop or destroy this
    ///       queue.
    //////////////////////////////////////////////////////////////////////////
    class job_dispatcher
    {
      //----------------------------------------------------------------------
      // Static Public Members
      //----------------------------------------------------------------------
    public:

      ///The number of max jobs in this dispatcher
      static constexpr std::size_t max_jobs = 4096;

      //----------------------------------------------------------------------
      // Constructors / Destructor / Assignment
      //----------------------------------------------------------------------
    public:

      /// \brief Default-constructs this job_dispatcher with threads equal
      ///        to the number of logical cores on the system - 1
      job_dispatcher();

      /// \brief Constructs the job_dispatcher to use \p threads worker
      ///        threads
      ///
      /// \param threads the number of worker threads
      explicit job_dispatcher( std::size_t threads );

      /// \brief Constructs this job_dispatcher with threads equal
      ///        to the number of logical cores on the system - 1, and
      ///        assigns affinity to each core
      explicit job_dispatcher( assign_affinity_t );

      /// \brief Constructs \p threads worker threads, each assigned
      ///        affinity to new cores
      ///
      /// \param threads the number of worker threads
      explicit job_dispatcher( assign_affinity_t, std::size_t threads );

      // Deleted move constructor
      job_dispatcher( job_dispatcher&& other ) = delete;

      // Deleted copy constructor
      job_dispatcher( const job_dispatcher& other ) = delete;

      //----------------------------------------------------------------------

      /// \brief Destructs and stops the dispatcher from running
      ~job_dispatcher();

      //----------------------------------------------------------------------

      // Deleted move assignment
      job_dispatcher& operator=( job_dispatcher&& other ) = delete;

      // Deleted copy assignment
      job_dispatcher& operator=( const job_dispatcher& other ) = delete;

      //----------------------------------------------------------------------
      // Modifiers
      //----------------------------------------------------------------------
    public:

      /// \brief Runs this job_dispatcher, invoking \p fn each iteration
      ///
      /// \note The calling thread becomes a worker_thread that invkes \p fn
      ///       before attempting to do work with jobs. Care should be taken
      ///       to not exhaust job input and starve the worker threads;
      ///       otherwise the system may never return.
      ///
      /// \param fn The function to invoke each iteration
      template<typename Fn>
      void run( Fn&& fn );

      //----------------------------------------------------------------------

      /// \brief Signals to stop running this job_dispatcher
      ///
      /// The remaining enqueued jobs will be invoked before this dispatcher
      /// comes to a full stop -- but any further calls to post_job will be
      /// silently ignored.
      void stop();

      //----------------------------------------------------------------------

      /// \brief Waits for a job \p j to be completed
      ///
      /// The calling thread participates in executing jobs while waiting
      /// for \p j to complete.
      ///
      /// \param j the job to wait for
      void wait( const job* j );

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    private:

      std::vector<std::thread>        m_threads;
      std::vector<detail::job_queue*> m_queues;
      std::thread::id                 m_owner;
      std::mutex                      m_lock;
      std::condition_variable         m_cv;
      std::atomic<std::size_t>        m_running_threads;
      bool                            m_running;
      bool                            m_set_affinity;

      //----------------------------------------------------------------------
      // Private Capacity
      //----------------------------------------------------------------------
    private:

      /// \brief Queries whether there are any remaining jobs in any of the
      ///        current queues
      ///
      /// \return \c true if there are any jobs remaining in any queue
      bool has_remaining_jobs() const noexcept;

      //----------------------------------------------------------------------
      // Private Modifiers
      //----------------------------------------------------------------------
    private:

      /// \brief Starts this job_dispatcher for the first time
      void start();

      /// \brief Makes a worker thread with the specified thread \p index
      ///
      /// \return the worker thread
      std::thread make_worker_thread( std::ptrdiff_t index );

      /// \brief Gets a job either through the current thread's queue, or
      ///        from stealing from another active thread
      ///
      /// \return the job
      job* get_job();

      /// \brief Pushes a job onto this queue
      ///
      /// \param j the job to push
      void push_job( job* j );

      /// \brief Helps in processing jobs while a condition is met
      ///
      /// \param condition the condition to check for
      template<typename Condition>
      void help_while( Condition&& condition );

      /// \brief Helps in processing jobs while the specified job \p j is
      ///        unavailable for processing
      ///
      /// \note This function exists to break infinite template recursion
      ///       caused by calling help_while with a lambda from inside of
      ///       help_while. Each lambda produces a new, unique type causing
      ///       unlimited recursion otherwise.
      ///
      /// \param j the job to check for availability
      void help_while_unavailable( const job* j );

      /// \brief Performs the basic work cycle
      void do_work();

      //----------------------------------------------------------------------
      // This Dispatcher
      //----------------------------------------------------------------------
    private:

      /// \brief Pushes a new job onto the currently active thread queue
      ///
      /// \param j the job to push
      static void push_this_job( job* j );

      /// \brief Waits for the job \p j to complete running
      ///
      /// \param j the job to wait on
      static void wait_this( const job* j );

      //----------------------------------------------------------------------
      // Friends
      //----------------------------------------------------------------------
    private:

      friend const job* post_job( job_dispatcher&, const job* );

      friend const job* post_job( const job* );

      friend void wait( const job* );
    };

    //------------------------------------------------------------------------
    // Free Functions
    //------------------------------------------------------------------------

    /// \{
    /// \brief Makes a new job that invokes \p fn with \p args
    ///
    /// \param parent the parent of this job
    /// \param fn the function to invoke
    /// \param args the arguments to forward to \p fn
    /// \return the created job
    template<typename Fn, typename...Args>
    const job* make_job( Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args>
    const job* make_job( const job* parent, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args>
    const job* make_job( std::nullptr_t, Fn&&, Args&&... ) = delete;
    /// \}

    //------------------------------------------------------------------------

    /// \brief Posts a job for execution to \p dispatcher
    ///
    /// \param dispatcher the dispatcher to post the job to
    /// \param job the job to dispatch
    /// \return the posted job
    const job* post_job( job_dispatcher& dispatcher, const job* job );
    const job* post_job( job_dispatcher&, std::nullptr_t ) = delete;

    /// \brief Posts a job for execution to the active dispatcher for the given
    ///        thread
    ///
    /// \param job the job to dispatch
    /// \return the posted job
    const job* post_job( const job* job );
    const job* post_job( std::nullptr_t ) = delete;

    //------------------------------------------------------------------------

    /// \{
    /// \brief Creates and posts a job to the specified \p dispatcher
    ///
    /// \param dispatcher the dispatcher to post the job to
    /// \param parent the parent job
    /// \param fn the function to invoke
    /// \param args the arguments to forward to \p fn
    /// \return the posted job
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    const job* post_job( job_dispatcher& dispatcher, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    const job* post_job( job_dispatcher& dispatcher,
                         const job* parent, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args>
    const job* post_job( job_dispatcher&,
                         std::nullptr_t, Fn&&, Args&&... ) = delete;
    /// \}

    /// \{
    /// \brief Creates and posts a job to this thread's active dispatcher
    ///
    /// \param parent the parent job
    /// \param fn the function to invoke
    /// \param args the arguments to forward to \p fn
    /// \return the posted job
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    const job* post_job( Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    const job* post_job( const job* parent, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args>
    const job* post_job( std::nullptr_t, Fn&&, Args&&... ) = delete;
    /// \}

    //------------------------------------------------------------------------

    /// \{
    /// \brief Creates, posts, and waits for the completion of a specified
    ///        job.
    ///
    /// This makes the result appear synchronous, despite the fact that it
    /// may be invoked on a different thread.
    ///
    /// \param dispatcher the dispatcher to post the job to
    /// \param parent the parent job
    /// \param fn the function to invoke
    /// \param args the arguments to forward to \p fn
    /// \return the result of the invocation
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    stl::invoke_result_t<Fn,Args...>
      post_job_and_wait( job_dispatcher& dispatcher, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    stl::invoke_result_t<Fn,Args...>
      post_job_and_wait( job_dispatcher& dispatcher,
                         const job* parent, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args>
    stl::invoke_result_t<Fn,Args...>
      post_job_and_wait( job_dispatcher&,
                         std::nullptr_t, Fn&&, Args&&... ) = delete;
    /// \}

    /// \{
    /// \brief Creates, posts, and waits for the completion of a specified
    ///        job to the active dispatcher for the given thread
    ///
    /// This makes the result appear synchronous, despite the fact that it
    /// may be invoked on a different thread.
    ///
    /// \param parent the parent job
    /// \param fn the function to invoke
    /// \param args the arguments to forward to \p fn
    /// \return the result of the invocation
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    stl::invoke_result_t<Fn,Args...>
      post_job_and_wait( Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args, typename = decltype(std::declval<Fn>()(std::declval<Args>()...),void())>
    stl::invoke_result_t<Fn,Args...>
      post_job_and_wait( const job* parent, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args>
    stl::invoke_result_t<Fn,Args...>
      post_job_and_wait( std::nullptr_t, Fn&&, Args&&... ) = delete;
    /// \}

    //------------------------------------------------------------------------

    /// \brief Waits for the job \p job to finish executing, while servicing
    ///        available jobs from \p dispatcher
    ///
    /// \param dispatcher the dispatcher to service jobs from
    /// \param job the job to wait for
    void wait( job_dispatcher& dispatcher, const job* job );
    void wait( job_dispatcher&, std::nullptr_t ) = delete;

    /// \brief Waits for the job \p job to finish executing, while servicing
    ///        available jobs from the active \p dispatcher
    ///
    /// \param job the job to wait for
    void wait( const job* job );
    void wait( std::nullptr_t ) = delete;

  } // namespace platform
} // namespace bit

#include "detail/job_dispatcher.inl"

#endif /* BIT_PLATFORM_THREADING_JOB_DISPATCHER_HPP */
