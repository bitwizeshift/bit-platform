#ifndef BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_INL
#define BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_INL

//----------------------------------------------------------------------------
// Devices
//----------------------------------------------------------------------------

template<typename...Devices, std::enable_if_t<bit::stl::conjunction<std::is_base_of<bit::platform::file_device,Devices>...>::value>*>
inline void bit::platform::filesystem::mount( file_device* device0,
                                              file_device* device1,
                                              Devices*... devices )
{
  file_device* all_devices[] = { device0, device1, devices... };

  for( auto& device : all_devices ) {
    mount( device );
  }
}

template<typename...Devices, std::enable_if_t<bit::stl::conjunction<std::is_base_of<bit::platform::file_device,Devices>...>::value>*>
inline void bit::platform::filesystem::unmount( file_device* device0,
                                                file_device* device1,
                                                Devices*... devices )
{
  file_device* all_devices[] = { device0, device1, devices... };

  for( auto& device : all_devices ) {
    unmount( device );
  }
}

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
