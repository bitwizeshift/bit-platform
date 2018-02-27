#include "task_queue.hpp"

#include <utility> // std::move

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

bit::platform::detail::task_queue::task_queue()
  : m_bottom(0),
    m_top(0)
{

}

//----------------------------------------------------------------------------
// Modifiers
//----------------------------------------------------------------------------

void bit::platform::detail::task_queue::push( task j )
{
  std::lock_guard<std::mutex> lock(m_lock);

  m_tasks[m_bottom++ % max_tasks] = std::move(j);
}

//----------------------------------------------------------------------------

bit::platform::task bit::platform::detail::task_queue::pop()
{
  std::lock_guard<std::mutex> lock(m_lock);

  // If there are no tasks, return null
  if( m_bottom <= m_top ) return task{};

  return std::move(m_tasks[--m_bottom % max_tasks]);
}

bit::platform::task bit::platform::detail::task_queue::steal()
{
  std::lock_guard<std::mutex> lock(m_lock);

  // If there are no tasks, return null
  if( m_bottom <= m_top ) return task{};

  return std::move(m_tasks[m_top++ % max_tasks]);
}

//----------------------------------------------------------------------------
// Capacity
//----------------------------------------------------------------------------

bool bit::platform::detail::task_queue::empty()
  const noexcept
{
  std::lock_guard<std::mutex> lock(m_lock);

  return m_bottom == m_top;
}
