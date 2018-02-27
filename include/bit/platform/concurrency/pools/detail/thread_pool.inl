#ifndef BIT_PLATFORM_CONCURRENCY_POOLS_DETAIL_THREAD_POOL_INL
#define BIT_PLATFORM_CONCURRENCY_POOLS_DETAIL_THREAD_POOL_INL

//============================================================================
// basic_thread_pool
//============================================================================

//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

template<typename Allocator>
bit::platform::basic_thread_pool<Allocator>::basic_thread_pool()
  : basic_thread_pool( std::thread::hardware_concurrency(), Allocator() )
{

}

template<typename Allocator>
bit::platform::basic_thread_pool<Allocator>
  ::basic_thread_pool( std::size_t capacity )
  : basic_thread_pool( capacity, Allocator() )
{

}

template<typename Allocator>
bit::platform::basic_thread_pool<Allocator>
  ::basic_thread_pool( const Allocator& allocator )
  : basic_thread_pool( std::thread::hardware_concurrency(), allocator )
{

}

template<typename Allocator>
bit::platform::basic_thread_pool<Allocator>
  ::basic_thread_pool( std::size_t capacity, const Allocator& allocator )
  : m_threads( allocator ),
    m_queue( allocator ),
    m_is_running(true)
{
  m_threads.resize( capacity );

  for( auto i = 0; i < m_threads.size(); ++i ) {
    m_threads[i] = std::thread([this]()
    {
      while( m_is_running ) {
        std::packaged_task<void()> task;
        m_queue.pop( &task );
        task();
      }
    });
  }
}

template<typename Allocator>
bit::platform::basic_thread_pool<Allocator>::~basic_thread_pool()
{
  m_is_running = false;
  for( auto& thread : m_threads ) {
    thread.join();
  }
}

template<typename Allocator>
template<typename Fn, typename...Args>
void bit::platform::basic_thread_pool<Allocator>::post( Fn&& fn, Args&&...args )
{
  // tuple is to account for performing a decay copy to simulate
  // behaviour of std::thread
  m_queue.emplace_back( std::allocator_arg,
                        m_queue.get_allocator(),
                        [fn,tuple=std::make_tuple(args...)]()
  {
    stl::apply( fn, tuple );
  });
}

template<typename Allocator>
template<typename Fn, typename...Args>
bit::stl::invoke_result_t<std::decay_t<Fn>>
  bit::platform::basic_thread_pool<Allocator>::post_and_wait( Fn&& fn,
                                                              Args&&...args )
{
  using result_type = bit::stl::invoke_result_t<std::decay_t<Fn>>;

  auto decay_copy = []( auto&& x ){ return x; };

  union output
  {
    result_type    type;
    struct empty{} empty;

    output() : empty{}{}
  };

  output out;

  waitable_event event;
  m_queue.emplace_back( std::allocator_arg,
                        m_queue.get_allocator(),
                        [&]()
  {
    out.type = decay_copy(fn)( decay_copy(args)... );
    event.signal();
  });
  event.wait();

  return out.type;
}

//============================================================================
// unlimited_thread_pool
//============================================================================

//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

template<typename Fn, typename...Args>
void bit::platform::unlimited_thread_pool::post( Fn&& fn, Args&&...args )
{
  std::thread t( std::forward<Fn>(fn), std::forward<Args>(args)... );
  t.detach();
}

template<typename Fn, typename...Args>
bit::stl::invoke_result_t<std::decay_t<Fn>>
  bit::platform::unlimited_thread_pool::post_and_wait( Fn&& fn,
                                                       Args&&...args )
{
  using result_type = bit::stl::invoke_result_t<std::decay_t<Fn>>;

  auto decay_copy = []( auto&& x ){ return x; };

  union output
  {
    result_type    type;
    struct empty{} empty;

    output() : empty{}{}
  };

  output out;

  auto thread = std::thread( [&]()
  {
    out.type = decay_copy(fn)( decay_copy(args)... );
  });
  thread.join();


  return out.type;
}

//============================================================================
// sequential_thread_pool
//============================================================================

//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

template<typename Fn, typename...Args>
void bit::platform::sequential_thread_pool::post( Fn&& fn, Args&&...args )
{
  // NOTE:
  // technically a performance degradation; but this is done to preserve
  // semantic behaviour with other thread pools
  auto decay_copy = []( auto&& x ){ return x; };

  decay_copy(fn)( decay_copy(args)... );
}

template<typename Fn, typename...Args>
bit::stl::invoke_result_t<std::decay_t<Fn>>
  bit::platform::sequential_thread_pool::post_and_wait( Fn&& fn,
                                                        Args&&...args )
{
  auto decay_copy = []( auto&& x ){ return x; };

  return decay_copy(fn)( decay_copy(args)... );
}
#endif /* BIT_PLATFORM_CONCURRENCY_POOLS_DETAIL_THREAD_POOL_INL */
