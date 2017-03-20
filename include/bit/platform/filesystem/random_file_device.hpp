/**
 * \file random_file_device.hpp
 *
 * \brief This header contains a file_device that behaves similar to reading
 *        from /dev/random or /dev/urandom
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */

#ifndef BIT_PLATFORM_FILESYSTEM_RANDOM_FILE_DEVICE_HPP
#define BIT_PLATFORM_FILESYSTEM_RANDOM_FILE_DEVICE_HPP

#include "filesystem.hpp"

#include <random>

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief file_device that exists only to read random data.
    ///
    /// Essentially, a random_file_device acts as though it is reading to and
    /// writing from /dev/random on *nix systems.
    //////////////////////////////////////////////////////////////////////////
    class random_file_device : public file_device
    {
      //----------------------------------------------------------------------
      // Constructors
      //----------------------------------------------------------------------
    public:

      /// \brief Default constructs a random_file_device
      random_file_device();

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

      /// \brief Returns the identifier for this random_file_device
      ///
      /// \return the identifier
      stl::string_view identifier() const noexcept override;

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    private:

      std::random_device m_device;
    };

  } // namespace platform
} // namespace bit

#endif /* BIT_PLATFORM_FILESYSTEM_RANDOM_FILE_DEVICE_HPP */
