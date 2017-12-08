/**
 * \file disk_file.hpp
 *
 * \brief This header contains an implementation of an underlying system file
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef SRC_BIT_PLATFORM_FILESYSTEM_WIN32_DISK_FILE_HPP
#define SRC_BIT_PLATFORM_FILESYSTEM_WIN32_DISK_FILE_HPP

#include <bit/platform/filesystem/file.hpp>
#include <bit/platform/filesystem/disk_filesystem.hpp>

#include <bit/memory/allocators/any_allocator.hpp>

#include <bit/stl/containers/string_view.hpp>
#include <bit/stl/containers/span.hpp>
#include <bit/stl/utilities/delegate.hpp>

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief A low-level disk device implementation for Windows
    //////////////////////////////////////////////////////////////////////////
    class disk_file : public file_interface
    {
      //----------------------------------------------------------------------
      // Public Member Types
      //----------------------------------------------------------------------
    public:

      using size_type        = file_interface::size_type;
      using index_type       = file_interface::index_type;
      using native_handle    = void*;
      using destroy_delegate = stl::delegate<void(void*)>;

      //----------------------------------------------------------------------
      // Constructor
      //----------------------------------------------------------------------
    public:

      /// \brief Opens a disk_file at the given \p path with mode \p m
      ///
      /// \param path the path to the file
      /// \param m the mode of the file
      /// \param destroy the function to destroy this disk file
      explicit disk_file( stl::string_view path,
                          mode m,
                          destroy_delegate destroy );

      //----------------------------------------------------------------------
      // File API
      //----------------------------------------------------------------------
    public:

      /// \brief Destroys this disk_file
      void destroy() override;

      /// \brief Reads data into the specified \p buffer
      ///
      /// \note a null_file never reads any data
      ///
      /// \param buffer a span of bytes to write into
      /// \return a span containing the bytes read
      stl::span<char> read( stl::span<char> buffer ) override;

      /// \brief Writes data into the specified \p buffer
      ///
      /// \note a null_file never writes any data
      ///
      /// \param buffer a span of bytes to write to the file
      /// \return a span containing the bytes written
      stl::span<const char> write( stl::span<const char> buffer ) override;

      //----------------------------------------------------------------------
      // Observers
      //----------------------------------------------------------------------
    public:

      /// \brief Returns the current position in the file
      ///
      /// \return the current position in the file
      index_type tell() const override;

      /// \brief Returns the size of the file in bytes
      ///
      /// \return the size of the file in bytes
      size_type size() const override;

      //----------------------------------------------------------------------
      // Seeking
      //----------------------------------------------------------------------
    public:

      /// \brief Seeks to the specified position in the file
      ///
      /// \param pos the position to seek to
      void seek( index_type pos ) override;

      /// \brief Seeks to the end of the file
      void seek_to_end() override;

      /// \brief Skips the number of bytes from the current position
      ///
      /// \param bytes the number of bytes to skip
      void skip( index_type bytes ) override;

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    private:

      native_handle    m_file;    ///< The underlying file handle
      destroy_delegate m_destroy; ///< Function to destroy this disk file
    };

  } // namespace platform
} // namespace bit

#endif /* SRC_BIT_PLATFORM_FILESYSTEM_WIN32_DISK_FILE_HPP */
