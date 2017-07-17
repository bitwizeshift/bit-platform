#include <bit/platform/threading/job_dispatcher.hpp>
#include <bit/platform/threading/thread.hpp>
#include <bit/stl/assert.hpp>

#include <random> // std::random_device, sdt::uniform_distribution, etc
#include <memory> // std::unique_ptr
#include <vector> // std::vector
#include <array>  // std::array

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

  thread_local bit::platform::job*  g_this_job     = nullptr;
  thread_local std::ptrdiff_t       g_thread_index = 0;

} // namespace anonymous

//============================================================================
// job_queue
//============================================================================

//////////////////////////////////////////////////////////////////////////////
///
///
//////////////////////////////////////////////////////////////////////////////
class bit::platform::detail::job_queue
{
  //--------------------------------------------------------------------------
  // Constructor
  //--------------------------------------------------------------------------
public:

  job_queue();

  ~job_queue(){

  }

  //--------------------------------------------------------------------------
  // Modifiers
  //--------------------------------------------------------------------------
public:

  /// \brief Pushes a new job into the queue
  ///
  /// \param j the job to push
  void push( job* j );

  /// \brief Pops a job from the front of this job_queue
  ///
  /// \return the popped job, or nullptr on failure
  job* pop();

  /// \brief Steals a job from the back of this job_queue
  ///
  /// \return the stolen job, or nullptr on failure
  job* steal();

  //--------------------------------------------------------------------------
  // Capacity
  //--------------------------------------------------------------------------
public:

  /// \brief Queries whether this job_queue is empty
  ///
  /// \return \c true when empty
  bool empty() const noexcept;

  //--------------------------------------------------------------------------
  // Private Members
  //--------------------------------------------------------------------------
private:

  static auto constexpr max_jobs = job_dispatcher::max_jobs;

  std::array<job*,max_jobs> m_jobs;
  std::ptrdiff_t            m_bottom;
  std::ptrdiff_t            m_top;
  mutable std::mutex        m_lock;
};

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

void bit::platform::detail::job_queue::push( job* j )
{
  std::lock_guard<std::mutex> lock(m_lock);

  m_jobs[m_bottom++ % max_jobs] = j;
}

//----------------------------------------------------------------------------

bit::platform::job* bit::platform::detail::job_queue::pop()
{
  std::lock_guard<std::mutex> lock(m_lock);

  // If there are no jobs, return null
  if( m_bottom <= m_top ) return nullptr;

  return m_jobs[--m_bottom % max_jobs];
}

bit::platform::job* bit::platform::detail::job_queue::steal()
{
  std::lock_guard<std::mutex> lock(m_lock);

  // If there are no jobs, return null
  if( m_bottom <= m_top ) return nullptr;

  return m_jobs[m_top++ % max_jobs];
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

//============================================================================
// Private Detail Function
//============================================================================

void* bit::platform::detail::allocate_job()
{
  thread_local std::array<job,job_dispatcher::max_jobs> s_jobs = {};
  thread_local auto s_index = std::size_t{0u};

  return &s_jobs[s_index++ % job_dispatcher::max_jobs];
}

//============================================================================
// job
//============================================================================

//----------------------------------------------------------------------------
// Modifiers
//----------------------------------------------------------------------------

void bit::platform::job::execute()
{
  g_this_job = this;
  m_function( static_cast<void*>(&m_padding[0]) );

  finalize();
}

//----------------------------------------------------------------------------
// Private Modifiers
//----------------------------------------------------------------------------

void bit::platform::job::finalize()
{
  auto unfinished = --m_unfinished;
  if( unfinished == 0 && m_parent ) {
    m_parent->finalize();
  }
  g_this_job = nullptr;
}

bit::platform::job* bit::platform::this_job()
{
  return g_this_job;
}

//============================================================================
// job_dispatcher
//============================================================================

//----------------------------------------------------------------------------
// Constructors / Destructor
//----------------------------------------------------------------------------

bit::platform::job_dispatcher::job_dispatcher()
  : job_dispatcher( std::thread::hardware_concurrency() - 1 )
{

}

bit::platform::job_dispatcher::job_dispatcher( std::size_t threads )
  : m_running(false),
    m_set_affinity(false),
    m_running_threads(0)
{
  m_threads.resize(threads);
  m_queues.resize(threads+1);
}


bit::platform::job_dispatcher::job_dispatcher( assign_affinity_t )
  : job_dispatcher( assign_affinity, std::thread::hardware_concurrency() - 1 )
{

}

bit::platform::job_dispatcher::job_dispatcher( assign_affinity_t, std::size_t threads )
: m_running(false),
  m_set_affinity(true),
  m_owner(),
  m_running_threads(0)

{
  m_threads.resize(threads);
  m_queues.resize(threads+1);
}

//----------------------------------------------------------------------------

bit::platform::job_dispatcher::~job_dispatcher()
{
  stop();
}

//----------------------------------------------------------------------------
// Modifiers
//----------------------------------------------------------------------------

void bit::platform::job_dispatcher::run()
{
  start();

  do_work();
}

void bit::platform::job_dispatcher::stop()
{
  BIT_ASSERT( m_owner == std::this_thread::get_id(), "job_dispatcher can only be stopped on the creating thread");

  if(!m_running) return;
  m_running = false;

  // Join all joinable threads
  for( auto& thread : m_threads ) {
    thread.join();
  }
}

void bit::platform::job_dispatcher::wait( const job* j )
{
  help_while([&]{ return !j->completed(); });
}

//----------------------------------------------------------------------------
// Private Capacity
//----------------------------------------------------------------------------

bool bit::platform::job_dispatcher::has_remaining_jobs()
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

void bit::platform::job_dispatcher::start()
{
  // Set the owner on 'start' call
  if( m_owner == std::thread::id() ) {
    m_owner = std::this_thread::get_id();
    g_thread_index = 0;
  }

  BIT_ASSERT( m_owner == std::this_thread::get_id(), "job_dispatcher can only be started on the creating thread");

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

    auto const cpus = std::thread::hardware_concurrency();
    auto index = std::size_t{1u};

    set_affinity( 0 );
    for( auto& thread : m_threads ) {
      set_affinity( thread, index++ % cpus );
    }
  }
}

std::thread bit::platform::job_dispatcher::make_worker_thread( std::ptrdiff_t index )
{
  return std::thread([this,index]()
  {
    g_thread_index = index;

    ++m_running_threads;
    do_work();
    --m_running_threads;

    std::unique_lock<std::mutex> lock(m_lock);
    m_cv.wait(lock,[&]{ return m_running_threads == 0; });
    m_cv.notify_all();
  });
}

//----------------------------------------------------------------------------

bit::platform::job* bit::platform::job_dispatcher::get_job()
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
    return nullptr;
  }

  // Attempt to steal a job
  j = steal_queue.steal();

  // If job is not stolen, yield processor time
  if( !j ) {
    std::this_thread::yield();
    return nullptr;
  }

  return j;
}

void bit::platform::job_dispatcher::push_job( std::ptrdiff_t index, job* j )
{
  if( !m_running ) return;
  m_queues[index]->push(j);
}

template<typename Condition>
void bit::platform::job_dispatcher::help_while( Condition&& condition )
{
  while( std::forward<Condition>(condition)() ) {
    auto j = get_job();

    if( j ) {
      help_while_unavailable( j );

      j->execute();
    }
  }
}

void bit::platform::job_dispatcher::help_while_unavailable( const job* j )
{
  help_while( [&]{ return !j->available(); } );
}

void bit::platform::job_dispatcher::do_work()
{
  help_while( [&]{ return m_running; } );

  // This duplication is to avoid breaking cache coherency per iteration
  // in the normal running case.
  help_while( [&]{ return has_remaining_jobs(); } );
}

//----------------------------------------------------------------------------
// Free Functions
//----------------------------------------------------------------------------

const bit::platform::job*
  bit::platform::post_job( job_dispatcher& dispatcher, const job* job )
{
  dispatcher.push_job( g_thread_index, const_cast<class job*>(job) );

  return job;
}

void bit::platform::wait( job_dispatcher& dispatcher, const job* job )
{
  dispatcher.wait(job);
}

namespace {

  std::ptrdiff_t generate_number_in_range( std::ptrdiff_t low,
                                           std::ptrdiff_t high )
  {
    thread_local auto s_engine = std::mt19937( std::random_device{}() );
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
