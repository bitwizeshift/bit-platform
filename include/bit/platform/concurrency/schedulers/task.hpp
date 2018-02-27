
/**
 * \file task.hpp
 *
 * \brief This header contains the definition for a 'task' class used for
 *        task distribution and balancing in dispatch queues
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_CONCURRENCY_SCHEDULERS_TASK_HPP
#define BIT_PLATFORM_CONCURRENCY_SCHEDULERS_TASK_HPP

#include "../utilities/true_share.hpp" // cache_line_size
#include <bit/stl/utilities/invoke.hpp> // stl::invoke

#include <atomic>  // std::atomic
#include <cassert> // assert
#include <cstddef> // std::size_t
#include <cstdint> // std::uint16_t
#include <memory>  // std::align, std::unique_ptr
#include <new>     // placement-new
#include <tuple>   // std::tuple
#include <utility> // std::index_sequence, std::move, std::forward, std::swap

namespace bit {
  namespace platform {

    class task;

    namespace detail {

      /// \brief Allocates a task
      ///
      /// \return the pointer to the allocated task
      void* allocate_task();

      template<typename T>
      std::decay_t<T> decay_copy( T&& v ) { return std::forward<T>(v); }

      class task_storage;
    } // namespace detail

    ///////////////////////////////////////////////////////////////////////////
    /// \brief This class represents active tasks that are available for
    ///        execution.
    ///
    /// Jobs are move-only data types that represent tasks that may be executed
    /// any number of times.
    ///////////////////////////////////////////////////////////////////////////
    class task
    {
      //----------------------------------------------------------------------
      // Public Static Members
      //----------------------------------------------------------------------
    public:

      // TODO(bitwizeshift): Make this configurable from a configure file
      static constexpr auto max_tasks = 4096u;

      // TODO(bitwizeshift): Make size of each task storage configurable

      //-----------------------------------------------------------------------
      // Constructors / Assignment / Destructor
      //-----------------------------------------------------------------------
    public:

      /// \brief Default-constructs a null task
      ///
      /// Calling \p execute() on a null
      task() noexcept;

      /// \brief Constructs a task from the given invocable \p fn and the
      ///        supplied \p args
      ///
      /// \param fn the function to invoke
      /// \param args the arguments to forward to the function
      template<typename Fn, typename...Args,
               typename=std::enable_if_t<stl::is_invocable<Fn,Args...>::value>,
               typename=std::enable_if_t<!std::is_same<std::decay_t<Fn>,task>::value>>
      explicit task( Fn&& fn, Args&&... args );

      /// \brief Constructs a task that is spawned as a sub-task of \p parent
      ///
      /// \pre parent is non-null task
      ///
      /// \param parent the parent task
      /// \param fn the function to invoke
      /// \param args the arguments to forward to the function
      template<typename Fn, typename...Args,
               typename=std::enable_if_t<stl::is_invocable<Fn,Args...>::value>>
      explicit task( const task& parent, Fn&& fn, Args&&...args );

      // TODO(bitwizeshift): Add allocator support for tasks that contain a
      //                     large storage space

      /// \brief Move-constructs this task from an existing task
      ///
      /// \param other the other task to move from
      task( task&& other ) noexcept;

      // Deleted copy constructor
      task( const task& other ) = delete;

      //-----------------------------------------------------------------------

      /// \brief Destructs the stored task data
      ~task();

      //-----------------------------------------------------------------------

      /// \brief Move-assigns this task from an existing task
      ///
      /// \param other the task to move
      /// \return reference to \c (*this)
      task& operator=( task other );

      //-----------------------------------------------------------------------
      // Observers
      //-----------------------------------------------------------------------
    public:

      /// \brief Returns whether this task has completed
      ///
      /// \return \c true if the task has completed
      bool completed() const noexcept;

      /// \brief Returns whether this task is available for execution
      ///
      /// A task is considered available only if all the child tasks have
      /// finished executing first
      ///
      /// \return \c true if this task is available to be executed
      bool available() const noexcept;

      //-----------------------------------------------------------------------
      // Modifiers
      //-----------------------------------------------------------------------
    public:

      /// \brief Swaps this task with \p other
      ///
      /// \post \p other contains the contents of \c this, and \c this contains
      ///       the old contents of \p other
      ///
      /// \param other the other entry to swap with
      void swap( task& other ) noexcept;

      //-----------------------------------------------------------------------
      // Execution
      //-----------------------------------------------------------------------
    public:

      /// \brief Executes this task using the stored arguments
      ///
      /// It is undefined behavior for the task to be invoked when null, or if
      /// it is not yet available (e.g. has child jobs that have not yet
      /// finished executing)
      ///
      /// \pre task must not be null
      ///
      /// \pre \c available() must return \c true
      ///
      /// \note Executing a task more than once is generally considered bad
      ///       practice, and may not yield appropriate behavior since tasks
      ///       are intended to form a proper ordering.
      void execute() const;

      /// \copydoc execute
      void operator()() const;

      //-----------------------------------------------------------------------
      // Conversions
      //-----------------------------------------------------------------------
    public:

      /// \brief Returns a bool indicating whether this task has a value
      explicit operator bool() const noexcept;

      //-----------------------------------------------------------------------
      // Private Members
      //-----------------------------------------------------------------------
    private:

      detail::task_storage* m_task; ///< The underlying task data

      // Friendships
      friend bool operator==( const task&, const task& ) noexcept;
      friend class task_handle;
    };

    static_assert( sizeof(task)<=sizeof(void*),
                   "Task must be at most the size of a single pointer" );

    //-------------------------------------------------------------------------
    // Equality
    //-------------------------------------------------------------------------

    bool operator==( const task& lhs, std::nullptr_t ) noexcept;
    bool operator==( std::nullptr_t, const task& rhs ) noexcept;
    bool operator==( const task& lhs, const task& rhs ) noexcept;

    //-------------------------------------------------------------------------

    bool operator!=( const task& lhs, std::nullptr_t ) noexcept;
    bool operator!=( std::nullptr_t, const task& rhs ) noexcept;
    bool operator!=( const task& lhs, const task& rhs ) noexcept;

    //-------------------------------------------------------------------------
    // Utilities
    //-------------------------------------------------------------------------

    /// \brief Swaps the contents of \p lhs with \p rhs
    ///
    /// \post \p lhs contains the old contents of \p rhs, and \p rhs contains
    ///       the old contents of \p lhs
    ///
    /// \param lhs the left task to swap
    /// \param rhs the right task to swap
    void swap( task& lhs, task& rhs ) noexcept;

    //-------------------------------------------------------------------------

    /// \{
    /// \brief Makes a new task that invokes \p fn with \p args
    ///
    /// \param parent the parent of this task
    /// \param fn the function to invoke
    /// \param args the arguments to forward to \p fn
    /// \return the created task
    template<typename Fn, typename...Args>
    task make_task( Fn&& fn, Args&&...args );
    template<typename Fn, typename...Args>
    task make_task( const task& parent, Fn&& fn, Args&&...args );
    /// \}

    //-------------------------------------------------------------------------
    // Handler Type
    //-------------------------------------------------------------------------

    /// \brief Handler for managing out-of-task error condition
    using out_of_task_handler_t = void(*)();

    //-------------------------------------------------------------------------
    // Handlers
    //-------------------------------------------------------------------------

    /// \brief Sets the global out-of-task handler.
    ///
    /// This handler will be called if too many tasks are allocated from a
    /// single thread.
    ///
    /// \param f the function handler
    /// \return the previous handler
    out_of_task_handler_t set_out_of_task_handler( out_of_task_handler_t f );

    /// \brief Gets the current active out-of-task handler
    ///
    /// \return the current handler
    out_of_task_handler_t get_out_of_task_handler();

    ///////////////////////////////////////////////////////////////////////////
    /// \brief A non-owning handle that refers to a given \ref task
    ///
    /// A task_handle can be used to wait on a task that has already been posted
    /// to a given scheduling mechanism, or it can be used
    ///////////////////////////////////////////////////////////////////////////
    class task_handle
    {
      //-----------------------------------------------------------------------
      // Constructors / Assignment
      //-----------------------------------------------------------------------
    public:

      /// \brief Default-constructs a task handle pointing to a null task
      task_handle() noexcept;

      /// \brief Constructs a task-handle that points to a \p task
      ///
      /// \param task the task to point to
      task_handle( const task& task ) noexcept;

      /// \brief Copy-constructs a task handle from an existing handle
      ///
      /// \param other the other handle to copy
      task_handle( const task_handle& other ) noexcept = default;

      /// \brief Move-constructs a task handle from an existing handle
      ///
      /// \param other the other handle to move
      task_handle( task_handle&& other ) noexcept = default;

      //-----------------------------------------------------------------------

      /// \brief Copy-assigns a task handle from an existing handle
      ///
      /// \param other the other task handle to copy
      /// \return reference to \c (*this)
      task_handle& operator=( const task_handle& other ) noexcept = default;

      /// \brief Move-assigns a task handle from an existing handle
      ///
      /// \param other the other task handle to move
      /// \return reference to \c (*this)
      task_handle& operator=( task_handle&& other ) noexcept = default;

      //-----------------------------------------------------------------------
      // Observers
      //-----------------------------------------------------------------------
    public:

      /// \brief Returns whether this task has completed
      ///
      /// \return \c true if the task has completed
      bool completed() const noexcept;

      /// \brief Returns whether this task is available for execution
      ///
      /// A task is considered available only if all the child tasks have
      /// finished executing first
      ///
      /// \return \c true if this task is available to be executed
      bool available() const noexcept;

      //-----------------------------------------------------------------------
      // Private Members
      //-----------------------------------------------------------------------
    private:

      detail::task_storage* m_task;
    };
  } // namespace platform
} // namespace bit

#include "detail/task.inl"

#endif /* BIT_PLATFORM_CONCURRENCY_SCHEDULERS_TASK_HPP */
