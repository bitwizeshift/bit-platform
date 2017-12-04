#include <bit/platform/filesystem/disk_file_device.hpp>

#include "disk_file.hpp"

#include <bit/stl/utilities/assert.hpp>

bit::platform::abstract_file*
  bit::platform::disk_file_device::open( stl::string_view path, mode m )
{
  // TODO: Add support for allocators
  return new disk_file( path, m );
}

bit::platform::async_file*
  bit::platform::disk_file_device::open_async( stl::string_view path, mode m )
{
  BIT_ASSERT(false, "disk_file_device::open_async: not yet implemented");
  return nullptr;
}

bit::stl::string_view
  bit::platform::disk_file_device::identifier()
  const noexcept
{
  return "disk";
}
