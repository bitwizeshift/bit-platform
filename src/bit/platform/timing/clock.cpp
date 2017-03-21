#include <bit/platform/timing/clock.hpp>
#include <bit/stl/assert.hpp>

#include <chrono>

//----------------------------------------------------------------------------

namespace { // anonymous

  using time_point = std::chrono::high_resolution_clock::time_point;

  /// \brief Gets the current time as a high-resolution time
  ///
  /// \return the current time
  time_point current_time();

} // anonymous namespace

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

bit::platform::clock::clock( double scale )
  noexcept
  : m_scale(scale),
    m_is_running(false)
{

}

//----------------------------------------------------------------------------
// Modifiers
//----------------------------------------------------------------------------

void bit::platform::clock::set_scale( double scale )
  noexcept
{
  m_scale = scale;
}

//----------------------------------------------------------------------------
// Timing
//----------------------------------------------------------------------------

void bit::platform::clock::start()
  noexcept
{
  m_is_running = true;
  m_start = current_time();
  m_split = m_start;
}

void bit::platform::clock::stop()
  noexcept
{
  m_is_running = false;
}

void bit::platform::clock::reset()
  noexcept
{
  if(m_is_running){
    m_start = current_time();
    m_split = m_start;
  }
}

//----------------------------------------------------------------------------

bit::platform::clock::time_unit bit::platform::clock::split()
{
  BIT_ASSERT(m_is_running,"clock::split: clock not running.");

  auto current = current_time();
  auto result = (current - m_start) * m_scale;
  m_split = current;

  return std::chrono::duration_cast<time_unit>(result);
}

bit::platform::clock::time_unit bit::platform::clock::elapsed()
{
  BIT_ASSERT(m_is_running,"clock::elapsed: clock not running");

  return std::chrono::duration_cast<time_unit>((current_time() - m_start) * m_scale);
}

//----------------------------------------------------------------------------
// Anonymous Functions
//----------------------------------------------------------------------------

namespace { // anonymous

  inline time_point current_time()
  {
    return std::chrono::high_resolution_clock::now();
  }

} // anonymous namespace
