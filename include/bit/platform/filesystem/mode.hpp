/**
 * \file mode.hpp
 *
 * \brief This header contains the mode for file types
 *
 * \note This is an internal header file, included by other library headers.
 *       Do not attempt to use it directly.
 */
#ifndef BIT_PLATFORM_FILESYSTEM_MODE_HPP
#define BIT_PLATFORM_FILESYSTEM_MODE_HPP

namespace bit {
  namespace platform {

    /// \brief Enumeration of file modes
    enum class mode {
      read   = 0x1, //!< read
      write  = 0x2, //!< write
      append = 0x4,
    };

    //------------------------------------------------------------------------
    // Operations
    //------------------------------------------------------------------------

    mode operator | (mode lhs, mode rhs) noexcept;
    mode operator & (mode lhs, mode rhs) noexcept;
    mode operator ^ (mode lhs, mode rhs) noexcept;

    mode& operator |= (mode& lhs, mode rhs) noexcept;
    mode& operator &= (mode& lhs, mode rhs) noexcept;
    mode& operator ^= (mode& lhs, mode rhs) noexcept;

  } // namespace platform
} // namespace bit

#include "detail/mode.inl"

#endif /* BIT_PLATFORM_FILESYSTEM_MODE_HPP */
