#ifndef BIT_PLATFORM_CONCURRENCY_UTILITIES_DETAIL_TRUE_SHARE_INL
#define BIT_PLATFORM_CONCURRENCY_UTILITIES_DETAIL_TRUE_SHARE_INL

inline constexpr std::size_t bit::platform::cache_line_size()
  noexcept
{
  // TODO: Determine actual cache-line size
  return 64;
}

//=============================================================================
// true_share
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

template<typename T>
template<typename Arg0, typename...Args, typename>
inline constexpr bit::platform::true_share<T>::true_share( Arg0&& arg0, Args&&...args )
  : m_entry{ std::forward<Arg0>(arg0), std::forward<Args>(args)... }
{

}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

template<typename T>
inline constexpr bit::platform::true_share<T>::operator T&()
  const noexcept
{
  return m_entry;
}

template<typename T>
inline constexpr typename bit::platform::true_share<T>::value_type&
  bit::platform::true_share<T>::get()
  const noexcept
{
  return m_entry;
}

//-----------------------------------------------------------------------------
// Private Static Functions
//-----------------------------------------------------------------------------

template<typename T>
inline constexpr std::size_t bit::platform::true_share<T>::max_align()
  noexcept
{
  return cache_line_size() > alignof(T) ? cache_line_size() : alignof(T);
}


#endif /* BIT_PLATFORM_CONCURRENCY_UTILITIES_DETAIL_TRUE_SHARE_INL */
