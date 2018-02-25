#include <bit/platform/threading/task.hpp>
#include <array> // std::array

//=============================================================================
// Private Detail Function
//=============================================================================

namespace {

  thread_local const bit::platform::task* g_this_task = nullptr;

} // anonymous namespace

void* bit::platform::detail::allocate_task()
{
  thread_local std::array<task_storage,task::max_tasks> s_tasks = {};
  thread_local auto s_index = std::size_t{0u};

  auto j = &s_tasks[s_index++ % task::max_tasks];

  // If there are any unfinished tasks in the task being allocated, it means that
  // we have allocated more than max_tasks worth of tasks -- and that the previous
  // task has not yet completed.
  assert( j->m_unfinished == 0 && "too many tasks allocated; buffer overflow occurred" );

  return j;
}

const bit::platform::task* bit::platform::detail::get_active_task() noexcept
{
  return g_this_task;
}

void bit::platform::detail::set_active_task( const task* j ) noexcept
{
  g_this_task = j;
}

//=============================================================================
// Free Functions
//=============================================================================

//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------

const bit::platform::task* bit::platform::this_task() noexcept
{
  return g_this_task;
}
