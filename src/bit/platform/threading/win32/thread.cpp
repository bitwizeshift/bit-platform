#include <bit/platform/threading/thread.hpp>

#ifndef NOMINMAX
# define NOMINMAX 1
#endif // NOMINMAX
#include <windows.h>

//----------------------------------------------------------------------------
// Affinity
//----------------------------------------------------------------------------

void bit::platform::set_affinity( std::size_t core_id )
{
  ::SetThreadAffinityMask( ::GetCurrentThread(), (1 << core_id) );
}

void
  bit::platform::set_affinity( std::thread& thread, std::size_t core_id )
{
  ::SetThreadAffinityMask( (::HANDLE) thread.native_handle(), (1 << core_id) );
}

//----------------------------------------------------------------------------

std::size_t bit::platform::affinity()
{
  // There must be an easier way for this to be implemented

  auto current = ::GetCurrentThread();
  auto mask = ::SetThreadAffinityMask(current, 0);
  ::SetThreadAffinityMask(current, mask);

  return (std::size_t) mask;
}

std::size_t bit::platform::affinity( std::thread& thread )
{
  auto mask = ::SetThreadAffinityMask( (::HANDLE) thread.native_handle(), 0);
  ::SetThreadAffinityMask( (::HANDLE) thread.native_handle(), mask);

  return (std::size_t) mask;
}
