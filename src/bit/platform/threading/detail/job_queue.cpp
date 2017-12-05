#include "job_queue.hpp"

#include <utility> // std::move

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

bit::platform::detail::job_queue::job_queue()
  : m_bottom(0),
    m_top(0)
{

}

//----------------------------------------------------------------------------
// Modifiers
//----------------------------------------------------------------------------

void bit::platform::detail::job_queue::push( job j )
{
  std::lock_guard<std::mutex> lock(m_lock);

  m_jobs[m_bottom++ % max_jobs] = std::move(j);
}

//----------------------------------------------------------------------------

bit::platform::job bit::platform::detail::job_queue::pop()
{
  std::lock_guard<std::mutex> lock(m_lock);

  // If there are no jobs, return null
  if( m_bottom <= m_top ) return job{};

  return std::move(m_jobs[--m_bottom % max_jobs]);
}

bit::platform::job bit::platform::detail::job_queue::steal()
{
  std::lock_guard<std::mutex> lock(m_lock);

  // If there are no jobs, return null
  if( m_bottom <= m_top ) return job{};

  return std::move(m_jobs[m_top++ % max_jobs]);
}

//----------------------------------------------------------------------------
// Capacity
//----------------------------------------------------------------------------

bool bit::platform::detail::job_queue::empty()
  const noexcept
{
  std::lock_guard<std::mutex> lock(m_lock);

  return m_bottom == m_top;
}
