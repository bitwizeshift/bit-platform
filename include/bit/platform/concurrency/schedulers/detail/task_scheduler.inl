#ifndef BIT_PLATFORM_CONCURRENCY_TASK_SCHEDULERS_DETAIL_TASK_SCHEDULER_INL
#define BIT_PLATFORM_CONCURRENCY_TASK_SCHEDULERS_DETAIL_TASK_SCHEDULER_INL

//=============================================================================
// Inline Definitions : bound_object
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

template<typename T>
inline bit::platform::task_scheduler::bound_object<T>::bound_object()
  noexcept
  : m_object(nullptr),
    m_state(nullptr)
{

}

template<typename T>
template<typename U, typename>
inline bit::platform::task_scheduler::bound_object<T>
  ::bound_object( bound_object<U>&& other )
  noexcept
  : m_object( other.m_object ),
    m_state( std::move(other.m_state) )
{
  other.unbind();
}

template<typename T>
template<typename U, typename>
inline bit::platform::task_scheduler::bound_object<T>
  ::bound_object( const bound_object<U>& other )
  noexcept
  : m_object( other.m_object ),
    m_state( other.m_state )
{

}

//-----------------------------------------------------------------------------

template<typename T>
inline bit::platform::task_scheduler::bound_object<T>&
  bit::platform::task_scheduler::bound_object<T>::operator=( bound_object<T> other )
  noexcept
{
  swap(other);
}

//-----------------------------------------------------------------------------
// Modifiers
//-----------------------------------------------------------------------------

template<typename T>
inline void bit::platform::task_scheduler::bound_object<T>
  ::swap( bound_object& other )
  noexcept
{
  using std::swap;

  swap(m_object,other.m_object);
  swap(m_state,other.m_state);
}

//-----------------------------------------------------------------------------

template<typename T>
template<typename Fn, typename>
inline void bit::platform::task_scheduler::bound_object<T>::visit_async( Fn&& fn )
{
  if( m_object == nullptr ) return;

  auto scheduler = m_state->bound_scheduler.load();
  if( scheduler == nullptr ) return;

  // Post the function back to the bound scheduler
  scheduler->post( [object = m_object,
                    state = m_state,
                    fn = std::forward<Fn>(fn)]() mutable
  {
    std::forward<Fn>(fn)( *object );
  });
}

template<typename T>
template<typename Fn, typename>
inline bit::stl::invoke_result_t<Fn,T&>
  bit::platform::task_scheduler::bound_object<T>::visit( Fn&& fn )
{
  if( m_object == nullptr ) return;

  auto scheduler = m_state->bound_scheduler.load();
  if( scheduler == nullptr ) return;

  // Post the function back to the bound scheduler
  scheduler->post_and_wait( [&]() mutable
  {
    std::forward<Fn>(fn)( *m_object );
  });
}

//-----------------------------------------------------------------------------

template<typename T>
inline void bit::platform::task_scheduler::bound_object<T>::unbind()
{
  if( m_state != nullptr ) {
    auto old = m_state->bound_scheduler.exchange(nullptr);

    assert( old != nullptr && "bound_object is not bound" );
    (void) old;
  }

  swap( bound_object{} );
}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

template<typename T>
inline bool bit::platform::task_scheduler::bound_object<T>::is_bound()
  const noexcept
{
  return m_object != nullptr;
}

template<typename T>
inline bit::platform::task_scheduler::bound_object<T>::operator bool()
  const noexcept
{
  return is_bound();
}

//-----------------------------------------------------------------------------
// Private Constructors
//-----------------------------------------------------------------------------

template<typename T>
inline bit::platform::task_scheduler::bound_object<T>
  ::bound_object( task_scheduler& scheduler, T& object )
  : m_object(std::addressof(object)),
    m_state( std::make_shared<bound_object_state>() )
{
  m_state->bound_scheduler.store( std::addressof(scheduler) );
}

template<typename T>
template<typename Allocator>
inline bit::platform::task_scheduler::bound_object<T>
  ::bound_object( const Allocator& allocator,
                  task_scheduler& scheduler,
                  T& object )
  : m_object( std::addressof(object) ),
    m_state( std::allocate_shared<bound_object_state>( allocator ) )
{
  m_state->bound_scheduler.store( std::addressof(scheduler) );
}

//=============================================================================
// Inline Definitions : task_scheduler
//=============================================================================

//-----------------------------------------------------------------------------
// Destructors
//-----------------------------------------------------------------------------

inline bit::platform::task_scheduler::~task_scheduler() = default;

//-----------------------------------------------------------------------------
// Posting / Waiting
//-----------------------------------------------------------------------------

inline void bit::platform::task_scheduler::post_task( task task )
{
  auto old = g_active_scheduler;
  g_active_scheduler = this;
  do_post_task( std::move(task) );
  g_active_scheduler = old;
}

//-----------------------------------------------------------------------------

template<typename Fn, typename...Args, typename>
inline void bit::platform::task_scheduler::post( Fn&& fn, Args&&...args )
{
  auto task = make_task( std::forward<Fn>(fn), std::forward<Args>(args)... );

  post_task( std::move(task) );
}

template<typename Fn, typename...Args, typename>
inline void bit::platform::task_scheduler::post( const task& parent,
                                                 Fn&& fn,
                                                 Args&&...args )
{
  auto task = make_task( parent,
                         std::forward<Fn>(fn),
                         std::forward<Args>(args)... );

  post_task( std::move(task) );
}


//-----------------------------------------------------------------------------

template<typename Fn, typename...Args, typename>
inline bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::task_scheduler::post_and_wait( Fn&& fn, Args&&...args )
{
  using type = bit::stl::invoke_result_t<Fn,Args...>;

  return post_and_wait_impl( type_tag<type>{},
                             std::forward<Fn>(fn),
                             std::forward<Args>(args)... );
}

template<typename Fn, typename...Args, typename>
inline bit::stl::invoke_result_t<Fn,Args...>
  bit::platform::task_scheduler::post_and_wait( const task& parent,
                                                Fn&& fn,
                                                Args&&...args )
{
  using type = bit::stl::invoke_result_t<Fn,Args...>;

  return post_and_wait_impl( type_tag<type>{},
                             parent,
                             std::forward<Fn>(fn),
                             std::forward<Args>(args)... );
}

//-----------------------------------------------------------------------------
// Binding
//-----------------------------------------------------------------------------

template<typename T>
inline bit::platform::task_scheduler::bound_object<T>
  bit::platform::task_scheduler::make_bound_object( T& object )
{
  return bound_object<T>{ object };
}

template<typename Allocator, typename T>
inline bit::platform::task_scheduler::bound_object<T>
  bit::platform::task_scheduler::allocate_bound_object( const Allocator& allocator,
                                                        T& object )
{
  return bound_object<T>{ allocator, object };
}

//-----------------------------------------------------------------------------
// Private Posting
//-----------------------------------------------------------------------------

template<typename Fn, typename...Args>
inline void bit::platform::task_scheduler::post_and_wait_impl( type_tag<void>,
                                                               Fn&& fn,
                                                               Args&&...args )
{
  auto task = make_task( [&]()
  {
    std::forward<Fn>(fn)( std::forward<Args>(args)... );
  });
  auto handle = task_handle{ task };

  post_task( std::move(task) );
  wait( handle );
}

template<typename Fn, typename...Args>
inline void bit::platform::task_scheduler::post_and_wait_impl( type_tag<void>,
                                                               const task& parent,
                                                               Fn&& fn,
                                                               Args&&...args )
{
  auto task = make_task( parent, [&]()
  {
    std::forward<Fn>(fn)( std::forward<Args>(args)... );
  });
  auto handle = task_handle{ task };

  post_task( std::move(task) );
  wait( handle );
}

//-----------------------------------------------------------------------------

template<typename T, typename Fn, typename...Args>
inline T bit::platform::task_scheduler::post_and_wait_impl( type_tag<T>,
                                                            Fn&& fn,
                                                            Args&&...args )
{
  using storage_type = std::aligned_storage_t<sizeof(T),alignof(T)>;
  auto storage = storage_type{};

  auto task = make_task( [&]()
  {
    auto p = static_cast<void*>(&storage);
    new (p) T( std::forward<Fn>(fn)( std::forward<Args>(args)... ) );
  });
  auto handle = task_handle{ task };

  post_task( std::move(task) );
  wait( handle );

  return std::move( *reinterpret_cast<T*>(&storage) );
}

template<typename T, typename Fn, typename...Args>
inline T bit::platform::task_scheduler::post_and_wait_impl( type_tag<T>,
                                                            const task& parent,
                                                            Fn&& fn,
                                                            Args&&...args )
{
  using storage_type = std::aligned_storage_t<sizeof(T),alignof(T)>;
  auto storage = storage_type{};

  auto task = make_task( parent, [&]()
  {
    auto p = static_cast<void*>(&storage);
    new (p) T( std::forward<Fn>(fn)( std::forward<Args>(args)... ) );
  });
  auto handle = task_handle{ task };

  post_task( std::move(task) );
  wait( handle );

  return std::move( *reinterpret_cast<T*>(&storage) );
}

//-----------------------------------------------------------------------------

template<typename T, typename Fn, typename...Args>
inline T& bit::platform::task_scheduler::post_and_wait_impl( type_tag<T&>,
                                                             Fn&& fn,
                                                             Args&&...args )
{
  auto p = static_cast<T*>(nullptr);

  auto task = make_task( [&]()
  {
    // Take the address of the referenced value
    p = std::addressof( std::forward<Fn>(fn)( std::forward<Args>(args)... ) );
  });
  auto handle = task_handle{ task };

  post_task( std::move(task) );
  wait( handle );

  return *p;
}

template<typename T, typename Fn, typename...Args>
inline T& bit::platform::task_scheduler::post_and_wait_impl( type_tag<T&>,
                                                             const task& parent,
                                                             Fn&& fn,
                                                             Args&&...args )
{
  auto p = static_cast<T*>(nullptr);

  auto task = make_task( parent, [&]()
  {
    // Take the address of the referenced value
    p = std::addressof( std::forward<Fn>(fn)( std::forward<Args>(args)... ) );
  });
  auto handle = task_handle{ task };

  post_task( std::move(task) );
  wait( handle );

  return *p;
}

//=============================================================================
// Inline Definitions : Free Functions
//=============================================================================

//-----------------------------------------------------------------------------
// Equality
//-----------------------------------------------------------------------------

template<typename T>
inline bool bit::platform::operator==( const task_scheduler::bound_object<T>& lhs,
                                       const task_scheduler::bound_object<T>& rhs )
  noexcept
{
  return lhs.m_state == rhs.m_state;
}

template<typename T>
inline bool bit::platform::operator!=( const task_scheduler::bound_object<T>& lhs,
                                       const task_scheduler::bound_object<T>& rhs )
  noexcept
{
  return !(lhs==rhs);
}

//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------

template<typename T>
inline void bit::platform::swap( task_scheduler::bound_object<T>& lhs,
                                 task_scheduler::bound_object<T>& rhs )
  noexcept
{
  lhs.swap(rhs);
}


#endif /* BIT_PLATFORM_CONCURRENCY_TASK_SCHEDULERS_DETAIL_TASK_SCHEDULER_INL */
