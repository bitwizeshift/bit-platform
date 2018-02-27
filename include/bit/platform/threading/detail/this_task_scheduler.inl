#ifndef BIT_PLATFORM_THREADING_DETAIL_THIS_TASK_SCHEDULER_INL
#define BIT_PLATFORM_THREADING_DETAIL_THIS_TASK_SCHEDULER_INL


//=============================================================================
// Inline Definitions : this_task_scheduler
//=============================================================================

//-----------------------------------------------------------------------------
// Posting / Waiting
//-----------------------------------------------------------------------------

inline void bit::platform::this_task_scheduler::post_task( task task )
{
  auto scheduler = task_scheduler::g_active_scheduler;

  assert( scheduler != nullptr );

  scheduler->post_task( std::move(task) );
}

inline void bit::platform::this_task_scheduler::wait( task_handle task )
{
  auto scheduler = task_scheduler::g_active_scheduler;

  assert( scheduler != nullptr );

  scheduler->wait( task );
}

//-----------------------------------------------------------------------------

template<typename Fn, typename...Args>
inline void bit::platform::this_task_scheduler::post( Fn&& fn, Args&&...args )
{
  auto scheduler = task_scheduler::g_active_scheduler;

  assert( scheduler != nullptr );

  scheduler->post( std::forward<Fn>(fn), std::forward<Args>(args)... );
}

template<typename Fn, typename...Args>
inline void bit::platform::this_task_scheduler
  ::post( const task& parent, Fn&& fn, Args&&...args )
{
  auto scheduler = task_scheduler::g_active_scheduler;

  assert( scheduler != nullptr );

  scheduler->post( parent, std::forward<Fn>(fn), std::forward<Args>(args)... );
}

//-----------------------------------------------------------------------------


template<typename Fn, typename...Args>
inline bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::this_task_scheduler::post_and_wait( Fn&& fn, Args&&...args )
{
  auto scheduler = task_scheduler::g_active_scheduler;

  assert( scheduler != nullptr );

  scheduler->post_and_wait( std::forward<Fn>(fn),
                            std::forward<Args>(args)... );
}

template<typename Fn, typename...Args>
inline bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::this_task_scheduler
  ::post_and_wait( const task& parent, Fn&& fn, Args&&...args )
{
  auto scheduler = task_scheduler::g_active_scheduler;

  assert( scheduler != nullptr );

  scheduler->post_and_wait( parent,
                            std::forward<Fn>(fn),
                            std::forward<Args>(args)... );
}

#endif /* BIT_PLATFORM_THREADING_DETAIL_THIS_TASK_SCHEDULER_INL */
