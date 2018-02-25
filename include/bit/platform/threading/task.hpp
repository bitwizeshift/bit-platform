
/**
 * \file task.hpp
 *
 * \brief This header contains the definition for a 'task' class used for
 *        task distribution and balancing in dispatch queues
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_THREADING_TASK_HPP
#define BIT_PLATFORM_THREADING_TASK_HPP

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

    class task;
    namespace detail {

      /// \brief Allocates a task
      ///
      /// \return the pointer to the allocated task
      void* allocate_task();

      /// \brief Gets the active task for this thread
      ///
      /// \return the currently active task
      const task* get_active_task() noexcept;

      /// \brief Sets the currently active task for this thread
      void set_active_task( const task* j ) noexcept;

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
      //---------------------------------------------------------------------
      // Public Static Members
      //---------------------------------------------------------------------
    public:

      static constexpr auto max_tasks = 4096u;

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
      /// \param parent the parent task
      /// \param fn the function to invoke
      /// \param args the arguments to forward to the function
      template<typename Fn, typename...Args,
               typename=std::enable_if_t<stl::is_invocable<Fn,Args...>::value>>
      explicit task( const task& parent, Fn&& fn, Args&&...args );

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
      task& operator=( task&& other );

      // Deleted copy assignment
      task& operator=( const task& other ) = delete;

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
      // Execution
      //-----------------------------------------------------------------------
    public:

      /// \brief Executes this task using the stored arguments
      void execute() const;

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

      detail::task_storage* m_task;

      friend bool operator==( const task&, const task& ) noexcept;

      friend class task_handle;
    };

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

    /// \brief Gets the currently active task, if a task is being processed.
    ///        Otherwise this returns \c nullptr
    ///
    /// \return the active task, or nullptr
    const task* this_task() noexcept;

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
    template<typename Fn, typename...Args>
    task make_task( std::nullptr_t, Fn&&, Args&&... ) = delete;
    /// \}

    ///////////////////////////////////////////////////////////////////////////
    /// \brief A non-owning handle that refers to a given \ref task
    ///
    /// A task_handle can be used to wait on a task that has already been posted
    /// to a given dispatching mechanism, or it can be used
    ///
    ///////////////////////////////////////////////////////////////////////////
    class task_handle
    {
      //-----------------------------------------------------------------------
      // Constructors / Assignment
      //-----------------------------------------------------------------------
    public:

      task_handle() noexcept;

      task_handle( const task& task ) noexcept;

      task_handle( const task_handle& other ) noexcept = default;

      task_handle( task_handle&& other ) noexcept = default;

      //-----------------------------------------------------------------------

      task_handle& operator=( const task_handle& other ) noexcept = default;

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

#endif /* BIT_PLATFORM_THREADING_TASK_HPP */
