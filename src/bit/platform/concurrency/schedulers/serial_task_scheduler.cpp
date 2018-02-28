#include <bit/platform/concurrency/schedulers/serial_task_scheduler.hpp>
#include <random> // std::random_device, sdt::uniform_distribution, etc
#include <memory> // std::unique_ptr
#include <vector> // std::vector
#include <cassert> // assert
#include <utility> // std::forward

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


//-----------------------------------------------------------------------------

void bit::platform::serial_task_scheduler::post_task( task task )
{
  if( !m_is_running ) return;

  m_worker.task_queue->push( std::move(task) );
  m_cv.notify_one();
}

void bit::platform::serial_task_scheduler::wait( task_handle task )
{
  // wait until task is completed
  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);

  m_cv.wait(lock, [&]{ return task.completed(); });
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

    // Check condition for breaking above lock
    if( !m_is_running && m_worker.task_queue->empty() ) break;

    auto task = get_task();

    assert( task != nullptr );

    task_scheduler::execute_task( std::move(task) );
    m_cv.notify_all();
  }
}

bit::platform::task bit::platform::serial_task_scheduler::get_task()
{
  return m_worker.task_queue->steal();
}
