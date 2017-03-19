#include "crypto_file.hpp"

#include <memory>

namespace {

  const auto prime1 = (bit::stl::byte) 58;
  const auto prime2 = (bit::stl::byte) 129;

} // anonymous namespace

//----------------------------------------------------------------------------
// File API
//----------------------------------------------------------------------------

void bit::platform::crypto_file::close()
{
  m_file->close();
}

bit::platform::crypto_file::size_type
  bit::platform::crypto_file::read( stl::span<stl::byte> buffer )
{
  const auto bytes_read = m_file->read(buffer);

  for( auto i = 0; i < bytes_read; ++i ){
    buffer[i] ^= prime1;
    buffer[i] ^= prime2;
  }
  return bytes_read;
}

bit::platform::crypto_file::size_type
  bit::platform::crypto_file::write( stl::span<const stl::byte> buffer )
{
  // writing requires a temporary buffer to perform the translation

  // TODO: use bit::allocators for allocation
  auto mutable_buffer = std::make_unique<stl::byte[]>( buffer.size() );

  for( auto i = 0; i < buffer.size(); ++i ){
    mutable_buffer[i] =  buffer[i];
    mutable_buffer[i] ^= prime1;
    mutable_buffer[i] ^= prime2;
  }

  const auto bytes_written = m_file->write( {mutable_buffer.get(), buffer.size()} );

  return bytes_written;
}

//----------------------------------------------------------------------------
// Observers
//----------------------------------------------------------------------------

bit::platform::crypto_file::index_type
  bit::platform::crypto_file::tell()
  const
{
  return m_file->tell();
}

//----------------------------------------------------------------------------
// Seeking
//----------------------------------------------------------------------------

void bit::platform::crypto_file::seek( index_type pos )
{
  m_file->seek(pos);
}


void bit::platform::crypto_file::seek_to_end()
{
  m_file->seek_to_end();
}

void bit::platform::crypto_file::skip( index_type bytes )
{
  m_file->skip(bytes);
}

