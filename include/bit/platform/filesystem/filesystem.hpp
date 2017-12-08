/**
 * \file filesystem.hpp
 *
 * \brief todo: fill in documentation
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_FILESYSTEM_FILESYSTEM_HPP
#define BIT_PLATFORM_FILESYSTEM_FILESYSTEM_HPP


// bit::stl headers
#include <bit/stl/containers/span.hpp>
#include <bit/stl/containers/string_view.hpp>
#include <bit/stl/traits/composition/negation.hpp>
#include <bit/stl/traits/composition/conjunction.hpp>

// std headers
#include <memory>

// Implementation types
#include "detail/filesystem/mode.hpp"
#include "detail/filesystem/abstract_file.hpp"
#include "detail/filesystem/file.hpp"
#include "detail/filesystem/async_file.hpp"
#include "detail/filesystem/file_device.hpp"

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief A representation of a stateful filesystem type
    ///
    /// This is capable of opening files using adapters provided by
    /// file_devices.
    //////////////////////////////////////////////////////////////////////////
    class filesystem
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

      /// \brief Constructs a filesystem that reserves spaces for \p devices
      ///
      /// \param devices the number of devices
      explicit filesystem( size_type devices );

      //----------------------------------------------------------------------
      // Devices
      //----------------------------------------------------------------------
    public:

      /// \brief Mounts a \p device to this \c filesystem
      ///
      /// \note mounted file_devices only observe the pointer; they do not
      ///       take ownership. Ideally, these instances would exist on the
      ///       stack and be passed by pointer, rather than constructed on the
      ///       heap
      ///
      /// \param device a pointer to the file_device to mount to this
      ///        filesystem
      void mount( file_device* device );

      /// \brief Mounts multiple devices to this \c filesystem
      ///
      /// \param device0 a pointer to the first file_device to mount
      /// \param device1 a pointer to the second file_device to mount
      /// \param devices variadic pack of pointers to mount
      template<typename...Devices, std::enable_if_t<stl::conjunction<std::is_base_of<file_device,Devices>...>::value>* = nullptr>
      void mount( file_device* device0, file_device* device1, Devices*... devices );

      /// \brief Unmounts a \p device to this \c filesystem
      ///
      /// \param device a pointer to the file_device to unmount from this
      ///        filesystem
      void unmount( file_device* device );

      /// \brief Unmounts multiple devices from this \c filesystem
      ///
      /// \param device0 a pointer to the first file_device to unmount
      /// \param device1 a pointer to the second file_device to immount
      /// \param devices variadic pack of pointers to unmount
      template<typename...Devices, std::enable_if_t<stl::conjunction<std::is_base_of<file_device,Devices>...>::value>* = nullptr>
      void unmount( file_device* device0, file_device* device1, Devices*... devices );

      /// \brief The current number of installed devices
      ///
      /// \return the number of devices
      size_type devices() const noexcept;

      /// \brief The number of devices that can be installed
      ///
      /// \return the max number of devices that can be installed
      size_type device_capacity() const noexcept;

      //----------------------------------------------------------------------
      // File Opening
      //----------------------------------------------------------------------
    public:

      /// \brief Opens a file synchronously at the given \p path, with the
      ///        specified \p devices.
      ///
      /// \param path the path to the file to open
      /// \param devices the devices to use to open the file
      /// \param access the file access to use
      ///abstract_fileturn the opened file
      template<std::size_t N>
      file open( stl::string_view path, const char*const (&devices)[N], mode access );

      /// \brief Opens a file synchronously at the given \p path, with the
      ///        specified \p devices.
      ///
      /// \note This is the same as calling open without a tag
      ///
      /// \param tag a tag used for tag dispatch to use synchronous file API
      /// \param path the path to the file to open
      /// \param devices the devices to use to open the file
      /// \param access the file access to use
      ///abstract_fileturn the opened file
      template<std::size_t N>
      file open( sync_t tag, stl::string_view path, const char*const (&devices)[N], mode access );

      /// \brief Opens a file to read asynchronously at the given \p path, with the
      ///        specified \p devices.
      ///
      /// \param tag a tag used for tag dispatch to use asynchronous file API
      /// \param path the path to the file to open
      /// \param devices the devices to use to open the file
      /// \param access the file access to use
      /// \return the opened file
      template<std::size_t N>
      async_file open( async_t tag, stl::string_view path, const char*const (&devices)[N], mode access );

      //----------------------------------------------------------------------
      // File Closing
      //----------------------------------------------------------------------
    public:

      /// \brief Closes a synchronous file \p f
      ///
      /// \param f the fabstract_fileto close
      void close( file& f );

      /// \brief Closes an asynchronous file \p f
      ///
      /// \param f the asynchronous file to close
      void close( async_file& f );

      /// \brief Closes all open files within this filesystem at once
      ///
      /// Any existing file handles that have not yet been closed are
      /// immediately invalidated.
      void close_all();

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    public:

      std::unique_ptr<file_device*[]> m_devices;
      std::size_t                     m_size;
      stl::index_t                    m_current;
      // memory::allocator m_allocator;

      //----------------------------------------------------------------------
      // Private Member Functions
      //----------------------------------------------------------------------
    public:

      file open( sync_t tag, stl::string_view path, const char*const* devices,
                 std::size_t size, mode access );

      async_file open( async_t tag, stl::string_view path, const char*const* devices,
                       std::size_t size, mode access );

      //----------------------------------------------------------------------

      /// \brief Recursively closes and destructs an underlying abstract_file
      ///
      /// \param is_allocated
      /// \param f the file
      void destroy_file( abstract_file* f );

    };

  } // namespace platform
} // namespace bit

#include "detail/filesystem.inl"

#endif /* BIT_PLATFORM_FILESYSTEM_FILESYSTEM_HPP */
