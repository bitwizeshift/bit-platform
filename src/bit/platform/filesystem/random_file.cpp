#include "random_file.hpp"

//----------------------------------------------------------------------------
// Construction
//----------------------------------------------------------------------------

bit::platform::random_file::random_file( unsigned int seed )
  : m_generator( seed )
  , m_distribution( 0u, 255u )
{

}

//----------------------------------------------------------------------------
// File API
//----------------------------------------------------------------------------

void bit::platform::random_file::close()
{

}

bit::platform::random_file::size_type
  bit::platform::random_file::read( stl::span<stl::byte> buffer )
{
  for( auto i=0; i<buffer.size(); ++i ) {
    buffer[i] = (stl::byte) m_distribution(m_generator);
  }

  return buffer.size();
}

bit::platform::random_file::size_type
  bit::platform::random_file::write( stl::span<const stl::byte> buffer )
{
  return buffer.size();
}

//----------------------------------------------------------------------------
// Observers
//----------------------------------------------------------------------------

bit::platform::random_file::index_type
  bit::platform::random_file::tell()
  const
{
  return 0;
}

//----------------------------------------------------------------------------
// Seeking
//----------------------------------------------------------------------------

void bit::platform::random_file::seek( index_type )
{

}


void bit::platform::random_file::seek_to_end()
{

}

void bit::platform::random_file::skip( index_type )
{

}
