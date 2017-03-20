#include <bit/platform/threading/semaphore.hpp>

#include <mach/mach.h>

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

  ::semaphore_create(::mach_task_self(), &m_semaphore, SYNC_POLICY_FIFO, initial_count );
}

//----------------------------------------------------------------------------

bit::platform::semaphore::~semaphore()
{
  ::semaphore_destroy(::mach_task_self(), m_semaphore);
}

//----------------------------------------------------------------------------
// Locking
//----------------------------------------------------------------------------

void bit::platform::semaphore::wait()
{
  ::semaphore_wait(m_semaphore);
}

void bit::platform::semaphore::signal( int count )
{
  while(count-- > 0) {
    ::semaphore_signal(m_semaphore);
  }

}

//----------------------------------------------------------------------------
// Private Member Functions
//----------------------------------------------------------------------------

bool bit::platform::semaphore::try_wait( std::uint64_t usecs )
{
  ::mach_timespec_t ts;
  ts.tv_sec = usecs / 1000000u;
  ts.tv_nsec = (usecs % 1000000u) * 1000u;

  // added in OSX 10.10: https://developer.apple.com/library/prerelease/mac/documentation/General/Reference/APIDiffsMacOSX10_10SeedDiff/modules/Darwin.html
  ::kern_return_t rc = ::semaphore_timedwait(m_semaphore, ts);

  return rc != KERN_OPERATION_TIMED_OUT
}
