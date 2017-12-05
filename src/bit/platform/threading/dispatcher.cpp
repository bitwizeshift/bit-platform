#include <bit/platform/threading/dispatcher.hpp>
#include <bit/platform/threading/thread.hpp>

#include <random> // std::random_device, sdt::uniform_distribution, etc
#include <memory> // std::unique_ptr
#include <vector> // std::vector

#include <cassert> // assert

#include "detail/job_queue.hpp" // detail::job_queue

//============================================================================
// Anonymous Namespaces
//============================================================================

namespace {

  //--------------------------------------------------------------------------
  // Utility Functions
  //--------------------------------------------------------------------------

  /// \brief Generates a random number between \p low and \p high
  ///
  /// \param low the low bound
  /// \param high the high bound
  /// \return the random number
  std::ptrdiff_t generate_number_in_range( std::ptrdiff_t low,
                                           std::ptrdiff_t high );

  /// \brief Gets the address of this thread's active job queue
  ///
  /// \return pointer to the job queue
  bit::platform::detail::job_queue* get_job_queue();

  //--------------------------------------------------------------------------
  // Globals
  //--------------------------------------------------------------------------

  thread_local std::ptrdiff_t     g_thread_index = 0;
  thread_local bit::platform::dispatcher* g_this_dispatcher = nullptr;

} // namespace anonymous

std::ptrdiff_t bit::platform::worker_thread_id()
{
  return g_thread_index;
}

//============================================================================
// job_dispatcher
//============================================================================

//----------------------------------------------------------------------------
// Constructors / Destructor
//----------------------------------------------------------------------------

bit::platform::dispatcher::dispatcher()
  : dispatcher( std::thread::hardware_concurrency() - 1 )
{

}

bit::platform::dispatcher::dispatcher( std::size_t threads )
  : m_running(false),
    m_set_affinity(false),
    m_running_threads(0)
{
  m_threads.resize(threads);
  m_queues.resize(threads+1);
}


bit::platform::dispatcher::dispatcher( assign_affinity_t )
  : dispatcher( assign_affinity, std::thread::hardware_concurrency() - 1 )
{

}

bit::platform::dispatcher::dispatcher( assign_affinity_t, std::size_t threads )
: m_running(false),
  m_set_affinity(true),
  m_owner(),
  m_running_threads(0)

{
  m_threads.resize(threads);
  m_queues.resize(threads+1);
}

//----------------------------------------------------------------------------

bit::platform::dispatcher::~dispatcher()
{
  stop();
}

//----------------------------------------------------------------------------
// Modifiers
//----------------------------------------------------------------------------

void bit::platform::dispatcher::stop()
{
  assert( m_owner == std::this_thread::get_id() && "job_dispatcher can only be stopped on the creating thread");

  if(!m_running) return;
  m_running = false;

  // Join all joinable threads
  for( auto& thread : m_threads ) {
    thread.join();
  }
}

//----------------------------------------------------------------------------

void bit::platform::dispatcher::wait( job_handle job )
{
  help_while([&]{ return !job.completed(); });
}

void bit::platform::dispatcher::post_job( job job )
{
  push_job( std::move(job) );
}

//----------------------------------------------------------------------------
// Private Capacity
//----------------------------------------------------------------------------

bool bit::platform::dispatcher::has_remaining_jobs()
  const noexcept
{
  for( auto& queue : m_queues ) {
    if( !queue->empty() ) return true;
  }
  return false;
}

//----------------------------------------------------------------------------
// Private Modifiers
//----------------------------------------------------------------------------

void bit::platform::dispatcher::start()
{
  // Set the owner on 'start' call
  if( m_owner == std::thread::id() ) {
    m_owner = std::this_thread::get_id();
    g_thread_index = 0;
    g_this_dispatcher = this;
  }

  assert( m_owner == std::this_thread::get_id() && "job_dispatcher can only be started on the creating thread");

  if( m_running ) return;

  m_running = true;

  // Populate n+1 job queues
  for( auto& queue : m_queues ) {
    queue = get_job_queue();
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

std::thread bit::platform::dispatcher::make_worker_thread( std::ptrdiff_t index )
{
  return std::thread([this,index]()
  {
    if( m_set_affinity ) {
      auto const cpus = std::thread::hardware_concurrency();
      this_thread::set_affinity( static_cast<std::size_t>(index) % cpus );
    }

    g_thread_index = index;
    g_this_dispatcher = this;

    ++m_running_threads;
    do_work();
    --m_running_threads;

    std::unique_lock<std::mutex> lock(m_lock);
    m_cv.wait(lock,[&]{ return m_running_threads == 0; });
    m_cv.notify_all();
  });
}

//----------------------------------------------------------------------------

bit::platform::job bit::platform::dispatcher::get_job()
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
    return job{};
  }

  // Attempt to steal a job
  j = steal_queue.steal();

  // If job is not stolen, yield processor time
  if( !j ) {
    std::this_thread::yield();
    return job{};
  }

  return j;
}

void bit::platform::dispatcher::push_job( job job )
{
  if( !m_running ) std::terminate();
  m_queues[g_thread_index]->push( std::move(job) );
  m_cv.notify_all();
}

template<typename Condition>
void bit::platform::dispatcher::help_while( Condition&& condition )
{
  while( std::forward<Condition>(condition)() ) {
    auto j = get_job();

    if( j ) {
      help_while_unavailable( j );

      j.execute();
    }
  }
}

void bit::platform::dispatcher::help_while_unavailable( const job& j )
{
  help_while( [&]{ return !j.available(); } );
}

void bit::platform::dispatcher::do_work()
{
  help_while( [&]{ return m_running; } );

  // This duplication is to avoid breaking cache coherency per iteration
  // in the normal running case.
  help_while( [&]{ return !m_queues[g_thread_index]->empty(); } );
}

//----------------------------------------------------------------------------
// Free Functions
//----------------------------------------------------------------------------

void bit::platform::post_job( dispatcher& dispatcher, job job )
{
  dispatcher.post_job( std::move(job) );
}

void bit::platform::wait( dispatcher& dispatcher, job_handle job )
{
  dispatcher.wait(job);
}

//----------------------------------------------------------------------------
// This Dispatcher : Free Functions
//----------------------------------------------------------------------------

void bit::platform::this_dispatcher::post_job( job job )
{
  assert( g_this_dispatcher && "post_job can only be called in a dispatcher's job queue" );

  auto& dispatcher = *g_this_dispatcher;
  dispatcher.post_job( std::move(job) );
}

void bit::platform::this_dispatcher::wait( job_handle job )
{
  assert( g_this_dispatcher && "wait can only be called in a dispatcher's job queue" );

  auto& dispatcher = *g_this_dispatcher;
  dispatcher.wait( job );
}


namespace {

  std::ptrdiff_t generate_number_in_range( std::ptrdiff_t low,
                                           std::ptrdiff_t high )
  {
    static std::random_device device{};
    thread_local auto s_engine = std::mt19937( device() );
    std::uniform_int_distribution<> distribution(low,high);

    return distribution(s_engine);
  }

  //--------------------------------------------------------------------------

  bit::platform::detail::job_queue* get_job_queue()
  {
    // TODO: unique_ptr is a temporary hack, since glibc doesn't support thread_local
    //       job_queue with nontrivial destructor
    static auto s_queues = std::vector<std::unique_ptr<bit::platform::detail::job_queue>>();

    s_queues.push_back( std::make_unique<bit::platform::detail::job_queue>() );

    return s_queues.back().get();
  }

} // namespace anonymous
