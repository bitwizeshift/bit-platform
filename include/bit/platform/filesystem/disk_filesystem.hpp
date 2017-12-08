/**
 * \file disk_filesystem.hpp
 *
 * \brief todo: fill in documentation
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_FILESYSTEM_DISK_FILESYSTEM_HPP
#define BIT_PLATFORM_FILESYSTEM_DISK_FILESYSTEM_HPP

#include "detail/tags.hpp"

#include "mode.hpp"
#include "file.hpp"

// bit::memory headers
#include <bit/memory/allocators/any_allocator.hpp>

// bit::stl headers
#include <bit/stl/containers/span.hpp>
#include <bit/stl/containers/string_view.hpp>
#include <bit/stl/utilities/types.hpp>

// std headers
#include <memory>


namespace bit {
  namespace platform {

    ///////////////////////////////////////////////////////////////////////////
    /// \brief A representation of a stateful filesystem type
    ///
    /// This is capable of opening files using adapters provided by
    /// file_devices.
    ///////////////////////////////////////////////////////////////////////////
    class disk_filesystem
    {
      //-----------------------------------------------------------------------
      // Public Member Types
      //-----------------------------------------------------------------------
    public:

      using size_type  = std::size_t;
      using index_type = stl::index_t;

      //-----------------------------------------------------------------------
      // Constructor
      //-----------------------------------------------------------------------
    public:

      /// \brief Default-constructs a filesystem
      disk_filesystem() noexcept;

      /// \brief Constructs a filesystem, using the specified \p allocator
      ///        for performing the allocations
      ///
      /// \param allocator the allocator for this filesystem
      explicit disk_filesystem( memory::any_allocator allocator ) noexcept;

      /// \brief Move-constructs this disk_filesystem from an existing instance
      ///
      /// \param other the other filesystem to move
      disk_filesystem( disk_filesystem&& other ) noexcept = default;

      // Deleted copy constructor
      disk_filesystem( const disk_filesystem& ) = delete;

      //----------------------------------------------------------------------

      // Deleted move assignment
      disk_filesystem& operator=( disk_filesystem&& ) = delete;

      // Deleted copy assignment
      disk_filesystem& operator=( const disk_filesystem& ) = delete;

      //----------------------------------------------------------------------
      // File Opening
      //----------------------------------------------------------------------
    public:

      /// \{
      /// \brief Opens a file synchronously at the given \p path, with the
      ///        specified \p devices.
      ///
      /// \param tag a tag used for tag dispatch to use synchronous file API
      /// \param path the path to the file to open
      /// \param access the file access to use
      /// \return the opened file
      file open( stl::string_view path, mode access );
      file open( sync_t tag, stl::string_view path, mode access );
      /// \}

      /// \brief Opens a file to read asynchronously at the given \p path, with the
      ///        specified \p devices.
      ///
      /// \param tag a tag used for tag dispatch to use asynchronous file API
      /// \param path the path to the file to open
      /// \param devices the devices to use to open the file
      /// \param access the file access to use
      /// \return the opened file
//      async_file open( async_t tag, stl::string_view path, mode access );

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
//      void close( async_file& f );

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    private:

      memory::any_allocator m_allocator;

      //----------------------------------------------------------------------
      // Destruction
      //----------------------------------------------------------------------
    private:

      void destroy_file(void* p);

      void destroy_async_file(void* p);
    };

  } // namespace platform
} // namespace bit

inline bit::platform::file
  bit::platform::disk_filesystem::open( stl::string_view path, mode access )
{
  return open( sync, path, access );
}


#endif /* BIT_PLATFORM_FILESYSTEM_DISK_FILESYSTEM_HPP */
