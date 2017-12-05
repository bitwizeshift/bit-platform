#include <bit/platform/threading/dispatch_queue.hpp>

#include <random> // std::random_device, sdt::uniform_distribution, etc
#include <memory> // std::unique_ptr
#include <vector> // std::vector

#include <cassert> // assert

#include "detail/job_queue.hpp" // detail::job_queue

//=============================================================================
// Anonymous Declarations
//=============================================================================

namespace {

  /// \brief Gets the address of this thread's active job queue
  ///
  /// \return pointer to the job queue
//  bit::platform::detail::job_queue* get_job_queue();

  //---------------------------------------------------------------------------
  // Globals
  //---------------------------------------------------------------------------

  thread_local bit::platform::dispatch_queue* g_this_queue = nullptr;

} // namespace anonymous

//=============================================================================
// Anonymous Declarations
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------

bit::platform::dispatch_queue::dispatch_queue()
  : m_is_running(false),
    m_queue(std::make_unique<detail::job_queue>()) // HACK: temporary m_queue
{

}

bit::platform::dispatch_queue::~dispatch_queue()
{
  stop();
}

//-----------------------------------------------------------------------------
// Modifiers
//-----------------------------------------------------------------------------

void bit::platform::dispatch_queue::start()
{
  if( m_is_running ) return;

  m_is_running = true;
  m_thread = std::thread(&dispatch_queue::run,this);
}

void bit::platform::dispatch_queue::stop()
{
  if( !m_is_running ) return;

  m_is_running = false;
  m_cv.notify_all();
  m_thread.join();
}

void bit::platform::dispatch_queue::wait( job_handle job )
{
  // wait until job is completed
  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);

  m_cv.wait(lock, [&]{ return job.completed(); });
}

//-----------------------------------------------------------------------------

void bit::platform::dispatch_queue::post_job( job job )
{
  if( !m_is_running )
    std::terminate();

  m_queue->push( std::move(job) );
}

//-----------------------------------------------------------------------------
// Private Modifiers
//-----------------------------------------------------------------------------

void bit::platform::dispatch_queue::run()
{
  g_this_queue = this;

  while( true ) {

    if( !m_is_running && m_queue->empty() ) break;

    // wait until job is entered into queue
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_cv.wait(lock, [&]{ return !m_queue->empty(); });
    }

    auto j = get_job();

    assert( j != nullptr );

    j.execute();
    m_cv.notify_all();
  }
}

bit::platform::job bit::platform::dispatch_queue::get_job()
{
  return m_queue->steal();
}

//-----------------------------------------------------------------------------
// Free Functions
//-----------------------------------------------------------------------------

void bit::platform::post_job( dispatch_queue& queue, job job )
{
  queue.post_job( std::move(job) );
}

void bit::platform::wait( dispatch_queue& queue, job_handle job )
{
  queue.wait( job );
}

//-----------------------------------------------------------------------------
// This Dispatch Queue : Free Functions
//-----------------------------------------------------------------------------

void bit::platform::this_dispatch_queue::post_job( job job )
{
  assert( g_this_queue != nullptr && "post_job can only be called in an active dispatch queue" );

  auto& queue = *g_this_queue;
  queue.post_job( std::move(job) );
}

void bit::platform::this_dispatch_queue::wait( job_handle job )
{
  assert( g_this_queue != nullptr && "wait can only be called in an active dispatch queue" );

  auto& queue = *g_this_queue;
  queue.wait( job );
}

//=============================================================================
// Anonymous Definitions
//=============================================================================

namespace {

//  bit::platform::detail::job_queue* get_job_queue()
//  {
//    thread_local bit::platform::detail::job_queue s_queue;
//
//    return &s_queue;
//  }

} // namespace anonymous
