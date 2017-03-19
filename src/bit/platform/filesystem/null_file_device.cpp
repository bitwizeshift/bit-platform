#include <bit/platform/filesystem/null_file_device.hpp>

#include "null_file.hpp"

//----------------------------------------------------------------------------
// Synchronous Files
//----------------------------------------------------------------------------

bit::platform::abstract_file*
  bit::platform::null_file_device::open( stl::string_view, mode )
{
  // TODO: Do with bit::memory allocators rather than raw new call
  return new null_file;
}

bit::platform::abstract_file*
  bit::platform::null_file_device::piggyback( abstract_file* f )
{
  return f;
}

//----------------------------------------------------------------------------
// Asynchronous Files
//----------------------------------------------------------------------------

bit::platform::async_file*
  bit::platform::null_file_device::open_async( stl::string_view, mode )
{
  // todo: implement the Async API
  BIT_ASSERT(false,"Not implemented");
  return nullptr;
}

bit::platform::async_file*
  bit::platform::null_file_device::piggyback_async( async_file* f )
{
  return f;
}

//----------------------------------------------------------------------------
// Element Access
//----------------------------------------------------------------------------

bit::stl::string_view bit::platform::null_file_device::identifier()
  const noexcept
{
  return "null";
}
