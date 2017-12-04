/**
 * \file file.hpp
 *
 * \brief This header contains the wrapper type 'file' which binds itself
 *        around the internal file type, 'abstract_file' to give a unique,
 *        value-interface
 *
 * \note This is an internal header file, included by other library headers.
 *       Do not attempt to use it directly.
 */

#ifndef BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_FILE_HPP
#define BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_FILE_HPP

#include <bit/stl/containers/span.hpp>

namespace bit {
  namespace platform {

    class filesystem;

    //////////////////////////////////////////////////////////////////////////
    /// \brief A value-based representation of a file.
    ///
    /// This is an owning-wrapper around the abstract_file type.
    //////////////////////////////////////////////////////////////////////////
    class file
    {
      //----------------------------------------------------------------------
      // Public Members
      //----------------------------------------------------------------------
    public:

      using size_type = abstract_file::size_type;
      using index_type = abstract_file::index_type;

      //----------------------------------------------------------------------
      // Constructors / Destructor / Assignment
      //----------------------------------------------------------------------
    public:

      /// \brief Constructs a file from an underlying abstract file
      ///
      /// \param file the file to take ownership of
      explicit file( abstract_file* file, filesystem* filesystem );

      /// \brief Move constructs this file from another file
      ///
      /// \param other the other file to move
      file( file&& other );

      /// deleted copy constructor
      file( const file& ) = delete;

      //----------------------------------------------------------------------

      /// \brief Closes this file on destruction via RAII
      ~file();

      //----------------------------------------------------------------------

      /// \brief Move assigns \p other to this file
      ///
      /// \param other the other file to move
      file& operator = ( file&& other );

      /// deleted copy assignment
      file& operator = ( const file& other ) = delete;

      //----------------------------------------------------------------------
      // File API
      //----------------------------------------------------------------------
    public:

      /// \brief Closes this file if it's currently open
      void close();

      /// \brief Reads data into the specified \p buffer
      ///
      /// \param buffer a span of bytes to write into
      /// \return a span containing the bytes read
      stl::span<stl::byte> read( stl::span<stl::byte> buffer );

      /// \brief Writes data into the specified \p buffer
      ///
      /// \param buffer a span of bytes to write to the file
      /// \return a span containing the bytes written
      stl::span<const stl::byte> write( stl::span<const stl::byte> buffer );

      //----------------------------------------------------------------------
      // Observers
      //----------------------------------------------------------------------
    public:

      /// \brief Checks whether this file is open
      ///
      /// \return \c true if the file is open
      bool is_open() const noexcept;

      /// \copydoc is_open()
      explicit operator bool() const noexcept;

      /// \brief Returns the current position in the file
      ///
      /// \return the current position in the file
      index_type tell() const;

      //----------------------------------------------------------------------
      // Seeking
      //----------------------------------------------------------------------
    public:

      /// \brief Seeks to the specified position in the file
      ///
      /// \param pos the position to seek to
      void seek( index_type pos );

      /// \brief Seeks to the beginning of the file
      void seek_to_beginning();

      /// \brief Seeks to the end of the file
      void seek_to_end();

      /// \brief Skips the number of bytes from the current position
      ///
      /// \param bytes the number of bytes to skip
      void skip( index_type bytes );

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    private:

      abstract_file* m_file;       ///< Instance of the abstract file
      filesystem*    m_filesystem; ///< Pointer to the filesystem

      friend class filesystem;

      //----------------------------------------------------------------------
      // Private Member Functions
      //----------------------------------------------------------------------
    private:

      /// \brief Gets the underlying abstract_file
      ///
      /// \return a pointer to the underlying file
      abstract_file* underlying() noexcept;

    };

  } // namespace platform
} // namespace bit

#include "file.inl"

#endif /* BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_FILE_HPP */
