#include <bit/platform/concurrency/schedulers/task.hpp>

#include <array>   // std::array
#include <atomic>  // std::atomic
#include <cstddef> // std::abort

namespace {

  [[noreturn]]
  void default_out_of_task_handler()
  {
    std::abort();
  }

  using atomic_out_of_task_handler_type = std::atomic<bit::platform::out_of_task_handler_t>;

  atomic_out_of_task_handler_type g_out_of_task_handler{&::default_out_of_task_handler};

} // anonymous namespace

//=============================================================================
// Private Detail Function
//=============================================================================

void* bit::platform::detail::allocate_task()
{
  thread_local std::array<task_storage,task::max_tasks> s_tasks = {};
  thread_local auto s_index = std::size_t{0u};

  auto j = &s_tasks[s_index++ % task::max_tasks];

  // If there are any unfinished tasks in the task being allocated, it means that
  // we have allocated more than max_tasks worth of tasks -- and that the previous
  // task has not yet completed.
  if( j->m_unfinished != 0 ) {
    (*get_out_of_task_handler())();
  }

  return j;
}

//-----------------------------------------------------------------------------
// Handlers
//-----------------------------------------------------------------------------

bit::platform::out_of_task_handler_t
  bit::platform::set_out_of_task_handler( out_of_task_handler_t f )
{
  return ::g_out_of_task_handler.exchange( f ? f : &::default_out_of_task_handler );
}

bit::platform::out_of_task_handler_t bit::platform::get_out_of_task_handler()
{
  return ::g_out_of_task_handler.load();
}
