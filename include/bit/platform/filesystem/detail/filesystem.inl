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

inline bit::platform::file
  bit::platform::filesystem::open( stl::string_view path,
                                   device_list devices,
                                   mode access )
{
  return open( sync, path, devices, access );
}

//----------------------------------------------------------------------------
// File Closing
//----------------------------------------------------------------------------

#endif /* BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_INL */
