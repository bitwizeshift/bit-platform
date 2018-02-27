/**
 * \file true_share.hpp
 *
 * \brief This header contains a type used for promoting true-sharing
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_CONCURRENCY_UTILITIES_TRUE_SHARE_HPP
#define BIT_PLATFORM_CONCURRENCY_UTILITIES_TRUE_SHARE_HPP

#include <cstddef>     // std::size_t
#include <type_traits> // std::is_reference, std::is_void
#include <utility>     // std::forward

namespace bit {
  namespace platform {

    /// \brief Gets the optimal cache-line size used for avoiding false-sharing
    ///
    /// \return the cache line size
    constexpr std::size_t cache_line_size() noexcept;

    ///////////////////////////////////////////////////////////////////////////
    /// \brief A wrapper type used to promote true-sharing by aligning the data
    ///        to a boundary equal to the cache line size
    ///
    /// This type has all the same constructors as the underlying type,
    /// which allows unmovable/uncopyable types to be constructed in-place.
    ///
    /// Accessing the underlying entry is done via either 'get()' or the
    /// implicit conversion to reference operator, which behaves similar to
    /// std::reference_wrapper
    ///////////////////////////////////////////////////////////////////////////
    template<typename T>
    class true_share
    {
      static_assert(!std::is_reference<T>::value, "T cannot be a reference type");
      static_assert(!std::is_void<T>::value, "T cannot be void");

      //-----------------------------------------------------------------------
      // Public Member Types
      //-----------------------------------------------------------------------
    public:

      using value_type = T;

      //-----------------------------------------------------------------------
      // Constructors
      //-----------------------------------------------------------------------
    public:

      template<typename Arg0, typename...Args, typename = std::enable_if_t<!std::is_same<std::decay_t<Arg0>,true_share>::value>>
      constexpr true_share( Arg0&& arg0, Args&&...args );

      //-----------------------------------------------------------------------
      // Conversions
      //-----------------------------------------------------------------------
    public:

      /// \brief Returns a reference to the underlying type
      ///
      /// \return a reference to the underlying type
      constexpr operator T&() const noexcept;
      constexpr value_type& get() const noexcept;

      //-----------------------------------------------------------------------
      // Private Members
      //-----------------------------------------------------------------------
    private:

      /// \brief Function to compute the largest alignment between a cache line
      ///        and the underlying type T
      static constexpr std::size_t max_align() noexcept;

      alignas(max_align()) T m_entry;
    };

  } // namespace platform
} // namespace bit

#include "detail/true_share.inl"

#endif /* BIT_PLATFORM_CONCURRENCY_UTILITIES_TRUE_SHARE_HPP */
