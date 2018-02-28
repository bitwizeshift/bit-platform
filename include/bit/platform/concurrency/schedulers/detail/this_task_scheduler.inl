#ifndef BIT_PLATFORM_CONCURRENCY_SCHEDULERS_DETAIL_THIS_TASK_SCHEDULER_INL
#define BIT_PLATFORM_CONCURRENCY_SCHEDULERS_DETAIL_THIS_TASK_SCHEDULER_INL


//=============================================================================
// Inline Definitions : this_task_scheduler
//=============================================================================

//-----------------------------------------------------------------------------
// Bound Objects
//-----------------------------------------------------------------------------

template<typename T>
inline bit::platform::task_scheduler::bound_object<T>
  bit::platform::this_task_scheduler::make_bound_object( T& object )
{
  auto scheduler = task_scheduler::g_active_scheduler;

  assert( scheduler != nullptr );

  return scheduler->make_bound_object( object );
}

template<typename Allocator, typename T>
inline bit::platform::task_scheduler::bound_object<T>
  bit::platform::this_task_scheduler::allocate_bound_object( const Allocator& alloc,
                                                             T& object )
{
  auto scheduler = task_scheduler::g_active_scheduler;

  assert( scheduler != nullptr );

  return scheduler->allocate_bound_object( alloc, object );
}

//-----------------------------------------------------------------------------
// Posting / Waiting
//-----------------------------------------------------------------------------

inline void bit::platform::this_task_scheduler::post_task( task task )
{
  auto scheduler = task_scheduler::g_active_scheduler;

  assert( scheduler != nullptr );

  scheduler->post_task( std::move(task) );
}

//-----------------------------------------------------------------------------

template<typename Fn, typename...Args, typename>
inline void bit::platform::this_task_scheduler::post( Fn&& fn, Args&&...args )
{
  auto scheduler = task_scheduler::g_active_scheduler;

  assert( scheduler != nullptr );

  scheduler->post( std::forward<Fn>(fn), std::forward<Args>(args)... );
}

template<typename Fn, typename...Args, typename>
inline void bit::platform::this_task_scheduler
  ::post( const task& parent, Fn&& fn, Args&&...args )
{
  auto scheduler = task_scheduler::g_active_scheduler;

  assert( scheduler != nullptr );

  scheduler->post( parent, std::forward<Fn>(fn), std::forward<Args>(args)... );
}

#endif /* BIT_PLATFORM_CONCURRENCY_SCHEDULERS_DETAIL_THIS_TASK_SCHEDULER_INL */
