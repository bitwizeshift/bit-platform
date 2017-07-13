#include <bit/platform/threading/semaphore.hpp>

#ifndef NOMINMAX
# define NOMINMAX 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

#include <cassert>

//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

bit::platform::semaphore::semaphore()
  : semaphore(1)
{
}

bit::platform::semaphore::semaphore( int initial_count )
{
  assert( initial_count >= 0 );
  static constexpr auto max_long = 0x7fffffffu;

  m_semaphore = ::CreateSemaphoreW(nullptr, initial_count, max_long, nullptr);
}

//----------------------------------------------------------------------------

bit::platform::semaphore::~semaphore()
{
  ::CloseHandle( m_semaphore );
}

//----------------------------------------------------------------------------
// Locking
//----------------------------------------------------------------------------

void bit::platform::semaphore::wait()
{
  static constexpr auto infinite = 0xffffffffu;

  ::WaitForSingleObject( m_semaphore, infinite);
}

void bit::platform::semaphore::signal( int count )
{
  ::ReleaseSemaphore( m_semaphore, count, nullptr );
}

//----------------------------------------------------------------------------
// Private Member Functions
//----------------------------------------------------------------------------

bool bit::platform::semaphore::try_wait( std::uint64_t usecs )
{
  return ::WaitForSingleObject( m_semaphore, (unsigned long)(usecs / 1000u)) != WAIT_TIMEOUT;
}
