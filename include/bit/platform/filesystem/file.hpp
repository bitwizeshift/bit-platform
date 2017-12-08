/**
 * \file file.hpp
 *
 * \brief todo: fill in documentation
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_FILESYSTEM_FILE_HPP
#define BIT_PLATFORM_FILESYSTEM_FILE_HPP

#include <bit/stl/utilities/expected.hpp>
#include <bit/stl/utilities/types.hpp>    // stl::index_t
#include <bit/stl/containers/span.hpp>    // stl::span

#include <cstddef> // std::size_t

namespace bit {
  namespace platform {

    ///////////////////////////////////////////////////////////////////////////
    /// \brief The interface used in constructing file objects
    ///
    ///z
    ///////////////////////////////////////////////////////////////////////////
    class file_interface
    {
      //-----------------------------------------------------------------------
      // Public Members
      //-----------------------------------------------------------------------
    public:

      using size_type  = std::size_t;  ///< Type used to indicate sizes
      using index_type = stl::index_t; ///< Type used for indices

      //-----------------------------------------------------------------------
      // Public Static Members
      //-----------------------------------------------------------------------
    public:

      static constexpr auto invalid_index = index_type(-1);

      //-----------------------------------------------------------------------
      // Destructor
      //-----------------------------------------------------------------------
    public:

      virtual ~file_interface() = 0;

      //-----------------------------------------------------------------------
      // File API
      //-----------------------------------------------------------------------
    public:

      /// \brief Destroys this file, reclaiming any resources
      ///
      /// This function should close any open resources, and deallocate any
      /// allocated memory -- including possibly this instance of
      /// file_interface
      ///
      /// It is assured that, after a call to 'destroy', the file_interface
      /// object will not be used again
      virtual void destroy() = 0;

      /// \brief Reads data into the specified \p buffer
      ///
      /// \param buffer a span of bytes to write into
      /// \return a span containing the bytes read
      virtual stl::span<char> read( stl::span<char> buffer ) = 0;

      /// \brief Writes data into the specified \p buffer
      ///
      /// \param buffer a span of bytes to write to the file
      /// \return a span containing the bytes written
      virtual stl::span<const char> write( stl::span<const char> buffer ) = 0;

      //-----------------------------------------------------------------------
      // Observers
      //-----------------------------------------------------------------------
    public:

      /// \brief Returns the current position in the file
      ///
      /// \return the current position in the file
      virtual index_type tell() const = 0;

      /// \brief Returns the size of the file in bytes
      ///
      /// \return the size of the file in bytes
      virtual size_type size() const = 0;

      //-----------------------------------------------------------------------
      // Seeking
      //-----------------------------------------------------------------------
    public:

      /// \brief Seeks to the specified position in the file
      ///
      /// \param pos the position to seek to
      virtual void seek( index_type pos ) = 0;

      /// \brief Seeks to the end of the file
      virtual void seek_to_end() = 0;

      /// \brief Skips the number of bytes from the current position
      ///
      /// \param bytes the number of bytes to skip
      virtual void skip( index_type bytes );
    };

    ///////////////////////////////////////////////////////////////////////////
    /// \brief A value-based representation of a file.
    ///
    /// This is an owning-wrapper around the abstract_file type.
    ///
    /// Reading any state of a closed or invalid file object other than whether
    /// the file is open is undefined behavior.
    ///////////////////////////////////////////////////////////////////////////
    class file final
    {
      //-----------------------------------------------------------------------
      // Public Members
      //-----------------------------------------------------------------------
    public:

      using size_type  = file_interface::size_type;
      using index_type = file_interface::index_type;

      //-----------------------------------------------------------------------
      // Public Static Members
      //-----------------------------------------------------------------------
    public:

      static constexpr auto invalid_index = file_interface::invalid_index;

      //-----------------------------------------------------------------------
      // Constructors / Destructor / Assignment
      //-----------------------------------------------------------------------
    public:

      /// \brief Constructs a file from an underlying abstract file
      ///
      /// \param file the file to take ownership of
      explicit file( file_interface& file ) noexcept;

      /// \brief Move constructs this file from another file
      ///
      /// \param other the other file to move
      file( file&& other ) noexcept;

      // deleted copy constructor
      file( const file& ) = delete;

      //-----------------------------------------------------------------------

      /// \brief Closes this file on destruction via RAII
      ~file();

      //-----------------------------------------------------------------------

      /// \brief Move assigns \p other to this file
      ///
      /// \param other the other file to move
      file& operator=( file other ) noexcept;

      //-----------------------------------------------------------------------
      // Reading
      //-----------------------------------------------------------------------
    public:

      /// \brief Reads data into the specified \p buffer
      ///
      /// \param buffer a span of bytes to write into
      /// \return a span containing the bytes read
      stl::span<char> read( stl::span<char> buffer );

//      template<typename T, typename=std::enable_if_t<std::is_integral<T>::value>>
//      stl::expected<T> load();

      //-----------------------------------------------------------------------
      // Writing
      //-----------------------------------------------------------------------
    public:

      /// \brief Writes data into the specified \p buffer
      ///
      /// \param buffer a span of bytes to write to the file
      /// \return a span containing the bytes written
      stl::span<const char> write( stl::span<const char> buffer );

      //-----------------------------------------------------------------------
      // Observers
      //-----------------------------------------------------------------------
    public:

      /// \brief Checks whether this file is open
      ///
      /// \return \c true if the file is open
      bool is_open() const noexcept;

      /// \copydoc is_open()
      explicit operator bool() const noexcept;

      /// \brief Returns the current position in the file
      ///
      /// \return the current position in the file
      index_type tell() const;

      //-----------------------------------------------------------------------
      // Seeking
      //-----------------------------------------------------------------------
    public:

      /// \brief Seeks to the specified position in the file
      ///
      /// \param pos the position to seek to
      void seek( index_type pos );

      /// \brief Seeks to the beginning of the file
      void seek_to_beginning();

      /// \brief Seeks to the end of the file
      void seek_to_end();

      /// \brief Skips the number of bytes from the current position
      ///
      /// \param bytes the number of bytes to skip
      void skip( index_type bytes );

      //-----------------------------------------------------------------------
      // Modifiers
      //-----------------------------------------------------------------------
    public:

      /// \brief Closes this file if it's currently open
      void close();

      /// \brief Swaps the contents of \c this with \p other
      ///
      /// \param other the other file to swap contents with
      void swap( file& other ) noexcept;

      //-----------------------------------------------------------------------
      // Private Members
      //-----------------------------------------------------------------------
    private:

      file_interface* m_file;       ///< Instance of the abstract file

      friend bool operator<( const file&, const file& ) noexcept;
    };

    //-------------------------------------------------------------------------
    // Utilities
    //-------------------------------------------------------------------------

    /// \brief Swaps the conents of two files
    ///
    /// \param lhs the left file to swap
    /// \param rhs the right file to swap
    void swap( file& lhs, file& rhs ) noexcept;

    //-------------------------------------------------------------------------
    // Comparisons
    //-------------------------------------------------------------------------

    bool operator<( const file& lhs, const file& rhs ) noexcept;

  } // namespace platform
} // namespace bit

//=============================================================================
// file_interface
//=============================================================================

//-----------------------------------------------------------------------------
// Destructors
//-----------------------------------------------------------------------------

inline bit::platform::file_interface::~file_interface() = default;

inline void bit::platform::file_interface::skip( index_type bytes )
{
  seek( tell() + bytes );
}

inline void bit::platform::file_interface::destroy()
{
  // do nothing
}

//=============================================================================
// file
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Destructor / Assignment
//-----------------------------------------------------------------------------

inline bit::platform::file::file( file_interface& file )
  noexcept
  : m_file( &file )
{

}

inline bit::platform::file::file( file&& other )
  noexcept
  : m_file( other.m_file )
{
  other.m_file = nullptr;
}

//-----------------------------------------------------------------------------

inline bit::platform::file::~file()
{
  close();
}

//-----------------------------------------------------------------------------

inline bit::platform::file& bit::platform::file::operator=( file other )
  noexcept
{
  swap( other );

  return (*this);
}

//-----------------------------------------------------------------------------
// Reading
//-----------------------------------------------------------------------------

inline bit::stl::span<char> bit::platform::file::read( stl::span<char> buffer )
{
  if( !m_file ) return {};

  return m_file->read( buffer );
}

//-----------------------------------------------------------------------------
// Writing
//-----------------------------------------------------------------------------

inline bit::stl::span<const char> bit::platform::file::write( stl::span<const char> buffer )
{
  if( !m_file ) return {};

  return m_file->write( buffer );
}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

inline bool bit::platform::file::is_open()
  const noexcept
{
  if( !m_file ) return false;

  return true;
}

inline bit::platform::file::operator bool()
  const noexcept
{
  return is_open();
}

inline bit::platform::file::index_type bit::platform::file::tell()
  const
{
  if( !m_file ) return invalid_index;

  return m_file->tell();
}

//-----------------------------------------------------------------------------
// Seeking
//-----------------------------------------------------------------------------

inline void bit::platform::file::seek( index_type pos )
{
  if( !m_file ) return;

  m_file->seek( pos );
}

inline void bit::platform::file::seek_to_beginning()
{
  seek(0);
}

/// \brief Seeks to the end of the file
inline void bit::platform::file::seek_to_end()
{
  if( !m_file ) return;

  m_file->seek_to_end();
}

inline void bit::platform::file::skip( index_type bytes )
{
  if( !m_file ) return;

  m_file->skip( bytes );
}

//-----------------------------------------------------------------------------
// Modifiers
//-----------------------------------------------------------------------------

inline void bit::platform::file::close()
{
  if( !m_file ) return;

  m_file->destroy();
  m_file = nullptr;
}

inline void bit::platform::file::swap( file& other )
  noexcept
{
  using std::swap;

  swap(m_file,other.m_file);
}

//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------

inline void bit::platform::swap( file& lhs, file& rhs )
  noexcept
{
  lhs.swap(rhs);
}

//-----------------------------------------------------------------------------
// Comparisons
//-----------------------------------------------------------------------------

inline bool bit::platform::operator<( const file& lhs, const file& rhs )
  noexcept
{
  return lhs.m_file < rhs.m_file;
}

#endif /* BIT_PLATFORM_FILESYSTEM_FILE_HPP */
