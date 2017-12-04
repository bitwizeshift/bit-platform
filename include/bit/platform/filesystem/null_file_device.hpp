/**
 * \file null_file_device.hpp
 *
 * \brief todo: fill in documentation
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_FILESYSTEM_NULL_FILE_DEVICE_HPP
#define BIT_PLATFORM_FILESYSTEM_NULL_FILE_DEVICE_HPP

#include "filesystem.hpp"

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief file_device that conforms to the null-object pattern
    ///
    /// All files constructed from a null_file_device return null data, and
    /// are unable to write any data. Any files piggybacked through a
    /// null_file_device simply return the input
    ///
    /// Essentially, a null_file_device acts as though it is reading to and
    /// writing from /dev/null on *nix systems.
    //////////////////////////////////////////////////////////////////////////
    class null_file_device final : public file_device
    {
      //----------------------------------------------------------------------
      // Constructors
      //----------------------------------------------------------------------
    public:

      /// \brief Default constructs a null_file_device
      null_file_device() = default;

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

      /// \brief Piggybacks a file with another file
      ///
      /// \param back the file to piggyback onto
      /// \return a pointer to the constructed file
      virtual abstract_file* piggyback( abstract_file* back ) override;

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

#endif /* BIT_PLATFORM_FILESYSTEM_NULL_FILE_DEVICE_HPP */
