#ifndef BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_INL
#define BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_INL

//----------------------------------------------------------------------------
// Devices
//----------------------------------------------------------------------------

inline bit::platform::filesystem::size_type
  bit::platform::filesystem::devices()
  const noexcept
{
  return static_cast<size_type>( m_current );
}

inline bit::platform::filesystem::size_type
  bit::platform::filesystem::device_capacity()
  const noexcept
{
  return m_size;
}

//----------------------------------------------------------------------------
// File Opening
//----------------------------------------------------------------------------

template<std::size_t N>
inline bit::platform::file
  bit::platform::filesystem::open( stl::string_view path,
                                   const char*const (&devices)[N],
                                   mode access )
{
  return open( sync, path, devices, N, access );
}

template<std::size_t N>
inline bit::platform::file
  bit::platform::filesystem::open( sync_t,
                                   stl::string_view path,
                                   const char*const (&devices)[N],
                                   mode access )
{
  return open( sync, path, devices, N, access );
}

template<std::size_t N>
inline bit::platform::async_file
  bit::platform::filesystem::open( async_t,
                                   stl::string_view path,
                                   const char*const (&devices)[N],
                                   mode access )
{
  return open( async, path, devices, N, access );
}

//----------------------------------------------------------------------------
// File Closing
//----------------------------------------------------------------------------

#endif /* BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_INL */
