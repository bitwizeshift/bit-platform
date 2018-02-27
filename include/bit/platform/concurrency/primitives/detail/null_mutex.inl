#ifndef BIT_PLATFORM_CONCURRENCY_PRIMITIVES_DETAIL_NULL_MUTEX_INL
#define BIT_PLATFORM_CONCURRENCY_PRIMITIVES_DETAIL_NULL_MUTEX_INL

//----------------------------------------------------------------------------
// Unique Locking
//----------------------------------------------------------------------------

inline constexpr void
  bit::platform::null_mutex::lock()
  noexcept
{

}

inline constexpr bool
  bit::platform::null_mutex::try_lock()
  noexcept
{
  return true;
}

template<typename Rep, typename Period>
inline constexpr bool
  bit::platform::null_mutex::try_lock_for( const duration<Rep,Period>& duration )
  noexcept
{
  return true;
}

template<typename Clock, typename Duration>
inline constexpr bool
  bit::platform::null_mutex::try_lock_until( const time_point<Clock,Duration>& time )
  noexcept
{
  return true;
}

inline constexpr void
  bit::platform::null_mutex::unlock()
  noexcept
{

}

//----------------------------------------------------------------------------
// Shared Locking
//----------------------------------------------------------------------------

inline constexpr void
  bit::platform::null_mutex::lock_shared()
  noexcept
{

}

inline constexpr bool
  bit::platform::null_mutex::try_lock_shared()
  noexcept
{
  return true;
}

template<typename Rep, typename Period>
inline constexpr bool
  bit::platform::null_mutex::try_lock_shared_for( const duration<Rep,Period>& duration )
  noexcept
{
  return true;
}

template<typename Clock, typename Duration>
inline constexpr bool
  bit::platform::null_mutex::try_lock_shared_until( const time_point<Clock,Duration>& time )
  noexcept
{
  return true;
}

inline constexpr void
  bit::platform::null_mutex::unlock_shared()
  noexcept
{

}

#endif /* BIT_PLATFORM_CONCURRENCY_PRIMITIVES_DETAIL_NULL_MUTEX_INL */
