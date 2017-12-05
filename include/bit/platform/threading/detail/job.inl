#ifndef BIT_PLATFORM_THREADING_DETAIL_JOB_INL
#define BIT_PLATFORM_THREADING_DETAIL_JOB_INL

namespace bit { namespace platform { namespace detail {

///////////////////////////////////////////////////////////////////////////////
/// \brief A job is the unit of dispatch used in the job_system
///
/// \note A job may only ever be executed exactly once; executing a job
///       more than once is undefined behaviour. This is best left up to
///       the dispatcher system for the job.
///////////////////////////////////////////////////////////////////////////////
class alignas(cache_line_size()) job_storage
{
  //---------------------------------------------------------------------------
  // Observers
  //---------------------------------------------------------------------------
public:

  /// \brief Returns whether this job has completed
  ///
  /// \return \c true if the job has completed
  bool completed() const noexcept;

  /// \brief Returns whether this job is available for execution
  ///
  /// A job is considered available only if all the child tasks have
  /// finished executing first
  ///
  /// \return \c true if this job is available to be executed
  bool available() const noexcept;

  //---------------------------------------------------------------------------
  // Element Access
  //---------------------------------------------------------------------------
public:

  /// \brief Returns the parent of this job, if any
  ///
  /// \note This returns nullptr for jobs with no parent
  ///
  /// \return the parent of the job
  job_storage* parent() const noexcept;

  //---------------------------------------------------------------------------
  // Execution
  //---------------------------------------------------------------------------
public:

  /// \brief Detaches the current active job to run indefinitely in the
  ///        background
  void execute() const;

  //---------------------------------------------------------------------------
  // Modifiers
  //---------------------------------------------------------------------------
public:

  /// \brief Finalizes this job
  ///
  /// \note This function must only be called once per job, otherwise it is
  ///       undefined behaviour. This destructs any stored arguments to the
  ///       job, and signals to any parent-jobs that this sub-job is no
  ///       longer valid
  void finalize();

  //---------------------------------------------------------------------------
  // Private Constructors
  //---------------------------------------------------------------------------
private:

  /// \brief Default-constructs this job
  job_storage();

  // Deleted move construction
  job_storage( job_storage&& other ) = delete;

  // Deleted copy construction
  job_storage( const job_storage& other ) = delete;

  //---------------------------------------------------------------------------

  // Deleted move assignment
  job_storage& operator=( job_storage&& other ) = delete;

  // Deleted copy assignment
  job_storage& operator=( const job_storage& other ) = delete;

  /// \brief Constructs a job from a given \p function to invoke
  ///
  /// \param fn the function to invoke
  /// \param
  template<typename Fn, typename...Args>
  explicit job_storage( Fn&& fn, Args&&...args );

  /// \brief Constructs a job from a given \p function to invoke
  ///
  /// \param fn the function to invoke
  template<typename Fn, typename...Args>
  explicit job_storage( job_storage* parent, Fn&& fn, Args&&...args );

  //---------------------------------------------------------------------------
  // Private Modifiers
  //---------------------------------------------------------------------------
private:

  /// \brief Stores arguments for the function to use
  ///
  /// \param args the arguments
  template<typename...Args>
  void store_arguments( Args&&...args );

  //---------------------------------------------------------------------------
  // Private Member Types
  //---------------------------------------------------------------------------
private:

  /////////////////////////////////////////////////////////////////////////////
  /// \brief An underlying storage type that converts the unused padding
  ///        of this job into a tuple of arguments
  /////////////////////////////////////////////////////////////////////////////
  class storage_type
  {
    //-------------------------------------------------------------------------
    // Constructor
    //-------------------------------------------------------------------------
  public:

    /// \brief Constructs the storage type from the specified memory
    ///        address
    ///
    /// \param ptr the memory address for the storage
    storage_type( void* ptr );

    //-------------------------------------------------------------------------
    // Modifiers
    //-------------------------------------------------------------------------
  public:

    /// \brief Sets the values of the storage type, forwarding the args
    ///
    /// \param args the arguments to store
    template<typename...Ts, typename...Args>
    void set( Args&&...args );

    //-------------------------------------------------------------------------
    // Observers
    //-------------------------------------------------------------------------
  public:

    /// \brief Gets the values of the storage type as a tuple
    ///
    /// \return the storage values as a tuple
    template<typename...Ts>
    std::tuple<Ts...>& get() const;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    void* m_ptr;
  };

  using atomic_type   = std::atomic<std::uint16_t>;
  using function_type = void(*)( void* );
  using move_function_type = void(*)(void*,void*);

  //---------------------------------------------------------------------------
  // Static Private Members
  //---------------------------------------------------------------------------
private:

  static constexpr std::size_t padding_size = cache_line_size()
                                            - sizeof(job_storage*)
                                            - 2*sizeof(function_type)
                                            - sizeof(atomic_type);

  template<typename T>
  static constexpr std::size_t max_storage_size = padding_size - alignof(T);

  //---------------------------------------------------------------------------
  // Private Members
  //---------------------------------------------------------------------------
private:

  job_storage*  m_parent;
  function_type m_function;
  function_type m_destructor;
  atomic_type   m_unfinished;
  mutable char  m_padding[padding_size];

  //---------------------------------------------------------------------------
  // Static Functions
  //---------------------------------------------------------------------------
private:

  /// \brief The function being wrapped in the job object
  ///
  /// \param padding pointer to the padding to convert to arguments
  template<typename...Types>
  static void function( void* padding );

  /// \brief The implementation of the above function that forwards all
  ///        stored arguments to the underlying function type
  ///
  /// \param tuple the tuple of arguments
  template<typename Tuple, std::size_t...Idxs>
  static void function_inner( Tuple&& tuple, std::index_sequence<Idxs...> );

  //---------------------------------------------------------------------------

  /// \brief The function to call the destructor for the job
  ///
  /// \param padding pointer to the padding to convert to arguments
  template<typename...Types>
  static void destruct_function( void* padding );

  //---------------------------------------------------------------------------

  /// \{
  /// \brief Destructs the underlying arguments stored in the padding
  ///
  /// If the type is trivially destructible, this is a no-op
  ///
  /// \param storage the underlying storage to destruct
  template<typename...Types>
  static void destruct_args( storage_type& storage, std::true_type );
  template<typename...Types>
  static void destruct_args( storage_type& storage, std::false_type );
  /// \}

  template<typename T>
  static void destroy( T& t );

  //---------------------------------------------------------------------------
  // Friends
  //---------------------------------------------------------------------------
private:

  template<typename Fn, typename...Args>
  friend job bit::platform::make_job( Fn&&, Args&&... );

  template<typename Fn, typename...Args>
  friend job bit::platform::make_job( const job&, Fn&&, Args&&... );

  friend void* allocate_job();
};

} } } // namespace bit::platform::detail

// the job class must be trivially destructible, since this is the primary
// storage used for allocating thread-safe jobs, and destructors are never
// called before reconstructions.
static_assert( std::is_trivially_destructible<bit::platform::detail::job_storage>::value, "job_storage must be trivially destructible!");

//=============================================================================
// job_storage
//=============================================================================

//-----------------------------------------------------------------------------
// Private Constructors
//-----------------------------------------------------------------------------

inline bit::platform::detail::job_storage::job_storage()
  : m_parent(nullptr),
    m_function(nullptr),
    m_destructor(nullptr),
    m_unfinished(0)
{

}

template<typename Fn, typename...Args>
bit::platform::detail::job_storage::job_storage( Fn&& fn, Args&&...args )
: m_parent(nullptr),
  m_function(&function<std::decay_t<Fn>,std::decay_t<Args>...>),
  m_destructor(&destruct_function<std::decay_t<Fn>,std::decay_t<Args>...>),
  m_unfinished(1)
{
  store_arguments( std::forward<Fn>(fn), std::forward<Args>(args)... );
}

//-----------------------------------------------------------------------------

template<typename Fn, typename...Args>
bit::platform::detail::job_storage::job_storage( job_storage* parent, Fn&& fn, Args&&...args )
  : m_parent(parent),
    m_function(&function<std::decay_t<Fn>,std::decay_t<Args>...>),
    m_destructor(&destruct_function<std::decay_t<Fn>,std::decay_t<Args>...>),
    m_unfinished(1)
{
  ++m_parent->m_unfinished;

  store_arguments( std::forward<Fn>(fn), std::forward<Args>(args)... );
}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

inline bool bit::platform::detail::job_storage::completed()
  const noexcept
{
  return m_unfinished == 0;
}

inline bool bit::platform::detail::job_storage::available()
  const noexcept
{
  return m_unfinished == 1;
}

//-----------------------------------------------------------------------------
// Element Access
//-----------------------------------------------------------------------------

inline bit::platform::detail::job_storage* bit::platform::detail::job_storage::parent()
  const noexcept
{
  return m_parent;
}

//-----------------------------------------------------------------------------
// Execution
//-----------------------------------------------------------------------------

inline void bit::platform::detail::job_storage::execute() const
{
  (*m_function)( static_cast<void*>(&m_padding[0]) );
}

//-----------------------------------------------------------------------------
// Modifiers
//-----------------------------------------------------------------------------

inline void bit::platform::detail::job_storage::finalize()
{
  (*m_destructor)( static_cast<void*>(&m_padding[0]) );
  auto unfinished = --m_unfinished;
  if( unfinished == 0 && m_parent ) {
    m_parent->finalize();
  }
}

//-----------------------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------------------

template<typename...Args>
void bit::platform::detail::job_storage::store_arguments( Args&&...args )
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
    using unique_type = std::unique_ptr<tuple_type>;

    auto p = static_cast<void*>(&m_padding[0]);
    std::align( alignof(unique_type), sizeof(unique_type), p, size );

    assert( p != nullptr && "Padding buffer must be suitable for std::unique_ptr" );

    new (p) unique_type( std::make_unique<tuple_type>( detail::decay_copy(std::forward<Args>(args))... ) );
  }
}

template<typename...Types>
void bit::platform::detail::job_storage::function( void* padding )
{
  using tuple_type = std::tuple<std::decay_t<Types>...>;

  auto p    = padding;
  auto size = padding_size;

  if( std::align(alignof(tuple_type),sizeof(tuple_type),p,size) ) {
    auto storage = storage_type(p);

    function_inner( storage.get<Types...>(), std::index_sequence_for<Types...>{} );
  } else {

    using unique_type = std::unique_ptr<tuple_type>;

    auto p = padding;
    std::align( alignof(unique_type), sizeof(unique_type), p, size );

    assert( p != nullptr && "Padding buffer must be suitable for std::unique_ptr");

    auto& ptr = *static_cast<unique_type*>(p);
    function_inner( *ptr, std::index_sequence_for<Types...>{} );
  }
}

template<typename Tuple, std::size_t...Idxs>
void bit::platform::detail::job_storage::function_inner( Tuple&& tuple, std::index_sequence<Idxs...> )
{
  stl::invoke( std::get<Idxs>( std::forward<Tuple>(tuple) )... );
}

//-----------------------------------------------------------------------------

template<typename...Types>
void bit::platform::detail::job_storage::destruct_function( void* padding )
{
  using tuple_type = std::tuple<std::decay_t<Types>...>;

  auto p    = padding;
  auto size = padding_size;

  if( std::align(alignof(tuple_type),sizeof(tuple_type),p,size) ) {
    auto storage = storage_type(p);

    destruct_args<Types...>( storage, std::is_trivially_destructible<std::tuple<Types...>>{} );
  } else {

    using unique_type = std::unique_ptr<tuple_type>;

    auto p = padding;
    std::align( alignof(unique_type), sizeof(unique_type), p, size );

    assert( p != nullptr && "Padding buffer must be suitable for std::unique_ptr");

    // casted unique_ptr will automatically destruct after scope
    auto ptr = std::move(*static_cast<unique_type*>(p));
  }
}

//-----------------------------------------------------------------------------

template<typename...Types>
void bit::platform::detail::job_storage::destruct_args( storage_type&,
                                                      std::true_type )
{
  // trivially destructible
}

template<typename...Types>
void bit::platform::detail::job_storage::destruct_args( storage_type& storage,
                                                      std::false_type )
{
  destroy( storage.get<Types...>() );
}

template<typename T>
void bit::platform::detail::job_storage::destroy( T& t )
{
  t.~T();
}

//=============================================================================
// job::storage_type
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

inline bit::platform::detail::job_storage::storage_type::storage_type( void* ptr )
  : m_ptr(ptr)
{

}

//-----------------------------------------------------------------------------
// Modifiers
//-----------------------------------------------------------------------------

template<typename...Ts, typename...Args>
inline void bit::platform::detail::job_storage::storage_type::set( Args&&...args )
{
  new (m_ptr) std::tuple<Ts...>( detail::decay_copy(std::forward<Args>(args))... );
}

//-----------------------------------------------------------------------------
// Element Access
//-----------------------------------------------------------------------------

template<typename...Ts>
inline std::tuple<Ts...>& bit::platform::detail::job_storage::storage_type::get()
  const
{
  return *static_cast<std::tuple<Ts...>*>( m_ptr );
}


//=============================================================================
// job
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment / Destructor
//-----------------------------------------------------------------------------

inline bit::platform::job::job()
  noexcept
  : m_job(nullptr)
{

}

template<typename Fn, typename...Args, typename, typename>
inline bit::platform::job::job( Fn&& fn, Args&&...args )
  : m_job(static_cast<detail::job_storage*>(detail::allocate_job()))
{
  new (m_job) detail::job_storage( std::forward<Fn>(fn),
                                   std::forward<Args>(args)... );
}


template<typename Fn, typename...Args, typename>
inline bit::platform::job::job( const job& parent, Fn&& fn, Args&&...args )
  : m_job(static_cast<detail::job_storage*>(detail::allocate_job()))
{
  new (m_job) detail::job_storage( parent.m_job,
                                   std::forward<Fn>(fn),
                                   std::forward<Args>(args)... );
}

inline bit::platform::job::job( job&& other )
  noexcept
  : m_job(other.m_job)
{
  other.m_job = nullptr;
}


//-----------------------------------------------------------------------------

inline bit::platform::job::~job()
{
  // Only finalize if job is not null
  if( m_job ) m_job->finalize();
}

//-----------------------------------------------------------------------------

inline bit::platform::job& bit::platform::job::operator=( job&& other )
{
  if( m_job ) m_job->finalize();

  m_job = other.m_job;
  other.m_job = nullptr;

  return (*this);
}


//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

inline bool bit::platform::job::completed()
  const noexcept
{
  return m_job->completed();
}

inline bool bit::platform::job::available()
  const noexcept
{
  return m_job->available();
}

//-----------------------------------------------------------------------------
// Execution
//-----------------------------------------------------------------------------

inline void bit::platform::job::execute() const
{
  assert( m_job && "execute can only be called on non-null jobs" );

  auto old = detail::get_active_job();
  detail::set_active_job(this);
  m_job->execute();
  detail::set_active_job(old);
}

//-----------------------------------------------------------------------------
// Conversions
//-----------------------------------------------------------------------------

inline bit::platform::job::operator bool() const noexcept
{
  return m_job != nullptr;
}

//=============================================================================
// Free Functions
//=============================================================================

//-----------------------------------------------------------------------------
// Equality
//-----------------------------------------------------------------------------

inline bool bit::platform::operator==( const job& lhs, std::nullptr_t )
  noexcept
{
  return !static_cast<bool>(lhs);
}

inline bool bit::platform::operator==( std::nullptr_t, const job& rhs )
  noexcept
{
  return !static_cast<bool>(rhs);
}

inline bool bit::platform::operator==( const job& lhs, const job& rhs )
  noexcept
{
  return lhs.m_job == rhs.m_job;
}

//-----------------------------------------------------------------------------

inline bool bit::platform::operator!=( const job& lhs, std::nullptr_t )
  noexcept
{
  return !(lhs == nullptr);
}

inline bool bit::platform::operator!=( std::nullptr_t, const job& rhs )
  noexcept
{
  return !(nullptr == rhs);
}

inline bool bit::platform::operator!=( const job& lhs, const job& rhs )
  noexcept
{
  return !(lhs==rhs);
}

//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------

template<typename Fn, typename...Args>
inline bit::platform::job
  bit::platform::make_job( Fn&& fn, Args&&...args )
{
  return job{ std::forward<Fn>(fn), std::forward<Args>(args)... };
}


template<typename Fn, typename...Args>
inline bit::platform::job
  bit::platform::make_job( const job& parent, Fn&& fn, Args&&...args )
{
  return job{ parent, std::forward<Fn>(fn), std::forward<Args>(args)... };
}

//=============================================================================
// job_handle
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

inline bit::platform::job_handle::job_handle()
  noexcept
  : m_job(nullptr)
{

}

inline bit::platform::job_handle::job_handle( const job& job )
  noexcept
  : m_job(job.m_job)
{

}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

inline bool bit::platform::job_handle::completed()
  const noexcept
{
  return m_job ? m_job->completed() : true;
}

inline bool bit::platform::job_handle::available()
  const noexcept
{
  return m_job ? m_job->available() : true;
}

#endif /* BIT_PLATFORM_THREADING_DETAIL_JOB_INL */
