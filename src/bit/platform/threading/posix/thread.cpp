#include <bit/platform/threading/thread.hpp>

#define _GNU_SOURCE
#include <sched.h>

#include <unistd.h>
#include <pthread.h>

//----------------------------------------------------------------------------
// Affinity
//----------------------------------------------------------------------------

void bit::platform::set_affinity( std::size_t core_id )
{
  int num_cores = ::sysconf(_SC_NPROCESSORS_ONLN);
  if (core_id < 0 || core_id >= num_cores) {

  }

  ::cpu_set_t cpuset;

  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);

  ::pthread_t current_thread = ::pthread_self();
  ::pthread_setaffinity_np(current_thread, sizeof(::cpu_set_t), &cpuset);
}

void
  bit::platform::set_affinity( std::thread& thread, std::size_t core_id )
{
  int num_cores = ::sysconf(_SC_NPROCESSORS_ONLN);
  if (core_id < 0 || core_id >= num_cores) {

  }

  ::cpu_set_t cpuset;

  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);

  ::pthread_t current_thread = thread.native_handle();
  ::pthread_setaffinity_np(current_thread, sizeof(::cpu_set_t), &cpuset);
}

//----------------------------------------------------------------------------

std::size_t bit::platform::affinity()
{
  ::cpu_set_t cpuset;

  CPU_ZERO(&cpuset);

  ::pthread_t current_thread = ::pthread_self();
  return ::pthread_getaffinity_np( current_thread, sizeof(::cpu_set_t), &cpuset);
}

std::size_t bit::platform::affinity( std::thread& thread )
{
  ::cpu_set_t cpuset;

  CPU_ZERO(&cpuset);

  ::pthread_t current_thread = thread.native_handle();
  return ::pthread_getaffinity_np( current_thread, sizeof(::cpu_set_t), &cpuset);
}
