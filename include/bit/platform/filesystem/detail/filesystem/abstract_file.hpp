/**
 * \file abstract_file.hpp
 *
 * \brief This header contains the base class 'abstract_file' type used
 *        internally to the filesystem
 *
 * \note This is an internal header file, included by other library headers.
 *       Do not attempt to use it directly.
 */
#ifndef BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_ABSTRACT_FILE_HPP
#define BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_ABSTRACT_FILE_HPP

#include <bit/stl/stddef.hpp>
#include <bit/stl/span.hpp>

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief An intermediate abstract base type held internally to the
    ///        filesystem.
    //////////////////////////////////////////////////////////////////////////
    class abstract_file
    {
      //----------------------------------------------------------------------
      // Public Members
      //----------------------------------------------------------------------
    public:

      using size_type  = std::size_t;  ///< Type used to indicate sizes
      using index_type = stl::index_t; ///< Type used for indices

      //----------------------------------------------------------------------
      // Destructor
      //----------------------------------------------------------------------
    public:

      virtual ~abstract_file() = 0;

      //----------------------------------------------------------------------
      // File API
      //----------------------------------------------------------------------
    public:

      /// \brief Closes this file if it's currently open
      virtual void close() = 0;

      /// \brief Reads data into the specified \p buffer
      ///
      /// \param buffer a span of bytes to write into
      /// \return a span containing the bytes read
      virtual size_type read( stl::span<stl::byte> buffer ) = 0;

      /// \brief Writes data into the specified \p buffer
      ///
      /// \param buffer a span of bytes to write to the file
      /// \return a span containing the bytes written
      virtual size_type write( stl::span<const stl::byte> buffer ) = 0;

      //----------------------------------------------------------------------
      // Observers
      //----------------------------------------------------------------------
    public:

      /// \brief Returns the current position in the file
      ///
      /// \return the current position in the file
      virtual index_type tell() const = 0;

      //----------------------------------------------------------------------
      // Seeking
      //----------------------------------------------------------------------
    public:

      /// \brief Seeks to the specified position in the file
      ///
      /// \param pos the position to seek to
      virtual void seek( index_type pos ) = 0;

      /// \brief Seeks to the end of the file
      virtual void seek_to_end() = 0;

      /// \brief Skips the number of bytes from the current position
      ///
      /// \param bytes the number of bytes to skip
      virtual void skip( index_type bytes ) = 0;

      /// \brief The next file in the chain, if on exists.
      ///
      /// This defaults to \c nullptr
      ///
      /// \return the next abstract_file
      virtual abstract_file* next();
    };

    //------------------------------------------------------------------------
    // Type Traits
    //------------------------------------------------------------------------

    /// \brief Type trait to determine whether the given type is a file
    ///
    /// The result is aliased as \c ::value
    template<typename T>
    struct is_abstract_file : std::false_type{};

    template<>
    struct is_abstract_file<abstract_file> : std::true_type{};

  } // namespace platform
} // namespace bit

inline bit::platform::abstract_file::~abstract_file() = default;

inline bit::platform::abstract_file* bit::platform::abstract_file::next()
{
  return nullptr;
}

#endif /* BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_ABSTRACT_FILE_HPP */
