#ifndef BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_FILE_INL
#define BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_FILE_INL

//----------------------------------------------------------------------------
// Constructors / Destructor / Assignment
//----------------------------------------------------------------------------

inline bit::platform::file::file( abstract_file* file, filesystem* filesystem )
  : m_file(file)
  , m_filesystem(filesystem)
{

}

inline bit::platform::file::file( file&& other )
  : m_file(other.m_file)
  , m_filesystem(other.m_filesystem)
{
  other.m_file = nullptr;
}

//----------------------------------------------------------------------------

inline bit::platform::file::~file()
{
  close();
}

//----------------------------------------------------------------------------

inline bit::platform::file& bit::platform::file::operator = ( file&& other )
{
  close();

  m_file = other.m_file;
  m_filesystem = other.m_filesystem;
  other.m_file = nullptr;

  return (*this);
}

//----------------------------------------------------------------------------
// File API
//----------------------------------------------------------------------------

inline bit::stl::span<bit::stl::byte>
  bit::platform::file::read( stl::span<stl::byte> buffer )
{
  auto count = m_file->read(buffer);
  return { buffer.data(), static_cast<std::ptrdiff_t>(count) };
}

inline bit::stl::span<const bit::stl::byte>
  bit::platform::file::write( stl::span<const stl::byte> buffer )
{
  auto count = m_file->write(buffer);
  return { buffer.data(), static_cast<std::ptrdiff_t>(count) };
}

//----------------------------------------------------------------------------
// Observers
//----------------------------------------------------------------------------

inline bool bit::platform::file::is_open()
  const noexcept
{
  return m_file;
}

inline bit::platform::file::operator bool()
  const noexcept
{
  return m_file;
}

inline bit::platform::file::index_type bit::platform::file::tell()
  const
{
  return m_file->tell();
}

//----------------------------------------------------------------------------
// Seeking
//----------------------------------------------------------------------------

inline void bit::platform::file::seek( index_type pos )
{
  m_file->seek(pos);
}

inline void bit::platform::file::seek_to_beginning()
{
  m_file->seek(0);
}

inline void bit::platform::file::seek_to_end()
{
  m_file->seek_to_end();
}

inline void bit::platform::file::skip( index_type bytes )
{
  m_file->skip(bytes);
}

//----------------------------------------------------------------------------
// Private Member Functions
//----------------------------------------------------------------------------

inline bit::platform::abstract_file* bit::platform::file::underlying()
  noexcept
{
  return m_file;
}

#endif /* BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_FILE_INL */
