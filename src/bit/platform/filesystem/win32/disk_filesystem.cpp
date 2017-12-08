#include <bit/platform/filesystem/disk_filesystem.hpp>

#include "disk_file.hpp"

#include <bit/memory/uninitialized_storage.hpp>
#include <bit/memory/allocators/new_allocator.hpp>

namespace {
  auto default_allocator = bit::memory::new_allocator{};
} // anonymous namespace

//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

bit::platform::disk_filesystem::disk_filesystem()
  noexcept
  : m_allocator( default_allocator )
{

}

bit::platform::disk_filesystem::disk_filesystem( memory::any_allocator allocator )
  noexcept
  : m_allocator(allocator)
{

}

//-----------------------------------------------------------------------------
// File Opening
//-----------------------------------------------------------------------------

bit::platform::file bit::platform::disk_filesystem::open( sync_t,
                                                          stl::string_view path,
                                                          mode access )
{
  auto destroy = disk_file::destroy_delegate{};
  destroy.bind<disk_filesystem,&disk_filesystem::destroy_file>(*this);

  auto* p = m_allocator.allocate( sizeof(disk_file), alignof(disk_file) );
  auto* f = memory::uninitialized_construct_at<disk_file>(p,path,access,destroy);

  return file{ *f };
}

//-----------------------------------------------------------------------------
// File Closing
//-----------------------------------------------------------------------------

void bit::platform::disk_filesystem::close( file& f )
{
  f.close();
}

//-----------------------------------------------------------------------------
// Destruction
//-----------------------------------------------------------------------------

void bit::platform::disk_filesystem::destroy_file(void* p)
{
  memory::destroy_at( static_cast<disk_file*>(p) );
  m_allocator.deallocate(p,sizeof(disk_file));
}

void bit::platform::disk_filesystem::destroy_async_file(void* p)
{
  // TODO: implement
}
