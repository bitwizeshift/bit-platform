#ifndef BIT_THREAD_THREADING_DETAIL_CONCURRENT_TASK_SCHEDULER_INL
#define BIT_THREAD_THREADING_DETAIL_CONCURRENT_TASK_SCHEDULER_INL

//=============================================================================
// detail::concurrent_task_scheduler_post_task_and_wait_impl
//=============================================================================

namespace bit { namespace platform { namespace detail {

  template<typename T>
  struct concurrent_task_scheduler_post_task_and_wait_impl
  {
    template<typename Fn, typename...Args>
    static T invoke( concurrent_task_scheduler& concurrent_task_scheduler, Fn&& fn, Args&&...args )
    {
      using storage_type = std::aligned_storage_t<sizeof(T),alignof(T)>;
      auto storage = storage_type{};

      auto task = make_task( [&]()
      {
        new (&storage) T( std::forward<Fn>(fn)( std::forward<Args>(args)... ) );
      });

      concurrent_task_scheduler.post_task( task );
      concurrent_task_scheduler.wait( task );

      return std::move( *reinterpret_cast<T*>(&storage) );
    }

    template<typename Fn, typename...Args>
    static T invoke( concurrent_task_scheduler& concurrent_task_scheduler,
                     const task& parent, Fn&& fn, Args&&...args )
    {
      using storage_type = std::aligned_storage_t<sizeof(T),alignof(T)>;
      auto storage = storage_type{};

      auto task = make_task( parent, [&]()
      {
        new (&storage) T( std::forward<Fn>(fn)( std::forward<Args>(args)... ) );
      });

      concurrent_task_scheduler.post_task( task );
      concurrent_task_scheduler.wait( task );

      return std::move( *static_cast<T*>(&storage) );
    }
  };

  //---------------------------------------------------------------------------

  template<typename T>
  struct concurrent_task_scheduler_post_task_and_wait_impl<T&>
  {
    template<typename Fn, typename...Args>
    static T& invoke( concurrent_task_scheduler& concurrent_task_scheduler, Fn&& fn, Args&&...args )
    {
      T* ptr;

      auto task = make_task( [&]()
      {
        ptr = &std::forward<Fn>(fn)( std::forward<Args>(args)... );
      });

      concurrent_task_scheduler.post_task( task );
      concurrent_task_scheduler.wait( task );

      return *ptr;
    }

    template<typename Fn, typename...Args>
    static T& invoke( concurrent_task_scheduler& concurrent_task_scheduler,
                      const task& parent, Fn&& fn, Args&&...args )
    {
      T* ptr;

      auto task = make_task( parent, [&]()
      {
        ptr = &std::forward<Fn>(fn)( std::forward<Args>(args)... );
      });

      concurrent_task_scheduler.post_task( task );
      concurrent_task_scheduler.wait( task );

      return *ptr;
    }
  };

  //---------------------------------------------------------------------------

  template<>
  struct concurrent_task_scheduler_post_task_and_wait_impl<void>
  {
    template<typename Fn, typename...Args>
    static void invoke( concurrent_task_scheduler& concurrent_task_scheduler, Fn&& fn, Args&&...args )
    {
      auto task = make_task( [&]()
      {
        std::forward<Fn>(fn)( std::forward<Args>(args)... );
      });

      concurrent_task_scheduler.post_task( task );
      concurrent_task_scheduler.wait( task );
    }

    template<typename Fn, typename...Args>
    static void invoke( concurrent_task_scheduler& concurrent_task_scheduler,
                        const task& parent, Fn&& fn, Args&&...args )
    {
      auto task = make_task( parent, [&]()
      {
        std::forward<Fn>(fn)( std::forward<Args>(args)... );
      });

      concurrent_task_scheduler.post_task( task );
      concurrent_task_scheduler.wait( task );
    }
  };

} } } // namespace bit::platform::detail

//=============================================================================
// concurrent_task_scheduler
//=============================================================================

//-----------------------------------------------------------------------------
// Modifiers
//-----------------------------------------------------------------------------

template<typename Fn>
void bit::platform::concurrent_task_scheduler::run( Fn&& fn )
{
  start();
  while( true ) {

    std::forward<Fn>(fn)();

    if( !m_running ) break;

    auto j = get_task();

    if( j ) {
      help_while_unavailable( j );

      j.execute();
    }
  }
}

//-----------------------------------------------------------------------------

template<typename Fn, typename...Args>
void bit::platform::concurrent_task_scheduler::post( Fn&& fn, Args&&...args )
{
  auto task = make_task( std::forward<Fn>(fn), std::forward<Args>(args)... );

  push_task( std::move(task) );
}

template<typename Fn, typename...Args>
void bit::platform::concurrent_task_scheduler::post( const task& parent,
                                                     Fn&& fn,
                                                     Args&&...args )
{
  auto task = make_task( parent, std::forward<Fn>(fn), std::forward<Args>(args)... );

  push_task( std::move(task) );
}

//-----------------------------------------------------------------------------

template<typename Fn, typename...Args>
bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::concurrent_task_scheduler::post_and_wait( Fn&& fn, Args&&...args )
{
  using result_type = stl::invoke_result_t<Fn,Args...>;

  return detail::concurrent_task_scheduler_post_task_and_wait_impl<result_type>
    ::invoke( *this, std::forward<Fn>(fn), std::forward<Args>(args)... );
}

template<typename Fn, typename...Args>
bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::concurrent_task_scheduler::post_and_wait( const task& parent,
                                                           Fn&& fn,
                                                           Args&&...args )
{
  using result_type = stl::invoke_result_t<Fn,Args...>;

  return detail::concurrent_task_scheduler_post_task_and_wait_impl<result_type>
    ::invoke( *this, parent, std::forward<Fn>(fn), std::forward<Args>(args)... );
}

//-----------------------------------------------------------------------------
// Free Functions
//-----------------------------------------------------------------------------

template<typename Fn, typename...Args, typename>
inline void bit::platform::post( concurrent_task_scheduler& scheduler,
                                 Fn&& fn,
                                 Args&&...args )
{
  scheduler.post( std::forward<Fn>(fn), std::forward<Args>(args)... );
}


template<typename Fn, typename...Args, typename>
inline void bit::platform::post( concurrent_task_scheduler& scheduler,
                                 const task& parent,
                                 Fn&& fn,
                                 Args&&...args )
{
  scheduler.post( parent, std::forward<Fn>(fn), std::forward<Args>(args)... );
}

//-----------------------------------------------------------------------------

template<typename Fn, typename...Args, typename>
bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::post_and_wait( concurrent_task_scheduler& scheduler,
                                Fn&& fn,
                                Args&&...args )
{
  return scheduler.post_and_wait( std::forward<Fn>(fn),
                                  std::forward<Args>(args)... );
}

template<typename Fn, typename...Args, typename>
bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::post_and_wait( concurrent_task_scheduler& scheduler,
                                const task& parent,
                                Fn&& fn,
                                Args&&...args )
{
  return scheduler.post_and_wait( parent,
                                  std::forward<Fn>(fn),
                                  std::forward<Args>(args)... );
}

//-----------------------------------------------------------------------------
// This Dispatcher Free Functions
//-----------------------------------------------------------------------------

template<typename Fn, typename...Args, typename>
inline void bit::platform::this_concurrent_task_scheduler::post( Fn&& fn,
                                                                 Args&&...args )
{
  auto task = make_task( std::forward<Fn>(fn), std::forward<Args>(args)... );

  post_task( task );
}


template<typename Fn, typename...Args, typename>
inline void bit::platform::this_concurrent_task_scheduler
  ::post( const task& parent, Fn&& fn, Args&&...args )
{
  auto task = make_task( parent, std::forward<Fn>(fn), std::forward<Args>(args)... );

  post_task( task );
}

#endif /* BIT_THREAD_THREADING_DETAIL_CONCURRENT_TASK_SCHEDULER_INL */
