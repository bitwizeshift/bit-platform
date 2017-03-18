#ifndef BIT_PLATFORM_THREADING_DETAIL_UNLOCK_GUARD_INL
#define BIT_PLATFORM_THREADING_DETAIL_UNLOCK_GUARD_INL

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

template<typename Mutex>
inline bit::platform::detail::unlockable<Mutex>::unlockable( Mutex& mutex )
  : m_mutex(mutex)
{

}

//----------------------------------------------------------------------------
// Locking
//----------------------------------------------------------------------------

template<typename Mutex>
inline void bit::platform::detail::unlockable<Mutex>::lock()
{
  m_mutex.unlock();
}

template<typename Mutex>
inline void bit::platform::detail::unlockable<Mutex>::unlock()
{
  m_mutex.lock();
}

#endif /* BIT_PLATFORM_THREADING_DETAIL_UNLOCK_GUARD_INL */
