#include <bit/platform/threading/job.hpp>
#include <array> // std::array

//=============================================================================
// Private Detail Function
//=============================================================================

namespace {

  thread_local const bit::platform::job* g_this_job = nullptr;

} // anonymous namespace

void* bit::platform::detail::allocate_job()
{
  thread_local std::array<job_storage,job::max_jobs> s_jobs = {};
  thread_local auto s_index = std::size_t{0u};

  auto j = &s_jobs[s_index++ % job::max_jobs];

  // If there are any unfinished jobs in the job being allocated, it means that
  // we have allocated more than max_jobs worth of jobs -- and that the previous
  // job has not yet completed.
  assert( j->m_unfinished == 0 && "too many jobs allocated; buffer overflow occurred" );

  return j;
}

const bit::platform::job* bit::platform::detail::get_active_job() noexcept
{
  return g_this_job;
}

void bit::platform::detail::set_active_job( const job* j ) noexcept
{
  g_this_job = j;
}

//=============================================================================
// Free Functions
//=============================================================================

//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------

const bit::platform::job* bit::platform::this_job() noexcept
{
  return g_this_job;
}
