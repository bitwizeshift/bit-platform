/**
 * \file task_scheduler.hpp
 *
 * \brief TODO: Add description
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_CONCURRENCY_SCHEDULERS_TASK_SCHEDULER_HPP
#define BIT_PLATFORM_CONCURRENCY_SCHEDULERS_TASK_SCHEDULER_HPP

#include "task.hpp" // task

#include <bit/stl/utilities/invoke.hpp> // stl::is_invocable

#include <type_traits> // std::enable_if
#include <utility>     // std::forward
#include <mutex>       // std::mutex

namespace bit {
  namespace platform {

    ///////////////////////////////////////////////////////////////////////////
    /// \brief A system for scheduling tasks.
    ///
    /// Schedulers must provide a way of posting the task itself, and for
    /// waiting on an existing/posted task.
    ///////////////////////////////////////////////////////////////////////////
    class task_scheduler
    {
      /////////////////////////////////////////////////////////////////////////
      /// \brief The shared state for a bound_object
      ///
      /// This contains the scheduler that the object is bound to
      /////////////////////////////////////////////////////////////////////////
      struct bound_object_state
      {
        std::atomic<task_scheduler*> bound_scheduler;
      };

      //-----------------------------------------------------------------------
      // Public Member Types
      //-----------------------------------------------------------------------
    public:

      /////////////////////////////////////////////////////////////////////////
      /// \brief An object that is bound to a given scheduler, and is only
      ///        accessible asynchronously.
      ///
      /// \tparam T the type to bind
      /////////////////////////////////////////////////////////////////////////
      template<typename T>
      class bound_object
      {
        static_assert( !std::is_void<T>::value,
                       "bound_object<void> is ill-formed" );
        static_assert( !std::is_reference<T>::value,
                       "bound_object<T&> is ill-formed" );

        //---------------------------------------------------------------------
        // Constructors / Assignment
        //---------------------------------------------------------------------
      public:

        /// \brief Default-constructs the bound_object without an object bound
        bound_object() noexcept;

        /// \brief Constructs a bound_object by move-converting a different
        ///        object to this type.
        ///
        /// \param other the other bound_object to convert
        template<typename U,
                 typename=std::enable_if_t<std::is_convertible<U*,T*>::value>>
        bound_object( bound_object<U>&& other ) noexcept;

        /// \brief Constructs a bound_object by copy-converting a different
        ///        object to this type.
        ///
        /// \param other the other bound_object to convert
        template<typename U,
                 typename=std::enable_if_t<std::is_convertible<U*,T*>::value>>
        bound_object( const bound_object<U>& other ) noexcept;

        /// \brief Move-constructs a bound_object from an existing object
        ///
        /// \param other the other bound_object to move
        bound_object( bound_object&& other ) noexcept = default;

        /// \brief Copy-constructs a bound_object from an existing object
        ///
        /// \param other the other bound_object to copy
        bound_object( const bound_object& other ) noexcept = default;

        //---------------------------------------------------------------------

        /// \brief Assigns a bound_object from an existing one
        ///
        /// \param other the other bound_object to assign
        /// \return reference to \c (*this)
        bound_object& operator=( bound_object other ) noexcept;

        //---------------------------------------------------------------------
        // Modifiers
        //---------------------------------------------------------------------
      public:

        /// \brief Swaps the contents of \c this and \p other
        ///
        /// \post \p other contains the old contents of \c this, and \c this
        ///       contains the old contents of \p other.
        ///
        /// \param other the other bound_object to swap with
        void swap( bound_object<T>& other ) noexcept;

        //---------------------------------------------------------------------

        /// \brief Visits the object asynchronously by posting a task back
        ///        onto the current task_scheduler.
        ///
        /// \pre bound_object must refer to a valid object at the time of
        ///      invocation
        ///
        /// \param fn the function to invoke asynchronously
        template<typename Fn,
                 typename=std::enable_if_t<stl::is_invocable<Fn,T&>::value>>
        void visit_async( Fn&& fn );

        /// \brief Visits the object synchronously by posting a task back
        ///        onto the current task_scheduler and waiting on the result
        ///
        /// \pre bound_object must refer to a valid object at the time of
        ///      invocation
        ///
        /// \note Since this function will cause blocking/waiting to occur on
        ///       the bound task_scheduler, care must be taken to ensure that
        ///       a deadlock does not occur by waiting on a scheduler that is
        ///       already waiting.
        ///
        /// \param fn the function to invoke synchronously
        /// \return the result returned by \p fn
        template<typename Fn,
                 typename=std::enable_if_t<stl::is_invocable<Fn,T&>::value>>
        stl::invoke_result_t<Fn,T&> visit( Fn&& fn );

        //---------------------------------------------------------------------

        /// \brief Unbinds and disables this bound_object
        void unbind();

        //---------------------------------------------------------------------
        // Observers
        //---------------------------------------------------------------------
      public:

        /// \brief Checks if the given bound_object has an object bound
        ///
        /// \return \c true if an object is bound
        bool is_bound() const noexcept;

        /// \copydoc is_bound
        explicit operator bool() const noexcept;

        //---------------------------------------------------------------------
        // Private Members
        //---------------------------------------------------------------------
      private:

        T*                                  m_object;
        std::shared_ptr<bound_object_state> m_state;

        friend class task_scheduler;

        friend bool operator==( const bound_object&, const bound_object& ) noexcept;

        //---------------------------------------------------------------------
        // Private Constructors
        //---------------------------------------------------------------------
      private:

        /// \brief Constructs a bound_object from a given \p object
        ///
        /// \param object the object to bind
        explicit bound_object( task_scheduler& scheduler,
                               T& object );

        /// \brief Constructs a bound_object from a given \p object, using the
        ///        specified \p allocator for allocations
        ///
        /// \param allocator the allocator to allocate with
        /// \param object the object to bind
        template<typename Allocator>
        bound_object( const Allocator& allocator,
                      task_scheduler& scheduler,
                      T& object );
      }; // class bound_object

      //-----------------------------------------------------------------------
      // Destructor
      //-----------------------------------------------------------------------
    public:

      virtual ~task_scheduler() = 0;

      //-----------------------------------------------------------------------
      // Posting (virtual)
      //-----------------------------------------------------------------------
    protected:

      virtual void do_post_task( task task ) = 0;

    public:

      /// \brief Waits for a given task to complete
      ///
      /// \param task the task handle to wait on
      virtual void wait( task_handle task ) = 0;

      //-----------------------------------------------------------------------
      // Posting / Waiting
      //-----------------------------------------------------------------------
    public:

      /// \brief Posts a given task \p t
      ///
      /// \param task the task to post
      void post_task( task task );

      //-----------------------------------------------------------------------

      /// \{
      /// \brief Posts a given function \p fn with the specified arguments
      ///        \p args as a task in this scheduler
      ///
      /// \param parent the task to designate as parent
      /// \param fn the function to post
      /// \param args the arguments to forward
      template<typename Fn, typename...Args,
               typename=std::enable_if_t<stl::is_invocable<Fn,Args...>::value>>
      void post( Fn&& fn, Args&&...args );
      template<typename Fn, typename...Args,
               typename=std::enable_if_t<stl::is_invocable<Fn,Args...>::value>>
      void post( const task& parent, Fn&& fn, Args&&...args );
      /// \}

      /// \{
      /// \brief Posts a given function and waits for the result
      ///
      /// \param parent the task to designate as parent
      /// \param fn the function to post
      /// \param args the arguments to forward
      /// \return the result of the function being executed
      template<typename Fn, typename...Args,
               typename=std::enable_if_t<stl::is_invocable<Fn,Args...>::value>>
      stl::invoke_result_t<Fn,Args...> post_and_wait( Fn&& fn, Args&&...args );
      template<typename Fn, typename...Args,
               typename=std::enable_if_t<stl::is_invocable<Fn,Args...>::value>>
      stl::invoke_result_t<Fn,Args...> post_and_wait( const task& parent,
                                                      Fn&& fn,
                                                      Args&&...args );
      /// \}

      //-----------------------------------------------------------------------
      // Binding
      //-----------------------------------------------------------------------
    public:

      /// \brief Makes a bound object from a given \p object
      ///
      /// A bound object may only be accessed from the given task scheduler
      ///
      /// \param object the object to bind
      /// \return the bound object
      template<typename T>
      bound_object<T> make_bound_object( T& object );

      /// \brief Allocates and makes a bound object using a specified \p
      ///        allocator, and binding the specified \p object
      ///
      /// A bound object may only be accessed from the given task scheduler
      ///
      /// \param allocator the allocator to allocate the bound object with
      /// \param object the object to bind
      /// \return the bound object
      template<typename Allocator, typename T>
      bound_object<T> allocate_bound_object( const Allocator& allocator,
                                             T& object );

      //-----------------------------------------------------------------------
      // Private Static Members
      //-----------------------------------------------------------------------
    private:

      static thread_local task_scheduler* g_active_scheduler;

      //-----------------------------------------------------------------------
      // Private Member Types
      //-----------------------------------------------------------------------
    private:

      template<typename T> struct type_tag{};

      //-----------------------------------------------------------------------
      // Private Posting
      //-----------------------------------------------------------------------
    private:

      // TODO(bitwizeshift): Remove code duplication

      /// \{
      /// \brief Implementation of 'post_and_wait' using tag-dispatching to
      ///        disambiguate return types
      ///
      /// \param fn the function
      /// \param args the arguments to forward to the function
      /// \param parent the parent task
      /// \return the result of the function invocation
      template<typename Fn, typename...Args>
      void post_and_wait_impl( type_tag<void>, Fn&& fn, Args&&...args );
      template<typename Fn, typename...Args>
      void post_and_wait_impl( type_tag<void>, const task& parent, Fn&& fn, Args&&...args );
      template<typename T, typename Fn, typename...Args>
      T post_and_wait_impl( type_tag<T>, Fn&& fn, Args&&...args );
      template<typename T, typename Fn, typename...Args>
      T post_and_wait_impl( type_tag<T>, const task& parent, Fn&& fn, Args&&...args );
      template<typename T, typename Fn, typename...Args>
      T& post_and_wait_impl( type_tag<T&>, Fn&& fn, Args&&...args );
      template<typename T, typename Fn, typename...Args>
      T& post_and_wait_impl( type_tag<T&>, const task& parent, Fn&& fn, Args&&...args );
      /// \}

      friend class this_task_scheduler;

    }; // class task_dispatcher

    //=========================================================================
    // Free Functions
    //=========================================================================

    //-------------------------------------------------------------------------
    // Equality
    //-------------------------------------------------------------------------

    template<typename T>
    bool operator==( const task_scheduler::bound_object<T>& lhs,
                     const task_scheduler::bound_object<T>& rhs ) noexcept;

    template<typename T>
    bool operator!=( const task_scheduler::bound_object<T>& lhs,
                     const task_scheduler::bound_object<T>& rhs ) noexcept;

    //-------------------------------------------------------------------------
    // Utilities
    //-------------------------------------------------------------------------

    template<typename T>
    void swap( task_scheduler::bound_object<T>& lhs,
               task_scheduler::bound_object<T>& rhs ) noexcept;

  } // namespace platform
} // namespace bit

#include "detail/task_scheduler.inl"

#endif /* BIT_PLATFORM_CONCURRENCY_SCHEDULERS_TASK_SCHEDULER_HPP */
