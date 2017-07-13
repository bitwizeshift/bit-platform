/**
 * \file concurrent_queue.hpp
 *
 * \brief This header contains an implementation of a thread-safe queue
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */

#ifndef BIT_PLATFORM_THREADING_CONCURRENT_QUEUE_HPP
#define BIT_PLATFORM_THREADING_CONCURRENT_QUEUE_HPP

#include "spin_lock.hpp"
#include <mutex>
#include <condition_variable>
#include <queue>

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \class bit::platform::concurrent_queue
    ///
    /// \brief This class is an implementation of a thread-safe concurrent
    ///        queue that supports thread-safe operations.
    ///
    /// The default locking mechanism uses a spin-lock, but can be replaced
    /// with heavier locking-mechanisms such as mutexes.
    ///
    /// \tparam T the type of the queue
    /// \tparam Lock the type of lock for this current queue
    /// \tparam Allocator the underlying allocator used for creating entries
    //////////////////////////////////////////////////////////////////////////
    template<typename T,
             typename Lock = spin_lock,
             typename Allocator = std::allocator<T>>
    class concurrent_queue
    {
      static_assert( !std::is_reference<T>::value, "T cannot be a reference type" );

      //----------------------------------------------------------------------
      // Public Member Types
      //----------------------------------------------------------------------
    public:

      using value_type      = T;
      using reference       = T&;
      using const_reference = const T&;
      using pointer         = T*;
      using const_pointer   = const T*;

      using lock_type       = Lock;
      using allocator_type  = Allocator;
      using size_type       = std::size_t;

      //----------------------------------------------------------------------
      // Constructors / Assignment
      //----------------------------------------------------------------------
    public:

      /// \brief Default constructs a concurrent_queue
      explicit concurrent_queue();

      /// \brief Constructs a concurrent_queue that uses the specified allocator
      ///
      /// \param alloc the allocator to use
      explicit concurrent_queue( const Allocator& alloc );

      /// \brief Move-constructs a concurrent queue
      ///
      /// \param other the other concurrent_queue to move
      concurrent_queue( concurrent_queue&& other );

      /// \brief Move-constructs a concurrent queue with a new allocator
      ///
      /// \param other the other concurrent_queue to move
      /// \param alloc the allocator to use
      concurrent_queue( concurrent_queue&& other,
                        const Allocator& alloc );

      /// Deleted copy constructor
      concurrent_queue( const concurrent_queue& ) = delete;

      //----------------------------------------------------------------------

      /// \brief Move-assigns another queue to this concurrent queue
      ///
      /// \note This operator is not threadsafe since the contents are being
      ///       moved from one queue to another.
      ///
      /// \param other the other queue to copy
      /// \return reference to \c (*this)
      concurrent_queue& operator=( concurrent_queue&& other ) noexcept;

      /// Deleted copy assignment
      concurrent_queue& operator=( const concurrent_queue& ) = delete;

      //----------------------------------------------------------------------
      // Capacity
      //----------------------------------------------------------------------
    public:

      /// \brief Returns whether this queue is empty
      ///
      /// \note this function is not thread safe, since the value returned
      ///       can easily be different between the time it's retrieved vs
      ///       the time it's read
      /// \return \c true if this queue is empty
      bool empty() const noexcept;

      /// \brief Returns the size of this queue
      ///
      /// \note this function is not thread safe, since the value returned
      ///       can easily be different between the time it's retrieved vs
      ///       the time it's read
      /// \return the size of this queue
      size_type size() const noexcept;

      //----------------------------------------------------------------------
      // Observers
      //----------------------------------------------------------------------
    public:

      /// \brief Gets the underlying allocator from this concurrent queue
      ///
      /// \return the allocator
      Allocator get_allocator() const;

      //----------------------------------------------------------------------
      // Element Access
      //----------------------------------------------------------------------
    public:

      /// \brief Pops the front element in the queue, blocking until one is
      ///        available
      ///
      /// The result is stored in the entry pointed to by \p value
      ///
      /// \note This uses move-assignment to store the result
      /// \param pointer to the entry to store the result
      void pop( T* value );

      //----------------------------------------------------------------------

      /// \brief Attempts to pop the front element in the queue, returning
      ///        immediately on failure
      ///
      /// The result is stored in the entry pointed to by \p value
      ///
      /// \note This uses move-assignment to store the result
      /// \param pointer to the entry to store the result
      /// \return \c true if a value was acquired
      bool try_pop( T* value );

      //----------------------------------------------------------------------
      // Modifiers
      //----------------------------------------------------------------------
    public:

      /// \brief Pushes an entry into the queue
      ///
      /// This function blocks until it is able to push the entry
      ///
      /// \param value the value to push
      void push_back( const T& value );

      /// \copydoc concurrent_queue::push_back
      void push_back( T&& value );

      /// \brief Emplaces an entry in the queue
      ///
      /// \param args the arguments to construct the entry
      template<typename...Args, std::enable_if_t<std::is_constructible<T,Args...>::value>* = nullptr>
      void emplace_back( Args&&...args );

      //----------------------------------------------------------------------

      /// \brief Attempts to push an entry into the queue, returning
      ///        immediately on failure
      ///
      /// \param value the value to push back
      /// \return \c true if the value was inserted
      bool try_push_back( const T& value );

      /// \copydoc concurrent_queue::try_push_back
      bool try_push_back( T&& value );

      /// \brief Attempts to emplace an entry into the queue, returning
      ///        immediately on failure
      ///
      /// \param args the arguments to construct the entry
      /// \return \c true if the value was inserted
      template<typename...Args, std::enable_if_t<std::is_constructible<T,Args...>::value>* = nullptr>
      bool try_emplace_back( Args&&...args );

      //----------------------------------------------------------------------

      /// \brief Clears this concurrent queue of all entries
      void clear();

      /// \brief Swaps this concurrent_queue with another queue
      ///
      /// \param other the other queue to swap with
      void swap( concurrent_queue& other ) noexcept;

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    private:

      using base_type = std::queue<T,std::deque<T,Allocator>>;

      class container : public base_type
      {
        using base_type::base_type;

        container( const container& other ) = default;
        container( container&& other ) = default;
        container& operator=( const container& other ) = default;
        container& operator=( container&& other ) = default;

        Allocator get_allocator() const
        {
          return base_type::c.get_allocator();
        }
      };

      container               m_queue; ///< The underlying queue
      lock_type               m_lock;  ///< The type of lock to wait on
      std::condition_variable m_cv;    ///< A condition to wait on
    };

    //------------------------------------------------------------------------
    // Free Functions
    //------------------------------------------------------------------------

    /// \brief Swaps the content of \p lhs with \p rhs
    ///
    /// \param lhs the left concurrent queue
    /// \param rhs the right concurrent queue
    template<typename T, typename Lock, typename Alloc>
    void swap( concurrent_queue<T,Lock,Alloc>& lhs, concurrent_queue<T,Lock,Alloc>& rhs );

  } // namespace platform
} // namespace bit

#include "detail/concurrent_queue.inl"

#endif /* BIT_PLATFORM_THREADING_CONCURRENT_QUEUE_HPP */
