#ifndef BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_MODE_INL
#define BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_MODE_INL

//----------------------------------------------------------------------------
// Operations
//----------------------------------------------------------------------------

inline bit::platform::mode bit::platform::operator | (mode lhs, mode rhs)
  noexcept
{
  return static_cast<mode>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline bit::platform::mode bit::platform::operator & (mode lhs, mode rhs)
  noexcept
{
  return static_cast<mode>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

inline bit::platform::mode bit::platform::operator ^ (mode lhs, mode rhs)
  noexcept
{
  return static_cast<mode>(static_cast<int>(lhs) ^ static_cast<int>(rhs));
}

//----------------------------------------------------------------------------

inline bit::platform::mode& bit::platform::operator |= (mode& lhs, mode rhs)
  noexcept
{
  return (lhs = (lhs | rhs));
}

inline bit::platform::mode& bit::platform::operator &= (mode& lhs, mode rhs)
  noexcept
{
  return (lhs = (lhs & rhs));
}

inline bit::platform::mode& bit::platform::operator ^= (mode& lhs, mode rhs)
  noexcept
{
  return (lhs = (lhs ^ rhs));
}

#endif /* BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_MODE_INL */
