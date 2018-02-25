#include <bit/platform/threading/dispatch_queue.hpp>

#include <random> // std::random_device, sdt::uniform_distribution, etc
#include <memory> // std::unique_ptr
#include <vector> // std::vector

#include <cassert> // assert

#include "detail/task_queue.hpp" // detail::task_queue

//=============================================================================
// Anonymous Declarations
//=============================================================================

namespace {

  /// \brief Gets the address of this thread's active task queue
  ///
  /// \return pointer to the task queue
//  bit::platform::detail::task_queue* get_task_queue();

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
    m_queue(std::make_unique<detail::task_queue>()) // HACK: temporary m_queue
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

void bit::platform::dispatch_queue::wait( task_handle task )
{
  // wait until task is completed
  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);

  m_cv.wait(lock, [&]{ return task.completed(); });
}

//-----------------------------------------------------------------------------

void bit::platform::dispatch_queue::post_task( task task )
{
  if( !m_is_running )
    std::terminate();

  m_queue->push( std::move(task) );
}

//-----------------------------------------------------------------------------
// Private Modifiers
//-----------------------------------------------------------------------------

void bit::platform::dispatch_queue::run()
{
  g_this_queue = this;

  while( true ) {

    if( !m_is_running && m_queue->empty() ) break;

    // wait until task is entered into queue
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_cv.wait(lock, [&]{ return !m_queue->empty(); });
    }

    auto j = get_task();

    assert( j != nullptr );

    j.execute();
    m_cv.notify_all();
  }
}

bit::platform::task bit::platform::dispatch_queue::get_task()
{
  return m_queue->steal();
}

//-----------------------------------------------------------------------------
// Free Functions
//-----------------------------------------------------------------------------

void bit::platform::post_task( dispatch_queue& queue, task task )
{
  queue.post_task( std::move(task) );
}

void bit::platform::wait( dispatch_queue& queue, task_handle task )
{
  queue.wait( task );
}

//-----------------------------------------------------------------------------
// This Dispatch Queue : Free Functions
//-----------------------------------------------------------------------------

void bit::platform::this_dispatch_queue::post_task( task task )
{
  assert( g_this_queue != nullptr && "post_task can only be called in an active dispatch queue" );

  auto& queue = *g_this_queue;
  queue.post_task( std::move(task) );
}

void bit::platform::this_dispatch_queue::wait( task_handle task )
{
  assert( g_this_queue != nullptr && "wait can only be called in an active dispatch queue" );

  auto& queue = *g_this_queue;
  queue.wait( task );
}

//=============================================================================
// Anonymous Definitions
//=============================================================================

namespace {

//  bit::platform::detail::task_queue* get_task_queue()
//  {
//    thread_local bit::platform::detail::task_queue s_queue;
//
//    return &s_queue;
//  }

} // namespace anonymous
