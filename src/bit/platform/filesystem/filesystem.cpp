#include <bit/platform/filesystem/filesystem.hpp>

#include <bit/stl/assert.hpp>

//============================================================================
// file_device
//============================================================================

//----------------------------------------------------------------------------
// Synchronous Files
//----------------------------------------------------------------------------

bit::platform::abstract_file*
  bit::platform::file_device::open( stl::string_view, mode )
{
  BIT_ASSERT(false, "file_device::open: not implemented for this device");
  return nullptr;
}

bit::platform::abstract_file*
  bit::platform::file_device::piggyback( abstract_file* )
{
  BIT_ASSERT(false, "file_device::piggyback: not implemented for this device");
  return nullptr;
}

//----------------------------------------------------------------------------
// Asynchronous Files
//----------------------------------------------------------------------------

bit::platform::async_file*
  bit::platform::file_device::open_async( stl::string_view, mode )
{
  BIT_ASSERT(false, "file_device::open_async: not implemented for this device");
  return nullptr;
}

bit::platform::async_file*
  bit::platform::file_device::piggyback_async( async_file* )
{
  BIT_ASSERT(false, "file_device::piggyback_async: not implemented for this device");
  return nullptr;
}

//============================================================================
// file
//============================================================================

void bit::platform::file::close()
{
  if(m_file) {
    m_filesystem->close(*this);
    m_file = nullptr;
  }
}

//============================================================================
// async_file
//============================================================================

//============================================================================
// filesystem
//============================================================================

namespace { // anonymous

  bit::platform::file_device*
    find_device( bit::stl::string_view identifier,
                 bit::stl::span<bit::platform::file_device*> devices )
  {
    for( auto i=0; i < devices.size(); ++i ) {
      if( devices[i]->identifier() == identifier ) return devices[i];
    }

    BIT_ASSERT( false, "Specified file_device does not exist" );
    return nullptr;
  }

} // anonymous namespace

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

bit::platform::filesystem::filesystem( size_type devices )
  : m_devices( std::make_unique<file_device*[]>(devices) )
  , m_size(devices)
  , m_current(0)
{

}

//----------------------------------------------------------------------------
// Devices
//----------------------------------------------------------------------------

void bit::platform::filesystem::mount( file_device* device )
{
  BIT_ASSERT_OR_THROW(std::out_of_range, m_current < m_size, "filesystem::mount: too many devices mounted");

  m_devices[m_current++] = device;
}

//----------------------------------------------------------------------------

void bit::platform::filesystem::unmount( file_device* device )
{
  auto it = std::remove(&m_devices[0], &m_devices[m_current], device);
  m_current = std::distance(&m_devices[0],it);
}

//----------------------------------------------------------------------------
// File Opening
//----------------------------------------------------------------------------

bit::platform::file
  bit::platform::filesystem::open( sync_t,
                                   stl::string_view path,
                                   const char*const* devices,
                                   std::size_t size,
                                   mode access )
{
  BIT_ASSERT( size != 0, "filesystem::open: device list cannot be empty" );

  auto devs     = stl::span<file_device*>{ m_devices.get(), m_current };
  auto device   = find_device(devices[0], devs);
  auto abstract = device->open( path, access );

  auto i = 1;
  while( i < size ) {
    device   = find_device(devices[i], devs);
    abstract = device->piggyback(abstract);
    ++i;
  }

  return file{ abstract, this };
}

//----------------------------------------------------------------------------

bit::platform::async_file
  bit::platform::filesystem::open( async_t,
                                   stl::string_view path,
                                   const char*const* devices,
                                   std::size_t size,
                                   mode access )
{
  BIT_ASSERT( size != 0, "filesystem::open: device list cannot be empty" );

  auto devs     = stl::span<file_device*>{ m_devices.get(), m_current };
  auto device   = find_device(devices[0], devs);
  auto abstract = device->open_async( path, access );

  auto i = 1;
  while( i < size ) {
    device   = find_device(devices[i], devs);
    abstract = device->piggyback_async(abstract);
  }

  BIT_ASSERT( false, "filesystem::open: not implemented");

  return async_file{};
//  return file{ abstract, this };
}


void bit::platform::filesystem::close( file& f )
{
  auto current = f.m_file;
  if( current ) {
    current->close();
    f.m_file = nullptr;
  }

  destroy_file( current );
}

void bit::platform::filesystem::close( async_file& f )
{
  BIT_ASSERT(false, "filesystem::close: not implemented");
}

//----------------------------------------------------------------------------
// Private Member Functions
//----------------------------------------------------------------------------

void bit::platform::filesystem::destroy_file( abstract_file* f )
{
  auto next = f->next();

  if( next ) destroy_file( next );

  // TODO: change to bit::memory allocator
  delete f;
}
