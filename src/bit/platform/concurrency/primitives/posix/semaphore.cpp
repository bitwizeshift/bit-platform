#include <bit/platform/concurrency/primitives/semaphore.hpp>

#include <semaphore.h>

#include <cerrno>
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
  ::sem_init( &m_semaphore, 0, initial_count );
}

//----------------------------------------------------------------------------

bit::platform::semaphore::~semaphore()
{
  ::sem_destroy( &m_semaphore );
}

//----------------------------------------------------------------------------
// Locking
//----------------------------------------------------------------------------

void bit::platform::semaphore::wait()
{
  int rc;
  do {
      rc = ::sem_wait(&m_semaphore);
  } while (rc == -1 && errno == EINTR);
}

void bit::platform::semaphore::signal( int count )
{
  while( count-- > 0 ) {
    ::sem_post(&m_semaphore);
  }
}

//----------------------------------------------------------------------------
// Private Member Functions
//----------------------------------------------------------------------------

bool bit::platform::semaphore::try_wait( std::uint64_t usecs )
{
  constexpr auto usecs_in_1_sec = 1000000u;
  constexpr auto nsecs_in_1_sec = 1000000000u;

  struct timespec ts;

  ::clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += usecs / usecs_in_1_sec;
  ts.tv_nsec += (usecs % usecs_in_1_sec) * 1000;

  if (ts.tv_nsec > nsecs_in_1_sec) {
    ts.tv_nsec -= nsecs_in_1_sec;
    ++ts.tv_sec;
  }

  int rc;
  do {
    rc = ::sem_timedwait(&m_semaphore, &ts);
  } while (rc == -1 && errno == EINTR);
  return !(rc == -1 && errno == ETIMEDOUT);
}
