#ifndef BIT_PLATFORM_THREADING_DETAIL_JOB_DISPATCHER_INL
#define BIT_PLATFORM_THREADING_DETAIL_JOB_DISPATCHER_INL

//============================================================================
// job
//============================================================================

//----------------------------------------------------------------------------
// Private Constructors
//----------------------------------------------------------------------------

inline bit::platform::job::job()
  : m_parent(nullptr),
    m_function(nullptr),
    m_unfinished(0)
{

}

template<typename Fn, typename...Args>
bit::platform::job::job( Fn&& fn, Args&&...args )
: m_parent(nullptr),
  m_function(&function<std::decay_t<Fn>,std::decay_t<Args>...>),
  m_unfinished(1)
{
  store_arguments( std::forward<Fn>(fn), std::forward<Args>(args)... );
}

//----------------------------------------------------------------------------

template<typename Fn, typename...Args>
bit::platform::job::job( job* parent, Fn&& fn, Args&&...args )
  : m_parent(parent),
    m_function(&function<std::decay_t<Fn>,std::decay_t<Args>...>),
    m_unfinished(1)
{
  ++m_parent->m_unfinished;

  store_arguments( std::forward<Fn>(fn), std::forward<Args>(args)... );
}

//----------------------------------------------------------------------------
// Observers
//----------------------------------------------------------------------------

inline bool bit::platform::job::completed()
  const noexcept
{
  return m_unfinished == 0;
}

inline bool bit::platform::job::available()
  const noexcept
{
  return m_unfinished == 1;
}

//----------------------------------------------------------------------------
// Element Access
//----------------------------------------------------------------------------

inline bit::platform::job* bit::platform::job::parent()
  const noexcept
{
  return m_parent;
}

//----------------------------------------------------------------------------
// Private Member Functions
//----------------------------------------------------------------------------

template<typename...Args>
void bit::platform::job::store_arguments( Args&&...args )
{
  using tuple_type = std::tuple<std::decay_t<Args>...>;

  auto p    = static_cast<void*>(&m_padding[0]);
  auto size = padding_size;

  if( std::align(alignof(tuple_type),sizeof(tuple_type),p,size) ) {
    // store it inline if it fits

    storage_type storage(p);
    storage.set<std::decay_t<Args>...>( std::forward<Args>(args)... );
  } else {
    // store it to heap if it doesn't fit

    using type = std::unique_ptr<tuple_type>;

    auto p = static_cast<void*>(&m_padding[0]);
    std::align(alignof(type),sizeof(type),p,size);

    BIT_ASSERT(p,"Padding buffer must be suitable for std::unique_ptr");

    new (p) type( std::make_unique<tuple_type>( decay_copy(std::forward<Args>(args))... ) );
  }
}

template<typename...Types>
void bit::platform::job::function( void* padding )
{
  using tuple_type = std::tuple<std::decay_t<Types>...>;

  auto p    = padding;
  auto size = padding_size;

  if( std::align(alignof(tuple_type),sizeof(tuple_type),p,size) ) {
    auto storage = storage_type(p);

    function_inner( storage.get<Types...>(), std::index_sequence_for<Types...>{} );
    destruct_args<Types...>( storage, std::is_trivially_destructible<std::tuple<Types...>>{} );
  } else {

    using type = std::unique_ptr<tuple_type>;

    auto p = padding;
    std::align(alignof(type),sizeof(type),p,size);

    BIT_ASSERT(p,"Padding buffer must be suitable for std::unique_ptr");

    // casted unique_ptr will automatically destruct after scope
    auto ptr = std::move(*static_cast<type*>(p));
    function_inner( *ptr, std::index_sequence_for<Types...>{} );
  }
}

template<typename Tuple, std::size_t...Idxs>
void bit::platform::job::function_inner( Tuple&& tuple, std::index_sequence<Idxs...> )
{
  stl::invoke( std::get<Idxs>( std::forward<Tuple>(tuple) )... );
}

//----------------------------------------------------------------------------

template<typename...Types>
void bit::platform::job::destruct_args( storage_type&,
                                        std::true_type )
{
  // trivially destructible
}

template<typename...Types>
void bit::platform::job::destruct_args( storage_type& storage,
                                        std::false_type )
{
  stl::destroy_at( &storage.get<Types...>() );
}


//============================================================================
// job::storage_type
//============================================================================

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

inline bit::platform::job::storage_type::storage_type( void* ptr )
  : m_ptr(ptr)
{

}

//----------------------------------------------------------------------------
// Modifiers
//----------------------------------------------------------------------------

template<typename...Ts, typename...Args>
inline void bit::platform::job::storage_type::set( Args&&...args )
{
  new (m_ptr) std::tuple<Ts...>( decay_copy(std::forward<Args>(args))... );
}

//----------------------------------------------------------------------------
// Element Access
//----------------------------------------------------------------------------

template<typename...Ts>
inline std::tuple<Ts...>& bit::platform::job::storage_type::get()
  const
{
  return *static_cast<std::tuple<Ts...>*>( m_ptr );
}

//----------------------------------------------------------------------------
// job_dispatcher
//----------------------------------------------------------------------------

template<typename Fn>
void bit::platform::job_dispatcher::run( Fn&& fn )
{
  start();
  while( true ) {

    std::forward<Fn>(fn)();

    if( !m_running ) break;

    auto j = get_job();

    if( j ) {
      help_while_unavailable( j );

      j->execute();
    }
  }
}

//----------------------------------------------------------------------------
// Free Functions
//----------------------------------------------------------------------------

template<typename Fn, typename...Args>
inline const bit::platform::job*
  bit::platform::make_job( Fn&& fn, Args&&...args )
{
  auto* p = detail::allocate_job();

  auto* job = new (p) class job( std::forward<Fn>(fn), std::forward<Args>(args)... );

  return job;
}


template<typename Fn, typename...Args>
inline const bit::platform::job*
  bit::platform::make_job( const job* parent, Fn&& fn, Args&&...args )
{
  auto* p = detail::allocate_job();

  // const_cast is safe here, since all jobs are allocated as non-const
  // objects.
  auto* job = new (p) class job( const_cast<class job*>(parent),
                                 std::forward<Fn>(fn),
                                 std::forward<Args>(args)... );

  return job;
}


template<typename Fn, typename...Args>
inline const bit::platform::job*
  bit::platform::post_job( job_dispatcher& dispatcher, Fn&& fn, Args&&...args )
{
  auto job = make_job( std::forward<Fn>(fn), std::forward<Args>(args)... );
  return post_job( dispatcher, job );
}


template<typename Fn, typename...Args>
inline const bit::platform::job*
  bit::platform::post_job( job_dispatcher& dispatcher,
                           const job* parent, Fn&& fn, Args&&...args )
{
  auto job = make_job( parent, std::forward<Fn>(fn), std::forward<Args>(args)... );
  return post_job( dispatcher, job );
}

#endif /* BIT_PLATFORM_THREADING_DETAIL_JOB_DISPATCHER_INL */
