#ifndef BIT_THREAD_CONCURRENCY_SCHEDULERS_DETAIL_CONCURRENT_TASK_SCHEDULER_INL
#define BIT_THREAD_CONCURRENCY_SCHEDULERS_DETAIL_CONCURRENT_TASK_SCHEDULER_INL

//=============================================================================
// concurrent_task_scheduler
//=============================================================================

//-----------------------------------------------------------------------------
// Modifiers
//-----------------------------------------------------------------------------

template<typename Fn>
inline void bit::platform::concurrent_task_scheduler::run( Fn&& fn )
{
  start();
  while( true ) {

    // Invoke preamble function first
    std::forward<Fn>(fn)();

    if( !m_running ) break;

    auto task = get_task();

    if( !task ) continue;

    help_while_unavailable( task );
    task_scheduler::execute_task( std::move(task) );
  }
}

#endif /* BIT_THREAD_CONCURRENCY_SCHEDULERS_DETAIL_CONCURRENT_TASK_SCHEDULER_INL */
