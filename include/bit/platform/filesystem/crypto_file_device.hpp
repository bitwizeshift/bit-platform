/**
 * \file crypto_file_device.hpp
 *
 * \brief todo: fill in documentation
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */

#ifndef BIT_PLATFORM_FILESYSTEM_CRYPTO_FILE_DEVICE_HPP
#define BIT_PLATFORM_FILESYSTEM_CRYPTO_FILE_DEVICE_HPP

#include "filesystem.hpp"

namespace bit {
  namespace platform {

    class crypto_file_device final : public file_device
    {
      //----------------------------------------------------------------------
      // Constructors
      //----------------------------------------------------------------------
    public:

      /// \brief Default constructs a crypto_file_device
      crypto_file_device() = default;

      //----------------------------------------------------------------------
      // Synchronous Files
      //----------------------------------------------------------------------
    public:

      /// \brief Piggybacks a file with another file
      ///
      /// \param back the file to piggyback onto
      /// \return a pointer to the constructed file
      virtual abstract_file* piggyback( abstract_file* back ) override;

      //----------------------------------------------------------------------
      // Asynchronous Files
      //----------------------------------------------------------------------
    public:

      /// \brief Piggybacks an async file with another file
      ///
      /// \param back the file to piggyback onto
      /// \return a pointer to the constructed file
      async_file* piggyback_async( async_file* back ) override;

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

#endif /* BIT_PLATFORM_FILESYSTEM_CRYPTO_FILE_DEVICE_HPP */
