#ifndef BIT_PLATFORM_THREADING_DETAIL_CONCURRENT_QUEUE_INL
#define BIT_PLATFORM_THREADING_DETAIL_CONCURRENT_QUEUE_INL

//----------------------------------------------------------------------------
// Constructors / Assignment
//----------------------------------------------------------------------------

template<typename T, typename Lock, typename Allocator>
bit::platform::concurrent_queue<T,Lock,Allocator>::concurrent_queue()
  : concurrent_queue( Allocator() )
{

}

template<typename T, typename Lock, typename Allocator>
bit::platform::concurrent_queue<T,Lock,Allocator>
  ::concurrent_queue( const Allocator& alloc )
  : m_queue( alloc )
{

}

template<typename T, typename Lock, typename Allocator>
bit::platform::concurrent_queue<T,Lock,Allocator>
  ::concurrent_queue( concurrent_queue&& other )
  : concurrent_queue( std::move(other), Allocator() )
{

}

template<typename T, typename Lock, typename Allocator>
bit::platform::concurrent_queue<T,Lock,Allocator>
  ::concurrent_queue( concurrent_queue&& other, const Allocator& alloc )
  : m_queue( std::move(other.m_queue), alloc )
{

}

//----------------------------------------------------------------------------

template<typename T, typename Lock, typename Allocator>
bit::platform::concurrent_queue<T,Lock,Allocator>&
  bit::platform::concurrent_queue<T,Lock,Allocator>::operator=( concurrent_queue&& other )
  noexcept
{
  m_queue = std::move(other.m_queue);
  return (*this);
}

//----------------------------------------------------------------------------
// Capacity
//----------------------------------------------------------------------------

template<typename T, typename Lock, typename Allocator>
bool bit::platform::concurrent_queue<T,Lock,Allocator>::empty()
  const noexcept
{
  return m_queue.empty();
}

template<typename T, typename Lock, typename Allocator>
typename bit::platform::concurrent_queue<T,Lock,Allocator>::size_type
  bit::platform::concurrent_queue<T,Lock,Allocator>::size()
  const noexcept
{
  return m_queue.size();
}

//----------------------------------------------------------------------------
// Modifiers
//----------------------------------------------------------------------------

template<typename T, typename Lock, typename Allocator>
void bit::platform::concurrent_queue<T,Lock,Allocator>::pop( T* value )
{
  BIT_ASSERT( value, "concurrent_queue::pop: value cannot be null");

  std::unique_lock<std::mutex> lock(m_lock);
  m_cv.wait(lock, [&]{ return !m_queue.empty(); });
  (*value) = std::move(m_queue.front());
  m_queue.pop();
}

//----------------------------------------------------------------------------

template<typename T, typename Lock, typename Allocator>
void bit::platform::concurrent_queue<T,Lock,Allocator>
  ::push_back( const T& value )
{
  std::unique_lock<lock_type> lock(m_lock);
  m_queue.push(value);
  lock.unlock();
  m_cv.notify_one();
}

template<typename T, typename Lock, typename Allocator>
void bit::platform::concurrent_queue<T,Lock,Allocator>
  ::push_back( T&& value )
{
  std::unique_lock<lock_type> lock(m_lock);
  m_queue.push(std::move(value));
  lock.unlock();
  m_cv.notify_one();
}


template<typename T, typename Lock, typename Allocator>
template<typename...Args, std::enable_if_t<std::is_constructible<T,Args...>::value>*>
void bit::platform::concurrent_queue<T,Lock,Allocator>
  ::emplace_back( Args&&...args )
{
  std::unique_lock<lock_type> lock(m_lock);
  m_queue.emplace( std::forward<Args>(args)... );
  lock.unlock();
  m_cv.notify_one();
}

//----------------------------------------------------------------------------


template<typename T, typename Lock, typename Allocator>
bool bit::platform::concurrent_queue<T,Lock,Allocator>::try_pop( T* value )
{
  BIT_ASSERT( value, "concurrent_queue::pop: value cannot be null");

  std::unique_lock<std::mutex> lock(m_lock);

  if( m_queue.empty() ) return false;

  m_cv.wait(lock, [&]{ return !m_queue.empty(); });
  (*value) = std::move(m_queue.front());
  m_queue.pop();
  return true;
}

template<typename T, typename Lock, typename Allocator>
bool bit::platform::concurrent_queue<T,Lock,Allocator>
  ::try_push_back( const T& value )
{
  std::unique_lock<lock_type> lock(m_lock, std::try_to_lock);
  if( !lock.owns_lock() ) return false;

  m_queue.push_back( value );
  lock.unlock();
  m_cv.notify_one();
  return true;
}

template<typename T, typename Lock, typename Allocator>
bool bit::platform::concurrent_queue<T,Lock,Allocator>
  ::try_push_back( T&& value )
{
  std::unique_lock<lock_type> lock(m_lock, std::try_to_lock);
  if( !lock.owns_lock() ) return false;

  m_queue.push_back( std::move(value) );
  lock.unlock();
  m_cv.notify_one();
  return true;
}

template<typename T, typename Lock, typename Allocator>
template<typename...Args, std::enable_if_t<std::is_constructible<T,Args...>::value>*>
bool bit::platform::concurrent_queue<T,Lock,Allocator>
  ::try_emplace_back( Args&&...args )
{
  std::unique_lock<lock_type> lock(m_lock, std::try_to_lock);
  if( !lock.owns_lock() ) return false;

  m_queue.emplace( std::forward<Args>(args)... );
  lock.unlock();
  m_cv.notify_one();
  return true;
}

//----------------------------------------------------------------------------

template<typename T, typename Lock, typename Allocator>
void bit::platform::concurrent_queue<T,Lock,Allocator>::clear()
{
  std::lock_guard<lock_type> lock(m_lock);

  m_queue = {};
}


template<typename T, typename Lock, typename Allocator>
void bit::platform::concurrent_queue<T,Lock,Allocator>
  ::swap( concurrent_queue& other )
   noexcept
{
  std::lock(m_lock,other.m_lock);
  std::lock_guard<lock_type> lock1(m_lock, std::adopt_lock);
  std::lock_guard<lock_type> lock2(other.m_lock, std::adopt_lock);

  m_queue.swap(other.m_queue);
}

//----------------------------------------------------------------------------
// Free Functions
//----------------------------------------------------------------------------

template<typename T, typename Lock, typename Allocator>
void bit::platform::swap( concurrent_queue<T,Lock,Allocator>& lhs,
                          concurrent_queue<T,Lock,Allocator>& rhs )
{
  lhs.swap(rhs);
}

#endif /* BIT_PLATFORM_THREADING_DETAIL_CONCURRENT_QUEUE_INL */
