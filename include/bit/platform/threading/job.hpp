
/**
 * \file job.hpp
 *
 * \brief This header contains the definition for a 'job' class used for
 *        job distribution and balancing in dispatch queues
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_THREADING_JOB_HPP
#define BIT_PLATFORM_THREADING_JOB_HPP

#include <bit/stl/utilities/invoke.hpp> // stl::invoke

#include "true_share.hpp" // true_share

#include <atomic>  // std::atomic
#include <cassert> // assert
#include <cstddef> // std::size_t
#include <cstdint> // std::uint16_t
#include <memory>  // std::align, std::unique_ptr
#include <new>     // placement-new
#include <tuple>   // std::tuple
#include <utility> // std::index_sequence

namespace bit {
  namespace platform {

    class job;
    namespace detail {

      /// \brief Allocates a job
      ///
      /// \return the pointer to the allocated job
      void* allocate_job();

      /// \brief Gets the active job for this thread
      ///
      /// \return the currently active job
      const job* get_active_job() noexcept;

      /// \brief Sets the currently active job for this thread
      void set_active_job( const job* j ) noexcept;

      template<typename T>
      std::decay_t<T> decay_copy( T&& v ) { return std::forward<T>(v); }

      class job_storage;
    } // namespace detail

    ///////////////////////////////////////////////////////////////////////////
    /// \brief This class represents active jobs that are available for
    ///        execution.
    ///
    /// Jobs are move-only data types that represent jobs that may be executed
    /// any number of times.
    ///////////////////////////////////////////////////////////////////////////
    class job
    {
      //---------------------------------------------------------------------
      // Public Static Members
      //---------------------------------------------------------------------
    public:

      static constexpr auto max_jobs = 4096u;

      //-----------------------------------------------------------------------
      // Constructors / Assignment / Destructor
      //-----------------------------------------------------------------------
    public:

      /// \brief Default-constructs a null job
      ///
      /// Calling \p execute() on a null
      job() noexcept;

      /// \brief Constructs a job from the given invocable \p fn and the
      ///        supplied \p args
      ///
      /// \param fn the function to invoke
      /// \param args the arguments to forward to the function
      template<typename Fn, typename...Args,
               typename=std::enable_if_t<stl::is_invocable<Fn,Args...>::value>,
               typename=std::enable_if_t<!std::is_same<std::decay_t<Fn>,job>::value>>
      explicit job( Fn&& fn, Args&&... args );

      /// \brief Constructs a job that is spawned as a sub-job of \p parent
      ///
      /// \param parent the parent job
      /// \param fn the function to invoke
      /// \param args the arguments to forward to the function
      template<typename Fn, typename...Args,
               typename=std::enable_if_t<stl::is_invocable<Fn,Args...>::value>>
      explicit job( const job& parent, Fn&& fn, Args&&...args );

      /// \brief Move-constructs this job from an existing job
      ///
      /// \param other the other job to move from
      job( job&& other ) noexcept;

      // Deleted copy constructor
      job( const job& other ) = delete;

      //-----------------------------------------------------------------------

      /// \brief Destructs the stored job data
      ~job();

      //-----------------------------------------------------------------------

      /// \brief Move-assigns this job from an existing job
      ///
      /// \param other the job to move
      /// \return reference to \c (*this)
      job& operator=( job&& other );

      // Deleted copy assignment
      job& operator=( const job& other ) = delete;

      //-----------------------------------------------------------------------
      // Observers
      //-----------------------------------------------------------------------
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

      //-----------------------------------------------------------------------
      // Execution
      //-----------------------------------------------------------------------
    public:

      /// \brief Executes this job using the stored arguments
      void execute() const;

      //-----------------------------------------------------------------------
      // Conversions
      //-----------------------------------------------------------------------
    public:

      /// \brief Returns a bool indicating whether this job has a value
      explicit operator bool() const noexcept;

      //-----------------------------------------------------------------------
      // Private Members
      //-----------------------------------------------------------------------
    private:

      detail::job_storage* m_job;

      friend bool operator==( const job&, const job& ) noexcept;

      friend class job_handle;
    };

    //-------------------------------------------------------------------------
    // Equality
    //-------------------------------------------------------------------------

    bool operator==( const job& lhs, std::nullptr_t ) noexcept;
    bool operator==( std::nullptr_t, const job& rhs ) noexcept;
    bool operator==( const job& lhs, const job& rhs ) noexcept;

    //-------------------------------------------------------------------------

    bool operator!=( const job& lhs, std::nullptr_t ) noexcept;
    bool operator!=( std::nullptr_t, const job& rhs ) noexcept;
    bool operator!=( const job& lhs, const job& rhs ) noexcept;

    //-------------------------------------------------------------------------
    // Utilities
    //-------------------------------------------------------------------------

    /// \brief Gets the currently active job, if a job is being processed.
    ///        Otherwise this returns \c nullptr
    ///
    /// \return the active job, or nullptr
    const job* this_job() noexcept;

    //-------------------------------------------------------------------------

    /// \{
    /// \brief Makes a new job that invokes \p fn with \p args
    ///
    /// \param parent the parent of this job
    /// \param fn the function to invoke
    /// \param args the arguments to forward to \p fn
    /// \return the created job
    template<typename Fn, typename...Args>
    job make_job( Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args>
    job make_job( const job& parent, Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args>
    job make_job( std::nullptr_t, Fn&&, Args&&... ) = delete;
    /// \}

    ///////////////////////////////////////////////////////////////////////////
    /// \brief A non-owning handle that refers to a given \ref job
    ///
    /// A job_handle can be used to wait on a job that has already been posted
    /// to a given dispatching mechanism, or it can be used
    ///
    ///////////////////////////////////////////////////////////////////////////
    class job_handle
    {
      //-----------------------------------------------------------------------
      // Constructors / Assignment
      //-----------------------------------------------------------------------
    public:

      job_handle() noexcept;

      job_handle( const job& job ) noexcept;

      job_handle( const job_handle& other ) noexcept = default;

      job_handle( job_handle&& other ) noexcept = default;

      //-----------------------------------------------------------------------

      job_handle& operator=( const job_handle& other ) noexcept = default;

      job_handle& operator=( job_handle&& other ) noexcept = default;

      //-----------------------------------------------------------------------
      // Observers
      //-----------------------------------------------------------------------
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

      //-----------------------------------------------------------------------
      // Private Members
      //-----------------------------------------------------------------------
    private:

      detail::job_storage* m_job;
    };
  } // namespace platform
} // namespace bit

#include "detail/job.inl"

#endif /* BIT_PLATFORM_THREADING_JOB_HPP */
