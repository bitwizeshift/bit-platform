#include <bit/platform/threading/thread.hpp>

#ifndef NOMINMAX
# define NOMINMAX 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

//-----------------------------------------------------------------------------
// Affinity
//-----------------------------------------------------------------------------

void bit::platform::set_affinity( std::thread& thread, std::size_t core_id )
{
  ::SetThreadAffinityMask( (::HANDLE) thread.native_handle(), (1 << core_id) );
}

//-----------------------------------------------------------------------------


std::size_t bit::platform::affinity( std::thread& thread )
{
  auto mask = ::SetThreadAffinityMask( (::HANDLE) thread.native_handle(), 0);
  ::SetThreadAffinityMask( (::HANDLE) thread.native_handle(), mask);

  return (std::size_t) mask;
}

//-----------------------------------------------------------------------------
// This thread : Affinity
//-----------------------------------------------------------------------------

void bit::platform::this_thread::set_affinity( std::size_t core_id )
{
  ::SetThreadAffinityMask( ::GetCurrentThread(), (1 << core_id) );
}

std::size_t bit::platform::this_thread::affinity()
{
  // There must be an easier way for this to be implemented

  auto current = ::GetCurrentThread();
  auto mask = ::SetThreadAffinityMask(current, 0);
  ::SetThreadAffinityMask(current, mask);

  return (std::size_t) mask;
}

//-----------------------------------------------------------------------------

std::size_t bit::platform::this_thread::active_core()
{
  return (std::size_t) ::GetCurrentProcessorNumber();
}
