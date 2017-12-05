#ifndef BIT_THREAD_THREADING_DETAIL_DISPATCHER_INL
#define BIT_THREAD_THREADING_DETAIL_DISPATCHER_INL

//=============================================================================
// detail::dispatcher_post_job_and_wait_impl
//=============================================================================

namespace bit { namespace platform { namespace detail {

  template<typename T>
  struct dispatcher_post_job_and_wait_impl
  {
    template<typename Fn, typename...Args>
    static T invoke( dispatcher& dispatcher, Fn&& fn, Args&&...args )
    {
      using storage_type = std::aligned_storage_t<sizeof(T),alignof(T)>;
      auto storage = storage_type{};

      auto job = make_job( [&]()
      {
        new (&storage) T( std::forward<Fn>(fn)( std::forward<Args>(args)... ) );
      });

      dispatcher.post_job( job );
      dispatcher.wait( job );

      return std::move( *reinterpret_cast<T*>(&storage) );
    }

    template<typename Fn, typename...Args>
    static T invoke( dispatcher& dispatcher,
                     const job& parent, Fn&& fn, Args&&...args )
    {
      using storage_type = std::aligned_storage_t<sizeof(T),alignof(T)>;
      auto storage = storage_type{};

      auto job = make_job( parent, [&]()
      {
        new (&storage) T( std::forward<Fn>(fn)( std::forward<Args>(args)... ) );
      });

      dispatcher.post_job( job );
      dispatcher.wait( job );

      return std::move( *static_cast<T*>(&storage) );
    }
  };

  //---------------------------------------------------------------------------

  template<typename T>
  struct dispatcher_post_job_and_wait_impl<T&>
  {
    template<typename Fn, typename...Args>
    static T& invoke( dispatcher& dispatcher, Fn&& fn, Args&&...args )
    {
      T* ptr;

      auto job = make_job( [&]()
      {
        ptr = &std::forward<Fn>(fn)( std::forward<Args>(args)... );
      });

      dispatcher.post_job( job );
      dispatcher.wait( job );

      return *ptr;
    }

    template<typename Fn, typename...Args>
    static T& invoke( dispatcher& dispatcher,
                      const job& parent, Fn&& fn, Args&&...args )
    {
      T* ptr;

      auto job = make_job( parent, [&]()
      {
        ptr = &std::forward<Fn>(fn)( std::forward<Args>(args)... );
      });

      dispatcher.post_job( job );
      dispatcher.wait( job );

      return *ptr;
    }
  };

  //---------------------------------------------------------------------------

  template<>
  struct dispatcher_post_job_and_wait_impl<void>
  {
    template<typename Fn, typename...Args>
    static void invoke( dispatcher& dispatcher, Fn&& fn, Args&&...args )
    {
      auto job = make_job( [&]()
      {
        std::forward<Fn>(fn)( std::forward<Args>(args)... );
      });

      dispatcher.post_job( job );
      dispatcher.wait( job );
    }

    template<typename Fn, typename...Args>
    static void invoke( dispatcher& dispatcher,
                        const job& parent, Fn&& fn, Args&&...args )
    {
      auto job = make_job( parent, [&]()
      {
        std::forward<Fn>(fn)( std::forward<Args>(args)... );
      });

      dispatcher.post_job( job );
      dispatcher.wait( job );
    }
  };

} } } // namespace bit::platform::detail

//=============================================================================
// detail::dispatcher_post_job_and_wait_this_impl
//=============================================================================

namespace bit { namespace platform { namespace detail {

  template<typename T>
  struct dispatcher_post_job_and_wait_this_impl
  {
    template<typename Fn, typename...Args>
    static T invoke( Fn&& fn, Args&&...args )
    {
      using storage_type = std::aligned_storage_t<sizeof(T),alignof(T)>;
      auto storage = storage_type{};

      auto job = make_job( [&]()
      {
        new (&storage) T( std::forward<Fn>(fn)( std::forward<Args>(args)... ) );
      });

      auto handle = job_handle(job);
      this_dispatcher::post_job( job );
      this_dispatcher::wait( handle );

      return std::move( *reinterpret_cast<T*>(&storage) );
    }

    template<typename Fn, typename...Args>
    static T invoke( const job& parent, Fn&& fn, Args&&...args )
    {
      using storage_type = std::aligned_storage_t<sizeof(T),alignof(T)>;
      auto storage = storage_type{};

      auto job = make_job( parent, [&]()
      {
        new (&storage) T( std::forward<Fn>(fn)( std::forward<Args>(args)... ) );
      });

      auto handle = job_handle(job);
      this_dispatcher::post_job( job );
      this_dispatcher::wait( handle );

      return std::move( *static_cast<T*>(&storage) );
    }
  };

  //---------------------------------------------------------------------------

  template<typename T>
  struct dispatcher_post_job_and_wait_this_impl<T&>
  {
    template<typename Fn, typename...Args>
    static T& invoke( Fn&& fn, Args&&...args )
    {
      T* ptr;

      auto job = make_job( [&]()
      {
        ptr = &std::forward<Fn>(fn)( std::forward<Args>(args)... );
      });

      auto handle = job_handle(job);
      this_dispatcher::post_job( job );
      this_dispatcher::wait( handle );

      return *ptr;
    }

    template<typename Fn, typename...Args>
    static T& invoke( const job& parent, Fn&& fn, Args&&...args )
    {
      T* ptr;

      auto job = make_job( parent, [&]()
      {
        ptr = &std::forward<Fn>(fn)( std::forward<Args>(args)... );
      });

      auto handle = job_handle(job);
      this_dispatcher::post_job( job );
      this_dispatcher::wait( handle );

      return *ptr;
    }
  };

  //---------------------------------------------------------------------------

  template<>
  struct dispatcher_post_job_and_wait_this_impl<void>
  {
    template<typename Fn, typename...Args>
    static void invoke( Fn&& fn, Args&&...args )
    {
      auto job = make_job( [&]()
      {
        std::forward<Fn>(fn)( std::forward<Args>(args)... );
      });

      auto handle = job_handle(job);
      this_dispatcher::post_job( std::move(job) );
      this_dispatcher::wait( handle );
    }

    template<typename Fn, typename...Args>
    static void invoke( const job& parent, Fn&& fn, Args&&...args )
    {
      auto job = make_job( parent, [&]()
      {
        std::forward<Fn>(fn)( std::forward<Args>(args)... );
      });

      auto handle = job_handle(job);
      this_dispatcher::post_job( std::move(job) );
      this_dispatcher::wait( handle );
    }
  };

} } } // namespace bit::platform::detail

//=============================================================================
// dispatcher
//=============================================================================

//-----------------------------------------------------------------------------
// Modifiers
//-----------------------------------------------------------------------------

template<typename Fn>
void bit::platform::dispatcher::run( Fn&& fn )
{
  start();
  while( true ) {

    std::forward<Fn>(fn)();

    if( !m_running ) break;

    auto j = get_job();

    if( j ) {
      help_while_unavailable( j );

      j.execute();
    }
  }
}

//-----------------------------------------------------------------------------

template<typename Fn, typename...Args>
void bit::platform::dispatcher::post( Fn&& fn, Args&&...args )
{
  auto job = make_job( std::forward<Fn>(fn), std::forward<Args>(args)... );

  push_job( std::move(job) );
}

template<typename Fn, typename...Args>
void bit::platform::dispatcher::post( const job& parent, Fn&& fn, Args&&...args )
{
  auto job = make_job( parent, std::forward<Fn>(fn), std::forward<Args>(args)... );

  push_job( std::move(job) );
}

//-----------------------------------------------------------------------------

template<typename Fn, typename...Args>
bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::dispatcher::post_and_wait( Fn&& fn, Args&&...args )
{
  using result_type = stl::invoke_result_t<Fn,Args...>;

  return detail::dispatcher_post_job_and_wait_impl<result_type>
    ::invoke( *this, std::forward<Fn>(fn), std::forward<Args>(args)... );
}

template<typename Fn, typename...Args>
bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::dispatcher::post_and_wait( const job& parent,
                                          Fn&& fn, Args&&...args )
{
  using result_type = stl::invoke_result_t<Fn,Args...>;

  return detail::dispatcher_post_job_and_wait_impl<result_type>
    ::invoke( *this, parent, std::forward<Fn>(fn), std::forward<Args>(args)... );
}

//-----------------------------------------------------------------------------
// Free Functions
//-----------------------------------------------------------------------------

template<typename Fn, typename...Args, typename>
inline void bit::platform::post( dispatcher& dispatcher,
                               Fn&& fn, Args&&...args )
{
  dispatcher.post( std::forward<Fn>(fn), std::forward<Args>(args)... );
}


template<typename Fn, typename...Args, typename>
inline void bit::platform::post( dispatcher& dispatcher,
                               const job& parent, Fn&& fn, Args&&...args )
{
  dispatcher.post( parent, std::forward<Fn>(fn), std::forward<Args>(args)... );
}

//-----------------------------------------------------------------------------

template<typename Fn, typename...Args, typename>
bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::post_and_wait( dispatcher& dispatcher, Fn&& fn, Args&&...args )
{
  return dispatcher.post_and_wait( std::forward<Fn>(fn),
                                   std::forward<Args>(args)... );
}

template<typename Fn, typename...Args, typename>
bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::post_and_wait( dispatcher& dispatcher,
                              const job& parent, Fn&& fn, Args&&...args )
{
  return dispatcher.post_and_wait( parent,
                                   std::forward<Fn>(fn),
                                   std::forward<Args>(args)... );
}

//-----------------------------------------------------------------------------
// This Dispatcher Free Functions
//-----------------------------------------------------------------------------

template<typename Fn, typename...Args, typename>
inline void bit::platform::this_dispatcher::post( Fn&& fn, Args&&...args )
{
  auto job = make_job( std::forward<Fn>(fn), std::forward<Args>(args)... );

  post_job( job );
}


template<typename Fn, typename...Args, typename>
inline void bit::platform::this_dispatcher::post( const job& parent,
                                                Fn&& fn, Args&&...args )
{
  auto job = make_job( parent, std::forward<Fn>(fn), std::forward<Args>(args)... );

  post_job( job );
}

//-----------------------------------------------------------------------------

template<typename Fn, typename...Args, typename>
bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::this_dispatcher::post_and_wait( Fn&& fn, Args&&...args )
{
  using result_type = stl::invoke_result_t<Fn,Args...>;

  return detail::dispatcher_post_job_and_wait_this_impl<result_type>
    ::invoke( std::forward<Fn>(fn), std::forward<Args>(args)... );
}

template<typename Fn, typename...Args, typename>
bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::this_dispatcher::post_and_wait( const job& parent,
                                               Fn&& fn, Args&&...args )
{
  using result_type = stl::invoke_result_t<Fn,Args...>;

  return detail::dispatcher_post_job_and_wait_this_impl<result_type>
    ::invoke( parent, std::forward<Fn>(fn), std::forward<Args>(args)... );
}

#endif /* BIT_THREAD_THREADING_DETAIL_DISPATCHER_INL */
