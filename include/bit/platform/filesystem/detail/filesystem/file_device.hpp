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

      virtual ~file_device() = 0;

      //----------------------------------------------------------------------
      // Synchronous Files
      //----------------------------------------------------------------------
    public:

      /// \brief
      ///
      /// \param path the path to the file to open
      /// \param m the mode of the file
      /// \return
      virtual abstract_file* open( stl::string_view path, mode m );

      virtual abstract_file* piggyback( abstract_file* back );

      //----------------------------------------------------------------------
      // Asynchronous Files
      //----------------------------------------------------------------------
    public:

      virtual async_file open_async( stl::string_view path, mode m );

      virtual async_file piggyback_async( async_file back );

      //----------------------------------------------------------------------
      // Element Access
      //----------------------------------------------------------------------
    public:

      virtual stl::string_view identifier() const noexcept = 0;
    };

    //////////////////////////////////////////////////////////////////////////
    /// \brief A non-owning list constructible from a temporary array at
    ///        compile time.
    ///
    /// This is used as a device-specifier for file devices in the filesystem
    //////////////////////////////////////////////////////////////////////////
    class device_list
    {
      //----------------------------------------------------------------------
      // Public Member Types
      //----------------------------------------------------------------------
    public:

      using size_type  = std::size_t;
      using index_type = stl::index_t;

      //----------------------------------------------------------------------
      // Constructor
      //----------------------------------------------------------------------
    public:

      /// \brief Constructs a device_list from a given array of ansi strings
      ///
      /// \param data the array of ansi strings
      template<std::size_t N>
      device_list( const char* const (&data)[N] );

      /// \brief Copy-constructs a device_list from another device_list
      ///
      /// \param other the other device_list to copy
      device_list( const device_list& other ) = default;

      /// \brief Move-constructs a device_list from another device_list
      ///
      /// \param other the other device_list to move
      device_list( device_list&& other ) = default;

      //----------------------------------------------------------------------
      // Capacity
      //----------------------------------------------------------------------
    public:

      /// \brief Returns whether this device_list is empty
      ///
      /// \return \c true if this device_list is empty
      bool empty() const noexcept;

      /// \brief Returns the size of this device_list
      ///
      /// \return the size of this device_list
      size_type size() const noexcept;

      //----------------------------------------------------------------------
      // Element Access
      //----------------------------------------------------------------------
    public:

      /// \brief Access the element in this device_list at index \p i
      ///
      /// \param i the index
      /// \return the string at index \p i
      stl::string_view operator[]( index_type i ) const noexcept;

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    private:

      const char* const * m_data;
      size_type           m_size;
    };

  } // namespace platform
} // namespace bit

#include "file_device.inl"

#endif /* BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_FILE_DEVICE_HPP */
