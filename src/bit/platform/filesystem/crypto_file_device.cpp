#include <bit/platform/filesystem/crypto_file_device.hpp>

#include "crypto_file.hpp"

#include <bit/stl/utilities/assert.hpp>

//----------------------------------------------------------------------------
// Synchronous Files
//----------------------------------------------------------------------------

bit::platform::abstract_file*
  bit::platform::crypto_file_device::piggyback( abstract_file* f )
{
  // TODO: use bit::allocators for allocation
  return new crypto_file( f );
}

//----------------------------------------------------------------------------
// Asynchronous Files
//----------------------------------------------------------------------------

bit::platform::async_file*
  bit::platform::crypto_file_device::piggyback_async( async_file* f )
{
  // TODO: use bit::allocators for allocation
  BIT_ASSERT(false, "crypto_file_device::piggyback_async: not implemented");
  return nullptr;
}

//----------------------------------------------------------------------------
// Element Access
//----------------------------------------------------------------------------

bit::stl::string_view bit::platform::crypto_file_device::identifier()
  const noexcept
{
  return "crypto";
}
