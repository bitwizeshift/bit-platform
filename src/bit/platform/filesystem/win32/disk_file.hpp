/**
 * \file disk_file.hpp
 *
 * \brief This header contains an implementation of an underlying system file
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef SRC_BIT_PLATFORM_FILESYSTEM_WIN32_DISK_FILE_HPP
#define SRC_BIT_PLATFORM_FILESYSTEM_WIN32_DISK_FILE_HPP

#include <bit/platform/filesystem/filesystem.hpp>

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief A low-level disk device implementation for Windows
    //////////////////////////////////////////////////////////////////////////
    class disk_file : public abstract_file
    {
      //----------------------------------------------------------------------
      // Public Member Types
      //----------------------------------------------------------------------
    public:

      using size_type  = abstract_file::size_type;
      using index_type = abstract_file::index_type;

      //----------------------------------------------------------------------
      // Constructor
      //----------------------------------------------------------------------
    public:

      /// \brief Opens a disk_file at the given \p path with mode \p m
      ///
      /// \param path the path to the file
      /// \param m the mode of the file
      explicit disk_file( stl::string_view path, mode m );

      //----------------------------------------------------------------------
      // File API
      //----------------------------------------------------------------------
    public:

      /// \brief Closes this null_file if it's currently open
      void close() override;

      /// \brief Reads data into the specified \p buffer
      ///
      /// \note a null_file never reads any data
      ///
      /// \param buffer a span of bytes to write into
      /// \return a span containing the bytes read
      size_type read( stl::span<stl::byte> buffer ) override;

      /// \brief Writes data into the specified \p buffer
      ///
      /// \note a null_file never writes any data
      ///
      /// \param buffer a span of bytes to write to the file
      /// \return a span containing the bytes written
      size_type write( stl::span<const stl::byte> buffer ) override;

      //----------------------------------------------------------------------
      // Observers
      //----------------------------------------------------------------------
    public:

      /// \brief Returns the current position in the file
      ///
      /// \return the current position in the file
      index_type tell() const override;

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

      using native_handle = void*;

      native_handle m_file; ///< The underlying file handle
    };

  } // namespace platform
} // namespace bit

#endif /* SRC_BIT_PLATFORM_FILESYSTEM_WIN32_DISK_FILE_HPP */
