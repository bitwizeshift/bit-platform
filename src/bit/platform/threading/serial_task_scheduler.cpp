#include <bit/platform/threading/serial_task_scheduler.hpp>
#include <random> // std::random_device, sdt::uniform_distribution, etc
#include <memory> // std::unique_ptr
#include <vector> // std::vector

#include <cassert> // assert

#include "detail/task_queue.hpp" // detail::task_queue

//=============================================================================
// Anonymous Declarations
//=============================================================================

namespace {

  //---------------------------------------------------------------------------
  // Globals
  //---------------------------------------------------------------------------

  thread_local bit::platform::serial_task_scheduler* g_this_serial_scheduler = nullptr;

} // namespace anonymous

//=============================================================================
// Anonymous Declarations
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------

bit::platform::serial_task_scheduler::serial_task_scheduler()
  : m_is_running(false),
    m_worker{ std::thread{},
              std::make_unique<detail::task_queue>() } // HACK: temporary m_queue
{

}

bit::platform::serial_task_scheduler::~serial_task_scheduler()
{
  stop();
}

//-----------------------------------------------------------------------------
// Modifiers
//-----------------------------------------------------------------------------

void bit::platform::serial_task_scheduler::start()
{
  if( m_is_running ) return;

  m_is_running = true;
  m_worker.thread = std::thread(&serial_task_scheduler::run,this);

#ifndef NDEBUG
  m_owner = std::this_thread::get_id();
#endif
}

void bit::platform::serial_task_scheduler::stop()
{
#ifndef NDEBUG
  assert( m_owner == std::this_thread::get_id() &&
          "serial_task_scheduler can only be stopped by the thread that started it" );
#endif

  if( !m_is_running ) return;

  m_is_running = false;
  m_cv.notify_all();
  m_worker.thread.join();
}

void bit::platform::serial_task_scheduler::wait( task_handle task )
{
  // wait until task is completed
  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);

  m_cv.wait(lock, [&]{ return task.completed(); });
}

//-----------------------------------------------------------------------------

void bit::platform::serial_task_scheduler::post_task( task task )
{
  if( !m_is_running ) return;

  m_worker.task_queue->push( std::move(task) );
}

//-----------------------------------------------------------------------------
// Private Modifiers
//-----------------------------------------------------------------------------

void bit::platform::serial_task_scheduler::run()
{
  g_this_serial_scheduler = this;

  while( true ) {

    // wait until task is entered into queue
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_cv.wait(lock, [&]{ return !m_is_running || !m_worker.task_queue->empty(); });
    }

    if( !m_is_running && m_worker.task_queue->empty() ) break;

    auto j = get_task();

    assert( j != nullptr );

    j.execute();
    m_cv.notify_all();
  }
}

bit::platform::task bit::platform::serial_task_scheduler::get_task()
{
  return m_worker.task_queue->steal();
}

//-----------------------------------------------------------------------------
// Free Functions
//-----------------------------------------------------------------------------

void bit::platform::post_task( serial_task_scheduler& scheduler, task task )
{
  scheduler.post_task( std::move(task) );
}

void bit::platform::wait( serial_task_scheduler& scheduler, task_handle task )
{
  scheduler.wait( task );
}

//-----------------------------------------------------------------------------
// This Dispatch Queue : Free Functions
//-----------------------------------------------------------------------------

void bit::platform::this_serial_task_scheduler::post_task( task task )
{
  assert( g_this_serial_scheduler != nullptr &&
          "post_task can only be called in an active scheduler" );

  auto& scheduler = *g_this_serial_scheduler;
  scheduler.post_task( std::move(task) );
}

void bit::platform::this_serial_task_scheduler::wait( task_handle task )
{
  assert( g_this_serial_scheduler != nullptr &&
          "wait can only be called in an active scheduler" );

  auto& scheduler = *g_this_serial_scheduler;
  scheduler.wait( task );
}