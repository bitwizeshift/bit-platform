#include <bit/platform/filesystem/random_file_device.hpp>

#include "random_file.hpp"

#include <bit/stl/utilities/assert.hpp>

//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

bit::platform::random_file_device::random_file_device()
  : m_device()
{

}

//----------------------------------------------------------------------------
// Synchronous Files
//----------------------------------------------------------------------------

bit::platform::abstract_file*
  bit::platform::random_file_device::open( stl::string_view, mode )
{
  // TODO: Do with bit::memory allocators rather than raw new call
  return new random_file(m_device());
}

//----------------------------------------------------------------------------
// Asynchronous Files
//----------------------------------------------------------------------------

bit::platform::async_file*
  bit::platform::random_file_device::open_async( stl::string_view, mode )
{
  // todo: implement the Async API
  BIT_ASSERT(false,"Not implemented");
  return nullptr;
}

//----------------------------------------------------------------------------
// Element Access
//----------------------------------------------------------------------------

bit::stl::string_view bit::platform::random_file_device::identifier()
  const noexcept
{
  return "random";
}
