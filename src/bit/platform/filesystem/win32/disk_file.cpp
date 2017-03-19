#include "disk_file.hpp"

#ifndef NOMINMAX
# define NOMINMAX 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

bit::platform::disk_file::disk_file( stl::string_view path, mode m )
{
  const bool is_read   = (( m & mode::read   ) == mode::read);
  const bool is_write  = (( m & mode::write  ) == mode::write);
  const bool is_append = (( m & mode::append ) == mode::append);
  // const bool is_update = ( mode & File_System::UPDATE );

  m_file = ::CreateFileA(
    path.data(),
    (
      (is_read  ? GENERIC_READ  : 0) |
      (is_write ? GENERIC_WRITE : 0)
    ),
    (FILE_SHARE_READ | FILE_SHARE_WRITE),
    0,
    (
       ((is_write && is_read) || is_append)  ? OPEN_ALWAYS   :
        (is_write)                           ? CREATE_ALWAYS :
        (is_read)                            ? OPEN_EXISTING :
                                               TRUNCATE_EXISTING
    ),
    FILE_ATTRIBUTE_NORMAL,
    NULL
  );
}


//----------------------------------------------------------------------------
// File API
//----------------------------------------------------------------------------

void bit::platform::disk_file::close()
{
  ::CloseHandle( m_file );
  m_file = nullptr;
}

bit::platform::disk_file::size_type
  bit::platform::disk_file::read( stl::span<stl::byte> buffer )
{
  ::DWORD read;
  ::ReadFile(m_file, buffer.data(), buffer.size(), &read, nullptr );
  return (size_type) read;
}

bit::platform::disk_file::size_type
  bit::platform::disk_file::write( stl::span<const stl::byte> buffer )
{
  ::DWORD read;

  ::WriteFile(m_file, buffer.data(), buffer.size(), &read, nullptr );
  return (size_type) read;
}

//----------------------------------------------------------------------------
// Observers
//----------------------------------------------------------------------------

bit::platform::disk_file::index_type
  bit::platform::disk_file::tell()
  const
{
  ::LARGE_INTEGER none;
  ::LARGE_INTEGER current;

  none.QuadPart = 0;

  ::SetFilePointerEx( m_file, none, &current, FILE_CURRENT );

  return (size_type) current.QuadPart;
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
