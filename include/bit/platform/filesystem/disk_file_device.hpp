/**
 * \file disk_file_device.hpp
 *
 * \brief This header contains an implementation of a low-level filesystem
 *        disk-device.
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */

#ifndef BIT_PLATFORM_FILESYSTEM_DISK_FILE_DEVICE_HPP
#define BIT_PLATFORM_FILESYSTEM_DISK_FILE_DEVICE_HPP

#include "filesystem.hpp"

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief A file_device that provides low-level disk-access to a given
    ///        file.
    ///
    /// This uses the underlying OS's filesystem API to open and use files.
    ///
    /// Disk devices do not piggyback, and will assert.
    //////////////////////////////////////////////////////////////////////////
    class disk_file_device final : public file_device
    {
      //----------------------------------------------------------------------
      // Constructors
      //----------------------------------------------------------------------
    public:

      /// \brief Default constructs a null_file_device
      disk_file_device() = default;

      //----------------------------------------------------------------------
      // Synchronous Files
      //----------------------------------------------------------------------
    public:

      /// \brief Opens a file at the given \p path with the given mode \p m
      ///
      /// \param path the path to the file to open
      /// \param m the mode of the file
      /// \return a pointer to the constructed file
      virtual abstract_file* open( stl::string_view path, mode m ) override;

      //----------------------------------------------------------------------
      // Asynchronous Files
      //----------------------------------------------------------------------
    public:

      /// \brief Opens an async file at the given \p path with the given
      ///        mode \p m
      ///
      /// \param path the path to the file to open
      /// \param m the mode of the file
      /// \return a pointer to the constructed file
      async_file* open_async( stl::string_view path, mode m ) override;

      //----------------------------------------------------------------------
      // Element Access
      //----------------------------------------------------------------------
    public:

      /// \brief Returns the identifier for this not_null_device
      ///
      /// \return the identifier
      stl::string_view identifier() const noexcept override;
    };

  } // namespace platform
} // namespace bit

#endif /* BIT_PLATFORM_FILESYSTEM_DISK_FILE_DEVICE_HPP */
