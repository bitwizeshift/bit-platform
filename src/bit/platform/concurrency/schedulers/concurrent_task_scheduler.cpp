#include <bit/platform/concurrency/schedulers/concurrent_task_scheduler.hpp>
#include <bit/platform/concurrency/utilities/thread.hpp>

#include <random> // std::random_device, sdt::uniform_distribution, etc
#include <memory> // std::unique_ptr
#include <vector> // std::vector

#include <cassert> // assert

#include "detail/task_queue.hpp" // detail::task_queue

//=============================================================================
// Anonymous Namespaces
//=============================================================================

namespace {

  //---------------------------------------------------------------------------
  // Utility Functions
  //---------------------------------------------------------------------------

  /// \brief Generates a random number between \p low and \p high
  ///
  /// \param low the low bound
  /// \param high the high bound
  /// \return the random number
  std::ptrdiff_t generate_number_in_range( std::ptrdiff_t low,
                                           std::ptrdiff_t high );

  /// \brief Gets the address of this thread's active task queue
  ///
  /// \return pointer to the task queue
  bit::platform::detail::task_queue* get_task_queue();

  //---------------------------------------------------------------------------
  // Globals
  //---------------------------------------------------------------------------

  thread_local std::ptrdiff_t     g_thread_index = 0;
  thread_local bit::platform::concurrent_task_scheduler* g_this_concurrent_task_scheduler = nullptr;

} // namespace anonymous

std::ptrdiff_t bit::platform::worker_thread_id()
{
  return g_thread_index;
}

//=============================================================================
// task_concurrent_task_scheduler
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Destructor
//-----------------------------------------------------------------------------

bit::platform::concurrent_task_scheduler::concurrent_task_scheduler()
  : concurrent_task_scheduler( std::thread::hardware_concurrency() - 1 )
{

}

bit::platform::concurrent_task_scheduler::concurrent_task_scheduler( std::size_t threads )
  : m_running(false),
    m_set_affinity(false),
    m_running_threads(0)
{
  m_threads.resize(threads);
  m_queues.resize(threads+1);
}


bit::platform::concurrent_task_scheduler::concurrent_task_scheduler( assign_affinity_t )
  : concurrent_task_scheduler( assign_affinity, std::thread::hardware_concurrency() - 1 )
{

}

bit::platform::concurrent_task_scheduler::concurrent_task_scheduler( assign_affinity_t, std::size_t threads )
: m_running(false),
  m_set_affinity(true),
  m_running_threads(0)

{
  m_threads.resize(threads);
  m_queues.resize(threads+1);
}

//-----------------------------------------------------------------------------

bit::platform::concurrent_task_scheduler::~concurrent_task_scheduler()
{
  stop();
}

//-----------------------------------------------------------------------------
// Modifiers
//-----------------------------------------------------------------------------

void bit::platform::concurrent_task_scheduler::stop()
{
  assert( m_owner == std::this_thread::get_id() &&
          "task_concurrent_task_scheduler can only be stopped on the creating thread");

  if(!m_running) return;
  m_running = false;

  // Join all joinable threads
  for( auto& thread : m_threads ) {
    thread.join();
  }
}

//-----------------------------------------------------------------------------

void bit::platform::concurrent_task_scheduler::wait( task_handle task )
{
  help_while([&]{ return !task.completed(); });
}

void bit::platform::concurrent_task_scheduler::post_task( task task )
{
  push_task( std::move(task) );
}

//-----------------------------------------------------------------------------
// Private Capacity
//-----------------------------------------------------------------------------

bool bit::platform::concurrent_task_scheduler::has_remaining_tasks()
  const noexcept
{
  for( auto& queue : m_queues ) {
    if( !queue->empty() ) return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
// Private Modifiers
//-----------------------------------------------------------------------------

void bit::platform::concurrent_task_scheduler::start()
{
  // Set the owner on 'start' call
  if( m_owner == std::thread::id() ) {
    m_owner = std::this_thread::get_id();
    g_thread_index = 0;
    g_this_concurrent_task_scheduler = this;
  }

  assert( m_owner == std::this_thread::get_id() &&
          "task_concurrent_task_scheduler can only be started on the creating thread" );

  if( m_running ) return;

  m_running = true;

  // Populate n+1 task queues
  for( auto& queue : m_queues ) {
    queue = get_task_queue();
  }

  // Makes n working threads
  auto index = std::ptrdiff_t{1};
  for( auto& thread : m_threads ) {
    thread = make_worker_thread( index++ );
  }

  // Sets the affinity on every thread
  if( m_set_affinity ) {
    this_thread::set_affinity( 0 );
  }
}

std::thread bit::platform::concurrent_task_scheduler::make_worker_thread( std::ptrdiff_t index )
{
  return std::thread([this,index]()
  {
    if( m_set_affinity ) {
      auto const cpus = std::thread::hardware_concurrency();
      this_thread::set_affinity( static_cast<std::size_t>(index) % cpus );
    }

    g_thread_index = index;
    g_this_concurrent_task_scheduler = this;

    ++m_running_threads;
    do_work();
    --m_running_threads;

    std::unique_lock<std::mutex> lock(m_lock);
    m_cv.wait(lock,[&]{ return m_running_threads == 0; });
    m_cv.notify_all();
  });
}

//-----------------------------------------------------------------------------

bit::platform::task bit::platform::concurrent_task_scheduler::get_task()
{
  auto& queue = *m_queues[g_thread_index];

  auto j = queue.pop();
  if( j ) return j;

  const auto max    = static_cast<std::ptrdiff_t>(m_queues.size()) - 1;
  const auto victim = generate_number_in_range(0, max);

  auto& steal_queue = *m_queues[victim];

  // If the two queues are the same, yield processor time
  if( &queue == &steal_queue ) {
    std::this_thread::yield();
    return task{};
  }

  // Attempt to steal a task
  j = steal_queue.steal();

  // If task is not stolen, yield processor time
  if( !j ) {
    std::this_thread::yield();
    return task{};
  }

  return j;
}

void bit::platform::concurrent_task_scheduler::push_task( task task )
{
  if( !m_running ) std::terminate();
  m_queues[g_thread_index]->push( std::move(task) );
  m_cv.notify_all();
}

template<typename Condition>
void bit::platform::concurrent_task_scheduler::help_while( Condition&& condition )
{
  while( std::forward<Condition>(condition)() ) {
    auto j = get_task();

    if( j ) {
      help_while_unavailable( j );

      j.execute();
    }
  }
}

void bit::platform::concurrent_task_scheduler::help_while_unavailable( const task& j )
{
  help_while( [&]{ return !j.available(); } );
}

void bit::platform::concurrent_task_scheduler::do_work()
{
  help_while( [&]{ return m_running; } );

  // This duplication is to avoid breaking cache coherency per iteration
  // in the normal running case.
  help_while( [&]{ return !m_queues[g_thread_index]->empty(); } );
}

//-----------------------------------------------------------------------------
// Free Functions
//-----------------------------------------------------------------------------

void bit::platform::post_task( concurrent_task_scheduler& scheduler, task task )
{
  scheduler.post_task( std::move(task) );
}

void bit::platform::wait( concurrent_task_scheduler& scheduler, task_handle task )
{
  scheduler.wait(task);
}

//-----------------------------------------------------------------------------
// This Dispatcher : Free Functions
//-----------------------------------------------------------------------------

namespace {

  std::ptrdiff_t generate_number_in_range( std::ptrdiff_t low,
                                           std::ptrdiff_t high )
  {
    static std::random_device device{};
    thread_local auto s_engine = std::mt19937( device() );
    std::uniform_int_distribution<> distribution(low,high);

    return distribution(s_engine);
  }

  //---------------------------------------------------------------------------

  bit::platform::detail::task_queue* get_task_queue()
  {
    // TODO: unique_ptr is a temporary hack, since glibc doesn't support thread_local
    //       task_queue with nontrivial destructor
    static auto s_queues = std::vector<std::unique_ptr<bit::platform::detail::task_queue>>();

    s_queues.push_back( std::make_unique<bit::platform::detail::task_queue>() );

    return s_queues.back().get();
  }

} // namespace anonymous
