#include <bit/platform/threading/thread.hpp>

// Mac, as far as I'm aware, does not support any manner of setting
// thread affinity to different cores.

//----------------------------------------------------------------------------
// Affinity
//----------------------------------------------------------------------------

void bit::platform::set_affinity( std::size_t core_id )
{
  (void) core_id;
}

void
  bit::platform::set_affinity( std::thread& thread, std::size_t core_id )
{
  (void) thread;
  (void) core_id;
}

//----------------------------------------------------------------------------

std::size_t bit::platform::affinity()
{
  return ((std::size_t)-1);
}

std::size_t bit::platform::affinity( std::thread& thread )
{
  (void) thread;
  return ((std::size_t)-1);
}

//----------------------------------------------------------------------------

std::size_t bit::platform::active_core()
{
  return ((std::size_t)-1);
}
