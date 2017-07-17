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

// TODO:
// - Make main queue enqueue into its own queue

#include <bit/stl/stddef.hpp>
#include <bit/stl/utility.hpp>
#include <bit/stl/memory.hpp>
#include <bit/stl/functional.hpp> // bit::stl::invoke
#include <bit/stl/assert.hpp>

#include <cstdlib> // std::size_t
#include <atomic>  // std::atomic
#include <thread>  // std::thread
#include <mutex>   // std::mutex
#include <condition_variable> // std::condition_variable
#include <array>   // std::array
#include <vector>  // std::vector
#include <tuple>   // std::tuple

namespace bit {
  namespace platform {
    namespace detail {
      void* allocate_job();

      class job_queue;
    }

    template<typename T>
    std::decay_t<T> decay_copy( T&& v ) { return std::forward<T>(v); }

    class job_dispatcher;

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

      template<typename...Args>
      void store_arguments( Args&&...args );

      //----------------------------------------------------------------------
      // Private Member Types
      //----------------------------------------------------------------------
    private:

      class storage_type
      {
        //--------------------------------------------------------------------
        // Constructor
        //--------------------------------------------------------------------
      public:

        storage_type( void* ptr );

        //--------------------------------------------------------------------
        // Modifiers
        //--------------------------------------------------------------------
      public:

        template<typename...Ts, typename...Args>
        void set( Args&&...args );

        //--------------------------------------------------------------------
        // Observers
        //--------------------------------------------------------------------
      public:

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

      template<typename...Types>
      static void function( void* padding );

      template<typename Tuple, std::size_t...Idxs>
      static void function_inner( Tuple&& tuple, std::index_sequence<Idxs...> );

      //----------------------------------------------------------------------

      template<typename...Types>
      static void destruct_args( storage_type& storage, std::true_type );

      template<typename...Types>
      static void destruct_args( storage_type& storage, std::false_type );

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

    struct assign_affinity_t{};

    constexpr assign_affinity_t assign_affinity = {};

    //////////////////////////////////////////////////////////////////////////
    /// \brief
    ///
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

      static constexpr std::size_t max_jobs = 4096;

      //----------------------------------------------------------------------
      // Constructors / Destructor / Assignment
      //----------------------------------------------------------------------
    public:

      job_dispatcher();

      explicit job_dispatcher( std::size_t threads );

      explicit job_dispatcher( assign_affinity_t );

      explicit job_dispatcher( assign_affinity_t, std::size_t threads );

      // Deleted move constructor
      job_dispatcher( job_dispatcher&& other ) = delete;

      // Deleted copy constructor
      job_dispatcher( const job_dispatcher& other ) = delete;

      //----------------------------------------------------------------------

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

      /// \brief Runs this job_dispatcher.
      ///
      /// \note The calling thread becomes a worker_thread upon invocation of
      ///       this function. Care should be taken to not exhaust input
      ///       and starve the worker threads; otherwise the system may never
      ///       return.
      void run();

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

      /// \brief Starts
      void start();

      std::thread make_worker_thread( std::ptrdiff_t index );

      job* get_job();

      void push_job( std::ptrdiff_t index, job* j );

      template<typename Condition>
      void help_while( Condition&& condition );

      void help_while_unavailable( const job* j );

      void do_work();

      //----------------------------------------------------------------------
      // Friends
      //----------------------------------------------------------------------
    private:

      friend const job* post_job( job_dispatcher&, const job* );
    };

    //------------------------------------------------------------------------
    // Free Functions
    //------------------------------------------------------------------------

    /// \brief Makes a new job that invokes \p fn with \p args
    ///
    /// \param fn the function to invoke
    /// \param args the arguments to forward to \p fn
    /// \return the created job
    template<typename Fn, typename...Args>
    const job* make_job( Fn&& fn, Args&&...args );

    /// \brief Makes a new job that invokes \p fn with \p args
    ///
    /// \param parent the parent of this job
    /// \param fn the function to invoke
    /// \param args the arguments to forward to \p fn
    /// \return the created job
    template<typename Fn, typename...Args>
    const job* make_job( const job* parent, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args>
    const job* make_job( std::nullptr_t, Fn&&, Args&&... ) = delete;

    /// \brief Posts a new job for execution to \p dispatcher
    ///
    /// \param dispatcher the dispatcher to post the job to
    /// \param job the job to dispatch
    const job* post_job( job_dispatcher& dispatcher, const job* job );
    const job* post_job( job_dispatcher&, std::nullptr_t ) = delete;

    /// \{
    /// \brief Creates and posts a job to the specified \p dispatcher
    ///
    /// \param parent the parent job
    /// \param fn the function to invoke
    /// \param args the arguments to forward to \p fn
    /// \return the posted job
    template<typename Fn, typename...Args>
    const job* post_job( job_dispatcher& dispatcher, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args>
    const job* post_job( job_dispatcher& dispatcher,
                         const job* parent, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args>
    const job* post_job( job_dispatcher&,
                         std::nullptr_t, Fn&&, Args&&... ) = delete;
    /// \}

    /// \brief Waits for the job \p job to finish executing, while servicing
    ///        available jobs from \p dispatcher
    ///
    /// \param dispatcher the dispatcher to service jobs from
    /// \param job the job to wait for
    void wait( job_dispatcher& dispatcher, const job* job );

  } // namespace platform
} // namespace bit

#include "detail/job_dispatcher.inl"

#endif /* BIT_PLATFORM_THREADING_JOB_DISPATCHER_HPP */
