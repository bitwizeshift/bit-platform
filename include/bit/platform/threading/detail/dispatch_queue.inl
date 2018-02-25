#ifndef BIT_THREAD_DETAIL_DISPATCH_QUEUE_INL
#define BIT_THREAD_DETAIL_DISPATCH_QUEUE_INL

//=============================================================================
// detail::dispatch_queue_post_task_and_wait_impl
//=============================================================================

namespace bit { namespace platform { namespace detail {

  template<typename T>
  struct dispatch_queue_post_task_and_wait_impl
  {
    template<typename Fn, typename...Args>
    static T invoke( dispatch_queue& dispatch_queue, Fn&& fn, Args&&...args )
    {
      using storage_type = std::aligned_storage_t<sizeof(T),alignof(T)>;
      auto storage = storage_type{};

      auto task = make_task( [&]()
      {
        new (&storage) T( std::forward<Fn>(fn)( std::forward<Args>(args)... ) );
      });

      auto handle = task_handle(task);
      dispatch_queue.post_task( std::move(task) );
      dispatch_queue.wait( handle );

      return std::move( *reinterpret_cast<T*>(&storage) );
    }

    template<typename Fn, typename...Args>
    static T invoke( dispatch_queue& dispatch_queue,
                     const task& parent, Fn&& fn, Args&&...args )
    {
      using storage_type = std::aligned_storage_t<sizeof(T),alignof(T)>;
      auto storage = storage_type{};

      auto task = make_task( parent, [&]()
      {
        new (&storage) T( std::forward<Fn>(fn)( std::forward<Args>(args)... ) );
      });

      auto handle = task_handle(task);
      dispatch_queue.post_task( std::move(task) );
      dispatch_queue.wait( handle );

      return std::move( *static_cast<T*>(&storage) );
    }
  };

  //---------------------------------------------------------------------------

  template<typename T>
  struct dispatch_queue_post_task_and_wait_impl<T&>
  {
    template<typename Fn, typename...Args>
    static T& invoke( dispatch_queue& dispatch_queue, Fn&& fn, Args&&...args )
    {
      T* ptr;

      auto task = make_task( [&]()
      {
        ptr = &std::forward<Fn>(fn)( std::forward<Args>(args)... );
      });

      auto handle = task_handle(task);
      dispatch_queue.post_task( std::move(task) );
      dispatch_queue.wait( handle );

      return *ptr;
    }

    template<typename Fn, typename...Args>
    static T& invoke( dispatch_queue& dispatch_queue,
                      const task& parent, Fn&& fn, Args&&...args )
    {
      T* ptr;

      auto task = make_task( parent, [&]()
      {
        ptr = &std::forward<Fn>(fn)( std::forward<Args>(args)... );
      });

      auto handle = task_handle(task);
      dispatch_queue.post_task( std::move(task) );
      dispatch_queue.wait( handle );

      return *ptr;
    }
  };

  //---------------------------------------------------------------------------

  template<>
  struct dispatch_queue_post_task_and_wait_impl<void>
  {
    template<typename Fn, typename...Args>
    static void invoke( dispatch_queue& dispatch_queue, Fn&& fn, Args&&...args )
    {
      auto task = make_task( [&]()
      {
        std::forward<Fn>(fn)( std::forward<Args>(args)... );
      });

      auto handle = task_handle(task);
      dispatch_queue.post_task( task );
      dispatch_queue.wait( handle );
    }

    template<typename Fn, typename...Args>
    static void invoke( dispatch_queue& dispatch_queue,
                        const task& parent, Fn&& fn, Args&&...args )
    {
      auto task = make_task( parent, [&]()
      {
        std::forward<Fn>(fn)( std::forward<Args>(args)... );
      });

      auto handle = task_handle(task);
      dispatch_queue.post_task( std::move(task) );
      dispatch_queue.wait( handle );
    }
  };

} } } // namespace bit::platform::detail

//=============================================================================
// dispatch_queue
//=============================================================================

//-----------------------------------------------------------------------------
// Modifiers
//-----------------------------------------------------------------------------

template<typename Fn, typename...Args>
void bit::platform::dispatch_queue::post( Fn&& fn, Args&&...args )
{
  auto task = make_task( std::forward<Fn>(fn), std::forward<Args>(args)... );

  post_task( std::move(task) );
  m_cv.notify_all();
}

template<typename Fn, typename...Args>
void bit::platform::dispatch_queue::post( const task& parent, Fn&& fn, Args&&...args )
{
  auto task = make_task( parent, std::forward<Fn>(fn), std::forward<Args>(args)... );

  post_task( std::move(task) );
  m_cv.notify_all();
}

//-----------------------------------------------------------------------------

template<typename Fn, typename...Args>
bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::dispatch_queue::post_and_wait( Fn&& fn, Args&&...args )
{
  using result_type = stl::invoke_result_t<Fn,Args...>;

  return detail::dispatch_queue_post_task_and_wait_impl<result_type>
    ::invoke( *this, std::forward<Fn>(fn), std::forward<Args>(args)... );
}

template<typename Fn, typename...Args>
bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::dispatch_queue::post_and_wait( const task& parent,
                                              Fn&& fn, Args&&...args )
{
  using result_type = stl::invoke_result_t<Fn,Args...>;

  return detail::dispatch_queue_post_task_and_wait_impl<result_type>
    ::invoke( *this, parent, std::forward<Fn>(fn), std::forward<Args>(args)... );
}

//-----------------------------------------------------------------------------
// Free Functions
//-----------------------------------------------------------------------------

template<typename Fn, typename...Args, typename>
inline void bit::platform::post( dispatch_queue& queue,
                               Fn&& fn, Args&&...args )
{
  queue.post( std::forward<Fn>(fn), std::forward<Args>(args)... );
}


template<typename Fn, typename...Args, typename>
inline void bit::platform::post( dispatch_queue& queue,
                               const task& parent, Fn&& fn, Args&&...args )
{
  queue.post( parent, std::forward<Fn>(fn), std::forward<Args>(args)... );
}

//-----------------------------------------------------------------------------

template<typename Fn, typename...Args, typename>
bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::post_and_wait( dispatch_queue& queue, Fn&& fn, Args&&...args )
{
  return queue.post_and_wait( std::forward<Fn>(fn),
                              std::forward<Args>(args)... );
}

template<typename Fn, typename...Args, typename>
bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::post_and_wait( dispatch_queue& queue,
                                const task& parent, Fn&& fn, Args&&...args )
{
  return queue.post_and_wait( parent,
                              std::forward<Fn>(fn),
                              std::forward<Args>(args)... );
}

//-----------------------------------------------------------------------------
// This Dispatcher Free Functions
//-----------------------------------------------------------------------------

template<typename Fn, typename...Args, typename>
inline void bit::platform::this_dispatch_queue::post( Fn&& fn, Args&&...args )
{
  auto task = make_task( std::forward<Fn>(fn), std::forward<Args>(args)... );

  post_task( std::move(task) );
}


template<typename Fn, typename...Args, typename>
inline void bit::platform::this_dispatch_queue::post( const task& parent,
                                                      Fn&& fn, Args&&...args )
{
  auto task = make_task( parent, std::forward<Fn>(fn), std::forward<Args>(args)... );

  post_task( std::move(task) );
}

#endif /* BIT_THREAD_DETAIL_DISPATCH_QUEUE_INL */
