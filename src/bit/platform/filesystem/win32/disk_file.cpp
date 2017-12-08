#include "disk_file.hpp"

#ifndef NOMINMAX
# define NOMINMAX 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

bit::platform::disk_file::disk_file( stl::string_view path,
                                     mode m,
                                     destroy_delegate destroy)
  : m_destroy(destroy)
{
  const bool is_read   = (( m & mode::read   ) == mode::read);
  const bool is_write  = (( m & mode::write  ) == mode::write);
  const bool is_append = (( m & mode::append ) == mode::append);

  const auto access = (is_read  ? GENERIC_READ  : 0) |
                      (is_write ? GENERIC_WRITE : 0);
  const auto share    = (FILE_SHARE_READ | FILE_SHARE_WRITE);
  const auto disposition = ((is_write && is_read) || is_append) ? OPEN_ALWAYS :
                           (is_write)                           ? CREATE_ALWAYS :
                           (is_read)                            ? OPEN_EXISTING :
                           TRUNCATE_EXISTING;
  const auto attributes = FILE_ATTRIBUTE_NORMAL;

  m_file = ::CreateFileA( path.data(),
                          access,
                          share,
                          nullptr,
                          disposition,
                          attributes,
                          nullptr );
}


//----------------------------------------------------------------------------
// File API
//----------------------------------------------------------------------------

void bit::platform::disk_file::destroy()
{
  ::CloseHandle( m_file );
  m_file = nullptr;

  m_destroy(this);
}

bit::stl::span<char> bit::platform::disk_file::read( stl::span<char> buffer )
{
  ::DWORD read;
  ::ReadFile(m_file, buffer.data(), buffer.size(), &read, nullptr );

  return {buffer.data(),read};
}

bit::stl::span<const char>
  bit::platform::disk_file::write( stl::span<const char> buffer )
{
  ::DWORD written;
  ::WriteFile(m_file, buffer.data(), buffer.size(), &written, nullptr );

  return {buffer.data(),written};
}

//----------------------------------------------------------------------------
// Observers
//----------------------------------------------------------------------------

bit::platform::disk_file::index_type bit::platform::disk_file::tell()
  const
{
  ::LARGE_INTEGER none;
  ::LARGE_INTEGER current;

  none.QuadPart = 0;

  if( ::SetFilePointerEx( m_file, none, &current, FILE_CURRENT ) == 0 ) {
    return file_interface::invalid_index;
  }

  return static_cast<size_type>(current.QuadPart);
}

bit::platform::disk_file::size_type bit::platform::disk_file::size()
  const
{
  ::LARGE_INTEGER size;
  ::GetFileSizeEx(m_file,&size);

  return static_cast<size_type>(size.QuadPart);
}

//----------------------------------------------------------------------------
// Seeking
//----------------------------------------------------------------------------

void bit::platform::disk_file::seek( index_type pos )
{
  ::LARGE_INTEGER move;
  move.QuadPart = pos;

  ::SetFilePointerEx( m_file, move, nullptr, FILE_BEGIN );
}

void bit::platform::disk_file::seek_to_end()
{
  ::LARGE_INTEGER none;
  none.QuadPart = 0;

  ::SetFilePointerEx( m_file, none, nullptr, FILE_END );
}

void bit::platform::disk_file::skip( index_type bytes )
{
  ::LARGE_INTEGER move;
  move.QuadPart = bytes;

  ::SetFilePointerEx( m_file, move, nullptr, FILE_CURRENT );
}
