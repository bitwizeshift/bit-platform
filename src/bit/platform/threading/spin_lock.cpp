#include <bit/platform/threading/spin_lock.hpp>

#include <thread>

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

bit::platform::spin_lock::spin_lock()
  noexcept
  : m_lock(ATOMIC_FLAG_INIT)
{

}

//----------------------------------------------------------------------------
// Locking
//----------------------------------------------------------------------------

void bit::platform::spin_lock::lock()
  noexcept
{
  while( m_lock.test_and_set(std::memory_order_acquire) ) {
    std::this_thread::yield();
  }
}

bool bit::platform::spin_lock::try_lock()
  noexcept
{
  return m_lock.test_and_set(std::memory_order_acquire);
}

void bit::platform::spin_lock::unlock()
  noexcept
{
  m_lock.clear(std::memory_order_release);
}
