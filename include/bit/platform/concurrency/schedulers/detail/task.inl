#ifndef BIT_PLATFORM_CONCURRENCY_SCHEDULERS_DETAIL_TASK_INL
#define BIT_PLATFORM_CONCURRENCY_SCHEDULERS_DETAIL_TASK_INL

namespace bit { namespace platform { namespace detail {

///////////////////////////////////////////////////////////////////////////////
/// \brief The internal storage for a given task in the task system
///
/// This class must satisfy TriviallyDestructible in order to properly be
/// reused by the task allocators.
///////////////////////////////////////////////////////////////////////////////
class alignas(cache_line_size()) task_storage
{
  //---------------------------------------------------------------------------
  // Observers
  //---------------------------------------------------------------------------
public:

  /// \brief Returns whether this task has completed
  ///
  /// \return \c true if the task has completed
  bool completed() const noexcept;

  /// \brief Returns whether this task is available for execution
  ///
  /// A task is considered available only if all the child tasks have
  /// finished executing first
  ///
  /// \return \c true if this task is available to be executed
  bool available() const noexcept;

  //---------------------------------------------------------------------------
  // Element Access
  //---------------------------------------------------------------------------
public:

  /// \brief Returns the parent of this task, if any
  ///
  /// \note This returns nullptr for tasks with no parent
  ///
  /// \return the parent of the task
  task_storage* parent() const noexcept;

  //---------------------------------------------------------------------------
  // Execution
  //---------------------------------------------------------------------------
public:

  /// \brief Invokes the underlying task with the given arguments
  void execute() const;

  //---------------------------------------------------------------------------
  // Modifiers
  //---------------------------------------------------------------------------
public:

  /// \brief Finalizes this task
  ///
  /// \note This function must only be called once per task, otherwise it is
  ///       undefined behaviour. This destructs any stored arguments to the
  ///       task, and signals to any parent-tasks that this sub-task is no
  ///       longer valid
  void finalize();

  //---------------------------------------------------------------------------
  // Private Constructors
  //---------------------------------------------------------------------------
private:

  /// \brief Default-constructs this task
  task_storage();

  // Deleted move construction
  task_storage( task_storage&& other ) = delete;

  // Deleted copy construction
  task_storage( const task_storage& other ) = delete;

  //---------------------------------------------------------------------------

  // Deleted move assignment
  task_storage& operator=( task_storage&& other ) = delete;

  // Deleted copy assignment
  task_storage& operator=( const task_storage& other ) = delete;

  /// \brief Constructs a task from a given \p function to invoke
  ///
  /// \param fn the function to invoke
  /// \param
  template<typename Fn, typename...Args>
  explicit task_storage( Fn&& fn, Args&&...args );

  /// \brief Constructs a task from a given \p function to invoke
  ///
  /// \param fn the function to invoke
  template<typename Fn, typename...Args>
  explicit task_storage( task_storage* parent, Fn&& fn, Args&&...args );

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
  ///        of this task into a tuple of arguments
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
                                            - sizeof(task_storage*)
                                            - 2*sizeof(function_type)
                                            - sizeof(atomic_type);

  template<typename T>
  static constexpr std::size_t max_storage_size = padding_size - alignof(T);

  //---------------------------------------------------------------------------
  // Private Members
  //---------------------------------------------------------------------------
private:

  task_storage*  m_parent;
  function_type m_function;
  function_type m_destructor;
  atomic_type   m_unfinished;
  mutable char  m_padding[padding_size];

  //---------------------------------------------------------------------------
  // Static Functions
  //---------------------------------------------------------------------------
private:

  /// \brief The function being wrapped in the task object
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

  /// \brief The function to call the destructor for the task
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
  friend task bit::platform::make_task( Fn&&, Args&&... );

  template<typename Fn, typename...Args>
  friend task bit::platform::make_task( const task&, Fn&&, Args&&... );

  friend void* allocate_task();
};

} } } // namespace bit::platform::detail

// the task class must be trivially destructible, since this is the primary
// storage used for allocating thread-safe tasks, and destructors are never
// called before reconstructions.
static_assert( std::is_trivially_destructible<bit::platform::detail::task_storage>::value,
               "task_storage must be trivially destructible!");

//=============================================================================
// task_storage
//=============================================================================

//-----------------------------------------------------------------------------
// Private Constructors
//-----------------------------------------------------------------------------

inline bit::platform::detail::task_storage::task_storage()
  : m_parent(nullptr),
    m_function(nullptr),
    m_destructor(nullptr),
    m_unfinished(0)
{

}

template<typename Fn, typename...Args>
bit::platform::detail::task_storage::task_storage( Fn&& fn, Args&&...args )
: m_parent(nullptr),
  m_function(&function<std::decay_t<Fn>,std::decay_t<Args>...>),
  m_destructor(&destruct_function<std::decay_t<Fn>,std::decay_t<Args>...>),
  m_unfinished(1)
{
  store_arguments( std::forward<Fn>(fn), std::forward<Args>(args)... );
}

//-----------------------------------------------------------------------------

template<typename Fn, typename...Args>
bit::platform::detail::task_storage::task_storage( task_storage* parent,
                                                   Fn&& fn,
                                                   Args&&...args )
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

inline bool bit::platform::detail::task_storage::completed()
  const noexcept
{
  return m_unfinished == 0;
}

inline bool bit::platform::detail::task_storage::available()
  const noexcept
{
  return m_unfinished == 1;
}

//-----------------------------------------------------------------------------
// Element Access
//-----------------------------------------------------------------------------

inline bit::platform::detail::task_storage* bit::platform::detail::task_storage::parent()
  const noexcept
{
  return m_parent;
}

//-----------------------------------------------------------------------------
// Execution
//-----------------------------------------------------------------------------

inline void bit::platform::detail::task_storage::execute() const
{
  (*m_function)( static_cast<void*>(&m_padding[0]) );
}

//-----------------------------------------------------------------------------
// Modifiers
//-----------------------------------------------------------------------------

inline void bit::platform::detail::task_storage::finalize()
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
void bit::platform::detail::task_storage::store_arguments( Args&&...args )
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
void bit::platform::detail::task_storage::function( void* padding )
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
void bit::platform::detail::task_storage::function_inner( Tuple&& tuple,
                                                          std::index_sequence<Idxs...> )
{
  stl::invoke( std::get<Idxs>( std::forward<Tuple>(tuple) )... );
}

//-----------------------------------------------------------------------------

template<typename...Types>
void bit::platform::detail::task_storage::destruct_function( void* padding )
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
void bit::platform::detail::task_storage::destruct_args( storage_type&,
                                                         std::true_type )
{
  // trivially destructible
}

template<typename...Types>
void bit::platform::detail::task_storage::destruct_args( storage_type& storage,
                                                         std::false_type )
{
  destroy( storage.get<Types...>() );
}

template<typename T>
void bit::platform::detail::task_storage::destroy( T& t )
{
  t.~T();
}

//=============================================================================
// task::storage_type
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

inline bit::platform::detail::task_storage::storage_type::storage_type( void* ptr )
  : m_ptr(ptr)
{

}

//-----------------------------------------------------------------------------
// Modifiers
//-----------------------------------------------------------------------------

template<typename...Ts, typename...Args>
inline void bit::platform::detail::task_storage::storage_type::set( Args&&...args )
{
  new (m_ptr) std::tuple<Ts...>( detail::decay_copy(std::forward<Args>(args))... );
}

//-----------------------------------------------------------------------------
// Element Access
//-----------------------------------------------------------------------------

template<typename...Ts>
inline std::tuple<Ts...>& bit::platform::detail::task_storage::storage_type::get()
  const
{
  return *static_cast<std::tuple<Ts...>*>( m_ptr );
}

//=============================================================================
// task
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment / Destructor
//-----------------------------------------------------------------------------

inline bit::platform::task::task()
  noexcept
  : m_task(nullptr)
{

}

template<typename Fn, typename...Args, typename, typename>
inline bit::platform::task::task( Fn&& fn, Args&&...args )
  : m_task(static_cast<detail::task_storage*>(detail::allocate_task()))
{
  new (m_task) detail::task_storage( std::forward<Fn>(fn),
                                     std::forward<Args>(args)... );
}


template<typename Fn, typename...Args, typename>
inline bit::platform::task::task( const task& parent, Fn&& fn, Args&&...args )
  : m_task(static_cast<detail::task_storage*>(detail::allocate_task()))
{
  assert( parent.m_task != nullptr && "parent task cannot refer to null task" );

  new (m_task) detail::task_storage( parent.m_task,
                                     std::forward<Fn>(fn),
                                     std::forward<Args>(args)... );
}

inline bit::platform::task::task( task&& other )
  noexcept
  : m_task(other.m_task)
{
  other.m_task = nullptr;
}

//-----------------------------------------------------------------------------

inline bit::platform::task::~task()
{
  // Only finalize if task is not null
  if( m_task ) m_task->finalize();
}

//-----------------------------------------------------------------------------

inline bit::platform::task& bit::platform::task::operator=( task other )
{
  swap(other);

  return (*this);
}


//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

inline bool bit::platform::task::completed()
  const noexcept
{
  return m_task->completed();
}

inline bool bit::platform::task::available()
  const noexcept
{
  return m_task->available();
}

//-----------------------------------------------------------------------------
// Modifiers
//-----------------------------------------------------------------------------

inline void bit::platform::task::swap( task& other )
  noexcept
{
  using std::swap;

  swap(m_task,other.m_task);
}

//-----------------------------------------------------------------------------
// Execution
//-----------------------------------------------------------------------------

inline void bit::platform::task::execute()
  const
{
  assert( m_task && "execute can only be called on non-null tasks" );
  assert( available() && "task may only be executed when available" );

  m_task->execute();
}

inline void bit::platform::task::operator()()
  const
{
  execute();
}

//-----------------------------------------------------------------------------
// Conversions
//-----------------------------------------------------------------------------

inline bit::platform::task::operator bool()
  const noexcept
{
  return m_task != nullptr;
}

//=============================================================================
// Free Functions
//=============================================================================

//-----------------------------------------------------------------------------
// Equality
//-----------------------------------------------------------------------------

inline bool bit::platform::operator==( const task& lhs, std::nullptr_t )
  noexcept
{
  return !static_cast<bool>(lhs);
}

inline bool bit::platform::operator==( std::nullptr_t, const task& rhs )
  noexcept
{
  return !static_cast<bool>(rhs);
}

inline bool bit::platform::operator==( const task& lhs, const task& rhs )
  noexcept
{
  return lhs.m_task == rhs.m_task;
}

//-----------------------------------------------------------------------------

inline bool bit::platform::operator!=( const task& lhs, std::nullptr_t )
  noexcept
{
  return !(lhs == nullptr);
}

inline bool bit::platform::operator!=( std::nullptr_t, const task& rhs )
  noexcept
{
  return !(nullptr == rhs);
}

inline bool bit::platform::operator!=( const task& lhs, const task& rhs )
  noexcept
{
  return !(lhs==rhs);
}

//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------

inline void bit::platform::swap( task& lhs, task& rhs )
  noexcept
{
  lhs.swap(rhs);
}

//-----------------------------------------------------------------------------


template<typename Fn, typename...Args>
inline bit::platform::task
  bit::platform::make_task( Fn&& fn, Args&&...args )
{
  return task{ std::forward<Fn>(fn), std::forward<Args>(args)... };
}


template<typename Fn, typename...Args>
inline bit::platform::task
  bit::platform::make_task( const task& parent, Fn&& fn, Args&&...args )
{
  return task{ parent, std::forward<Fn>(fn), std::forward<Args>(args)... };
}

//=============================================================================
// task_handle
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

inline bit::platform::task_handle::task_handle()
  noexcept
  : m_task(nullptr)
{

}

inline bit::platform::task_handle::task_handle( const task& task )
  noexcept
  : m_task(task.m_task)
{

}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

inline bool bit::platform::task_handle::completed()
  const noexcept
{
  return m_task ? m_task->completed() : true;
}

inline bool bit::platform::task_handle::available()
  const noexcept
{
  return m_task ? m_task->available() : false;
}

#endif /* BIT_PLATFORM_CONCURRENCY_SCHEDULERS_DETAIL_TASK_INL */
