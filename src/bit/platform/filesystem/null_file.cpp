#include "null_file.hpp"

//----------------------------------------------------------------------------
// File API
//----------------------------------------------------------------------------

void bit::platform::null_file::close()
{

}

bit::platform::null_file::size_type
  bit::platform::null_file::read( stl::span<stl::byte> )
{
  return 0;
}

bit::platform::null_file::size_type
  bit::platform::null_file::write( stl::span<const stl::byte> buffer )
{
  return buffer.size();
}

//----------------------------------------------------------------------------
// Observers
//----------------------------------------------------------------------------

bit::platform::null_file::index_type
  bit::platform::null_file::tell()
  const
{
  return 0;
}

//----------------------------------------------------------------------------
// Seeking
//----------------------------------------------------------------------------

void bit::platform::null_file::seek( index_type )
{

}


void bit::platform::null_file::seek_to_end()
{

}

void bit::platform::null_file::skip( index_type )
{

}
