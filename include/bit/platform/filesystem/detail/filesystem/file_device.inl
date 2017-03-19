#ifndef BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_FILE_DEVICE_INL
#define BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_FILE_DEVICE_INL

//============================================================================
// file_device
//============================================================================

inline bit::platform::file_device::~file_device() = default;

//============================================================================
// device_list
//============================================================================

//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

template<std::size_t N>
inline bit::platform::device_list::device_list( const char* const (&data)[N] )
  : m_data(data),
    m_size(N)
{

}

//----------------------------------------------------------------------------
// Capacity
//----------------------------------------------------------------------------

inline bool bit::platform::device_list::empty()
  const noexcept
{
  return m_size == 0;
}

inline bit::platform::device_list::size_type
  bit::platform::device_list::size()
  const noexcept
{
  return m_size;
}

//----------------------------------------------------------------------------
// Element Access
//----------------------------------------------------------------------------

inline bit::stl::string_view
  bit::platform::device_list::operator[]( index_type i )
  const noexcept
{
  return m_data[m_size];
}

#endif /* BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_FILE_DEVICE_INL */
