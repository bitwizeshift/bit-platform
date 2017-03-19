/**
 * \file file_device.hpp
 *
 * \brief This header contains a non-owning 'file_device' type along with
 *        a device_list
 *
 * \note This is an internal header file, included by other library headers.
 *       Do not attempt to use it directly.
 */
#ifndef BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_FILE_DEVICE_HPP
#define BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_FILE_DEVICE_HPP

#include <bit/stl/string_view.hpp>

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief This abstract class represents a device that creates files.
    ///
    /// Files can either be constructed from path identifiers with their
    /// access mode, or can be piggybacked on top of other file types
    //////////////////////////////////////////////////////////////////////////
    class file_device
    {
      //----------------------------------------------------------------------
      // Destructor
      //----------------------------------------------------------------------
    public:

      /// \brief Virtual destructor
      virtual ~file_device() = 0;

      //----------------------------------------------------------------------
      // Synchronous Files
      //----------------------------------------------------------------------
    public:

      /// \brief Opens a file at the given \p path with the given mode \p m
      ///
      /// \param path the path to the file to open
      /// \param m the mode of the file
      /// \return a pointer to the constructed file
      virtual abstract_file* open( stl::string_view path, mode m );

      /// \brief Piggybacks a file with another file
      ///
      /// \param back the file to piggyback onto
      /// \return a pointer to the constructed file
      virtual abstract_file* piggyback( abstract_file* back );

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
      virtual async_file* open_async( stl::string_view path, mode m );

      /// \brief Piggybacks an async file with another file
      ///
      /// \param back the file to piggyback onto
      /// \return a pointer to the constructed file
      virtual async_file* piggyback_async( async_file* back );

      //----------------------------------------------------------------------
      // Element Access
      //----------------------------------------------------------------------
    public:

      /// \brief Gets the identifier for this file_device
      ///
      /// \return the string identifier for the file_device
      virtual stl::string_view identifier() const noexcept = 0;
    };

  } // namespace platform
} // namespace bit

#include "file_device.inl"

#endif /* BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_FILE_DEVICE_HPP */
